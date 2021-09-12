#pragma once

#include <CFXS/HW/Peripherals/GPIO.hpp>
#include <CFXS/HW/Peripherals/SPI.hpp>

namespace CFXS::HW {

    class ADAU146X {
    public:
        /// Create ADAU146X object with Desc_GPIO descriptors
        /// \param spi SPI peripheral
        /// \param gpiodesc_nReset GPIO descriptor for NRESET pin - nullptr if reset pin not used
        ADAU146X(SPI* spi, const void* gpiodesc_nReset = nullptr);

        void Initialize();

        void ExecuteSafeLoad(uint32_t* data, size_t count, uint32_t address, size_t pageIndex);
        void ReadWord(uint8_t* readTo, uint32_t address, size_t count);

    private:
        void Initialize_SPI();        // Initialize SPI peripheral
        void SetSlavePortModeToSPI(); // Place DSP slave port into SPI mode (default is I2C)
        void TestProgram();

        void SIGMA_WRITE_REGISTER_BLOCK(uint8_t chipAddr, uint16_t subAddr, size_t dataLen, uint8_t* data);
        void SIGMA_WRITE_DELAY(uint8_t chipAddr, size_t dataLen, uint8_t* data);

    private:
        SPI* m_SPI;
        GPIO m_pin_nReset;

        bool m_Initialized = false;
    };

} // namespace CFXS::HW