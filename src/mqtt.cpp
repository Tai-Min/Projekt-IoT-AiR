#include "../include/mqtt.hpp"
#include "../include/wifi.hpp"
#include "esp_log.h"
#include "nvs_flash.h"

static const char *TAG_MQTT = "MQTT";

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

/**
 * @brief
 */
void MQTT_init();

/**
 * @brief
 */
void MQTT_reInit();

/**
 * @brief
 * @param ip Ip to set.
 */
void MQTT_updateIP(const char *ip);

/**
 * @brief
 * @param port Port to set.
 */
void MQTT_updatePort(const char *port);

/**
 * @brief
 * @param usr Username to set.
 */
void MQTT_updateUser(const char *usr);

/**
 * @brief
 * @param passwd Password to set.
 */
void MQTT_updatePassword(const char *passwd);

/**
 * @brief
 * @param ns Namespace to set.
 */
void MQTT_updateNamespace(const char *ns);

/**
 * @brief
 * @return
 */
const char *MQTT_getIP();

/**
 * @brief
 * @return
 */
const char *MQTT_getPort();

/**
 * @brief
 * @return
 */
const char *MQTT_getUser();

/**
 * @brief
 * @return
 */
const char *MQTT_getPassword();

/**
 * @brief
 * @return
 */
const char *MQTT_getNamespace();

static void loadFromFlash();

void MQTT_init()
{
    loadFromFlash();
}

void MQTT_reInit()
{
    MQTT_init();
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

static void loadFromFlash()
{
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
}