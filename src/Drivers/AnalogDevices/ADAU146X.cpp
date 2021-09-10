#include <CFXS/HW/Drivers/AnalogDevices/ADAU146X.hpp>
#include <CFXS/Base/CPU.hpp>
#include <CFXS/Base/Debug.hpp>
#include <_LoggerConfig.hpp>

#include <CFXS/HW/System/SystemControl_TM4C.hpp>
#include <driverlib/ssi.h>
#include <inc/hw_ssi.h>
#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
using CFXS::HW::TM4C::SystemControl;

namespace CFXS::HW {

    static constexpr auto SPI_BITRATE_INITIAL = 1000000; // 1MHz
    static constexpr auto SPI_BITRATE_NORMAL  = 8000000; // 8MHz

    ADAU146X::ADAU146X(const void* nreset, const void* cs, const void* sclk, const void* mosi, const void* miso) :
        m_pin_nReset(nreset), m_pin_CS(cs), m_pin_SCLK(sclk), m_pin_MOSI(mosi), m_pin_MISO(miso) {
        HardwareLogger_Base::LogConstructor("ADAU146X[%p](%p, %p, %p, %p, %p)", this, nreset, cs, sclk, mosi, miso);
    }

    void ADAU146X::Initialize() {
        CFXS_ASSERT(m_Initialized == false, "Already initialized");
        HardwareLogger_Base::Log("ADAU146X[%p] Initialize", this);

        m_pin_nReset.Initialize(GPIO::Direction::OUTPUT, 1); // Run
        m_pin_CS.Initialize(GPIO::Direction::HARDWARE);
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
        SSIConfigSetExpClk(SSI1_BASE, CPU::CLOCK_FREQUENCY, SSI_FRF_MOTO_MODE_3, SSI_MODE_MASTER, SPI_BITRATE_INITIAL, 8);
        SSIAdvModeSet(SSI1_BASE, SSI_ADV_MODE_LEGACY);
        SSIAdvFrameHoldEnable(SSI1_BASE);
        SSIEnable(SSI1_BASE);
    }

    // The slave port can be put into SPI mode by performing 3 dummy writes to any subaddress
    // These writes are ignored by the slave port
    void ADAU146X::SetSlavePortModeToSPI() {
        HardwareLogger_Base::Log(" - Set slave port to SPI mode");
        CPU::Delay_ms(1);

        for (int i = 0; i < 3; i++) {
            SSIDataPut(SSI1_BASE, 0);
            SSIDataPut(SSI1_BASE, 0);
            SSIAdvDataPutFrameEnd(SSI1_BASE, 0);

            HardwareLogger_Base::Log(" - Wrote dummy byte %d", i + 1);
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

        while (SSIDataGetNonBlocking(SSI1_BASE, &temp)) {}

        SSIDataPut(SSI1_BASE, 0x80);
        SSIDataPut(SSI1_BASE, 0xF4);
        SSIDataPut(SSI1_BASE, 0x05);
        SSIDataPut(SSI1_BASE, 0x00);
        SSIAdvDataPutFrameEnd(SSI1_BASE, 0x00);

        uint32_t dat[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        while (!(HWREG(SSI1_BASE + SSI_O_SR) & SSI_SR_TFE)) {}
        int i = 0;
        while (SSIDataGetNonBlocking(SSI1_BASE, &dat[i++])) {}
        HardwareLogger_Base::Log(" - read %d bytes", i);

        for (int x = 0; x < i; x++)
            HardwareLogger_Base::Log(" - resp[%d]: %02X", x, dat[x] & 0xFF);
    }

} // namespace CFXS::HW