#include "calibration_engine.h"
#include <cstdio>

namespace garage {

// ── begin
void CalibrationEngine::begin() {
    result_ = CalibrationResult{};
    phase_  = Phase::FindingBottom;
    printf("[Calib] begin — phase: FindingBottom\n");
}

// ── active
bool CalibrationEngine::active() const {
    return phase_ == Phase::FindingBottom
        || phase_ == Phase::FindingTop;
}

// ── finished
bool CalibrationEngine::finished() const {
    return phase_ == Phase::Done
        || phase_ == Phase::Failed;
}

// ── failed
bool CalibrationEngine::failed() const {
    return phase_ == Phase::Failed;
}

// ── notify_bottom_limit
void CalibrationEngine::notify_bottom_limit(int32_t current_pos) {
    if (phase_ != Phase::FindingBottom) {
        set_failed("notify_bottom_limit called in wrong phase");
        return;
    }

    printf("[Calib] bottom limit hit (raw_pos=%ld) — A must reset position to 0\n",
           static_cast<long>(current_pos));

    phase_ = Phase::FindingTop;
    printf("[Calib] phase: FindingTop\n");
}

// ── notify_top_limit
void CalibrationEngine::notify_top_limit(int32_t current_pos) {
    if (phase_ != Phase::FindingTop) {
        set_failed("notify_top_limit called in wrong phase");
        return;
    }

    if (current_pos <= 0) {
        set_failed("top_limit position <= 0: encoder direction wrong or "
                   "position not reset after bottom");
        return;
    }

    result_.travel_steps = current_pos;
    result_.valid        = true;
    phase_               = Phase::Done;

    printf("[Calib] complete — travel_steps=%ld\n",
           static_cast<long>(result_.travel_steps));
}

// ── abort
void CalibrationEngine::abort() {
    if (phase_ == Phase::Done || phase_ == Phase::Failed) return;
    set_failed("abort() called externally");
}

// ── result
CalibrationResult CalibrationEngine::result() const {
    return result_;
}

// ── phase_name
const char* CalibrationEngine::phase_name() const {
    switch (phase_) {
        case Phase::Idle:          return "Idle";
        case Phase::FindingBottom: return "FindingBottom";
        case Phase::FindingTop:    return "FindingTop";
        case Phase::Done:          return "Done";
        case Phase::Failed:        return "Failed";
        default:                   return "Unknown";
    }
}

// ── private: set_failed
void CalibrationEngine::set_failed(const char* reason) {
    printf("[Calib] FAILED: %s\n", reason);
    result_ = CalibrationResult{};   // valid = false
    phase_  = Phase::Failed;
}

} // namespace garage
