#pragma once

#include <cstdint>
#include "garage_types.h"

namespace garage {

    class ISafetyMonitor {
    public:
        virtual ~ISafetyMonitor() = default;

        virtual void on_motor_started(MoveDirection dir, int32_t start_pos) = 0;

        virtual void on_motor_stopped() = 0;

        virtual bool is_stuck(bool motor_running, int32_t current_pos) = 0;
    };

    class SafetyMonitor : public ISafetyMonitor {
    public:
        void on_motor_started(MoveDirection dir, int32_t start_pos) override;
        void on_motor_stopped() override;
        bool is_stuck(bool motor_running, int32_t current_pos) override;

    private:
        bool monitoring_active_{false};
        MoveDirection direction_{MoveDirection::Up};

        int32_t last_position_{0};
        uint32_t last_movement_time_ms_{0};

        static constexpr uint32_t stuck_timeout_ms_ = 30000;
    };

} // namespace garage