#pragma once

#include <cstdint>
#include "garage_types.h"
#include "board_config.h"

namespace garage {

    class IStatusLeds {
    public:
        virtual ~IStatusLeds() = default;
        virtual void init() = 0;
        virtual void show_status(const DoorStatus& status) = 0;
        virtual void tick() = 0;
    };

    class StatusLeds : public IStatusLeds {
    public:
        StatusLeds() = default;

        void init() override;
        void show_status(const DoorStatus& status) override;
        void tick() override;

    private:
        enum class LedMode {
            GreenSolid,
            YellowSolid,
            RedSolid,
            RedBlink,
            Off
        };

        void apply_mode(LedMode mode);
        void set_leds(bool green_on, bool yellow_on, bool red_on);

        DoorStatus last_status_{};
        LedMode current_mode_{LedMode::Off};

        bool blink_on_{false};
        uint32_t last_blink_ms_{0};

        static constexpr uint32_t blink_interval_ms_ = 500;
    };

} // namespace garage