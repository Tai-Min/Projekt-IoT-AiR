#pragma once

#include "driver/gpio.h"

class DHT11
{
private:
    gpio_config_t conf;
    gpio_num_t gpio;

    inline void IRAM_ATTR waitMicros(uint64_t us);
    inline void IRAM_ATTR setInput();
    inline void IRAM_ATTR waitForState(bool state, uint64_t tout, bool &ok);
    inline void IRAM_ATTR setOutputAndPullHigh();

public:
    void init(gpio_num_t dataPin);
    float read();
};