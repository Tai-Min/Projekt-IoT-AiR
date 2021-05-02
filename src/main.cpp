#include "../include/config.hpp"
#include "../include/i2c.hpp"
#include "../include/bmp180.hpp"
#include "../include/dht11.hpp"
#include "../include/wifi.hpp"
#include "../include/mqtt.hpp"
#include "../include/http.hpp"

#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_log.h"

static void pressureTask(void *arg);
static void humidityTask(void *arg);

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

        // Init stuff.
        I2C_init(I2C_PORT, I2C_SDA, I2C_SCL, I2C_FREQ);
        WiFi_init(SMART_CONFIG_BUTTON_PIN, SMART_CONFIG_LED_PIN, WIFI_LED_PIN);
        MQTT_init(MQTT_LED_PIN);
        HTTP_init(HTTP_BUTTON_PIN, HTTP_LED_PIN);

        // Create tasks.
        xTaskCreate(pressureTask, "pressureTask", 2048, (void *)5000, tskIDLE_PRIORITY, NULL);
        xTaskCreate(humidityTask, "humidityTask", 3072, (void *)5000, tskIDLE_PRIORITY, NULL);

        while (true)
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}

static void pressureTask(void *arg)
{
    uint16_t delayTime = (uint16_t)((size_t)arg);

    BMP180 sensor;

    while (true)
    {
        MQTT_publish("pressure", sensor.read(BMP180::MeasurementType::ULTRA_HIGH_RES), 1);
        MQTT_publish("temperature", sensor.read(BMP180::MeasurementType::TEMPERATURE), 1);
        vTaskDelay(delayTime / portTICK_PERIOD_MS);
    }
}

static void humidityTask(void *arg)
{
    int16_t delayTime = (uint16_t)((size_t)arg);

    DHT11 sensor;
    sensor.init(DHT11_DATA_PIN);

    while (true)
    {
        float result = sensor.read();
        if (result >= 0)
        {
            MQTT_publish("humidity", result, 1);
        }

        // This sensor is too slow to handle faster readings.
        if (delayTime < 2000)
            vTaskDelay(2100 / portTICK_PERIOD_MS);
        else
            vTaskDelay(delayTime / portTICK_PERIOD_MS);
    }
}