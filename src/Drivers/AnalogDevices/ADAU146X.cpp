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

float level_dB[5];
float peak_level_dB[5];
bool threshold_hit[5];

#include "IC.h"
#include "SigmaStudioFW.h"
#define SIGMA_WRITE_REGISTER_BLOCK g_DSP->xSIGMA_WRITE_REGISTER_BLOCK
#define SIGMA_WRITE_DELAY          g_DSP->xSIGMA_WRITE_DELAY
#include <X:\CFXS\CFXS-ARM-Framework-Dev\Dev\SigmaDSP\ha_IC_1.h>
#include <X:\CFXS\CFXS-ARM-Framework-Dev\Dev\SigmaDSP\ha_IC_1_PARAM.h>

float fixed_to_float(uint32_t input, uint8_t fractional_bits) {
    return ((float)input / (float)(1 << fractional_bits));
}

#define FIXED_BIT 24
bool s_ResetMaxLevels = true;

unsigned short int float2fix(float n) {
    unsigned short int int_part = 0, frac_part = 0;
    int i;
    float t;

    int_part = (int)floor(n) << FIXED_BIT;
    n -= (int)floor(n);

    t = 0.5;
    for (i = 0; i < FIXED_BIT; i++) {
        if ((n - t) >= 0) {
            n -= t;
            frac_part += (1 << (FIXED_BIT - 1 - i));
        }
        t = t / 2;
    }

    return int_part + frac_part;
}

namespace CFXS::HW {

    using CommandHeader = CommandHeader_ADAU146X;

    static constexpr auto SPI_BITRATE_INITIAL = 1000000;  // 1MHz
    static constexpr auto SPI_BITRATE_NORMAL  = 10000000; // 10MHz

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
                g_DSP->ReadWord((uint8_t*)&w, MOD_SIGNALDETECT_1_LEVEL_1_ALG0_SINGLEBANDLEVELDETS3001X_ADDR, 4);
                level_dB[0] = 10 * log10f(fixed_to_float(w, 23));
                g_DSP->ReadWord((uint8_t*)&w, MOD_SIGNALDETECT_2_LEVEL_2_ALG0_SINGLEBANDLEVELDETS3002X_ADDR, 4);
                level_dB[1] = 10 * log10f(fixed_to_float(w, 23));
                g_DSP->ReadWord((uint8_t*)&w, MOD_SIGNALDETECT_3_LEVEL_3_ALG0_SINGLEBANDLEVELDETS3003X_ADDR, 4);
                level_dB[2] = 10 * log10f(fixed_to_float(w, 23));
                g_DSP->ReadWord((uint8_t*)&w, MOD_SIGNALDETECT_4_LEVEL_4_ALG0_SINGLEBANDLEVELDETS3004X_ADDR, 4);
                level_dB[3] = 10 * log10f(fixed_to_float(w, 23));
                g_DSP->ReadWord((uint8_t*)&w, MOD_SIGNALDETECT_5_LEVEL_5_ALG0_SINGLEBANDLEVELDETS3005X_ADDR, 4);
                level_dB[4] = 10 * log10f(fixed_to_float(w, 23));
            },
            10);
        t->Start();

        auto tx = Task::Create(
            LOW_PRIORITY,
            "Clip Detect",
            [](auto...) {
                uint32_t w;
                if (s_ResetMaxLevels) {
                    s_ResetMaxLevels = false;

                    w = 0x00000000;
                    g_DSP->SafeLoad(&w, 1, MOD_SIGNALDETECT_1_PEAK_HOLD_1_ISON_ADDR, 0);
                    g_DSP->SafeLoad(&w, 1, MOD_SIGNALDETECT_2_PEAK_HOLD_2_ISON_ADDR, 0);
                    g_DSP->SafeLoad(&w, 1, MOD_SIGNALDETECT_3_PEAK_HOLD_3_ISON_ADDR, 0);
                    g_DSP->SafeLoad(&w, 1, MOD_SIGNALDETECT_4_PEAK_HOLD_4_ISON_ADDR, 0);
                    g_DSP->SafeLoad(&w, 1, MOD_SIGNALDETECT_5_PEAK_HOLD_5_ISON_ADDR, 0);
                    w = 0x00000001;
                    g_DSP->SafeLoad(&w, 1, MOD_SIGNALDETECT_1_PEAK_HOLD_1_ISON_ADDR, 0);
                    g_DSP->SafeLoad(&w, 1, MOD_SIGNALDETECT_2_PEAK_HOLD_2_ISON_ADDR, 0);
                    g_DSP->SafeLoad(&w, 1, MOD_SIGNALDETECT_3_PEAK_HOLD_3_ISON_ADDR, 0);
                    g_DSP->SafeLoad(&w, 1, MOD_SIGNALDETECT_4_PEAK_HOLD_4_ISON_ADDR, 0);
                    g_DSP->SafeLoad(&w, 1, MOD_SIGNALDETECT_5_PEAK_HOLD_5_ISON_ADDR, 0);
                } else {
                    // g_DSP->ReadWord((uint8_t*)&w, MOD_SIGNALDETECT_1_THRESHOLD_HIT_1_READBACKALGNEWSIGMA3001VALUE_ADDR, 4);
                    // threshold_hit[0] = !w;
                    // g_DSP->ReadWord((uint8_t*)&w, MOD_SIGNALDETECT_2_THRESHOLD_HIT_2_READBACKALGNEWSIGMA3003VALUE_ADDR, 4);
                    // threshold_hit[1] = !w;
                    // g_DSP->ReadWord((uint8_t*)&w, MOD_SIGNALDETECT_3_THRESHOLD_HIT_3_READBACKALGNEWSIGMA3005VALUE_ADDR, 4);
                    // threshold_hit[2] = !w;
                    // g_DSP->ReadWord((uint8_t*)&w, MOD_SIGNALDETECT_4_THRESHOLD_HIT_4_READBACKALGNEWSIGMA3007VALUE_ADDR, 4);
                    // threshold_hit[3] = !w;
                    // g_DSP->ReadWord((uint8_t*)&w, MOD_SIGNALDETECT_5_THRESHOLD_HIT_5_READBACKALGNEWSIGMA3009VALUE_ADDR, 4);
                    // threshold_hit[4] = !w;
                }
            },
            5);
        tx->Start();

        auto tx2 = Task::Create(
            LOW_PRIORITY,
            "Peak dB Detect",
            [](auto...) {
                uint32_t w;
                g_DSP->ReadWord((uint8_t*)&w, MOD_SIGNALDETECT_1_PEAK_1_READBACKALGNEWSIGMA3002VALUE_ADDR, 4);
                peak_level_dB[0] = 20 * log10f(fixed_to_float(w, 24));
                g_DSP->ReadWord((uint8_t*)&w, MOD_SIGNALDETECT_2_PEAK_2_READBACKALGNEWSIGMA3004VALUE_ADDR, 4);
                peak_level_dB[1] = 20 * log10f(fixed_to_float(w, 24));
                g_DSP->ReadWord((uint8_t*)&w, MOD_SIGNALDETECT_3_PEAK_3_READBACKALGNEWSIGMA3006VALUE_ADDR, 4);
                peak_level_dB[2] = 20 * log10f(fixed_to_float(w, 24));
                g_DSP->ReadWord((uint8_t*)&w, MOD_SIGNALDETECT_4_PEAK_4_READBACKALGNEWSIGMA3008VALUE_ADDR, 4);
                peak_level_dB[3] = 20 * log10f(fixed_to_float(w, 24));
                g_DSP->ReadWord((uint8_t*)&w, MOD_SIGNALDETECT_5_PEAK_5_READBACKALGNEWSIGMA30010VALUE_ADDR, 4);
                peak_level_dB[4] = 20 * log10f(fixed_to_float(w, 24));
            },
            100);
        tx2->Start();
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
            xSIGMA_WRITE_REGISTER_BLOCK(
                0, static_cast<uint16_t>(Regs_ADAU146X::DATA_SAFELOAD0) + i, 4, reinterpret_cast<uint8_t*>(&dat), true);
        }

        // Write starting target address to safeload address register
        address = __builtin_bswap32(address);
        xSIGMA_WRITE_REGISTER_BLOCK(
            0, static_cast<uint16_t>(Regs_ADAU146X::ADDRESS_SAFELOAD), 4, reinterpret_cast<uint8_t*>(&address), true);

        // Write number of words to be transferred to memory page to safeload num registers
        // Second write triggers safeload operation
        count = __builtin_bswap32(count);
        if (pageIndex == 0) {
            xSIGMA_WRITE_REGISTER_BLOCK(
                0, static_cast<uint16_t>(Regs_ADAU146X::NUM_SAFELOAD_LOWER), 4, reinterpret_cast<uint8_t*>(&count), true);
            count = 0;
            xSIGMA_WRITE_REGISTER_BLOCK(
                0, static_cast<uint16_t>(Regs_ADAU146X::NUM_SAFELOAD_UPPER), 4, reinterpret_cast<uint8_t*>(&count), true);
        } else {
            xSIGMA_WRITE_REGISTER_BLOCK(
                0, static_cast<uint16_t>(Regs_ADAU146X::NUM_SAFELOAD_UPPER), 4, reinterpret_cast<uint8_t*>(&count), true);
            count = 0;
            xSIGMA_WRITE_REGISTER_BLOCK(
                0, static_cast<uint16_t>(Regs_ADAU146X::NUM_SAFELOAD_LOWER), 4, reinterpret_cast<uint8_t*>(&count), true);
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
        // at least 10ns
        asm volatile("nop"); // 1 nop is ~8.33ns @ 120MHz
        asm volatile("nop");
    }

    void ADAU146X::xSIGMA_WRITE_REGISTER_BLOCK(uint8_t chipAddr, uint16_t subAddr, size_t dataLen, const void* data, bool safeload) {
        m_SPI->SetCS(false);
        m_SPI->Write(chipAddr);
        m_SPI->Write(subAddr >> 8);
        m_SPI->Write(subAddr & 0xFF);
        m_SPI->Write((uint8_t*)data, dataLen, true);
        m_SPI->SetCS(true);
        // at least 10ns
        asm volatile("nop"); // 1 nop is ~8.33ns @ 120MHz
        asm volatile("nop");
        if (!safeload)
            CFXS::CPU::BlockingMilliseconds(1);
    }

    void ADAU146X::xSIGMA_WRITE_DELAY(uint8_t chipAddr, size_t dataLen, const void* data) {
        m_SPI->SetCS(false);
        m_SPI->Write(chipAddr);
        m_SPI->Write((uint8_t*)data, dataLen, true);
        m_SPI->SetCS(true);
        // at least 10ns
        asm volatile("nop"); // 1 nop is ~8.33ns @ 120MHz
        asm volatile("nop");
        CFXS::CPU::BlockingMilliseconds(1);
    }

    void ADAU146X::TestProgram() {
        default_download_IC_1();
    }

} // namespace CFXS::HW