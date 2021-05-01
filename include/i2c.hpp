#pragma once

#include "driver/i2c.h"

/**
 * @brief Initialize I2C peripheral.
 * @param port I2C port.
 * @param sda SDA GPIO.
 * @param scl SCL GPIO.
 * @param freq Frequency.
 */
void I2C_init(i2c_port_t  port, gpio_num_t sda, gpio_num_t scl, uint32_t freq);

/**
 * @brief Write single byte to I2C slave.
 * @param addr Slave address.
 * @param reg Register to write byte to.
 * @param b Byte to write.
 */
void I2C_writeByte(uint8_t addr,  uint8_t reg, uint8_t b);

/**
 * @brief Read single byte from I2C slave.
 * @param addr Slave address.
 * @return Byte from slave.
 */
uint8_t I2C_readbyte(uint8_t addr, uint8_t reg);