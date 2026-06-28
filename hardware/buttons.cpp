#include "buttons.h"
#include "board_config.h"

namespace garage {

Buttons::Buttons() = default;

void Buttons::init() {
    gpio_init(BoardConfig::sw0);
    gpio_set_dir(BoardConfig::sw0, GPIO_IN);
    gpio_pull_up(BoardConfig::sw0);

    gpio_init(BoardConfig::sw1);
    gpio_set_dir(BoardConfig::sw1, GPIO_IN);
    gpio_pull_up(BoardConfig::sw1);

    gpio_init(BoardConfig::sw2);
    gpio_set_dir(BoardConfig::sw2, GPIO_IN);
    gpio_pull_up(BoardConfig::sw2);

    prev_sw0_ = read_sw0();
    prev_sw1_ = read_sw1();
    prev_sw2_ = read_sw2();

    last_event_ms_ = to_ms_since_boot(get_absolute_time());
}

std::optional<ButtonEvent> Buttons::poll_event() {
    const uint32_t now_ms = to_ms_since_boot(get_absolute_time());

    bool sw0 = read_sw0();
    bool sw1 = read_sw1();
    bool sw2 = read_sw2();

    bool sw0_pressed_edge = (!prev_sw0_ && sw0);
    bool sw1_pressed_edge = (!prev_sw1_ && sw1);
    bool sw2_pressed_edge = (!prev_sw2_ && sw2);

    prev_sw0_ = sw0;
    prev_sw1_ = sw1;
    prev_sw2_ = sw2;

    if (now_ms - last_event_ms_ < debounce_ms_) {
        return std::nullopt;
    }

    if (sw0 && sw2 && (sw0_pressed_edge || sw2_pressed_edge)) {
        last_event_ms_ = now_ms;
        return ButtonEvent::Sw0Sw2Pressed;
    }

    if (sw1_pressed_edge) {
        last_event_ms_ = now_ms;
        return ButtonEvent::Sw1Pressed;
    }

    if (sw0_pressed_edge) {
        last_event_ms_ = now_ms;
        return ButtonEvent::Sw0Pressed;
    }

    if (sw2_pressed_edge) {
        last_event_ms_ = now_ms;
        return ButtonEvent::Sw2Pressed;
    }

    return std::nullopt;
}

bool Buttons::read_sw0() const {
    // pull-up + active-low
    return gpio_get(BoardConfig::sw0) == 0;
}

bool Buttons::read_sw1() const {
    return gpio_get(BoardConfig::sw1) == 0;
}

bool Buttons::read_sw2() const {
    return gpio_get(BoardConfig::sw2) == 0;
}

} // namespace garage