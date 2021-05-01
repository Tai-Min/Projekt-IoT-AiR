#pragma once
#include "driver/gpio.h"

/**
 * @brief Initialize HTTP server.
 * @param btn Button to toggle server on / off.
 * @param led Indicator LED.
 */
void HTTP_init(gpio_num_t btn, gpio_num_t led);