#pragma once

#include <optional>
#include "pico/stdlib.h"
#include "pico/util/queue.h"

namespace garage {

    enum class EncoderTurn {
        Clockwise,
        CounterClockwise
    };

    class IRotaryEncoder {
    public:
        virtual ~IRotaryEncoder() = default;
        virtual void init() = 0;
        virtual std::optional<EncoderTurn> poll_event() = 0;
    };

    class RotaryEncoder : public IRotaryEncoder {
    public:
        RotaryEncoder(uint pin_a, uint pin_b);
        ~RotaryEncoder() override = default;

        void init() override;
        std::optional<EncoderTurn> poll_event() override;

    private:
        static void gpio_irq_handler(uint gpio, uint32_t events);

        uint pin_a_;
        uint pin_b_;

        static queue_t        s_queue_;
        static bool           s_queue_ready_;
        static RotaryEncoder* s_instance_;
    };

} // namespace garage