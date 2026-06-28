#include "limit_switches.h"
#include "hardware/gpio.h"
#include <cstdio>

namespace garage {

LimitSwitches::LimitSwitches(uint pin_top, uint pin_bottom)
    : pin_top_(pin_top), pin_bottom_(pin_bottom) {}

void LimitSwitches::init() {
    gpio_init(pin_top_);
    gpio_set_dir(pin_top_, GPIO_IN);
    gpio_pull_up(pin_top_);    //  limit switches require pull-up resistors

    gpio_init(pin_bottom_);
    gpio_set_dir(pin_bottom_, GPIO_IN);
    gpio_pull_up(pin_bottom_); // pull-up resistors

    printf("[LimitSW] init — top=GP%u  bottom=GP%u  (internal pull-ups enabled)\n",
           pin_top_, pin_bottom_);
}

bool LimitSwitches::top_active() const {
    return !gpio_get(pin_top_);
}

bool LimitSwitches::bottom_active() const {
    return !gpio_get(pin_bottom_);
}

} // namespace garage
