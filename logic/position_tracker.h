#pragma once
#include <cstdint>
#include "rotary_encoder.h"

namespace garage {

class IPositionTracker {
public:
    virtual ~IPositionTracker() = default;

    virtual void reset_position(int32_t pos = 0) = 0;

    virtual void on_encoder_event(EncoderTurn ev) = 0;

    virtual int32_t current_position() const = 0;
};

class PositionTracker : public IPositionTracker {
public:
    PositionTracker() = default;

    void    reset_position(int32_t pos = 0) override;
    void    on_encoder_event(EncoderTurn ev) override;
    int32_t current_position() const override;

private:
    int32_t position_{0};
};

} // namespace garage
