// ---------------------------------------------------------------------
// CFXS Framework Hardware Module <https://github.com/CFXS/CFXS-Hardware>
// Copyright (C) 2022 | CFXS / Rihards Veips
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>
// ---------------------------------------------------------------------
// [CFXS] //
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

        void SafeLoad(uint32_t* data, size_t count, uint32_t address, size_t pageIndex);
        void ReadWord(uint8_t* readTo, uint32_t address, size_t count);

    private:
        void Initialize_SPI();        // Initialize SPI peripheral
        void SetSlavePortModeToSPI(); // Place DSP slave port into SPI mode (default is I2C)
        void TestProgram();

        void xSIGMA_WRITE_REGISTER_BLOCK(uint8_t chipAddr, uint16_t subAddr, size_t dataLen, uint8_t* data);
        void xSIGMA_WRITE_DELAY(uint8_t chipAddr, size_t dataLen, uint8_t* data);

    private:
        SPI* m_SPI;
        GPIO m_pin_nReset;

        bool m_Initialized = false;
    };

} // namespace CFXS::HW