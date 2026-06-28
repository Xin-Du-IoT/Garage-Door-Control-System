#pragma once

#include <optional>
#include <cstdint>
#include "pico/stdlib.h"

namespace garage {

    enum class ButtonEvent {
        Sw0Pressed,
        Sw1Pressed,
        Sw2Pressed,
        Sw0Sw2Pressed
    };

    class IButtons {
    public:
        virtual ~IButtons() = default;
        virtual void init() = 0;
        virtual std::optional<ButtonEvent> poll_event() = 0;
    };

    class Buttons : public IButtons {
    public:
        Buttons();

        void init() override;
        std::optional<ButtonEvent> poll_event() override;

    private:
        bool read_sw0() const;
        bool read_sw1() const;
        bool read_sw2() const;

    private:
        bool prev_sw0_ {false};
        bool prev_sw1_ {false};
        bool prev_sw2_ {false};

        static constexpr uint32_t debounce_ms_ = 30;
        uint32_t last_event_ms_ {0};
    };

} // namespace garage