#pragma once

#include <CFXS/HW/Peripherals/GPIO.hpp>

namespace CFXS::HW {

    class ADAU146X {
    public:
        /// Create ADAU146X object with Desc_GPIO descriptors
        ADAU146X(const void* nreset, const void* cs, const void* sclk, const void* mosi, const void* miso);

        void Initialize();

    private:
        void Initialize_SPI();        // Initialize SPI peripheral
        void SetSlavePortModeToSPI(); // Place DSP slave port into SPI mode (default is I2C)
        void TestCommunication();

    private:
        GPIO m_pin_nReset;
        GPIO m_pin_CS;
        GPIO m_pin_SCLK;
        GPIO m_pin_MOSI;
        GPIO m_pin_MISO;

        bool m_Initialized = false;
    };

} // namespace CFXS::HW