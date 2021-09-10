#include <CFXS/HW/Drivers/AnalogDevices/ADAU146X.hpp>
#include <driverlib/sysctl.h>

namespace CFXS::HW {

    ADAU146X::ADAU146X(const void* nreset, const void* cs, const void* sclk, const void* mosi, const void* miso) :
        m_pin_nReset(nreset), m_pin_CS(cs), m_pin_SCLK(sclk), m_pin_MOSI(mosi), m_pin_MISO(miso) {}

    void ADAU146X::Initialize() {
        m_pin_nReset.Initialize(GPIO::Direction::OUTPUT, 1); // Run
        m_pin_CS.Initialize(GPIO::Direction::OUTPUT);
        m_pin_SCLK.Initialize(GPIO::Direction::HARDWARE);
        m_pin_MOSI.Initialize(GPIO::Direction::HARDWARE);
        m_pin_MISO.Initialize(GPIO::Direction::HARDWARE);

        m_pin_CS.Write(true);
        SysCtlDelay(40 * 1000 * 2);
        m_pin_CS.Write(false);
        SysCtlDelay(40 * 1000 * 2);
        m_pin_CS.Write(true);
        SysCtlDelay(40 * 1000 * 2);
        m_pin_CS.Write(false);
        SysCtlDelay(40 * 1000 * 2);
        m_pin_CS.Write(true);
        SysCtlDelay(40 * 1000 * 2);
        m_pin_CS.Write(false);
        SysCtlDelay(40 * 1000 * 2);
        m_pin_CS.MakeHardwareControlled();
    }

} // namespace CFXS::HW