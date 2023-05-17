#pragma once
#include <CFXS/Base/Debug.hpp>

namespace CFXS::Ethernet {
#ifdef RELEASE_FAST
    static constexpr CFXS::DebugLevel_t DEBUG_LEVEL = CFXS::DebugLevel::OFF;
#else
    static constexpr CFXS::DebugLevel_t DEBUG_LEVEL = CFXS::DebugLevel::TRACE;
#endif
} // namespace CFXS::Ethernet

#define CFXS_ETH_printf(level, ...)                       \
    if constexpr (level >= CFXS::Ethernet::DEBUG_LEVEL) { \
        CFXS_printf("Ethernet> " __VA_ARGS__);            \
    }
