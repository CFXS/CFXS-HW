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
#include <CFXS/HW/Drivers/AnalogDevices/ADAU146X.hpp>
#include <CFXS/HW/Drivers/AnalogDevices/_Def_ADAU146X.hpp>
#include <CFXS/Platform/CPU.hpp>
#include <CFXS/Base/Debug.hpp>
#include <_LoggerConfig.hpp>
#include <CFXS/Platform/Task.hpp>
#include <cmath>

CFXS::HW::ADAU146X* g_DSP;

float level_dB;

#include "IC.h"
#include "SigmaStudioFW.h"
#define SIGMA_WRITE_REGISTER_BLOCK g_DSP->xSIGMA_WRITE_REGISTER_BLOCK
#define SIGMA_WRITE_DELAY          g_DSP->xSIGMA_WRITE_DELAY
#include <X:\CFXS\CFXS-ARM-Framework-Dev\Dev\SigmaDSP\ha_IC_1.h>
#include <X:\CFXS\CFXS-ARM-Framework-Dev\Dev\SigmaDSP\ha_IC_1_PARAM.h>

float fixed_to_float(uint32_t input, uint8_t fractional_bits) {
    return ((float)input / (float)(1 << fractional_bits));
}

namespace CFXS::HW {

    using CommandHeader = CommandHeader_ADAU146X;

    static constexpr auto SPI_BITRATE_INITIAL = 1000000;  // 1MHz
    static constexpr auto SPI_BITRATE_NORMAL  = 20000000; // 16MHz

    static bool safeload = false;

    ADAU146X::ADAU146X(SPI* spi, const void* gpiodesc_nReset) : m_SPI(spi), m_pin_nReset(gpiodesc_nReset) {
        HardwareLogger_Base::LogConstructor("ADAU146X[%p](%p, %p)", this, spi, gpiodesc_nReset);
    }

    void ADAU146X::Initialize() {
        g_DSP = this;
        CFXS_ASSERT(m_Initialized == false, "Already initialized");
        HardwareLogger_Base::Log("ADAU146X[%p] Initialize", this);

        if (m_pin_nReset.GetDescriptor<void*>())
            m_pin_nReset.Initialize(GPIO::PinType::OUTPUT, 0xFFFFFFFF); // Run

        Initialize_SPI();        // Initialize SPI peripheral
        SetSlavePortModeToSPI(); // Place slave port in SPI mode
        TestProgram();

        m_SPI->Disable();
        m_SPI->ConfigureAsMaster(SPI::Mode::MODE_3, SPI_BITRATE_NORMAL, 8);
        m_SPI->Enable();

        m_Initialized = true;

        auto t = Task::Create(
            LOW_PRIORITY,
            "dB",
            [](auto...) {
                uint32_t w;
                g_DSP->ReadWord((uint8_t*)&w, MOD_LEVEL1_ALG0_SINGLEBANDLEVELDETS3001X_ADDR, 4);
                level_dB = 10 * log10f(fixed_to_float(w, 23));
                // CFXS_printf("%.1f\n", level_dB);
            },
            10);
        t->Start();
    }

    void ADAU146X::Initialize_SPI() {
        HardwareLogger_Base::Log(" - Initialize SPI");
        m_SPI->Initialize();
        m_SPI->Disable();
        m_SPI->ConfigureAsMaster(SPI::Mode::MODE_3, SPI_BITRATE_INITIAL, 8);
        m_SPI->Enable();
    }

    // The slave port can be put into SPI mode by performing 3 dummy writes to any subaddress
    // These writes are ignored by the slave port
    void ADAU146X::SetSlavePortModeToSPI() {
        HardwareLogger_Base::Log(" - Set slave port to SPI mode");
        CPU::BlockingMilliseconds(1);

        for (int i = 0; i < 3; i++) {
            m_SPI->SetCS(false);
            CPU::BlockingMilliseconds(1);
            m_SPI->SetCS(true);
            CPU::BlockingMilliseconds(1);
        }
    }

    void ADAU146X::SafeLoad(uint32_t* data, size_t count, uint32_t address, size_t pageIndex) {
        safeload = true;
        CFXS_ASSERT(count >= 1 && count <= 5, "Invalid count");
        CFXS_ASSERT(pageIndex <= 1, "Invalid page index");

        // Write data to safeload data registers
        for (int i = 0; i < count; i++) {
            uint32_t dat = __builtin_bswap32(data[i]);
            xSIGMA_WRITE_REGISTER_BLOCK(0, static_cast<uint16_t>(Regs_ADAU146X::DATA_SAFELOAD0) + i, 4, reinterpret_cast<uint8_t*>(&dat));
        }

        // Write starting target address to safeload address register
        address = __builtin_bswap32(address);
        xSIGMA_WRITE_REGISTER_BLOCK(0, static_cast<uint16_t>(Regs_ADAU146X::ADDRESS_SAFELOAD), 4, reinterpret_cast<uint8_t*>(&address));

        // Write number of words to be transferred to memory page to safeload num registers
        // Second write triggers safeload operation
        count = __builtin_bswap32(count);
        if (pageIndex == 0) {
            xSIGMA_WRITE_REGISTER_BLOCK(0, static_cast<uint16_t>(Regs_ADAU146X::NUM_SAFELOAD_LOWER), 4, reinterpret_cast<uint8_t*>(&count));
            count = 0;
            xSIGMA_WRITE_REGISTER_BLOCK(0, static_cast<uint16_t>(Regs_ADAU146X::NUM_SAFELOAD_UPPER), 4, reinterpret_cast<uint8_t*>(&count));
        } else {
            xSIGMA_WRITE_REGISTER_BLOCK(0, static_cast<uint16_t>(Regs_ADAU146X::NUM_SAFELOAD_UPPER), 4, reinterpret_cast<uint8_t*>(&count));
            count = 0;
            xSIGMA_WRITE_REGISTER_BLOCK(0, static_cast<uint16_t>(Regs_ADAU146X::NUM_SAFELOAD_LOWER), 4, reinterpret_cast<uint8_t*>(&count));
        }
    }

    void ADAU146X::ReadWord(uint8_t* readTo, uint32_t address, size_t count) {
        m_SPI->SetCS(false);
        m_SPI->Write(0x01);
        m_SPI->Write(address >> 8);
        m_SPI->Write(address & 0xFF);

        m_SPI->Clear_RX_FIFO();

        readTo += count;
        while (count--) {
            m_SPI->Write(0);
            m_SPI->Read(--readTo);
        }

        m_SPI->SetCS(true);
        CPU::BlockingMicroseconds(32);
    }

    void ADAU146X::xSIGMA_WRITE_REGISTER_BLOCK(uint8_t chipAddr, uint16_t subAddr, size_t dataLen, uint8_t* data) {
        m_SPI->SetCS(false);
        m_SPI->Write(chipAddr);
        m_SPI->Write(subAddr >> 8);
        m_SPI->Write(subAddr & 0xFF);
        m_SPI->Write(data, dataLen, true);
        m_SPI->SetCS(true);
        CPU::BlockingMicroseconds(safeload ? 8 : 500);
    }

    void ADAU146X::xSIGMA_WRITE_DELAY(uint8_t chipAddr, size_t dataLen, uint8_t* data) {
        m_SPI->SetCS(false);
        m_SPI->Write(chipAddr);
        m_SPI->Write(data, dataLen, true);
        m_SPI->SetCS(true);
        CPU::BlockingMicroseconds(safeload ? 8 : 500);
    }

    void ADAU146X::TestProgram() {
        default_download_IC_1();
    }

} // namespace CFXS::HW