#include "nvs_flash.h"
#include "esp_event.h"

#include "../include/wifi.hpp"
#include "../include/mqtt.hpp"
#include "../include/http.hpp"

#define SMART_CONFIG_BUTTON_PIN (gpio_num_t)19
#define SMART_CONFIG_LED_PIN (gpio_num_t)13
#define WIFI_LED_PIN (gpio_num_t)12
#define MQTT_LED_PIN (gpio_num_t)14

extern "C"
{
    void app_main(void)
    {
        // Flash for WiFi and MQTT config.
        esp_err_t err = nvs_flash_init();
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            // NVS partition was truncated and needs to be erased
            // Retry nvs_flash_init
            nvs_flash_erase();
            err = nvs_flash_init();
        }

        // Create ESP event loop.
        esp_event_loop_create_default(); 

        WiFi_init(SMART_CONFIG_BUTTON_PIN, SMART_CONFIG_LED_PIN, WIFI_LED_PIN);
        MQTT_init(MQTT_LED_PIN);
        HTTP_startWebServer();

        while (true)
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            MQTT_publish("pressure", 0.5, 1);
        }
    }
}