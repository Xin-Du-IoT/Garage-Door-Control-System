#include <cstdio>

#include "pico/stdlib.h"

#include "board_config.h"
#include "wifi_config.h"

#include "stepper_motor.h"
#include "buttons.h"
#include "status_leds.h"
#include "persistent_storage.h"
#include "safety_monitor.h"

#include "rotary_encoder.h"
#include "position_tracker.h"
#include "limit_switches.h"
#include "calibration_engine.h"

#include "mqtt_client.h"
#include "garage_door_controller.h"

static const char* door_state_to_string(garage::DoorState s) {
    switch (s) {
        case garage::DoorState::Open:        return "Open";
        case garage::DoorState::Closed:      return "Closed";
        case garage::DoorState::InBetween:   return "InBetween";
        case garage::DoorState::Opening:     return "Opening";
        case garage::DoorState::Closing:     return "Closing";
        case garage::DoorState::Stopped:     return "Stopped";
        case garage::DoorState::Calibrating: return "Calibrating";
        case garage::DoorState::Error:       return "Error";
        default:                             return "Unknown";
    }
}

int main() {
    stdio_init_all();
    sleep_ms(2000);

    garage::StepperMotor motor;
    garage::Buttons buttons;
    garage::StatusLeds leds;
    garage::PersistentStorage storage;
    garage::SafetyMonitor safety;

    garage::RotaryEncoder encoder(
        garage::BoardConfig::encoder_pin_a,
        garage::BoardConfig::encoder_pin_b
    );

    garage::PositionTracker tracker;

    garage::LimitSwitches limits(
        garage::BoardConfig::limit_top,
        garage::BoardConfig::limit_bottom
    );

    garage::CalibrationEngine calibration;

    garage::MqttClient::Config mqtt_cfg {
        garage::config::WIFI_SSID,
        garage::config::WIFI_PASSWORD,
        garage::config::MQTT_BROKER_IP,
        garage::config::MQTT_BROKER_PORT,
        garage::config::MQTT_CLIENT_ID,
        garage::config::WIFI_CONNECT_TIMEOUT_MS,
        garage::config::MQTT_YIELD_TIMEOUT_MS
    };

    garage::MqttClient mqtt(mqtt_cfg);

    garage::GarageDoorController controller(
        motor,
        buttons,
        leds,
        storage,
        safety,
        encoder,
        tracker,
        limits,
        calibration,
        mqtt
    );

    controller.init();

    bool mqtt_ready = mqtt.init();
    if (!mqtt_ready) {
        printf("[MQTT] init failed, continuing in local-only mode\n");
    } else if (!mqtt.connect()) {
        printf("[MQTT] initial connect failed, will retry in loop\n");
    }
    //lambda
    auto print_status = [&](const char* label) {
        garage::DoorStatus s = controller.current_status();

        const char* running_str = s.motor_running ? "Yes" : "No";
        const char* top_str = s.top_limit_active ? "ON" : "OFF";
        const char* bottom_str = s.bottom_limit_active ? "ON" : "OFF";
        const char* error_str =
            (s.error_state == garage::ErrorState::Normal) ? "Normal" : "DoorStuck";
        const char* cal_str =
            (s.calibration_state == garage::CalibrationState::Calibrated) ? "Yes" : "No";

        printf(
            "%s | door=%s | running=%s | pos=%ld | travel=%ld | top=%s | bottom=%s | error=%s | calibrated=%s\n",
            label,
            door_state_to_string(s.door_state),
            running_str,
            static_cast<long>(s.current_position_steps),
            static_cast<long>(s.travel_steps),
            top_str,
            bottom_str,
            error_str,
            cal_str
        );
    };

    print_status("STATUS");

    uint32_t last_status_ms = 0;
    uint32_t last_reconnect_ms = 0;

    while (true) {
        if (mqtt_ready) {
            mqtt.loop();

            const uint32_t now_ms = to_ms_since_boot(get_absolute_time());
            if (!mqtt.is_connected() && (now_ms - last_reconnect_ms >= 5000)) {
                last_reconnect_ms = now_ms;
                printf("[MQTT] reconnect attempt...\n");
                mqtt.connect();
            }
        }

        controller.tick();

        const uint32_t now_ms = to_ms_since_boot(get_absolute_time());
        if (now_ms - last_status_ms >= 5000) {
            last_status_ms = now_ms;
            print_status("STATUS");
        }
    }
}