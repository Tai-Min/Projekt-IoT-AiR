#include "../include/bmp180.hpp"
#include "../include/i2c.hpp"
#include <cstdint>

namespace
{
    const uint8_t BMP180_READ_ADDR = 0b11101110;
    const uint8_t BMP180_WRITE_ADDR = (BMP180_READ_ADDR & 1);

    // Register addresses.
    const uint8_t OUT_XLSB = 0xF8;
    const uint8_t OUT_LSB = 0xF7;
    const uint8_t OUT_MSB = 0xF6;
    const uint8_t CTRL_MEAS = 0xF4;
    const uint8_t SOFT_RESET = 0xE0;
    const uint8_t ID = 0xD0;

    // Some register values.
    const uint8_t CTRL_MEAS_TEMPERATURE_VAL = 0x2E;                //!< Set in CTRL_MEAS to measure temperature.
    const uint8_t CTRL_MEAS_ULTRA_LOW_POWER_MODE_VAL = 0x34;       //!< Set in CTRL_MEAS to measure in ultra low power mode (1 sample, 4.5ms conversion duration)
    const uint8_t CTRL_MEAS_STANDARD_MODE_VAL = 0x74;              //!< Set in CTRL_MEAS to measure in standard mode (2 samples, 7.5ms conversion duration)
    const uint8_t CTRL_MEAS_HIGH_RESOLUTION_MODE_VAL = 0xB4;       //!< Set in CTRL_MEAS to measure in high resolution mode (4 samples, 13.5ms conversion duration)
    const uint8_t CTRL_MEAS_ULTRA_HIGH_RESOLUTION_MODE_VAL = 0xF4; //!< Set in CTRL_MEAS to measure in ultra high resolution mode (8 samples, 25.5ms conversion duration)
    const uint8_t SOFT_RESET_VAL = 0xB6;                           //!< Set in SOFT_RESET register to reset the device.
    const uint8_t ID_VAL = 0x55;                                   //!< Constant chip ID value in ID register.

    // Other values.
    const double PRESSURE_STEP = 0.01;  //!< Value to multiply with result of measurement to get pressure in hPa.
    const double TEMPERATURE_STEP = 0.1; //!< Value to multiply with result of measurement to get temperature in °C.
}

float BMP180::read(MeasurementType type)
{
    uint8_t measurementTypeValue = 0;
    uint8_t delayTime = 0;
    switch (type)
    {
    case MeasurementType::LOW_POWER:
        measurementTypeValue = CTRL_MEAS_ULTRA_LOW_POWER_MODE_VAL;
        delayTime = 5;
        break;
    case MeasurementType::STANDARD:
        measurementTypeValue = CTRL_MEAS_STANDARD_MODE_VAL;
        delayTime = 8;
        break;
    case MeasurementType::HIGH_RES:
        measurementTypeValue = CTRL_MEAS_HIGH_RESOLUTION_MODE_VAL;
        delayTime = 15;
        break;
    case MeasurementType::ULTRA_HIGH_RES:
        measurementTypeValue = CTRL_MEAS_ULTRA_HIGH_RESOLUTION_MODE_VAL;
        delayTime = 26;
        break;
    case MeasurementType::TEMPERATURE:
        measurementTypeValue = CTRL_MEAS_TEMPERATURE_VAL;
        delayTime = 5;
        break;
    }

    //I2C_writeByte(BMP180_WRITE_ADDR, CTRL_MEAS, measurementTypeValue);

    vTaskDelay(delayTime / portTICK_PERIOD_MS);

    //uint16_t res = (uint16_t)(((uint16_t)I2C_readbyte(BMP180_READ_ADDR, OUT_MSB) << 8) || (uint16_t)I2C_readbyte(BMP180_READ_ADDR, OUT_LSB));
    uint16_t res = 0;
    
    if (type == MeasurementType::TEMPERATURE)
        return res * TEMPERATURE_STEP;
    return res * PRESSURE_STEP;
}