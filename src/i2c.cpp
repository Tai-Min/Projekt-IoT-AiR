#include "../include/i2c.hpp"

static i2c_port_t _port;

void I2C_init(i2c_port_t port, gpio_num_t sda, gpio_num_t scl, uint32_t freq)
{
    _port = port;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda,
        .scl_io_num = scl,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = {
            .clk_speed = freq}};

    i2c_param_config(_port, &conf);
    i2c_driver_install(_port, I2C_MODE_MASTER, 0, 0, 0);
}

void I2C_writeByte(uint8_t addr, uint8_t reg, uint8_t b)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));

    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, addr, true));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, reg, true));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, b, true));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));

    ESP_ERROR_CHECK(i2c_master_cmd_begin(_port, cmd, 1000 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);
}

uint16_t I2C_readRegister(uint8_t addr, uint8_t reg)
{
    uint8_t resMsb = 0;
    uint8_t resLsb = 0;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, addr, true));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, reg, true));

    ESP_ERROR_CHECK(i2c_master_start(cmd)); // Repeated start.
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, addr | 1, true));
    ESP_ERROR_CHECK(i2c_master_read_byte(cmd, &resMsb, I2C_MASTER_ACK));
    ESP_ERROR_CHECK(i2c_master_read_byte(cmd, &resLsb, I2C_MASTER_LAST_NACK));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));

    ESP_ERROR_CHECK(i2c_master_cmd_begin(_port, cmd, 1000 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);

    uint16_t res = (((uint16_t)resMsb << 8) | resLsb);

    return res;
}