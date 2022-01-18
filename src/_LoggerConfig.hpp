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
        static constexpr const char* LOG_PREFIX         = __DEBUG_TERMINAL_RESET_COLOR__ "CFXS-HW| ";
        static constexpr const char* WARNING_PREFIX     = __DEBUG_TERMINAL_WARNING_COLOR__ "CFXS-HW| ";
        static constexpr const char* ERROR_PREFIX       = __DEBUG_TERMINAL_ERROR_COLOR__ "CFXS-HW| ";
        static constexpr const char* CONSTRUCTOR_PREFIX = __ANSI_CYAN__ "CFXS-HW| ";
        static constexpr const char* DESTRUCTOR_PREFIX  = __ANSI_CYAN__ "CFXS-HW| ";

        HardwareLogger_Base() = default;
    };

} // namespace CFXS::HW