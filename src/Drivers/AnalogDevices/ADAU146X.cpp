#include <CFXS/HW/Drivers/AnalogDevices/ADAU146X.hpp>
#include <CFXS/Base/CPU.hpp>
#include <CFXS/Base/Debug.hpp>
#include <CFXS/HW/Drivers/AnalogDevices/_Reg_ADAU146X.hpp>
#include <_LoggerConfig.hpp>

#include <CFXS/HW/System/SystemControl_TM4C.hpp>
#include <driverlib/ssi.h>
#include <inc/hw_ssi.h>
#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
using CFXS::HW::TM4C::SystemControl;

namespace CFXS::HW {

    static constexpr auto SPI_BITRATE_INITIAL = 1000000; // 1MHz
    static constexpr auto SPI_BITRATE_NORMAL  = 4000000; // 8MHz

    ADAU146X::ADAU146X(const void* nreset, const void* cs, const void* sclk, const void* mosi, const void* miso) :
        m_pin_nReset(nreset), m_pin_CS(cs), m_pin_SCLK(sclk), m_pin_MOSI(mosi), m_pin_MISO(miso) {
        HardwareLogger_Base::LogConstructor("ADAU146X[%p](%p, %p, %p, %p, %p)", this, nreset, cs, sclk, mosi, miso);
    }

    void ADAU146X::Initialize() {
        CFXS_ASSERT(m_Initialized == false, "Already initialized");
        HardwareLogger_Base::Log("ADAU146X[%p] Initialize", this);

        m_pin_nReset.Initialize(GPIO::Direction::OUTPUT, 0xFFFFFFFF); // Run
        m_pin_CS.Initialize(GPIO::Direction::OUTPUT, 0xFFFFFFFF);
        m_pin_SCLK.Initialize(GPIO::Direction::HARDWARE);
        m_pin_MOSI.Initialize(GPIO::Direction::HARDWARE);
        m_pin_MISO.Initialize(GPIO::Direction::HARDWARE);

        Initialize_SPI();        // Initialize SPI peripheral
        SetSlavePortModeToSPI(); // Place slave port in SPI mode
        TestCommunication();     // Check if SPI works

        m_Initialized = true;
    }

    void ADAU146X::Initialize_SPI() {
        HardwareLogger_Base::Log(" - Initialize SPI");

        SystemControl::EnablePeripheral(SYSCTL_PERIPH_SSI1);
        SSIConfigSetExpClk(SSI1_BASE, CPU::CLOCK_FREQUENCY, SSI_FRF_MOTO_MODE_3, SSI_MODE_MASTER, SPI_BITRATE_NORMAL, 8);
        SSIEnable(SSI1_BASE);
    }

    // The slave port can be put into SPI mode by performing 3 dummy writes to any subaddress
    // These writes are ignored by the slave port
    void ADAU146X::SetSlavePortModeToSPI() {
        HardwareLogger_Base::Log(" - Set slave port to SPI mode");
        CPU::Delay_ms(1);

        // Slave port is set to SPI mode after 3 HIGH->LOW transitions on CS
        for (int i = 0; i < 3; i++) {
            m_pin_CS.Write(false);
            CPU::Delay_ms(1);
            m_pin_CS.Write(true);
            CPU::Delay_ms(1);
        }
    }

    // Byte 0  - Chip Address [6:0], R/!W
    // Byte 1  - Subaddress [15:8]
    // Byte 2  - Subaddress [7:0]
    // Byte 3  - Data
    // Byte 4+ - Data

    void ADAU146X::TestCommunication() {
        uint32_t temp;

        uint16_t addr = 0xF890;

        while (SSIDataGetNonBlocking(SSI1_BASE, &temp)) {}

        m_pin_CS.Write(false);
        SSIDataPut(SSI1_BASE, 0x01);
        SSIDataPut(SSI1_BASE, addr >> 8);   // soft reset register
        SSIDataPut(SSI1_BASE, addr & 0xFF); // after reset should be 0x0001
        SSIDataPut(SSI1_BASE, 0x00);
        SSIDataPut(SSI1_BASE, 0x00);
        while (SSIBusy(SSI1_BASE)) {}
        m_pin_CS.Write(true);

        uint32_t dat[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        int i           = 0;
        while (SSIDataGetNonBlocking(SSI1_BASE, &dat[i++])) {}
        HardwareLogger_Base::Log(" - read %d bytes", i);

        for (int x = 0; x < i; x++)
            HardwareLogger_Base::Log(" - resp[%d]: %02X", x, dat[x] & 0xFF);
    }

} // namespace CFXS::HW