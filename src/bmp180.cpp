#include "../include/bmp180.hpp"
#include "../include/i2c.hpp"

namespace
{
    const uint8_t BMP180_ADDR = 0b11101110;

    // Register addresses.
    const uint8_t OUT_MSB = 0xF6;
    const uint8_t AC1_MSB = 0xAA;
    const uint8_t AC2_MSB = 0xAC;
    const uint8_t AC3_MSB = 0xAE;
    const uint8_t AC4_MSB = 0xB0;
    const uint8_t AC5_MSB = 0xB2;
    const uint8_t AC6_MSB = 0xB4;
    const uint8_t B1_MSB = 0xB6;
    const uint8_t B2_MSB = 0xB8;
    const uint8_t MB_MSB = 0xBA;
    const uint8_t MC_MSB = 0xBC;
    const uint8_t MD_MSB = 0xBE;
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
    const double PRESSURE_STEP = 0.01;   //!< Value to multiply with result of measurement to get pressure in hPa.
    const double TEMPERATURE_STEP = 0.1; //!< Value to multiply with result of measurement to get temperature in °C.
}

BMP180::BMP180()
{
    AC1 = I2C_readRegister(BMP180_ADDR, AC1_MSB);
    AC2 = I2C_readRegister(BMP180_ADDR, AC2_MSB);
    AC3 = I2C_readRegister(BMP180_ADDR, AC3_MSB);
    AC4 = I2C_readRegister(BMP180_ADDR, AC4_MSB);
    AC5 = I2C_readRegister(BMP180_ADDR, AC5_MSB);
    AC6 = I2C_readRegister(BMP180_ADDR, AC6_MSB);

    B1 = I2C_readRegister(BMP180_ADDR, B1_MSB);
    B2 = I2C_readRegister(BMP180_ADDR, B2_MSB);

    MB = I2C_readRegister(BMP180_ADDR, MB_MSB);
    MC = I2C_readRegister(BMP180_ADDR, MC_MSB);
    MD = I2C_readRegister(BMP180_ADDR, MD_MSB);
}

float BMP180::calculateTemp(int32_t reading)
{
    int32_t X1 = (reading - AC6) * AC5 / 32768;
    int32_t X2 = MC * 2048 / (X1 + MD);
    B5 = X1 + X2;
    int32_t T = (B5 + 8) / 16;

    return T * TEMPERATURE_STEP;
}

float BMP180::calculatePress(int32_t reading, uint8_t oss)
{
    reading = ((reading << 8) >> (8 - oss));

    int32_t B6 = B5 - 4000;
    int32_t X1 = (B2 * (B6 * B6 / 4096)) / 2048;
    int32_t X2 = AC2 * B6 / 2048;
    int32_t X3 = X1 + X2;
    int32_t B3 = (((AC1 * 4 + X3) << oss) + 2) / 4;
    X1 = AC3 * B6 / 8192;
    X2 = (B1 *(B6 * B6 / 4096)) / 65536;
    X3 = ((X1 + X2) + 2) / 4;
    uint32_t B4 = AC4 * (uint32_t)(X3 + 32768) / 32768;
    uint32_t B7 = ((uint32_t)reading - B3) * (50000 >> oss);

    int32_t p;
    if(B7 < 0x80000000){
        p = (B7 * 2) / B4;
    } else{
        p = (B7 / B4) * 2;
    }

    X1 = (p / 256) * (p / 256);
    X1 = (X1 * 3038) / 65536;
    X2 = (-7357 * p) / 65536;
    p = p + (X1 + X2 + 3791) / 16;

    return p * PRESSURE_STEP;
}

float BMP180::read(MeasurementType type)
{
    uint8_t measurementTypeValue = 0;
    uint8_t oss = 0;
    uint8_t delayTime = 0;

    switch (type)
    {
    case MeasurementType::LOW_POWER:
        measurementTypeValue = CTRL_MEAS_ULTRA_LOW_POWER_MODE_VAL;
        oss = 0;
        delayTime = 10;
        break;
    case MeasurementType::STANDARD:
        measurementTypeValue = CTRL_MEAS_STANDARD_MODE_VAL;
        oss = 0b01;
        delayTime = 20;
        break;
    case MeasurementType::HIGH_RES:
        measurementTypeValue = CTRL_MEAS_HIGH_RESOLUTION_MODE_VAL;
        oss = 0b10;
        delayTime = 20;
        break;
    case MeasurementType::ULTRA_HIGH_RES:
        measurementTypeValue = CTRL_MEAS_ULTRA_HIGH_RESOLUTION_MODE_VAL;
        oss = 0b11;
        delayTime = 30;
        break;
    case MeasurementType::TEMPERATURE:
        measurementTypeValue = CTRL_MEAS_TEMPERATURE_VAL;
        delayTime = 10;
        break;
    }

    I2C_writeByte(BMP180_ADDR, CTRL_MEAS, measurementTypeValue);

    vTaskDelay(delayTime / portTICK_PERIOD_MS);

    int32_t res = I2C_readRegister(BMP180_ADDR, OUT_MSB);

    if (type == MeasurementType::TEMPERATURE)
        return calculateTemp(res);
    return calculatePress(res, oss);
}