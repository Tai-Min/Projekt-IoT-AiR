#include "../include/wifi.hpp"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_smartconfig.h"
#include "esp_log.h"

static const char *TAG_WIFI = "WIFI";
static const char *TAG_SC = "SC";

static gpio_num_t _smartConfigLED;
static gpio_num_t _WiFiLed;

// External functions.
void WiFi_init(gpio_num_t smartConfigBtnPin, gpio_num_t smartConfigLED, gpio_num_t WiFiLed);

// Helper functions.
/**
 * @brief Manage network events.
 * @param arg Unused.
 * @param eventBase Type of event.
 * @param eventId Specific id of event.
 * @param eventData Some data sent with the event.
 */
static void networkEventHandler(void *arg, esp_event_base_t eventBase,
                                int32_t eventId, void *eventData);

/**
 * @brief Init GPIO that will be used for buttons and leds associated with WiFi.
 * @param smartConfigBtnPin Smart config button's GPIO.
 */
static void initGPIO(gpio_num_t smartConfigBtnPin, gpio_num_t smartConfigLED, gpio_num_t WiFiLed);

/**
 * @brief Copy data from smart config event struct to WiFi config struct.
 * @param sc Smart config event struct.
 * @param wc WiFi config struct.
 */
static void smartConfigToWiFiConfig(smartconfig_event_got_ssid_pswd_t *sc, wifi_config_t *wc);

/**
 * @brief Start smart config.
 */
static void startSmartConfig();

/**
 * @brief Stop smart config.
 */
static void stopSmartConfig();

/**
 * @brief Load WiFi credentials from flash and try to connect to the network.
 * @param conf Config to use or nullptr to retreive config from flash.
 */
static void connectToNetwork(wifi_config_t *conf);

// Function definitions.
void WiFi_init(gpio_num_t smartConfigBtnPin, gpio_num_t smartConfigLED, gpio_num_t WiFiLed)
{
    initGPIO(smartConfigBtnPin, smartConfigLED, WiFiLed);

    ESP_ERROR_CHECK(esp_netif_init()); // Initialize TCP/IP stack.

    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta(); // Create wifi station.
    assert(sta_netif);                                            // Check if wifi STA was initialized (no null pointer).

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    cfg.nvs_enable = true;

    ESP_ERROR_CHECK(esp_wifi_init(&cfg)); // Allocate stuff for WiFi and start WiFi task.

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &networkEventHandler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &networkEventHandler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &networkEventHandler, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Button pressed - use smart config via ESPTouch.
    if (gpio_get_level(smartConfigBtnPin) == 0)
    {
        startSmartConfig();
    }
    // Use flash credentials.
    else
    {
        connectToNetwork(nullptr);
    }
}

static void networkEventHandler(void *arg, esp_event_base_t eventBase,
                                int32_t eventId, void *eventData)
{
    if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG_WIFI, "Disconnected from network");
        gpio_set_level(_WiFiLed, 0);
        ESP_ERROR_CHECK(esp_wifi_connect());
    }
    else if (eventBase == IP_EVENT && eventId == IP_EVENT_STA_GOT_IP)
    {
        ESP_LOGI(TAG_WIFI, "Connected to network");
        gpio_set_level(_WiFiLed, 1);
        gpio_set_level(_smartConfigLED, 0);
    }
    else if (eventBase == SC_EVENT && eventId == SC_EVENT_GOT_SSID_PSWD)
    {
        ESP_LOGI(TAG_SC, "Got SSID and password");
        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)eventData; // Cast to valid struct.
        wifi_config_t WiFiConfig;
        smartConfigToWiFiConfig(evt, &WiFiConfig);
        connectToNetwork(&WiFiConfig);
    }
    // Smart config finished.
    else if (eventBase == SC_EVENT && eventId == SC_EVENT_SEND_ACK_DONE)
    {
        stopSmartConfig();
    }
}

static void initGPIO(gpio_num_t smartConfigBtnPin, gpio_num_t smartConfigLED, gpio_num_t WiFiLed)
{
    // Button.
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = ((uint64_t)1 << smartConfigBtnPin);
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);
    gpio_set_level(smartConfigBtnPin, 0);

    // Smart config LED.
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << smartConfigLED);
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    gpio_set_level(smartConfigLED, 0);
    _smartConfigLED = smartConfigLED;

    // WiFi LED.
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << WiFiLed);
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    gpio_set_level(WiFiLed, 0);
    _WiFiLed = WiFiLed;
}

static void smartConfigToWiFiConfig(smartconfig_event_got_ssid_pswd_t *sc, wifi_config_t *wc)
{
    bzero(wc, sizeof(wifi_config_t));
    memcpy(wc->sta.ssid, sc->ssid, sizeof(wc->sta.ssid));
    memcpy(wc->sta.password, sc->password, sizeof(wc->sta.password));
    wc->sta.bssid_set = sc->bssid_set;
    if (wc->sta.bssid_set == true)
    {
        memcpy(wc->sta.bssid, sc->bssid, sizeof(wc->sta.bssid));
    }
}

static void startSmartConfig()
{
    ESP_LOGI(TAG_SC, "Smart config started");
    gpio_set_level(_smartConfigLED, 1);
    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));
}

static void stopSmartConfig()
{
    ESP_LOGI(TAG_SC, "Smart config finished");
    gpio_set_level(_smartConfigLED, 0);
    ESP_ERROR_CHECK(esp_smartconfig_stop());
}

static void connectToNetwork(wifi_config_t *conf)
{
    ESP_LOGI(TAG_WIFI, "Connecting to network");
    ESP_ERROR_CHECK(esp_wifi_disconnect());

    // Use config from flash instead.
    wifi_config_t confAlt;
    if (conf == nullptr)
    {
        ESP_LOGI(TAG_WIFI, "Using config from flash");
        ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_STA, &confAlt));
        conf = &confAlt;
    }

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, conf));
    ESP_ERROR_CHECK(esp_wifi_connect());
}