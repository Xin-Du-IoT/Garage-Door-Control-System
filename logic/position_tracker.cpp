#include "position_tracker.h"

namespace garage {

    void PositionTracker::reset_position(int32_t pos) {
        position_ = pos;
    }

    void PositionTracker::on_encoder_event(EncoderTurn ev) {
        if (ev == EncoderTurn::Clockwise) {
            ++position_;
        } else {
            --position_;
        }
    }

    int32_t PositionTracker::current_position() const {
        return position_;
    }

} // namespace garage