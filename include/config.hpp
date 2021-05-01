#pragma once

#define SMART_CONFIG_BUTTON_PIN (gpio_num_t)19
#define SMART_CONFIG_LED_PIN (gpio_num_t)13

#define WIFI_LED_PIN (gpio_num_t)12

#define HTTP_BUTTON_PIN (gpio_num_t)23
#define HTTP_LED_PIN (gpio_num_t)27

#define MQTT_LED_PIN (gpio_num_t)14

#define DHT11_DATA_PIN (gpio_num_t)18

#define I2C_PORT I2C_NUM_0
#define I2C_SDA (gpio_num_t)21
#define I2C_SCL (gpio_num_t)22
#define I2C_FREQ 100000