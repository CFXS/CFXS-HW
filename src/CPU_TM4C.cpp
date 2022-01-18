// [CFXS] //
#include <CFXS/Base/CPU.hpp>
#include <driverlib/sysctl.h>

namespace CFXS::CPU {

    void Delay_us(size_t us) {
        SysCtlDelay(CLOCK_FREQUENCY / 3 / 1000000 * us);
    }

    void Delay_ms(size_t ms) {
        SysCtlDelay(CLOCK_FREQUENCY / 3 / 1000 * ms);
    }

} // namespace CFXS::CPU