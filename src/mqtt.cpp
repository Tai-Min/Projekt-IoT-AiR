#include "../include/mqtt.hpp"
#include "../include/wifi.hpp"

/**
 * @brief
 */
void MQTT_init();

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

void MQTT_init()
{
    /*esp_mqtt_client_config_t mqtt_cfg = {
        .uri = CONFIG_BROKER_URL,
    };*/
}

void updateIP(const char *ip)
{
}
void updatePort(const char *port)
{
}
void updateUser(const char *usr)
{
}
void updatePassword(const char *passwd)
{
}
void updateNamespace(const char *ns)
{
}