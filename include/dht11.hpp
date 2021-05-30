#pragma once

#include "driver/gpio.h"

class DHT11
{
private:
    gpio_num_t gpio; //!< DATA gpio.

    /**
     * @brief Wait microseconds.
     * @param us Microseconds to wait.
     */

    inline void IRAM_ATTR waitMicros(uint64_t us);

    /**
     * @brief Set DATA gpio as input.
     */
    inline void IRAM_ATTR setInput();

    /**
     * @brief Wait for given state.
     * @param state State to wait.
     * @param tout Timeout in microseconds.
     * @return True if state appeared.
     */
    inline bool IRAM_ATTR waitForState(bool state, uint64_t tout);

    /**
     * @brief Set DATA pin to output and pull it to VCC.
     */
    inline void IRAM_ATTR setOutputAndPullHigh();

public:

    /**
     * @brief Initialize DHT11 sensor.
     * @param dataPin DATA pin.
     */
    void init(gpio_num_t dataPin);

    /**
     * @brief Get humidity in %.
     */
    float read();
};