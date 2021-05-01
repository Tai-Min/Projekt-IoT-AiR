#include "../include/wifi.hpp"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_smartconfig.h"
#include "esp_log.h"

static const char *TAG_WIFI = "WIFI";
static const char *TAG_SC = "SC";

static const int CONNECTED_BIT = BIT0;
static EventGroupHandle_t s_wifi_event_group;

/**
 * @brief Init WiFi.
 * 
 * This function will initialize WiFi stack and use network's credentials from flash to secure connection.
 * Optionally, this function can ignore flash data and run ESP's smart config to sniff new WiFi credentials.
 * 
 * @param useSmartConfig Whether to use smart config sniffer instead of flash data.
 */
void WiFi_init(bool useSmartConfig);

/**
 * @brief Check whether device is connected to WiFi.
 * @return True if connected.
 */
bool WiFi_isConnected();

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

void WiFi_init(bool useSmartConfig)
{
    ESP_ERROR_CHECK(esp_netif_init()); // Initialize TCP/IP stack.

    s_wifi_event_group = xEventGroupCreate();                     // FreeRTOS event group to store wifi event bits.
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

    // Seek new credentials.
    if (useSmartConfig)
    {
        startSmartConfig();
    }
    // Use flash credentials.
    else
    {
        connectToNetwork(nullptr);
    }
}

bool WiFi_isConnected()
{
    EventBits_t uxBits = xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT, false, false, portMAX_DELAY);
    return uxBits & CONNECTED_BIT;
}

static void networkEventHandler(void *arg, esp_event_base_t eventBase,
                                int32_t eventId, void *eventData)
{
    if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG_WIFI, "Disconnected from network");
        ESP_ERROR_CHECK(esp_wifi_connect());
        xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
    }
    else if (eventBase == IP_EVENT && eventId == IP_EVENT_STA_GOT_IP)
    {
        ESP_LOGI(TAG_WIFI, "Connected to network");
        xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
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
    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));
}

static void stopSmartConfig()
{
    ESP_LOGI(TAG_SC, "Smart config finished");
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
        printf("AAAAA: %s\n", confAlt.sta.ssid);
        conf = &confAlt;
    }

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, conf));
    ESP_ERROR_CHECK(esp_wifi_connect());
}