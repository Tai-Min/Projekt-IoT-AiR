#pragma once

void MQTT_init();
void MQTT_reInit();
void MQTT_updateIP(const char *ip);
void MQTT_updatePort(const char *port);
void MQTT_updateUser(const char *usr);
void MQTT_updatePassword(const char *passwd);
void MQTT_updateNamespace(const char *ns);

const char* MQTT_getIP();
const char* MQTT_getPort();
const char* MQTT_getUser();
const char* MQTT_getPassword();
const char* MQTT_getNamespace();