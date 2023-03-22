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

#include <CFXS/Base/Debug.hpp>
#include <CFXS/Base/Math.hpp>
#include <CFXS/Platform/CPU.hpp>
#include <CFXS/HW/Peripherals/SPI.hpp>
#include "_Def_ADAU146X.hpp"

namespace CFXS::HW {

    template<typename SPI_INTERFACE, typename RESET_PIN>
    class ADAU146X {
        using CommandHeader = CommandHeader_ADAU146X;

    public:
        using Address_t = uint16_t;
        using Float_t   = CFXS::Math::Float_t;
        using Register  = Regs_ADAU146X;

        static constexpr auto SPI_BITRATE_INITIAL = 1000000;  // 1MHz
        static constexpr auto SPI_BITRATE_NORMAL  = 10000000; // 10MHz

    public:
        constexpr ADAU146X() = default;

        void Initialize() {
            CFXS_ASSERT(m_Initialized == false, "Already initialized");
            // HardwareLogger_Base::Log("ADAU146X[%p] Initialize", this);

            RESET_PIN{}.ConfigureAsOutput();
            RESET_PIN{}.Write(true);

            Initialize_SPI();        // Initialize SPI peripheral
            SetSlavePortModeToSPI(); // Place slave port in SPI mode

            m_Initialized = true;
        }

        void SafeLoad(uint32_t* data, size_t count, uint32_t address, size_t pageIndex) {
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

        void ReadMemory(void* readToPtr, uint32_t address, size_t count) {
            uint8_t* readTo = static_cast<uint8_t*>(readToPtr);
            SPI_INTERFACE{}.SetCS(false);
            SPI_INTERFACE{}.WriteList(0x01, address >> 8, address & 0xFF);
            SPI_INTERFACE{}.WaitForTransferFinished();
            SPI_INTERFACE{}.Clear_RX_FIFO();

            readTo += count;
            while (count--) {
                SPI_INTERFACE{}.Write(0);
                SPI_INTERFACE{}.WaitForTransferFinished();
                SPI_INTERFACE{}.Read(--readTo);
            }

            SPI_INTERFACE{}.SetCS(true);
            // at least 10ns
            asm volatile("nop"); // 1 nop is ~8.33ns @ 120MHz
            asm volatile("nop");
        }

        void ReadRegister(uint16_t* readToPtr, Register reg) {
            uint16_t address = (uint16_t)reg;
            uint8_t* readTo  = reinterpret_cast<uint8_t*>(readToPtr);
            SPI_INTERFACE{}.SetCS(false);
            SPI_INTERFACE{}.WriteList(0x01, address >> 8, address & 0xFF);
            SPI_INTERFACE{}.WaitForTransferFinished();
            SPI_INTERFACE{}.Clear_RX_FIFO();

            SPI_INTERFACE{}.Write(0);
            SPI_INTERFACE{}.WaitForTransferFinished();
            SPI_INTERFACE{}.Read(readTo + 1);
            SPI_INTERFACE{}.Write(0);
            SPI_INTERFACE{}.WaitForTransferFinished();
            SPI_INTERFACE{}.Read(readTo);

            SPI_INTERFACE{}.SetCS(true);
            // at least 10ns
            asm volatile("nop"); // 1 nop is ~8.33ns @ 120MHz
            asm volatile("nop");
        }

        void WriteRegisterBlock(uint16_t addr, size_t dataLen, const void* data, bool safeload = false) {
            SPI_INTERFACE{}.SetCS(false);
            SPI_INTERFACE{}.WriteList(0, addr >> 8, addr & 0xFF);
            SPI_INTERFACE{}.Write((uint8_t*)data, dataLen);
            SPI_INTERFACE{}.WaitForTransferFinished();
            SPI_INTERFACE{}.SetCS(true);
            // at least 10ns
            asm volatile("nop"); // 1 nop is ~8.33ns @ 120MHz
            asm volatile("nop");
            if (!safeload)
                CFXS::CPU::BlockingMilliseconds(1);
        }

        void WriteDelay(size_t dataLen, const void* data) {
            SPI_INTERFACE{}.SetCS(false);
            SPI_INTERFACE{}.Write(0);
            SPI_INTERFACE{}.Write((uint8_t*)data, dataLen);
            SPI_INTERFACE{}.WaitForTransferFinished();
            SPI_INTERFACE{}.SetCS(true);
            // at least 10ns
            asm volatile("nop"); // 1 nop is ~8.33ns @ 120MHz
            asm volatile("nop");
            CFXS::CPU::BlockingMilliseconds(1);
        }

    private:
        void Initialize_SPI() { // Initialize SPI peripheral
            SPI_INTERFACE{}.Initialize();
            SPI_INTERFACE{}.ConfigureAsMaster(SPI::Mode::MODE_3, SPI_BITRATE_NORMAL, 8);
            SPI_INTERFACE{}.Enable();
        }

        // The slave port can be put into SPI mode by performing 3 dummy writes to any subaddress
        // These writes are ignored by the slave port
        void SetSlavePortModeToSPI() {
            // HardwareLogger_Base::Log(" - Set slave port to SPI mode");
            CPU::BlockingMilliseconds(1);
            for (int i = 0; i < 3; i++) {
                SPI_INTERFACE{}.SetCS(false);
                CPU::BlockingMilliseconds(1);
                SPI_INTERFACE{}.SetCS(true);
                CPU::BlockingMilliseconds(1);
            }
        }

    private:
        bool m_Initialized = false;
    };

} // namespace CFXS::HW