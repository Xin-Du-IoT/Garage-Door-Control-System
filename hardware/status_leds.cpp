#include "status_leds.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

namespace garage {

void StatusLeds::init() {
    gpio_init(BoardConfig::led_green);
    gpio_set_dir(BoardConfig::led_green, GPIO_OUT);

    gpio_init(BoardConfig::led_red);
    gpio_set_dir(BoardConfig::led_red, GPIO_OUT);

    gpio_init(BoardConfig::led_yellow);
    gpio_set_dir(BoardConfig::led_yellow, GPIO_OUT);

    set_leds(false, false, false);

    blink_on_ = false;
    last_blink_ms_ = to_ms_since_boot(get_absolute_time());
    current_mode_ = LedMode::Off;
}

void StatusLeds::show_status(const DoorStatus &status) {
    last_status_ = status;
    if (status.error_state != ErrorState::Normal) {
        current_mode_ = LedMode::RedBlink;
        return;
    }

    switch (status.door_state) {
        case DoorState::Open:
            current_mode_ = LedMode::GreenSolid;
            break;

        case DoorState::Closed:
            current_mode_ = LedMode::RedSolid;
            break;

        case DoorState::Opening:
        case DoorState::Closing:
        case DoorState::InBetween:
        case DoorState::Stopped:
        case DoorState::Calibrating:
            current_mode_ = LedMode::YellowSolid;
            break;

        case DoorState::Error:
            current_mode_ = LedMode::RedBlink;
            break;

        default:
            current_mode_ = LedMode::Off;
            break;
    }

    apply_mode(current_mode_);
}

void StatusLeds::tick() {
    if (current_mode_ != LedMode::RedBlink) {
        return;
    }

    uint32_t now_ms = to_ms_since_boot(get_absolute_time());
    if (now_ms - last_blink_ms_ >= blink_interval_ms_) {
        last_blink_ms_ = now_ms;
        blink_on_ = !blink_on_;
        set_leds(false, false, blink_on_);
    }
}

void StatusLeds::apply_mode(LedMode mode) {
    switch (mode) {
        case LedMode::GreenSolid:
            set_leds(true, false, false);
            break;

        case LedMode::YellowSolid:
            set_leds(false, true, false);
            break;

        case LedMode::RedSolid:
            set_leds(false, false, true);
            break;

        case LedMode::RedBlink:
            blink_on_ = true;
            last_blink_ms_ = to_ms_since_boot(get_absolute_time());
            set_leds(false, false, true);
            break;

        case LedMode::Off:
        default:
            set_leds(false, false, false);
            break;
    }
}

void StatusLeds::set_leds(bool green_on, bool yellow_on, bool red_on) {
    gpio_put(BoardConfig::led_green, green_on);
    gpio_put(BoardConfig::led_yellow, yellow_on);
    gpio_put(BoardConfig::led_red, red_on);
}


}