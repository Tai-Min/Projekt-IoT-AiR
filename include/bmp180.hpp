#pragma once
#include <cstdint>

class BMP180
{
private:
    int16_t AC1;
    int16_t AC2;
    int16_t AC3;
    uint16_t AC4;
    uint16_t AC5;
    uint16_t AC6;
    int16_t B1;
    int16_t B2;
    int16_t MB;
    int16_t MC;
    int16_t MD;

    int32_t B5;

    float calculateTemp(int32_t reading);
    float calculatePress(int32_t reading, uint8_t oss);

public:
    enum class MeasurementType
    {
        LOW_POWER,
        STANDARD,
        HIGH_RES,
        ULTRA_HIGH_RES,
        TEMPERATURE
    };

    BMP180();

    /**
     * @brief Read some value from the sensor.
     * I2C must be initialized before call to this function.
     */
    float read(MeasurementType type);
};