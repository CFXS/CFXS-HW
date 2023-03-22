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

namespace CFXS::HW {

    class SPI {
    public:
        enum class Mode {
            MODE_0,
            MODE_1,
            MODE_2,
            MODE_3,
            PHASE_0_POLARITY_0 = MODE_0,
            PHASE_0_POLARITY_1 = MODE_1,
            PHASE_1_POLARITY_0 = MODE_2,
            PHASE_1_POLARITY_1 = MODE_3
        };

    public:
        template<typename SPI_DESC_T, typename GPIO_DESC_T>
        constexpr SPI(SPI_DESC_T* descriptor     = nullptr,
                      GPIO_DESC_T* gpiodesc_CS   = nullptr,
                      GPIO_DESC_T* gpiodesc_CLK  = nullptr,
                      GPIO_DESC_T* gpiodesc_MOSI = nullptr,
                      GPIO_DESC_T* gpiodesc_MISO = nullptr) :
            m_Descriptor((const void*)descriptor),
            m_pin_CS(gpiodesc_CS),
            m_gpiodesc_CLK(gpiodesc_CLK),
            m_gpiodesc_MOSI(gpiodesc_MOSI),
            m_gpiodesc_MISO(gpiodesc_MISO) {
        }

        /// Initialize SPI
        void Initialize();

        /// Configure SPI peripheral for master operation
        void ConfigureAsMaster(Mode mode, size_t bitrate, size_t dataWidth);

        /// Enable SPI peripheral
        void Enable();

        /// Disable SPI peripheral
        void Disable();

        /// Write single data unit
        void Write(size_t data, bool waitUntilTransmitted = true);

        /// Write multiple data units
        void Write(uint8_t* data, size_t count, bool waitUntilTransmitted = true);
        void Write(uint16_t* data, size_t count, bool waitUntilTransmitted = true);

        /// Read single data unit
        void Read(uint8_t* readTo);

        /// Is SPI busy
        bool IsBusy();

        /// Clear RX FIFO
        void Clear_RX_FIFO();

        /// Is TX FIFO empty
        bool Is_TX_FIFO_Empty();

        /// Is TX FIFO full
        bool Is_TX_FIFO_Full();

        /// Is RX FIFO empty
        bool Is_RX_FIFO_Empty();

        /// Is RX FIFO full
        bool Is_RX_FIFO_Full();

        /// Get size of FIFO
        uint32_t Get_FIFO_Size() const;

        /// Set CS state
        void SetCS(bool state);

        /// Set descriptor
        template<typename T>
        void SetDescriptor(T* descriptor = nullptr) {
            m_Descriptor = (void*)descriptor;
        }

        /// Get descriptor as type T*
        template<typename T>
        const T* GetDescriptor() const {
            return static_cast<const T*>(m_Descriptor);
        }

    private:
        const void* m_Descriptor;
        GPIO m_pin_CS;
        const void* m_gpiodesc_CLK;
        const void* m_gpiodesc_MOSI;
        const void* m_gpiodesc_MISO;
    };

} // namespace CFXS::HW