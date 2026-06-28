#pragma once

#include <cstdint>
#include "board_config.h"
#include "garage_types.h"

namespace garage {

    class IStepperMotor {
    public:
        virtual ~IStepperMotor() = default;

        virtual void init() = 0;
        virtual void start(MoveDirection dir, uint32_t step_interval_us) = 0;
        virtual void stop() = 0;
        virtual bool is_running() const = 0;
        virtual MoveDirection direction() const = 0;
        virtual void tick() = 0;
    };

    class StepperMotor : public IStepperMotor {
    public:
        void init() override;
        void start(MoveDirection dir, uint32_t step_interval_us) override;
        void stop() override;
        bool is_running() const override;
        MoveDirection direction() const override;
        void tick() override;

    private:
        void apply_phase(uint8_t phase_index);
        void deenergize_all();

        bool running_{false};
        MoveDirection direction_{MoveDirection::Up};

        uint32_t step_interval_us_{2000};
        uint64_t last_step_time_us_{0};

        uint8_t phase_index_{0};
    };

} // namespace garage