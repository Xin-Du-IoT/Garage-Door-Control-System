#pragma once
#include "pico/stdlib.h"

namespace garage {

class ILimitSwitches {
public:
    virtual ~ILimitSwitches() = default;
    virtual void init() = 0;
    virtual bool top_active()    const = 0;
    virtual bool bottom_active() const = 0;
};

class LimitSwitches : public ILimitSwitches {
public:
    LimitSwitches(uint pin_top, uint pin_bottom);

    void init() override;
    bool top_active()    const override;
    bool bottom_active() const override;

private:
    uint pin_top_;
    uint pin_bottom_;
};

} // namespace garage
