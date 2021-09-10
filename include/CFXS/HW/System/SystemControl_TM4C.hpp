#pragma once

#include <driverlib/sysctl.h>

namespace CFXS::HW::TM4C {

    class SystemControl {
    public:
        static bool IsPeripheralEnabled(uint32_t periph) {
            return SysCtlPeripheralReady(periph); //
        }

        static void WaitForPeripheralReady(uint32_t periph) {
            while (!IsPeripheralEnabled(periph)) {}
        }

        static void EnablePeripheral(uint32_t periph, bool waitForReady = true) {
            if (!IsPeripheralEnabled(periph))
                SysCtlPeripheralEnable(periph);
            if (waitForReady)
                WaitForPeripheralReady(periph);
        }

        static void DisablePeripheral(uint32_t periph) {
            if (IsPeripheralEnabled(periph))
                SysCtlPeripheralDisable(periph);
        }

        static void ResetPeripheral(uint32_t periph) {
            SysCtlPeripheralReset(periph); //
        }
    };

} // namespace CFXS::HW::TM4C