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

#include <driverlib/sysctl.h>

namespace CFXS::HW::TM4C {

    class SystemControl {
    public:
        static bool IsPeripheralEnabled(uint32_t periph) {
            return SysCtlPeripheralReady(periph); //
        }

        static void WaitForPeripheralReady(uint32_t periph) {
            while (!IsPeripheralEnabled(periph)) {
            }
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