#include <CFXS/HW/Drivers/AnalogDevices/ADAU146X.hpp>
#include <CFXS/HW/Drivers/AnalogDevices/_Def_ADAU146X.hpp>
#include <CFXS/Base/CPU.hpp>
#include <CFXS/Base/Debug.hpp>
#include <_LoggerConfig.hpp>

namespace CFXS::HW {

    using CommandHeader = CommandHeader_ADAU146X;

    static constexpr auto SPI_BITRATE_INITIAL = 1000000;  // 1MHz
    static constexpr auto SPI_BITRATE_NORMAL  = 20000000; // 16MHz

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

    void ADAU146X::ExecuteSafeLoad(uint32_t* data, size_t count, uint32_t address, size_t pageIndex) {
        CFXS_ASSERT(count >= 1 && count <= 5, "Invalid count");
        CFXS_ASSERT(pageIndex <= 1, "Invalid page index");

        // Write data to safeload data registers
        for (int i = 0; i < count; i++) {
            uint32_t dat = __builtin_bswap32(data[i]);
            SIGMA_WRITE_REGISTER_BLOCK(0, static_cast<uint16_t>(Regs_ADAU146X::DATA_SAFELOAD0) + i, 4, reinterpret_cast<uint8_t*>(&dat));
        }

        // Write starting target address to safeload address register
        address = __builtin_bswap32(address);
        SIGMA_WRITE_REGISTER_BLOCK(0, static_cast<uint16_t>(Regs_ADAU146X::ADDRESS_SAFELOAD), 4, reinterpret_cast<uint8_t*>(&address));

        // Write number of words to be transferred to memory page to safeload num registers
        // Second write triggers safeload operation
        count = __builtin_bswap32(count);
        if (pageIndex == 0) {
            SIGMA_WRITE_REGISTER_BLOCK(0, static_cast<uint16_t>(Regs_ADAU146X::NUM_SAFELOAD_LOWER), 4, reinterpret_cast<uint8_t*>(&count));
            count = 0;
            SIGMA_WRITE_REGISTER_BLOCK(0, static_cast<uint16_t>(Regs_ADAU146X::NUM_SAFELOAD_UPPER), 4, reinterpret_cast<uint8_t*>(&count));
        } else {
            SIGMA_WRITE_REGISTER_BLOCK(0, static_cast<uint16_t>(Regs_ADAU146X::NUM_SAFELOAD_UPPER), 4, reinterpret_cast<uint8_t*>(&count));
            count = 0;
            SIGMA_WRITE_REGISTER_BLOCK(0, static_cast<uint16_t>(Regs_ADAU146X::NUM_SAFELOAD_LOWER), 4, reinterpret_cast<uint8_t*>(&count));
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
        CPU::Delay_us(500);
    }

#include "IC.h"

#define ADI_REG_TYPE uint8_t

#define DEVICE_ARCHITECTURE_IC_1 "ADAU1466"
#define DEVICE_ADDR_IC_1         0x0

/* DSP Ram Data */
#define DM1_DATA_SIZE_IC_1 80
#define DM1_DATA_ADDR_IC_1 24576
    ADI_REG_TYPE DM1_DATA_Data_IC_1[DM1_DATA_SIZE_IC_1] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

/* DSP Program Data */
#define PROGRAM_SIZE_IC_1 524
#define PROGRAM_ADDR_IC_1 49152
    ADI_REG_TYPE Program_Data_IC_1[PROGRAM_SIZE_IC_1] = {
        0x00, 0x00, 0x00, 0x02, 0x0C, 0x00, 0xDC, 0xDC, 0x0D, 0x00, 0xFF, 0xD2, 0x0D, 0x00, 0xFF, 0xD0, 0x0D, 0x00, 0xF4, 0x50, 0x0D, 0x00,
        0xF4, 0x00, 0xC0, 0x00, 0x23, 0x80, 0x80, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x23, 0x00, 0x80, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x23, 0xC0,
        0x80, 0x00, 0x00, 0x10, 0xC0, 0x00, 0x22, 0x00, 0x80, 0x00, 0x00, 0x00, 0x08, 0x80, 0xDC, 0xE0, 0x0C, 0x00, 0xDC, 0xDC, 0x0A, 0x21,
        0x00, 0x12, 0x08, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x14, 0x06, 0x40, 0x50, 0x10, 0xC0, 0x00, 0x20, 0x00, 0x80, 0x00, 0x00, 0x27,
        0x0D, 0x00, 0xFF, 0xC1, 0xC0, 0x00, 0x20, 0x00, 0x80, 0x00, 0x00, 0x01, 0x0D, 0x00, 0xF4, 0x62, 0xC0, 0x00, 0x20, 0x00, 0x80, 0x00,
        0x00, 0x00, 0x0D, 0x00, 0xF4, 0x62, 0xC0, 0x00, 0x20, 0x00, 0x80, 0x00, 0x00, 0x02, 0x0D, 0x00, 0xFF, 0xD1, 0x00, 0x00, 0x00, 0x03,
        0x0D, 0x00, 0xFF, 0xD2, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0x00, 0x22, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x9E, 0xDC, 0xE0, 0x08, 0x9C, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00,
        0x82, 0x4F, 0x00, 0x00, 0x08, 0x8A, 0xDC, 0xE0, 0x0A, 0x23, 0x00, 0x06, 0x0A, 0x2B, 0x00, 0x07, 0x00, 0x88, 0x20, 0x00, 0x0A, 0xA7,
        0x00, 0x05, 0x00, 0x00, 0x00, 0x22, 0xC0, 0x00, 0x2D, 0x30, 0x80, 0x00, 0x50, 0x00, 0x30, 0x00, 0xB4, 0x6A, 0x00, 0x88, 0x20, 0x00,
        0xC0, 0x00, 0x28, 0x70, 0x80, 0x00, 0x60, 0x00, 0x02, 0x09, 0x00, 0x47, 0x00, 0x8C, 0x11, 0x91, 0x00, 0x00, 0x00, 0x24, 0x00, 0x8C,
        0x2A, 0x02, 0x30, 0x00, 0xBC, 0xE9, 0x00, 0x84, 0x0D, 0x9C, 0x00, 0x20, 0x0B, 0x2D, 0x00, 0x00, 0xC2, 0xF1, 0x01, 0x00, 0x00, 0x45,
        0x06, 0x00, 0x50, 0x14, 0x36, 0x40, 0x50, 0x15, 0x26, 0x40, 0x40, 0x15, 0x09, 0x2B, 0x00, 0x06, 0x09, 0x2B, 0x00, 0x07, 0x00, 0x00,
        0x00, 0x00, 0x0C, 0x00, 0xFC, 0x41, 0x06, 0x40, 0x10, 0x0E, 0x08, 0x90, 0x00, 0x1B, 0x0C, 0x70, 0x00, 0x14, 0x0E, 0x3D, 0x00, 0x17,
        0x00, 0xF4, 0xAE, 0x0F, 0x0A, 0x25, 0xDC, 0xDC, 0x0A, 0xE1, 0x00, 0x15, 0x00, 0x00, 0xDC, 0x70, 0x00, 0x00, 0xA4, 0xD8, 0x00, 0xD6,
        0x21, 0x01, 0x00, 0x00, 0xE0, 0x29, 0x07, 0x05, 0x88, 0x08, 0x00, 0x84, 0x19, 0xB3, 0x00, 0x04, 0x45, 0x90, 0x00, 0x44, 0x2C, 0x30,
        0x00, 0x04, 0x49, 0x28, 0x00, 0x94, 0x08, 0x87, 0x0B, 0xE1, 0x00, 0x15, 0x05, 0x4D, 0x10, 0x1E, 0x0C, 0x10, 0x00, 0x1E, 0x06, 0x10,
        0x10, 0x0E, 0x06, 0x10, 0x90, 0x1E, 0xE0, 0x20, 0x80, 0x01, 0x80, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x05, 0x4C, 0x10, 0x2E,
        0x0C, 0x40, 0x00, 0x2E, 0x06, 0x00, 0x10, 0x2E, 0x00, 0x04, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x05, 0x4C, 0x10, 0x3E, 0x06, 0x00,
        0x10, 0x3E, 0x06, 0x40, 0x10, 0x4E, 0x0D, 0x00, 0x00, 0x2C, 0x00, 0x04, 0x00, 0x00, 0x0A, 0xE1, 0x00, 0x23, 0x0C, 0x40, 0x00, 0x20,
        0x00, 0x1C, 0x80, 0x40, 0x00, 0x84, 0x08, 0x81, 0x00, 0x8C, 0x18, 0x89, 0x0B, 0xE1, 0x00, 0x23, 0x0B, 0xA1, 0x00, 0x2A, 0x00, 0xD7,
        0xED, 0x08, 0x0B, 0xB5, 0x00, 0x2B, 0x06, 0x00, 0x10, 0x4E, 0x0D, 0x00, 0xFC, 0xC1, 0x08, 0x9C, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x2E, 0x01, 0x00, 0x00, 0x00, 0x00, 0x09, 0x9C, 0x00, 0x10, 0x00, 0x00, 0x00, 0x02, 0x0C, 0x10, 0xF4, 0x60, 0x02, 0xC2,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

/* DSP Parameter (Coefficient) Data */
#define PARAM_SIZE_IC_1 204
#define PARAM_ADDR_IC_1 0
    ADI_REG_TYPE Param_Data_IC_1[PARAM_SIZE_IC_1] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x02, 0x8F, 0x5C, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x9D, 0x61, 0x00, 0x00, 0x01, 0xE0, 0x00, 0x00, 0x0E, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

    /* Register Default - IC 1.SOFT_RESET */
    ADI_REG_TYPE R0_SOFT_RESET_IC_1_Default[REG_SOFT_RESET_IC_1_BYTE] = {0x00, 0x00};

    /* Register Default - IC 1.SOFT_RESET */
    ADI_REG_TYPE R1_SOFT_RESET_IC_1_Default[REG_SOFT_RESET_IC_1_BYTE] = {0x00, 0x01};

/* Register Default - IC 1.Reset Delay */
#define R2_RESET_DELAY_IC_1_ADDR 0x0
#define R2_RESET_DELAY_IC_1_SIZE 2
    ADI_REG_TYPE R2_RESET_DELAY_IC_1_Default[R2_RESET_DELAY_IC_1_SIZE] = {0x00, 0xFF};

    /* Register Default - IC 1.HIBERNATE */
    ADI_REG_TYPE R3_HIBERNATE_IC_1_Default[REG_HIBERNATE_IC_1_BYTE] = {0x00, 0x00};

    /* Register Default - IC 1.HIBERNATE */
    ADI_REG_TYPE R4_HIBERNATE_IC_1_Default[REG_HIBERNATE_IC_1_BYTE] = {0x00, 0x01};

/* Register Default - IC 1.Hibernate Delay */
#define R5_HIBERNATE_DELAY_IC_1_ADDR 0x0
#define R5_HIBERNATE_DELAY_IC_1_SIZE 2
    ADI_REG_TYPE R5_HIBERNATE_DELAY_IC_1_Default[R5_HIBERNATE_DELAY_IC_1_SIZE] = {0x00, 0xFF};

    /* Register Default - IC 1.KILL_CORE */
    ADI_REG_TYPE R6_KILL_CORE_IC_1_Default[REG_KILL_CORE_IC_1_BYTE] = {0x00, 0x00};

    /* Register Default - IC 1.KILL_CORE */
    ADI_REG_TYPE R7_KILL_CORE_IC_1_Default[REG_KILL_CORE_IC_1_BYTE] = {0x00, 0x01};

    /* Register Default - IC 1.PLL_ENABLE */
    ADI_REG_TYPE R8_PLL_ENABLE_IC_1_Default[REG_PLL_ENABLE_IC_1_BYTE] = {0x00, 0x00};

    /* Register Default - IC 1.PLL_CTRL1 Register */
    ADI_REG_TYPE R9_PLL_CTRL1_IC_1_Default[REG_PLL_CTRL1_IC_1_BYTE] = {0x00, 0x02};

    /* Register Default - IC 1.PLL_CLK_SRC Register */
    ADI_REG_TYPE R10_PLL_CLK_SRC_IC_1_Default[REG_PLL_CLK_SRC_IC_1_BYTE] = {0x00, 0x01};

    /* Register Default - IC 1.MCLK_OUT Register */
    ADI_REG_TYPE R11_MCLK_OUT_IC_1_Default[REG_MCLK_OUT_IC_1_BYTE] = {0x00, 0x05};

    /* Register Default - IC 1.PLL_ENABLE Register */
    ADI_REG_TYPE R12_PLL_ENABLE_IC_1_Default[REG_PLL_ENABLE_IC_1_BYTE] = {0x00, 0x01};

/* Register Default - IC 1.PLL Lock Delay */
#define R13_PLL_LOCK_DELAY_IC_1_ADDR 0x0
#define R13_PLL_LOCK_DELAY_IC_1_SIZE 2
    ADI_REG_TYPE R13_PLL_LOCK_DELAY_IC_1_Default[R13_PLL_LOCK_DELAY_IC_1_SIZE] = {0x00, 0xFF};

    /* Register Default - IC 1.POWER_ENABLE0 Register */
    ADI_REG_TYPE R14_POWER_ENABLE0_IC_1_Default[REG_POWER_ENABLE0_IC_1_BYTE] = {0x1F, 0xFF};

    /* Register Default - IC 1.POWER_ENABLE1 Register */
    ADI_REG_TYPE R15_POWER_ENABLE1_IC_1_Default[REG_POWER_ENABLE1_IC_1_BYTE] = {0x00, 0x1F};

    /* Register Default - IC 1.SOUT_SOURCE0 */
    ADI_REG_TYPE R16_SOUT_SOURCE0_IC_1_Default[REG_SOUT_SOURCE0_IC_1_BYTE] = {0x00, 0x02};

    /* Register Default - IC 1.SOUT_SOURCE1 */
    ADI_REG_TYPE R17_SOUT_SOURCE1_IC_1_Default[REG_SOUT_SOURCE1_IC_1_BYTE] = {0x00, 0x02};

    /* Register Default - IC 1.SOUT_SOURCE2 */
    ADI_REG_TYPE R18_SOUT_SOURCE2_IC_1_Default[REG_SOUT_SOURCE2_IC_1_BYTE] = {0x00, 0x02};

    /* Register Default - IC 1.SOUT_SOURCE3 */
    ADI_REG_TYPE R19_SOUT_SOURCE3_IC_1_Default[REG_SOUT_SOURCE3_IC_1_BYTE] = {0x00, 0x02};

    /* Register Default - IC 1.SOUT_SOURCE4 */
    ADI_REG_TYPE R20_SOUT_SOURCE4_IC_1_Default[REG_SOUT_SOURCE4_IC_1_BYTE] = {0x00, 0x02};

    /* Register Default - IC 1.SOUT_SOURCE5 */
    ADI_REG_TYPE R21_SOUT_SOURCE5_IC_1_Default[REG_SOUT_SOURCE5_IC_1_BYTE] = {0x00, 0x02};

    /* Register Default - IC 1.SOUT_SOURCE6 */
    ADI_REG_TYPE R22_SOUT_SOURCE6_IC_1_Default[REG_SOUT_SOURCE6_IC_1_BYTE] = {0x00, 0x02};

    /* Register Default - IC 1.SOUT_SOURCE7 */
    ADI_REG_TYPE R23_SOUT_SOURCE7_IC_1_Default[REG_SOUT_SOURCE7_IC_1_BYTE] = {0x00, 0x02};

    /* Register Default - IC 1.SOUT_SOURCE8 */
    ADI_REG_TYPE R24_SOUT_SOURCE8_IC_1_Default[REG_SOUT_SOURCE8_IC_1_BYTE] = {0x00, 0x02};

    /* Register Default - IC 1.SOUT_SOURCE9 */
    ADI_REG_TYPE R25_SOUT_SOURCE9_IC_1_Default[REG_SOUT_SOURCE9_IC_1_BYTE] = {0x00, 0x02};

    /* Register Default - IC 1.SOUT_SOURCE10 */
    ADI_REG_TYPE R26_SOUT_SOURCE10_IC_1_Default[REG_SOUT_SOURCE10_IC_1_BYTE] = {0x00, 0x02};

    /* Register Default - IC 1.SOUT_SOURCE11 */
    ADI_REG_TYPE R27_SOUT_SOURCE11_IC_1_Default[REG_SOUT_SOURCE11_IC_1_BYTE] = {0x00, 0x02};

    /* Register Default - IC 1.SOUT_SOURCE12 */
    ADI_REG_TYPE R28_SOUT_SOURCE12_IC_1_Default[REG_SOUT_SOURCE12_IC_1_BYTE] = {0x00, 0x02};

    /* Register Default - IC 1.SOUT_SOURCE13 */
    ADI_REG_TYPE R29_SOUT_SOURCE13_IC_1_Default[REG_SOUT_SOURCE13_IC_1_BYTE] = {0x00, 0x02};

    /* Register Default - IC 1.SOUT_SOURCE14 */
    ADI_REG_TYPE R30_SOUT_SOURCE14_IC_1_Default[REG_SOUT_SOURCE14_IC_1_BYTE] = {0x00, 0x02};

    /* Register Default - IC 1.SOUT_SOURCE15 */
    ADI_REG_TYPE R31_SOUT_SOURCE15_IC_1_Default[REG_SOUT_SOURCE15_IC_1_BYTE] = {0x00, 0x02};

    /* Register Default - IC 1.SOUT_SOURCE16 */
    ADI_REG_TYPE R32_SOUT_SOURCE16_IC_1_Default[REG_SOUT_SOURCE16_IC_1_BYTE] = {0x00, 0x02};

    /* Register Default - IC 1.SOUT_SOURCE17 */
    ADI_REG_TYPE R33_SOUT_SOURCE17_IC_1_Default[REG_SOUT_SOURCE17_IC_1_BYTE] = {0x00, 0x02};

    /* Register Default - IC 1.SOUT_SOURCE18 */
    ADI_REG_TYPE R34_SOUT_SOURCE18_IC_1_Default[REG_SOUT_SOURCE18_IC_1_BYTE] = {0x00, 0x02};

    /* Register Default - IC 1.SOUT_SOURCE19 */
    ADI_REG_TYPE R35_SOUT_SOURCE19_IC_1_Default[REG_SOUT_SOURCE19_IC_1_BYTE] = {0x00, 0x02};

    /* Register Default - IC 1.SOUT_SOURCE20 */
    ADI_REG_TYPE R36_SOUT_SOURCE20_IC_1_Default[REG_SOUT_SOURCE20_IC_1_BYTE] = {0x00, 0x02};

    /* Register Default - IC 1.SOUT_SOURCE21 */
    ADI_REG_TYPE R37_SOUT_SOURCE21_IC_1_Default[REG_SOUT_SOURCE21_IC_1_BYTE] = {0x00, 0x02};

    /* Register Default - IC 1.SOUT_SOURCE22 */
    ADI_REG_TYPE R38_SOUT_SOURCE22_IC_1_Default[REG_SOUT_SOURCE22_IC_1_BYTE] = {0x00, 0x02};

    /* Register Default - IC 1.SOUT_SOURCE23 */
    ADI_REG_TYPE R39_SOUT_SOURCE23_IC_1_Default[REG_SOUT_SOURCE23_IC_1_BYTE] = {0x00, 0x02};

    /* Register Default - IC 1.SERIAL_BYTE_0_0 */
    ADI_REG_TYPE R40_SERIAL_BYTE_0_0_IC_1_Default[REG_SERIAL_BYTE_0_0_IC_1_BYTE] = {0x90, 0x00};

    /* Register Default - IC 1.SERIAL_BYTE_1_0 */
    ADI_REG_TYPE R41_SERIAL_BYTE_1_0_IC_1_Default[REG_SERIAL_BYTE_1_0_IC_1_BYTE] = {0x90, 0x00};

    /* Register Default - IC 1.SERIAL_BYTE_2_0 */
    ADI_REG_TYPE R42_SERIAL_BYTE_2_0_IC_1_Default[REG_SERIAL_BYTE_2_0_IC_1_BYTE] = {0x90, 0x00};

    /* Register Default - IC 1.SERIAL_BYTE_3_0 */
    ADI_REG_TYPE R43_SERIAL_BYTE_3_0_IC_1_Default[REG_SERIAL_BYTE_3_0_IC_1_BYTE] = {0x90, 0x00};

    /* Register Default - IC 1.SERIAL_BYTE_4_0 */
    ADI_REG_TYPE R44_SERIAL_BYTE_4_0_IC_1_Default[REG_SERIAL_BYTE_4_0_IC_1_BYTE] = {0x90, 0x00};

    /* Register Default - IC 1.SERIAL_BYTE_5_0 */
    ADI_REG_TYPE R45_SERIAL_BYTE_5_0_IC_1_Default[REG_SERIAL_BYTE_5_0_IC_1_BYTE] = {0x90, 0x00};

    /* Register Default - IC 1.SERIAL_BYTE_6_0 */
    ADI_REG_TYPE R46_SERIAL_BYTE_6_0_IC_1_Default[REG_SERIAL_BYTE_6_0_IC_1_BYTE] = {0x90, 0x00};

    /* Register Default - IC 1.SERIAL_BYTE_7_0 */
    ADI_REG_TYPE R47_SERIAL_BYTE_7_0_IC_1_Default[REG_SERIAL_BYTE_7_0_IC_1_BYTE] = {0x90, 0x00};

    /* Register Default - IC 1.KILL_CORE */
    ADI_REG_TYPE R51_KILL_CORE_IC_1_Default[REG_KILL_CORE_IC_1_BYTE] = {0x00, 0x00};

    /* Register Default - IC 1.START_ADDRESS */
    ADI_REG_TYPE R52_START_ADDRESS_IC_1_Default[REG_START_ADDRESS_IC_1_BYTE] = {0x00, 0x00};

    /* Register Default - IC 1.START_PULSE */
    ADI_REG_TYPE R53_START_PULSE_IC_1_Default[REG_START_PULSE_IC_1_BYTE] = {0x00, 0x02};

    /* Register Default - IC 1.START_CORE */
    ADI_REG_TYPE R54_START_CORE_IC_1_Default[REG_START_CORE_IC_1_BYTE] = {0x00, 0x00};

    /* Register Default - IC 1.START_CORE */
    ADI_REG_TYPE R55_START_CORE_IC_1_Default[REG_START_CORE_IC_1_BYTE] = {0x00, 0x01};

/* Register Default - IC 1.Start Delay */
#define R56_START_DELAY_IC_1_ADDR 0x0
#define R56_START_DELAY_IC_1_SIZE 2
    ADI_REG_TYPE R56_START_DELAY_IC_1_Default[R56_START_DELAY_IC_1_SIZE] = {0x00, 0x01};

    /* Register Default - IC 1.HIBERNATE */
    ADI_REG_TYPE R57_HIBERNATE_IC_1_Default[REG_HIBERNATE_IC_1_BYTE] = {0x00, 0x00};

/*
 * Default Download
 */
#define DEFAULT_DOWNLOAD_SIZE_IC_1 58

    void ADAU146X::SIGMA_WRITE_REGISTER_BLOCK(uint8_t chipAddr, uint16_t subAddr, size_t dataLen, uint8_t* data) {
        m_SPI->SetCS(false);
        m_SPI->Write(chipAddr);
        m_SPI->Write(subAddr >> 8);
        m_SPI->Write(subAddr & 0xFF);
        m_SPI->Write(data, dataLen, true);
        m_SPI->SetCS(true);
        CPU::Delay_us(500);
    }

    void ADAU146X::SIGMA_WRITE_DELAY(uint8_t chipAddr, size_t dataLen, uint8_t* data) {
        m_SPI->SetCS(false);
        m_SPI->Write(chipAddr);
        m_SPI->Write(data, dataLen, true);
        m_SPI->SetCS(true);
        CPU::Delay_us(500);
    }

    void ADAU146X::TestProgram() {
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_SOFT_RESET_IC_1_ADDR, REG_SOFT_RESET_IC_1_BYTE, R0_SOFT_RESET_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_SOFT_RESET_IC_1_ADDR, REG_SOFT_RESET_IC_1_BYTE, R1_SOFT_RESET_IC_1_Default);
        SIGMA_WRITE_DELAY(DEVICE_ADDR_IC_1, R2_RESET_DELAY_IC_1_SIZE, R2_RESET_DELAY_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_HIBERNATE_IC_1_ADDR, REG_HIBERNATE_IC_1_BYTE, R3_HIBERNATE_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_HIBERNATE_IC_1_ADDR, REG_HIBERNATE_IC_1_BYTE, R4_HIBERNATE_IC_1_Default);
        SIGMA_WRITE_DELAY(DEVICE_ADDR_IC_1, R5_HIBERNATE_DELAY_IC_1_SIZE, R5_HIBERNATE_DELAY_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_KILL_CORE_IC_1_ADDR, REG_KILL_CORE_IC_1_BYTE, R6_KILL_CORE_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_KILL_CORE_IC_1_ADDR, REG_KILL_CORE_IC_1_BYTE, R7_KILL_CORE_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_PLL_ENABLE_IC_1_ADDR, REG_PLL_ENABLE_IC_1_BYTE, R8_PLL_ENABLE_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_PLL_CTRL1_IC_1_ADDR, REG_PLL_CTRL1_IC_1_BYTE, R9_PLL_CTRL1_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_PLL_CLK_SRC_IC_1_ADDR, REG_PLL_CLK_SRC_IC_1_BYTE, R10_PLL_CLK_SRC_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_MCLK_OUT_IC_1_ADDR, REG_MCLK_OUT_IC_1_BYTE, R11_MCLK_OUT_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_PLL_ENABLE_IC_1_ADDR, REG_PLL_ENABLE_IC_1_BYTE, R12_PLL_ENABLE_IC_1_Default);
        SIGMA_WRITE_DELAY(DEVICE_ADDR_IC_1, R13_PLL_LOCK_DELAY_IC_1_SIZE, R13_PLL_LOCK_DELAY_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(
            DEVICE_ADDR_IC_1, REG_POWER_ENABLE0_IC_1_ADDR, REG_POWER_ENABLE0_IC_1_BYTE, R14_POWER_ENABLE0_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(
            DEVICE_ADDR_IC_1, REG_POWER_ENABLE1_IC_1_ADDR, REG_POWER_ENABLE1_IC_1_BYTE, R15_POWER_ENABLE1_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_SOUT_SOURCE0_IC_1_ADDR, REG_SOUT_SOURCE0_IC_1_BYTE, R16_SOUT_SOURCE0_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_SOUT_SOURCE1_IC_1_ADDR, REG_SOUT_SOURCE1_IC_1_BYTE, R17_SOUT_SOURCE1_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_SOUT_SOURCE2_IC_1_ADDR, REG_SOUT_SOURCE2_IC_1_BYTE, R18_SOUT_SOURCE2_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_SOUT_SOURCE3_IC_1_ADDR, REG_SOUT_SOURCE3_IC_1_BYTE, R19_SOUT_SOURCE3_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_SOUT_SOURCE4_IC_1_ADDR, REG_SOUT_SOURCE4_IC_1_BYTE, R20_SOUT_SOURCE4_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_SOUT_SOURCE5_IC_1_ADDR, REG_SOUT_SOURCE5_IC_1_BYTE, R21_SOUT_SOURCE5_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_SOUT_SOURCE6_IC_1_ADDR, REG_SOUT_SOURCE6_IC_1_BYTE, R22_SOUT_SOURCE6_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_SOUT_SOURCE7_IC_1_ADDR, REG_SOUT_SOURCE7_IC_1_BYTE, R23_SOUT_SOURCE7_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_SOUT_SOURCE8_IC_1_ADDR, REG_SOUT_SOURCE8_IC_1_BYTE, R24_SOUT_SOURCE8_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_SOUT_SOURCE9_IC_1_ADDR, REG_SOUT_SOURCE9_IC_1_BYTE, R25_SOUT_SOURCE9_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(
            DEVICE_ADDR_IC_1, REG_SOUT_SOURCE10_IC_1_ADDR, REG_SOUT_SOURCE10_IC_1_BYTE, R26_SOUT_SOURCE10_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(
            DEVICE_ADDR_IC_1, REG_SOUT_SOURCE11_IC_1_ADDR, REG_SOUT_SOURCE11_IC_1_BYTE, R27_SOUT_SOURCE11_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(
            DEVICE_ADDR_IC_1, REG_SOUT_SOURCE12_IC_1_ADDR, REG_SOUT_SOURCE12_IC_1_BYTE, R28_SOUT_SOURCE12_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(
            DEVICE_ADDR_IC_1, REG_SOUT_SOURCE13_IC_1_ADDR, REG_SOUT_SOURCE13_IC_1_BYTE, R29_SOUT_SOURCE13_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(
            DEVICE_ADDR_IC_1, REG_SOUT_SOURCE14_IC_1_ADDR, REG_SOUT_SOURCE14_IC_1_BYTE, R30_SOUT_SOURCE14_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(
            DEVICE_ADDR_IC_1, REG_SOUT_SOURCE15_IC_1_ADDR, REG_SOUT_SOURCE15_IC_1_BYTE, R31_SOUT_SOURCE15_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(
            DEVICE_ADDR_IC_1, REG_SOUT_SOURCE16_IC_1_ADDR, REG_SOUT_SOURCE16_IC_1_BYTE, R32_SOUT_SOURCE16_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(
            DEVICE_ADDR_IC_1, REG_SOUT_SOURCE17_IC_1_ADDR, REG_SOUT_SOURCE17_IC_1_BYTE, R33_SOUT_SOURCE17_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(
            DEVICE_ADDR_IC_1, REG_SOUT_SOURCE18_IC_1_ADDR, REG_SOUT_SOURCE18_IC_1_BYTE, R34_SOUT_SOURCE18_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(
            DEVICE_ADDR_IC_1, REG_SOUT_SOURCE19_IC_1_ADDR, REG_SOUT_SOURCE19_IC_1_BYTE, R35_SOUT_SOURCE19_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(
            DEVICE_ADDR_IC_1, REG_SOUT_SOURCE20_IC_1_ADDR, REG_SOUT_SOURCE20_IC_1_BYTE, R36_SOUT_SOURCE20_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(
            DEVICE_ADDR_IC_1, REG_SOUT_SOURCE21_IC_1_ADDR, REG_SOUT_SOURCE21_IC_1_BYTE, R37_SOUT_SOURCE21_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(
            DEVICE_ADDR_IC_1, REG_SOUT_SOURCE22_IC_1_ADDR, REG_SOUT_SOURCE22_IC_1_BYTE, R38_SOUT_SOURCE22_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(
            DEVICE_ADDR_IC_1, REG_SOUT_SOURCE23_IC_1_ADDR, REG_SOUT_SOURCE23_IC_1_BYTE, R39_SOUT_SOURCE23_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(
            DEVICE_ADDR_IC_1, REG_SERIAL_BYTE_0_0_IC_1_ADDR, REG_SERIAL_BYTE_0_0_IC_1_BYTE, R40_SERIAL_BYTE_0_0_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(
            DEVICE_ADDR_IC_1, REG_SERIAL_BYTE_1_0_IC_1_ADDR, REG_SERIAL_BYTE_1_0_IC_1_BYTE, R41_SERIAL_BYTE_1_0_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(
            DEVICE_ADDR_IC_1, REG_SERIAL_BYTE_2_0_IC_1_ADDR, REG_SERIAL_BYTE_2_0_IC_1_BYTE, R42_SERIAL_BYTE_2_0_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(
            DEVICE_ADDR_IC_1, REG_SERIAL_BYTE_3_0_IC_1_ADDR, REG_SERIAL_BYTE_3_0_IC_1_BYTE, R43_SERIAL_BYTE_3_0_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(
            DEVICE_ADDR_IC_1, REG_SERIAL_BYTE_4_0_IC_1_ADDR, REG_SERIAL_BYTE_4_0_IC_1_BYTE, R44_SERIAL_BYTE_4_0_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(
            DEVICE_ADDR_IC_1, REG_SERIAL_BYTE_5_0_IC_1_ADDR, REG_SERIAL_BYTE_5_0_IC_1_BYTE, R45_SERIAL_BYTE_5_0_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(
            DEVICE_ADDR_IC_1, REG_SERIAL_BYTE_6_0_IC_1_ADDR, REG_SERIAL_BYTE_6_0_IC_1_BYTE, R46_SERIAL_BYTE_6_0_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(
            DEVICE_ADDR_IC_1, REG_SERIAL_BYTE_7_0_IC_1_ADDR, REG_SERIAL_BYTE_7_0_IC_1_BYTE, R47_SERIAL_BYTE_7_0_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, PROGRAM_ADDR_IC_1, PROGRAM_SIZE_IC_1, Program_Data_IC_1);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, PARAM_ADDR_IC_1, PARAM_SIZE_IC_1, Param_Data_IC_1);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, DM1_DATA_ADDR_IC_1, DM1_DATA_SIZE_IC_1, DM1_DATA_Data_IC_1);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_KILL_CORE_IC_1_ADDR, REG_KILL_CORE_IC_1_BYTE, R51_KILL_CORE_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(
            DEVICE_ADDR_IC_1, REG_START_ADDRESS_IC_1_ADDR, REG_START_ADDRESS_IC_1_BYTE, R52_START_ADDRESS_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_START_PULSE_IC_1_ADDR, REG_START_PULSE_IC_1_BYTE, R53_START_PULSE_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_START_CORE_IC_1_ADDR, REG_START_CORE_IC_1_BYTE, R54_START_CORE_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_START_CORE_IC_1_ADDR, REG_START_CORE_IC_1_BYTE, R55_START_CORE_IC_1_Default);
        SIGMA_WRITE_DELAY(DEVICE_ADDR_IC_1, R56_START_DELAY_IC_1_SIZE, R56_START_DELAY_IC_1_Default);
        SIGMA_WRITE_REGISTER_BLOCK(DEVICE_ADDR_IC_1, REG_HIBERNATE_IC_1_ADDR, REG_HIBERNATE_IC_1_BYTE, R57_HIBERNATE_IC_1_Default);
    }

} // namespace CFXS::HW