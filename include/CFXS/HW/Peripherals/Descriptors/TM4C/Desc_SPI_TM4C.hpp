// [CFXS] //
#pragma once

#include <array>
#include <inc/hw_memmap.h>
#include <driverlib/ssi.h>
#include <driverlib/sysctl.h>

namespace CFXS::HW::TM4C {

    struct Desc_SPI {
        constexpr Desc_SPI(uint32_t spiPeriph, uint32_t spiBase) : periph(spiPeriph), base(spiBase) {
        }
        uint32_t periph;
        uint32_t base;
    };

} // namespace CFXS::HW::TM4C