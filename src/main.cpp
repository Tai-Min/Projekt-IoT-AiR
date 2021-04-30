#include "nvs_flash.h"
#include "esp_event.h"
#include "driver/gpio.h"

#include "../include/wifi_init.hpp"

#define SMART_CONFIG_GPIO 19

#define BTN_PRESSED(p) gpio_get_level((gpio_num_t)p) == 0

void initSmartConfigGPIO()
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << SMART_CONFIG_GPIO);
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);
}

extern "C"
{
    void app_main(void)
    {

        ESP_ERROR_CHECK(nvs_flash_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default()); // Create ESP event loop.

        initSmartConfigGPIO();

        // Button pressed - use smart config.
        bool useSmartConfig = false;
        if (BTN_PRESSED(SMART_CONFIG_GPIO))
        {
            useSmartConfig = true;
        }

        initWiFi(useSmartConfig);

        while(true){
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}