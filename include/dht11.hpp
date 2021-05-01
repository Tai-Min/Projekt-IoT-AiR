#pragma once

#include "driver/gpio.h"

class DHT11
{
public:
    void init(gpio_num_t dataPin);
    float read();
};