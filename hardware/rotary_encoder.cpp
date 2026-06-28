#include "rotary_encoder.h"
#include "hardware/gpio.h"
#include <cstdio>

namespace garage {

    queue_t        RotaryEncoder::s_queue_;
    bool           RotaryEncoder::s_queue_ready_ = false;
    RotaryEncoder* RotaryEncoder::s_instance_    = nullptr;

    RotaryEncoder::RotaryEncoder(uint pin_a, uint pin_b)
        : pin_a_(pin_a), pin_b_(pin_b) {}

    void RotaryEncoder::init() {
        if (!s_queue_ready_) {
            queue_init(&s_queue_, sizeof(EncoderTurn), 128);
            s_queue_ready_ = true;
        }

        gpio_init(pin_a_);
        gpio_set_dir(pin_a_, GPIO_IN);
        gpio_disable_pulls(pin_a_);

        gpio_init(pin_b_);
        gpio_set_dir(pin_b_, GPIO_IN);
        gpio_disable_pulls(pin_b_);

        s_instance_ = this;

        // register interrupt only on channel A: trigger on rising edge
        gpio_set_irq_enabled_with_callback(
            pin_a_,
            GPIO_IRQ_EDGE_RISE,
            true,
            &RotaryEncoder::gpio_irq_handler
        );

        printf("[Encoder] init (GPIO IRQ) - A=GP%u  B=GP%u\n", pin_a_, pin_b_);
    }

    void RotaryEncoder::gpio_irq_handler(uint gpio, uint32_t events) {
        if (!s_instance_) return;

        if (gpio != s_instance_->pin_a_) return;

        if ((events & GPIO_IRQ_EDGE_RISE) == 0) return;

        const bool b_high = gpio_get(s_instance_->pin_b_);
        const EncoderTurn turn = b_high
            ? EncoderTurn::CounterClockwise
            : EncoderTurn::Clockwise;

        queue_try_add(&s_queue_, &turn);
    }

    std::optional<EncoderTurn> RotaryEncoder::poll_event() {
        EncoderTurn turn;
        if (queue_try_remove(&s_queue_, &turn)) {
            return turn;
        }
        return std::nullopt;
    }

} // namespace garage