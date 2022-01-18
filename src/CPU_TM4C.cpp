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