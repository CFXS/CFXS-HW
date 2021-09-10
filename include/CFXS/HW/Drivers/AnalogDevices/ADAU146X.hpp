#pragma once

#include <CFXS/HW/Peripherals/GPIO.hpp>

namespace CFXS::HW {

    class ADAU146X {
    public:
        /// Create ADAU146X object with Desc_GPIO descriptors
        ADAU146X(const void* nreset, const void* cs, const void* sclk, const void* mosi, const void* miso);

        void Initialize();

        void WriteReset(bool state) { m_pin_nReset.Write(state); }

    private:
        GPIO m_pin_nReset;
        GPIO m_pin_CS;
        GPIO m_pin_SCLK;
        GPIO m_pin_MOSI;
        GPIO m_pin_MISO;
    };

} // namespace CFXS::HW