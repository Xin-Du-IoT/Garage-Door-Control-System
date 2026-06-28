#pragma once
#include "garage_types.h"

namespace garage {
class ICalibrationEngine {
public:
    virtual ~ICalibrationEngine() = default;

    virtual void begin() = 0;

    virtual bool active() const = 0;

    virtual bool finished() const = 0;

    virtual bool failed() const = 0;

    virtual void notify_bottom_limit(int32_t current_pos) = 0;

    virtual void notify_top_limit(int32_t current_pos) = 0;

    virtual void abort() = 0;

    virtual CalibrationResult result() const = 0;

    // status
    virtual const char* phase_name() const = 0;
};

class CalibrationEngine : public ICalibrationEngine {
public:
    CalibrationEngine() = default;

    void begin()           override;
    bool active()    const override;
    bool finished()  const override;
    bool failed()    const override;

    void notify_bottom_limit(int32_t current_pos) override;
    void notify_top_limit(int32_t current_pos)    override;
    void abort()           override;

    CalibrationResult result()     const override;
    const char*       phase_name() const override;

private:
    enum class Phase { Idle, FindingBottom, FindingTop, Done, Failed };

    // failed，
    void set_failed(const char* reason);

    Phase             phase_  {Phase::Idle};
    CalibrationResult result_ {};
};

} // namespace garage
