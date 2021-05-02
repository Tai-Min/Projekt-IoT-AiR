#include "../include/dht11.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#include "esp_log.h"

inline void IRAM_ATTR DHT11::waitMicros(uint64_t us)
{
    uint64_t start = esp_timer_get_time();
    while (esp_timer_get_time() - start < us)
    {
        asm volatile("nop");
    }
}

inline void IRAM_ATTR DHT11::setInput()
{
    gpio_set_direction(gpio, GPIO_MODE_INPUT);
}

inline void IRAM_ATTR DHT11::waitForState(bool state, uint64_t tout, bool &ok)
{
    ok = false;
    uint64_t start = esp_timer_get_time();
    while (gpio_get_level(gpio) != state)
    {
        if (esp_timer_get_time() - start > tout)
        {
            setOutputAndPullHigh();
        }
    }
    ok = true;
}

inline void IRAM_ATTR DHT11::setOutputAndPullHigh()
{
    gpio_set_direction(gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio, 1);
}

void DHT11::init(gpio_num_t dataPin)
{
    conf.intr_type = GPIO_INTR_DISABLE;
    conf.mode = GPIO_MODE_OUTPUT;
    conf.pin_bit_mask = ((uint64_t)1 << dataPin);
    conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio = dataPin;
    gpio_config(&conf);
    setOutputAndPullHigh();
}

float DHT11::read()
{
    bool ok;

    // Start signal.
    gpio_set_level(gpio, 0);
    waitMicros(200000);

    // DHT11 starts sending stuff.
    setInput();

    // Wait for DHT11 response LOW - HIGH should last max 40us.
    waitForState(0, 50, ok);
    if (!ok)
        return -1;

    // Wait for DHT11 response HIGH - LOW should last max 80us.
    waitForState(1, 90, ok);
    if (!ok)
        return -1;

    // Wait for DHT11 response LOW - HIGH should last max 80us.
    waitForState(0, 90, ok);
    if (!ok)
        return -1;

    // Standard data transmission - 40 bits.
    uint64_t data = 0;
    for (int i = 39; i >= 0; i--)
    {
        // Wait for next HIGH state - LOW should last max 50us.
        waitForState(1, 60, ok);
        if (!ok)
            return -1;

        uint64_t start = esp_timer_get_time(); // Measure time of high state.

        // Wait for next LOW state - HIGH should last max 70us.
        waitForState(0, 90, ok);
        if (!ok)
            return -1;

        // Measure time of HIGH state.
        if (esp_timer_get_time() - start > 38)    // Less than 30 means "0", bigger than that means "1".
            data |= ((uint64_t)1 << i); // Add bit to storage.
    }

    // Finish transmission.
    setOutputAndPullHigh();

    // Get the results.
    uint8_t checksum = data;
    uint8_t tempDecimal = (data >> 8); // 0 for DHT11.
    uint8_t tempIntegral = (data >> 16);
    uint8_t humDecimal = (data >> 24); // 0 for DHT11.
    uint8_t humIntegral = (data >> 32);

    // Check if data is valid.
    uint8_t requiredChecksum = (uint64_t)(tempDecimal + tempIntegral + humDecimal + humIntegral); // Last 8 bits of sum.
    if (checksum != requiredChecksum)
        return -1;

    return humIntegral;
}