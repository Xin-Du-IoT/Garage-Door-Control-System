#pragma once
#include <cstdint>

namespace garage {

    struct BoardConfig {
        static constexpr uint32_t motor_in1 = 2;
        static constexpr uint32_t motor_in2 = 3;
        static constexpr uint32_t motor_in3 = 6;
        static constexpr uint32_t motor_in4 = 13;

        static constexpr uint32_t led_green  = 20;
        static constexpr uint32_t led_yellow = 21;
        static constexpr uint32_t led_red    = 22;

        static constexpr uint32_t encoder_pin_a = 16;
        static constexpr uint32_t encoder_pin_b = 17;

        static constexpr uint32_t limit_top    = 28;
        static constexpr uint32_t limit_bottom = 27;

        static constexpr uint32_t sw0 = 9;
        static constexpr uint32_t sw1 = 8;
        static constexpr uint32_t sw2 = 7;
    };

} // namespace garage