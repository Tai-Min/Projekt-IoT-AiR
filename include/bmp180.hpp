#pragma once

class BMP180
{
public:
    enum class MeasurementType{
        LOW_POWER,
        STANDARD,
        HIGH_RES,
        ULTRA_HIGH_RES,
        TEMPERATURE
    };

    /**
     * @brief Read some value from the sensor.
     * I2C must be initialized before call to this function.
     */
    float read(MeasurementType type);
};