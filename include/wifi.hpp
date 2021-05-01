#pragma once
#include "driver/gpio.h"

/**
 * @brief Init WiFi.
 * 
 * This function will initialize WiFi stack and use network's credentials from flash to secure connection.
 * Optionally, if smartConfigBtnPin was presset on boot, 
 * this function will ignore flash data and run ESP's smart config to sniff new WiFi credentials.
 * 
 * @param smartConfigBtnPin GPIO for smart config button.
 * @param smartConfigLED GPIO for smart config led.
 * @param WiFiLed GPIO for WiFi led.
 */
void WiFi_init(gpio_num_t smartConfigBtnPin, gpio_num_t smartConfigLED, gpio_num_t WiFiLed);