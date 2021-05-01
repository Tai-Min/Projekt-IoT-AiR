#pragma once

/**
 * @brief Init MQTT client. 
 * Must be able to take resources using MQTT_resourceTake().
 */
void MQTT_init();

/**
 * @brief Reinit MQTT client. Should be called after MQTT_updateX functions.
 * Must be able to take resources using MQTT_resourceTake().
 */
void MQTT_reInit();

/**
 * @brief Take ownership of MQTT's resources.
 * Use this before any MQTT_getX().
 */
void MQTT_resourceTake();

/**
 * @brief Release ownership of MQTT's resources.
 * Use this after finished using all of the MQTT_getX().
 */
void MQTT_resourceRelease();

/**
 * @brief Publish float to MQTT broker.
 * @param topic Topic to publish to.
 * @param data Data to publish.
 * @param qos QoS.
 */
void MQTT_publish(const char* topic, float data, int qos);

/**
 * @brief Update IP in flash.
 * @param ip IP to set.
 */
void MQTT_updateIP(const char *ip);

/**
 * @brief Update port in flas.
 * @param port Port to set.
 */
void MQTT_updatePort(const char *port);

/**
 * @brief Update username in flash.
 * @param usr Username to set.
 */
void MQTT_updateUser(const char *usr);

/**
 * @brief Update password in flash.
 * @param passwd Password to set.
 */
void MQTT_updatePassword(const char *passwd);

/**
 * @brief Update namespace in flash.
 * @param ns Namespace to set.
 */
void MQTT_updateNamespace(const char *ns);

/**
 * @brief Get currently set IP.
 * @return IP.
 */
const char* MQTT_getIP();

/**
 * @brief Get currently set port.
 * @return Port.
 */
const char* MQTT_getPort();

/**
 * @brief Get currently set username.
 * @return Username.
 */
const char* MQTT_getUser();

/**
 * @brief Get currently set password.
 * @return Password.
 */
const char* MQTT_getPassword();

/**
 * @brief Get currently set namespace.
 * @return Namespace.
 */
const char* MQTT_getNamespace();