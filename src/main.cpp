#include "nvs_flash.h"
#include "esp_event.h"
#include "driver/gpio.h"

#include "../include/wifi.hpp"
#include "../include/mqtt.hpp"
#include "../include/http.hpp"

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
        // Flash for WiFi and MQTT data.
        esp_err_t err = nvs_flash_init();
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            // NVS partition was truncated and needs to be erased
            // Retry nvs_flash_init
            nvs_flash_erase();
            err = nvs_flash_init();
        }

        esp_event_loop_create_default(); // Create ESP event loop.

        initSmartConfigGPIO();

        // Button pressed - use smart config via ESPTouch.
        bool useSmartConfig = false;
        if (BTN_PRESSED(SMART_CONFIG_GPIO))
        {
            useSmartConfig = true;
        }

        WiFi_init(useSmartConfig);
        MQTT_init();
        HTTP_startWebServer();

        while (true)
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}