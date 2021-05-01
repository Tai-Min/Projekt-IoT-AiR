#pragma once

void MQTT_init();
void MQTT_updateIP(const char *ip);
void MQTT_updatePort(const char *port);
void MQTT_updateUser(const char *usr);
void MQTT_updatePassword(const char *passwd);
void MQTT_updateNamespace(const char *ns);