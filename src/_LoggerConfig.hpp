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

#include <CFXS/Base/DebugTerminal.hpp>
#include <CFXS/Base/Logger.hpp>

namespace CFXS::HW {

    class HardwareLogger_Base;
    class HardwareLogger_Base : public Logger<HardwareLogger_Base> {
        friend class Logger<HardwareLogger_Base>;

    private:
        static constexpr bool ENABLED                   = true;
        static constexpr const char* LOG_PREFIX         = DEBUG_TERMINAL_RESET_COLOR "CFXS-HW| ";
        static constexpr const char* WARNING_PREFIX     = DEBUG_TERMINAL_WARNING_COLOR "CFXS-HW| ";
        static constexpr const char* ERROR_PREFIX       = DEBUG_TERMINAL_ERROR_COLOR "CFXS-HW| ";
        static constexpr const char* CONSTRUCTOR_PREFIX = ANSI_CYAN "CFXS-HW| ";
        static constexpr const char* DESTRUCTOR_PREFIX  = ANSI_CYAN "CFXS-HW| ";

        HardwareLogger_Base() = default;
    };

} // namespace CFXS::HW