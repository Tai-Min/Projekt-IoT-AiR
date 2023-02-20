#include "../include/mqtt.hpp"
#include "esp_log.h"
#include "nvs_flash.h"
#include "mqtt_client.h"
#include "driver/gpio.h"

static const char *TAG_MQTT = "MQTT";

static SemaphoreHandle_t mqttResourceSemaphore;

static bool connected = false;
static gpio_num_t _led;

static const size_t maxIpSize = 16;
static const size_t maxPortSize = 6;
static const size_t maxUsernameSize = 33;
static const size_t maxPasswordSize = 33;
static const size_t maxNamespaceSize = 33;

static char ip[maxIpSize], port[maxPortSize], username[maxUsernameSize], password[maxPasswordSize], ns[maxNamespaceSize];
static size_t ipSize = maxIpSize,
              portSize = maxPortSize,
              usernameSize = maxUsernameSize,
              passwordSize = maxPasswordSize,
              namespaceSize = maxNamespaceSize;

static esp_mqtt_client_handle_t client;

// External functions.
void MQTT_init(gpio_num_t LEDGPIO);
void MQTT_reInit();

void MQTT_resourceTake();
void MQTT_resourceRelease();

void MQTT_publish(const char *topic, float data, int qos);

void MQTT_updateIP(const char *ip);
void MQTT_updatePort(const char *port);
void MQTT_updateUser(const char *usr);
void MQTT_updatePassword(const char *passwd);
void MQTT_updateNamespace(const char *ns);

const char *MQTT_getIP();
const char *MQTT_getPort();
const char *MQTT_getUser();
const char *MQTT_getPassword();
const char *MQTT_getNamespace();

// Helper functions.
/**
 * @brief Init MQTT.
 */
void init_impl();

/**
 * @brief Init GPIO that will be used for MQTT LED.
 */
static void initGPIO(gpio_num_t gpio);

/**
 * @brief Publish any primitive data to broker.
 * @param topic Topic to publish.
 * @param data Data to publish.
 * @param qos QoS.
 * @param formatter Formatter string such as in printf().
 */
template <typename T>
void MQTT_publish_impl(const char *topic, T data, int qos, const char *formatter);

/**
 * @brief Load MQTT config from flash. 
 * Must be able to take resources using MQTT_resourceTake().
 */
static void loadFromFlash();

/**
 * @brief 
 * @param handlerArgs Unused.
 * @param base Unused.
 * @param eventId Event id.
 * @param eventData Unused.
 */
static void eventHandler(void *handlerArgs, esp_event_base_t base, int32_t eventId, void *eventData);

// Function definitions.
void MQTT_init(gpio_num_t LEDGPIO)
{
    initGPIO(LEDGPIO);
    init_impl();
}

void MQTT_reInit()
{
    esp_mqtt_client_disconnect(client);
    esp_mqtt_client_stop(client);
    init_impl();
}

void MQTT_resourceTake()
{
    xSemaphoreTake(mqttResourceSemaphore, (TickType_t)10);
}

void MQTT_resourceRelease()
{
    xSemaphoreGive(mqttResourceSemaphore);
}

void MQTT_publish(const char *topic, float data, int qos)
{
    MQTT_publish_impl(topic, data, qos, "%f");
}

void MQTT_updateIP(const char *ip)
{
    ESP_LOGI(TAG_MQTT, "Updated IP: %s", ip);

    nvs_handle_t nvsHandle;
    ESP_ERROR_CHECK(nvs_open("mqtt", NVS_READWRITE, &nvsHandle));
    ESP_ERROR_CHECK(nvs_set_str(nvsHandle, "ip", ip));
    ESP_ERROR_CHECK(nvs_commit(nvsHandle));
    nvs_close(nvsHandle);
}

void MQTT_updatePort(const char *port)
{
    ESP_LOGI(TAG_MQTT, "Updated port: %s", port);

    nvs_handle_t nvsHandle;
    ESP_ERROR_CHECK(nvs_open("mqtt", NVS_READWRITE, &nvsHandle));
    ESP_ERROR_CHECK(nvs_set_str(nvsHandle, "port", port));
    ESP_ERROR_CHECK(nvs_commit(nvsHandle));
    nvs_close(nvsHandle);
}

void MQTT_updateUser(const char *usr)
{
    ESP_LOGI(TAG_MQTT, "Updated username: %s", usr);

    nvs_handle_t nvsHandle;
    ESP_ERROR_CHECK(nvs_open("mqtt", NVS_READWRITE, &nvsHandle));
    ESP_ERROR_CHECK(nvs_set_str(nvsHandle, "usr", usr));
    ESP_ERROR_CHECK(nvs_commit(nvsHandle));
    nvs_close(nvsHandle);
}

void MQTT_updatePassword(const char *passwd)
{
    ESP_LOGI(TAG_MQTT, "Updated password: %s", passwd);

    nvs_handle_t nvsHandle;
    ESP_ERROR_CHECK(nvs_open("mqtt", NVS_READWRITE, &nvsHandle));
    ESP_ERROR_CHECK(nvs_set_str(nvsHandle, "pwd", passwd));
    ESP_ERROR_CHECK(nvs_commit(nvsHandle));
    nvs_close(nvsHandle);
}

void MQTT_updateNamespace(const char *ns)
{
    ESP_LOGI(TAG_MQTT, "Updated namespace: %s", ns);

    nvs_handle_t nvsHandle;
    ESP_ERROR_CHECK(nvs_open("mqtt", NVS_READWRITE, &nvsHandle));
    ESP_ERROR_CHECK(nvs_set_str(nvsHandle, "ns", ns));
    ESP_ERROR_CHECK(nvs_commit(nvsHandle));
    nvs_close(nvsHandle);
}

const char *MQTT_getIP()
{
    return ip;
}

const char *MQTT_getPort()
{
    return port;
}

const char *MQTT_getUser()
{
    return username;
}

const char *MQTT_getPassword()
{
    return password;
}

const char *MQTT_getNamespace()
{
    return ns;
}

void init_impl()
{
    ESP_LOGI(TAG_MQTT, "Starting MQTT client");

    if (mqttResourceSemaphore == NULL)
        mqttResourceSemaphore = xSemaphoreCreateMutex();

    loadFromFlash();

    esp_mqtt_client_config_t mqtt_cfg;
    mqtt_cfg.broker.address.uri = ip,
    mqtt_cfg.broker.address.port = (uint32_t)atoi(port),
    mqtt_cfg.credentials.username = username,
    mqtt_cfg.credentials.authentication.password = password;

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, eventHandler, client);
    esp_mqtt_client_start(client);
}

static void initGPIO(gpio_num_t led)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = ((uint64_t)1 << led);
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    gpio_set_level(led, 0);
    _led = led;
}

template <typename T>
void MQTT_publish_impl(const char *topic, T data, int qos, const char *formatter)
{
    static char dataStr[128];
    static char completedTopic[64];

    if (!connected)
        return;

    // Prepare topic.
    memset(completedTopic, 0, sizeof(completedTopic)); // Zero just in case.
    memcpy(completedTopic, ns, strlen(ns));            // Prepend namespace.
    strcat(completedTopic, "/");                       // Append slash.
    strcat(completedTopic, topic);                     // Append topic.

    // Prepare data.
    memset(dataStr, 0, sizeof(dataStr));                 // Zero just in case.
    snprintf(dataStr, sizeof(dataStr), formatter, data); // Replace formatter with given data.

    esp_mqtt_client_publish(client, completedTopic, dataStr, 0, qos, false);
    ESP_LOGI(TAG_MQTT, "%s\n", completedTopic);
}

static void loadFromFlash()
{
    MQTT_resourceTake();

    nvs_handle_t nvsHandle;
    ESP_ERROR_CHECK(nvs_open("mqtt", NVS_READONLY, &nvsHandle));

    esp_err_t err = nvs_get_str(nvsHandle, "ip", ip, &ipSize);
    if (err == ESP_OK)
        ESP_LOGI(TAG_MQTT, "Loaded IP: %s", ip);
    else
        ESP_LOGI(TAG_MQTT, "%s", esp_err_to_name(err));
    ipSize = maxIpSize;

    err = nvs_get_str(nvsHandle, "port", port, &portSize);
    if (err == ESP_OK)
        ESP_LOGI(TAG_MQTT, "Loaded port: %s", port);
    else
        ESP_LOGI(TAG_MQTT, "%s", esp_err_to_name(err));
    portSize = maxPortSize;

    err = nvs_get_str(nvsHandle, "usr", username, &usernameSize);
    if (err == ESP_OK)
        ESP_LOGI(TAG_MQTT, "Loaded user: %s", username);
    else
        ESP_LOGI(TAG_MQTT, "%s", esp_err_to_name(err));
    usernameSize = maxUsernameSize;

    err = nvs_get_str(nvsHandle, "pwd", password, &passwordSize);
    if (err == ESP_OK)
        ESP_LOGI(TAG_MQTT, "Loaded password: %s", password);
    else
        ESP_LOGI(TAG_MQTT, "%s", esp_err_to_name(err));
    passwordSize = maxPasswordSize;

    err = nvs_get_str(nvsHandle, "ns", ns, &namespaceSize);
    if (err == ESP_OK)
        ESP_LOGI(TAG_MQTT, "Loaded namespace: %s", ns);
    else
        ESP_LOGI(TAG_MQTT, "%s", esp_err_to_name(err));
    namespaceSize = maxNamespaceSize;

    nvs_close(nvsHandle);

    MQTT_resourceRelease();
}

static void eventHandler(void *handlerArgs, esp_event_base_t base, int32_t eventId, void *eventData)
{
    switch ((esp_mqtt_event_id_t)eventId)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG_MQTT, "Connected to broker");
        gpio_set_level(_led, 1);
        connected = true;
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG_MQTT, "Disconnected from broker");
        gpio_set_level(_led, 0);
        connected = false;
        break;
    default:
        break;
    }
}