#include "stepper_motor.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

namespace garage {

namespace {

constexpr bool kPhaseTable[4][4] = {
    {true,  true,  false, false}, // IN1 + IN2
    {false, true,  true,  false}, // IN2 + IN3
    {false, false, true,  true }, // IN3 + IN4
    {true,  false, false, true }  // IN4 + IN1
};

} // namespace

void StepperMotor::init() {
    gpio_init(BoardConfig::motor_in1);
    gpio_set_dir(BoardConfig::motor_in1, GPIO_OUT);

    gpio_init(BoardConfig::motor_in2);
    gpio_set_dir(BoardConfig::motor_in2, GPIO_OUT);

    gpio_init(BoardConfig::motor_in3);
    gpio_set_dir(BoardConfig::motor_in3, GPIO_OUT);

    gpio_init(BoardConfig::motor_in4);
    gpio_set_dir(BoardConfig::motor_in4, GPIO_OUT);

    deenergize_all();

    running_ = false;
    direction_ = MoveDirection::Up;
    step_interval_us_ = 4000;
    last_step_time_us_ = to_us_since_boot(get_absolute_time());
    phase_index_ = 0;
}

void StepperMotor::start(MoveDirection dir, uint32_t step_interval_us) {
    direction_ = dir;
    step_interval_us_ = step_interval_us;
    running_ = true;

    last_step_time_us_ = to_us_since_boot(get_absolute_time());

    apply_phase(phase_index_);
}

void StepperMotor::stop() {
    running_ = false;
    deenergize_all();
}

bool StepperMotor::is_running() const {
    return running_;
}

MoveDirection StepperMotor::direction() const {
    return direction_;
}

void StepperMotor::tick() {
    if (!running_) {
        return;
    }

    uint64_t now_us = to_us_since_boot(get_absolute_time());
    if (now_us - last_step_time_us_ < step_interval_us_) {
        return;
    }

    last_step_time_us_ = now_us;

    if (direction_ == MoveDirection::Up) {
        phase_index_ = static_cast<uint8_t>((phase_index_ + 3) % 4);
    } else {
        phase_index_ = static_cast<uint8_t>((phase_index_ + 1) % 4);
    }

    apply_phase(phase_index_);
}

void StepperMotor::apply_phase(uint8_t phase_index) {
    gpio_put(BoardConfig::motor_in1, kPhaseTable[phase_index][0]);
    gpio_put(BoardConfig::motor_in2, kPhaseTable[phase_index][1]);
    gpio_put(BoardConfig::motor_in3, kPhaseTable[phase_index][2]);
    gpio_put(BoardConfig::motor_in4, kPhaseTable[phase_index][3]);
}

void StepperMotor::deenergize_all() {
    gpio_put(BoardConfig::motor_in1, false);
    gpio_put(BoardConfig::motor_in2, false);
    gpio_put(BoardConfig::motor_in3, false);
    gpio_put(BoardConfig::motor_in4, false);
}

} // namespace garage