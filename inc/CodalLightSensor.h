#pragma once

#include "CodalComponent.h"
#include "I2C.h"
#include "SPI.h"

namespace codal
{

    enum ColorFormat
    {
        RGB,
        BGR,
        RGBD,
        BGRD,
        W,
        RGBW,
        BGRW,
        RGBWI //RGB + White + Infrared  , stored as RGBDW
    };

    struct ColorData
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t d; // Optional brightness/depth
        uint8_t w; // Optional Lux/Raw ADC/IR/UV index
    };
    /**
     * @class CodalLightSensor
     * @brief A flexible light sensor interface supporting SPI and I2C communication.
     *        Supports multiple color formats including RGB, BGR, RGBD, and BGRD.
     */
    class CodalLightSensor : public CodalComponent
    {
    private:
        I2C *i2c;           // Pointer to I2C bus (if used)
        SPI *spi;           // Pointer to SPI bus (if used)
        uint8_t address;    // I2C address of the sensor
        ColorFormat format; // Current color format
        bool useSPI;        // True if using SPI, false if using I2C
        uint8_t dummybyte;  // Dummy byte used for SPI reads
    public:
        /**
         * @brief Constructor for I2C-based light sensor.
         * @param i2cBus Reference to I2C bus
         * @param addr I2C address of the sensor
         * @param fmt Desired color format (default: RGBD)
         */

        CodalLightSensor(I2C &i2cBus, uint8_t addr, ColorFormat fmt = RGBD)
            : i2c(&i2cBus), spi(nullptr), address(addr), format(fmt), useSPI(false), dummyByte(0) {}
        /**
         * @brief Constructor for SPI-based light sensor.
         * @param spiBus Reference to SPI bus
         * @param fmt Desired color format (default: RGBD)
         */
        CodalLightSensor(SPI &spiBus, ColorFormat fmt = RGBD)
            : i2c(nullptr), spi(&spiBus), address(0), format(fmt), useSPI(true) {}
        /**
         * @brief Sets the color format used when reading sensor data.
         * @param fmt New color format
         */
        void setFormat(ColorFormat fmt)
        {
            format = fmt;
        }
        /**
         * @brief Sets the dummy byte used for SPI reads.
         * @param dummyByte Byte to send before reading from SPI sensor
         * @return DEVICE_OK if successful, DEVICE_NOT_SUPPORTED if using I2C
         */
        int setDummyByte(uint8_t dummyByte)
        {
            if (this->useSPI)
            {
                this->dummybyte = dummyByte;
                return DEVICE_OK;
            }
            else
            {
                return DEVICE_NOT_SUPPORTED; // I2C does not need this.
            }
        }
        /**
         * @brief Reads color data from the sensor and maps it to the selected format.
         *        Supports RGB, BGR, RGBD, and BGRD formats. If the format is invalid or unsupported,
         *        the method returns DEVICE_PERIPHERAL_ERROR.
         *
         * @param out Reference to ColorData struct to store the result.
         * @return DEVICE_OK if successful, DEVICE_PERIPHERAL_ERROR if format is unknown.
         */
        int read(ColorData &out)
        {
            uint8_t buffer[5] = {0};

            if (useSPI)
            {
                spi->write(this->dummybyte); // Dummy command
                spi->read(buffer, sizeof(buffer));
            }
            else
            {
                i2c->read(address, buffer, sizeof(buffer));
            }

            switch (format)
            {
            case RGB:
                out.r = buffer[0];
                out.g = buffer[1];
                out.b = buffer[2];
                out.d = 0;
                out.w = 0;
                break;
            case BGR:
                out.b = buffer[0];
                out.g = buffer[1];
                out.r = buffer[2];
                out.d = 0;
                out.w = 0;
                break;
            case RGBD:
                out.r = buffer[0];
                out.g = buffer[1];
                out.b = buffer[2];
                out.d = buffer[3];
                out.w = 0;
                break;
            case BGRD:
                out.b = buffer[0];
                out.g = buffer[1];
                out.r = buffer[2];
                out.d = buffer[3];
                out.w = 0;
                break;
            case W:
                out.r = 0;
                out.g = 0;
                out.b = 0;
                out.d = 0;
                out.w = buffer[0];
                break;
            case RGBW:
                out.r = buffer[0];
                out.g = buffer[1];
                out.b = buffer[2];
                out.d = 0;
                out.w = 0;
                out.w = buffer[3];
                break;
            case RGBWI:
                out.r = buffer[0];
                out.g = buffer[1];
                out.b = buffer[2];
                out.d = buffer[3];
                out.w = buffer[4];
                break;
            case BGRW:
                out.b = buffer[0];
                out.g = buffer[1];
                out.r = buffer[2];
                out.d = 0;
                out.w = 0;
                out.w = buffer[3];
                break;
            default:
                return DEVICE_PERIPHERAL_ERROR;
            }

            return DEVICE_OK;
        }
    };

} // namespace codal

typedef CODAL_LIGHTSENSOR codal::CodalLightSensor;
