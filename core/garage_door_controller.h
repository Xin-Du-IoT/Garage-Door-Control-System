#pragma once

#include "garage_types.h"

#include "stepper_motor.h"
#include "status_leds.h"
#include "persistent_storage.h"
#include "safety_monitor.h"
#include "buttons.h"
#include "rotary_encoder.h"
#include "position_tracker.h"
#include "limit_switches.h"
#include "calibration_engine.h"
#include "mqtt_client.h"

namespace garage {

class GarageDoorController {
public:
    GarageDoorController(
        IStepperMotor& motor,
        IButtons& buttons,
        IStatusLeds& leds,
        IPersistentStorage& storage,
        ISafetyMonitor& safety,
        IRotaryEncoder& encoder,
        IPositionTracker& tracker,
        ILimitSwitches& limits,
        ICalibrationEngine& calibration,
        IMqttClient& mqtt)
        : motor_(motor),
          buttons_(buttons),
          leds_(leds),
          storage_(storage),
          safety_(safety),
          encoder_(encoder),
          tracker_(tracker),
          limits_(limits),
          calibration_(calibration),
          mqtt_(mqtt) {}

    void init();
    void tick();

    DoorStatus current_status() const;
    CommandResponse submit_event(const Event& event);

private:
    void handle_boot();

    CommandResponse handle_local_sw1();
    CommandResponse handle_local_calibrate();

    CommandResponse handle_remote_toggle();
    CommandResponse handle_remote_open();
    CommandResponse handle_remote_close();
    CommandResponse handle_remote_stop();
    CommandResponse handle_remote_calibrate();

    CommandResponse handle_toggle_command(const char* command_name);

    CommandResponse handle_calibrate_command(const char* command_name);

    void start_opening();
    void start_closing();
    void stop_motion();

    void start_calibration_down();
    void continue_calibration_up();
    void tick_calibration();

    void process_button_events();
    void process_remote_events();
    void process_encoder_events();
    void process_limit_switches();

    void enter_error_stuck();
    void enter_error_calibration_failed();

    void refresh_feedback();
    void publish_status_if_needed();
    void save_if_needed();

private:
    IStepperMotor& motor_;
    IButtons& buttons_;
    IStatusLeds& leds_;
    IPersistentStorage& storage_;
    ISafetyMonitor& safety_;

    IRotaryEncoder& encoder_;
    IPositionTracker& tracker_;
    ILimitSwitches& limits_;
    ICalibrationEngine& calibration_;
    IMqttClient& mqtt_;

    bool publish_needed_ {true};
    bool save_needed_ {false};

    DoorStatus status_ {};

    MoveDirection last_motion_direction_ {MoveDirection::Up};
};

} // namespace garage