#include "safety_monitor.h"
#include "pico/stdlib.h"

namespace garage {

    void SafetyMonitor::on_motor_started(MoveDirection dir, int32_t start_pos) {
        monitoring_active_ = true;
        direction_ = dir;
        last_position_ = start_pos;
        last_movement_time_ms_ = to_ms_since_boot(get_absolute_time());
    }

    void SafetyMonitor::on_motor_stopped() {
        monitoring_active_ = false;
    }

    bool SafetyMonitor::is_stuck(bool motor_running, int32_t current_pos) {
        if (!monitoring_active_ || !motor_running) {
            return false;
        }

        uint32_t now_ms = to_ms_since_boot(get_absolute_time());

        if (current_pos != last_position_) {
            last_position_ = current_pos;
            last_movement_time_ms_ = now_ms;
            return false;
        }

        if (now_ms - last_movement_time_ms_ >= stuck_timeout_ms_) {
            return true;
        }

        return false;
    }

} // namespace garage