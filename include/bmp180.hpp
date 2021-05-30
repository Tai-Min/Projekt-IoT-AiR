#pragma once
#include <cstdint>

class BMP180
{
private:
    // Constant factory calibration settings.
    const int16_t AC1;
    const int16_t AC2;
    const int16_t AC3;
    const uint16_t AC4;
    const uint16_t AC5;
    const uint16_t AC6;
    const int16_t B1;
    const int16_t B2;
    const int16_t MB;
    const int16_t MC;
    const int16_t MD;
    int32_t B5; //!< This one will change with every temperature measurement.

    /**
     * @brief Process reading into true temperature value in °C.
     * @param reading I2C reading.
     * @return Temperature.
     */
    float trueTemperature(int32_t reading);

    /**
     * @brief Process reading into true pressure in hPa.
     * @param reading I2C reading,
     * @param oss Measurement quality bits.
     * @return PRessure.
     */
    float truePressure(int32_t reading, uint8_t oss);

public:
    /**
     * @brief Measurement type for read() function.
     */
    enum class MeasurementType
    {
        LOW_POWER,
        STANDARD,
        HIGH_RES,
        ULTRA_HIGH_RES,
        TEMPERATURE
    };

    /**
     * @brief Class constructor.
     * I2C must be initialized before constructing this object.
     */
    BMP180();

    /**
     * @brief Read some value from the sensor.
     * @param type Type of measurement.
     */
    float read(MeasurementType type);
};