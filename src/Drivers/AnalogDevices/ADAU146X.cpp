// [CFXS] //
#include <CFXS/HW/Drivers/AnalogDevices/ADAU146X.hpp>
#include <CFXS/HW/Drivers/AnalogDevices/_Def_ADAU146X.hpp>
#include <CFXS/Base/CPU.hpp>
#include <CFXS/Base/Debug.hpp>
#include <_LoggerConfig.hpp>

namespace CFXS::HW {

    using CommandHeader = CommandHeader_ADAU146X;

    static constexpr auto SPI_BITRATE_INITIAL = 1000000;  // 1MHz
    static constexpr auto SPI_BITRATE_NORMAL  = 20000000; // 16MHz

    static bool safeload = false;

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
        TestProgram();

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
        CPU::Delay_ms(1);

        for (int i = 0; i < 3; i++) {
            m_SPI->SetCS(false);
            CPU::Delay_ms(1);
            m_SPI->SetCS(true);
            CPU::Delay_ms(1);
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

        while (count--) {
            m_SPI->Write(0);
            m_SPI->Read(readTo++);
        }

        m_SPI->SetCS(true);
        CPU::Delay_us(32);
    }

#include "IC.h"
#define SIGMA_WRITE_REGISTER_BLOCK
#define SIGMA_WRITE_DELAY
//#include <../../../Dev/SigmaDSP/Test_IC_1.h>
#undef SIGMA_WRITE_REGISTER_BLOCK
#undef SIGMA_WRITE_DELAY

    void ADAU146X::xSIGMA_WRITE_REGISTER_BLOCK(uint8_t chipAddr, uint16_t subAddr, size_t dataLen, uint8_t* data) {
        m_SPI->SetCS(false);
        m_SPI->Write(chipAddr);
        m_SPI->Write(subAddr >> 8);
        m_SPI->Write(subAddr & 0xFF);
        m_SPI->Write(data, dataLen, true);
        m_SPI->SetCS(true);
        CPU::Delay_us(safeload ? 8 : 500);
    }

    void ADAU146X::xSIGMA_WRITE_DELAY(uint8_t chipAddr, size_t dataLen, uint8_t* data) {
        m_SPI->SetCS(false);
        m_SPI->Write(chipAddr);
        m_SPI->Write(data, dataLen, true);
        m_SPI->SetCS(true);
        CPU::Delay_us(safeload ? 8 : 500);
    }

    //void ADAU146X::TestProgram() {
    //    xSIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_SOFT_RESET_IC_1_ADDR, REG_SOFT_RESET_IC_1_BYTE, R0_SOFT_RESET_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_SOFT_RESET_IC_1_ADDR, REG_SOFT_RESET_IC_1_BYTE, R1_SOFT_RESET_IC_1_Default);
    //    xSIGMA_WRITE_DELAY(DEVICE_ADDR_IC_1, R2_RESET_DELAY_IC_1_SIZE, R2_RESET_DELAY_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_HIBERNATE_IC_1_ADDR, REG_HIBERNATE_IC_1_BYTE, R3_HIBERNATE_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_HIBERNATE_IC_1_ADDR, REG_HIBERNATE_IC_1_BYTE, R4_HIBERNATE_IC_1_Default);
    //    xSIGMA_WRITE_DELAY(DEVICE_ADDR_IC_1, R5_HIBERNATE_DELAY_IC_1_SIZE, R5_HIBERNATE_DELAY_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_KILL_CORE_IC_1_ADDR, REG_KILL_CORE_IC_1_BYTE, R6_KILL_CORE_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_KILL_CORE_IC_1_ADDR, REG_KILL_CORE_IC_1_BYTE, R7_KILL_CORE_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_PLL_ENABLE_IC_1_ADDR, REG_PLL_ENABLE_IC_1_BYTE, R8_PLL_ENABLE_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_PLL_CTRL1_IC_1_ADDR, REG_PLL_CTRL1_IC_1_BYTE, R9_PLL_CTRL1_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_PLL_CLK_SRC_IC_1_ADDR, REG_PLL_CLK_SRC_IC_1_BYTE, R10_PLL_CLK_SRC_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_MCLK_OUT_IC_1_ADDR, REG_MCLK_OUT_IC_1_BYTE, R11_MCLK_OUT_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_PLL_ENABLE_IC_1_ADDR, REG_PLL_ENABLE_IC_1_BYTE, R12_PLL_ENABLE_IC_1_Default);
    //    xSIGMA_WRITE_DELAY(DEVICE_ADDR_IC_1, R13_PLL_LOCK_DELAY_IC_1_SIZE, R13_PLL_LOCK_DELAY_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_POWER_ENABLE0_IC_1_ADDR, REG_POWER_ENABLE0_IC_1_BYTE, R14_POWER_ENABLE0_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_POWER_ENABLE1_IC_1_ADDR, REG_POWER_ENABLE1_IC_1_BYTE, R15_POWER_ENABLE1_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SOUT_SOURCE0_IC_1_ADDR, REG_SOUT_SOURCE0_IC_1_BYTE, R16_SOUT_SOURCE0_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SOUT_SOURCE1_IC_1_ADDR, REG_SOUT_SOURCE1_IC_1_BYTE, R17_SOUT_SOURCE1_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SOUT_SOURCE2_IC_1_ADDR, REG_SOUT_SOURCE2_IC_1_BYTE, R18_SOUT_SOURCE2_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SOUT_SOURCE3_IC_1_ADDR, REG_SOUT_SOURCE3_IC_1_BYTE, R19_SOUT_SOURCE3_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SOUT_SOURCE4_IC_1_ADDR, REG_SOUT_SOURCE4_IC_1_BYTE, R20_SOUT_SOURCE4_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SOUT_SOURCE5_IC_1_ADDR, REG_SOUT_SOURCE5_IC_1_BYTE, R21_SOUT_SOURCE5_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SOUT_SOURCE6_IC_1_ADDR, REG_SOUT_SOURCE6_IC_1_BYTE, R22_SOUT_SOURCE6_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SOUT_SOURCE7_IC_1_ADDR, REG_SOUT_SOURCE7_IC_1_BYTE, R23_SOUT_SOURCE7_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SOUT_SOURCE8_IC_1_ADDR, REG_SOUT_SOURCE8_IC_1_BYTE, R24_SOUT_SOURCE8_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SOUT_SOURCE9_IC_1_ADDR, REG_SOUT_SOURCE9_IC_1_BYTE, R25_SOUT_SOURCE9_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SOUT_SOURCE10_IC_1_ADDR, REG_SOUT_SOURCE10_IC_1_BYTE, R26_SOUT_SOURCE10_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SOUT_SOURCE11_IC_1_ADDR, REG_SOUT_SOURCE11_IC_1_BYTE, R27_SOUT_SOURCE11_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SOUT_SOURCE12_IC_1_ADDR, REG_SOUT_SOURCE12_IC_1_BYTE, R28_SOUT_SOURCE12_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SOUT_SOURCE13_IC_1_ADDR, REG_SOUT_SOURCE13_IC_1_BYTE, R29_SOUT_SOURCE13_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SOUT_SOURCE14_IC_1_ADDR, REG_SOUT_SOURCE14_IC_1_BYTE, R30_SOUT_SOURCE14_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SOUT_SOURCE15_IC_1_ADDR, REG_SOUT_SOURCE15_IC_1_BYTE, R31_SOUT_SOURCE15_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SOUT_SOURCE16_IC_1_ADDR, REG_SOUT_SOURCE16_IC_1_BYTE, R32_SOUT_SOURCE16_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SOUT_SOURCE17_IC_1_ADDR, REG_SOUT_SOURCE17_IC_1_BYTE, R33_SOUT_SOURCE17_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SOUT_SOURCE18_IC_1_ADDR, REG_SOUT_SOURCE18_IC_1_BYTE, R34_SOUT_SOURCE18_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SOUT_SOURCE19_IC_1_ADDR, REG_SOUT_SOURCE19_IC_1_BYTE, R35_SOUT_SOURCE19_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SOUT_SOURCE20_IC_1_ADDR, REG_SOUT_SOURCE20_IC_1_BYTE, R36_SOUT_SOURCE20_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SOUT_SOURCE21_IC_1_ADDR, REG_SOUT_SOURCE21_IC_1_BYTE, R37_SOUT_SOURCE21_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SOUT_SOURCE22_IC_1_ADDR, REG_SOUT_SOURCE22_IC_1_BYTE, R38_SOUT_SOURCE22_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SOUT_SOURCE23_IC_1_ADDR, REG_SOUT_SOURCE23_IC_1_BYTE, R39_SOUT_SOURCE23_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SERIAL_BYTE_0_0_IC_1_ADDR, REG_SERIAL_BYTE_0_0_IC_1_BYTE, R40_SERIAL_BYTE_0_0_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SERIAL_BYTE_1_0_IC_1_ADDR, REG_SERIAL_BYTE_1_0_IC_1_BYTE, R41_SERIAL_BYTE_1_0_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SERIAL_BYTE_2_0_IC_1_ADDR, REG_SERIAL_BYTE_2_0_IC_1_BYTE, R42_SERIAL_BYTE_2_0_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SERIAL_BYTE_3_0_IC_1_ADDR, REG_SERIAL_BYTE_3_0_IC_1_BYTE, R43_SERIAL_BYTE_3_0_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SERIAL_BYTE_4_0_IC_1_ADDR, REG_SERIAL_BYTE_4_0_IC_1_BYTE, R44_SERIAL_BYTE_4_0_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SERIAL_BYTE_5_0_IC_1_ADDR, REG_SERIAL_BYTE_5_0_IC_1_BYTE, R45_SERIAL_BYTE_5_0_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SERIAL_BYTE_6_0_IC_1_ADDR, REG_SERIAL_BYTE_6_0_IC_1_BYTE, R46_SERIAL_BYTE_6_0_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_SERIAL_BYTE_7_0_IC_1_ADDR, REG_SERIAL_BYTE_7_0_IC_1_BYTE, R47_SERIAL_BYTE_7_0_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, PROGRAM_ADDR_IC_1, PROGRAM_SIZE_IC_1, Program_Data_IC_1);
    //    xSIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, PARAM_ADDR_IC_1, PARAM_SIZE_IC_1, Param_Data_IC_1);
    //    xSIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, DM1_DATA_ADDR_IC_1, DM1_DATA_SIZE_IC_1, DM1_DATA_Data_IC_1);
    //    xSIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_KILL_CORE_IC_1_ADDR, REG_KILL_CORE_IC_1_BYTE, R51_KILL_CORE_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(
    //        DEVICE_ADDR_IC_1, REG_START_ADDRESS_IC_1_ADDR, REG_START_ADDRESS_IC_1_BYTE, R52_START_ADDRESS_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_START_PULSE_IC_1_ADDR, REG_START_PULSE_IC_1_BYTE, R53_START_PULSE_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_START_CORE_IC_1_ADDR, REG_START_CORE_IC_1_BYTE, R54_START_CORE_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_START_CORE_IC_1_ADDR, REG_START_CORE_IC_1_BYTE, R55_START_CORE_IC_1_Default);
    //    xSIGMA_WRITE_DELAY(DEVICE_ADDR_IC_1, R56_START_DELAY_IC_1_SIZE, R56_START_DELAY_IC_1_Default);
    //    xSIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_HIBERNATE_IC_1_ADDR, REG_HIBERNATE_IC_1_BYTE, R57_HIBERNATE_IC_1_Default);
    //}

} // namespace CFXS::HW