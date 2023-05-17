#pragma once
#include <CFXS/Base/Debug.hpp>

namespace CFXS::Ethernet {
    static constexpr CFXS::DebugLevel_t DEBUG_LEVEL = CFXS::DebugLevel::TRACE;
}

#define CFXS_ETH_printf(level, ...)                       \
    if constexpr (level >= CFXS::Ethernet::DEBUG_LEVEL) { \
        CFXS_printf("Ethernet> " __VA_ARGS__);            \
    }
