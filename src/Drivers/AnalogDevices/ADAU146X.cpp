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
#include <CFXS/Base/Utility.hpp>
#include <_LoggerConfig.hpp>
#include <CFXS/Platform/Task.hpp>
#include <cmath>

namespace CFXS::HW {

    using CommandHeader = CommandHeader_ADAU146X;

    static constexpr auto SPI_BITRATE_INITIAL = 1000000;  // 1MHz
    static constexpr auto SPI_BITRATE_NORMAL  = 10000000; // 10MHz

    ADAU146X::ADAU146X(SPI* spi, const void* gpiodesc_nReset) : m_SPI(spi), m_pin_nReset(gpiodesc_nReset) {
        HardwareLogger_Base::LogConstructor("ADAU146X[%p](%p, %p)", this, spi, gpiodesc_nReset);
    }

    void ADAU146X::Initialize() {
        CFXS_ASSERT(m_Initialized == false, "Already initialized");
        HardwareLogger_Base::Log("ADAU146X[%p] Initialize", this);

        if (m_pin_nReset.GetDescriptor<void*>())
            m_pin_nReset.Initialize(GPIO::PinType::OUTPUT, 0xFFFFFFFF); // Run

        Initialize_SPI();        // Initialize SPI peripheral
        SetSlavePortModeToSPI(); // Place slave port in SPI mode

        m_SPI->Disable();
        m_SPI->ConfigureAsMaster(SPI::Mode::MODE_3, SPI_BITRATE_NORMAL, 8);
        m_SPI->Enable();

        m_Initialized = true;
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
        // safeload = true;
        CFXS_ASSERT(count >= 1 && count <= 5, "Invalid count");
        CFXS_ASSERT(pageIndex <= 1, "Invalid page index");

        // Write data to safeload data registers
        for (int i = 0; i < count; i++) {
            uint32_t dat = __builtin_bswap32(data[i]);
            WriteRegisterBlock(static_cast<uint16_t>(Regs_ADAU146X::DATA_SAFELOAD0) + i, 4, reinterpret_cast<uint8_t*>(&dat), true);
        }

        // Write starting target address to safeload address register
        address = __builtin_bswap32(address);
        WriteRegisterBlock(static_cast<uint16_t>(Regs_ADAU146X::ADDRESS_SAFELOAD), 4, reinterpret_cast<uint8_t*>(&address), true);

        // Write number of words to be transferred to memory page to safeload num registers
        // Second write triggers safeload operation
        count = __builtin_bswap32(count);
        if (pageIndex == 0) {
            WriteRegisterBlock(static_cast<uint16_t>(Regs_ADAU146X::NUM_SAFELOAD_LOWER), 4, reinterpret_cast<uint8_t*>(&count), true);
            count = 0;
            WriteRegisterBlock(static_cast<uint16_t>(Regs_ADAU146X::NUM_SAFELOAD_UPPER), 4, reinterpret_cast<uint8_t*>(&count), true);
        } else {
            WriteRegisterBlock(static_cast<uint16_t>(Regs_ADAU146X::NUM_SAFELOAD_UPPER), 4, reinterpret_cast<uint8_t*>(&count), true);
            count = 0;
            WriteRegisterBlock(static_cast<uint16_t>(Regs_ADAU146X::NUM_SAFELOAD_LOWER), 4, reinterpret_cast<uint8_t*>(&count), true);
        }
    }

    void ADAU146X::ReadMemory(void* readToPtr, uint32_t address, size_t count) {
        uint8_t* readTo = static_cast<uint8_t*>(readToPtr);
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
        // at least 10ns
        asm volatile("nop"); // 1 nop is ~8.33ns @ 120MHz
        asm volatile("nop");
    }

    void ADAU146X::WriteRegisterBlock(uint16_t addr, size_t dataLen, const void* data, bool safeload) {
        m_SPI->SetCS(false);
        m_SPI->Write(0);
        m_SPI->Write(addr >> 8);
        m_SPI->Write(addr & 0xFF);
        m_SPI->Write((uint8_t*)data, dataLen, true);
        m_SPI->SetCS(true);
        // at least 10ns
        asm volatile("nop"); // 1 nop is ~8.33ns @ 120MHz
        asm volatile("nop");
        if (!safeload)
            CFXS::CPU::BlockingMilliseconds(1);
    }

    void ADAU146X::WriteDelay(size_t dataLen, const void* data) {
        m_SPI->SetCS(false);
        m_SPI->Write(0);
        m_SPI->Write((uint8_t*)data, dataLen, true);
        m_SPI->SetCS(true);
        // at least 10ns
        asm volatile("nop"); // 1 nop is ~8.33ns @ 120MHz
        asm volatile("nop");
        CFXS::CPU::BlockingMilliseconds(1);
    }

} // namespace CFXS::HW