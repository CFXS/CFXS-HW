#pragma once

#include <array>
#include <inc/hw_memmap.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>

namespace CFXS::HW::TM4C {

    struct Desc_GPIO {
        constexpr Desc_GPIO(uint32_t gpioPeriph                          = 0,
                            uint32_t gpioBase                            = 0,
                            uint32_t gpioPins                            = 0,
                            uint32_t gpioDriveCurrent                    = 0,
                            uint32_t gpioPinType                         = 0,
                            const std::array<uint32_t, 8>& gpioPinConfig = {}) :
            periph(gpioPeriph),
            base(gpioBase),
            pins(gpioPins),
            driveCurrent(gpioDriveCurrent),
            pinType(gpioPinType),
            pinConfig(gpioPinConfig) {}
        uint32_t periph;
        uint32_t base;
        uint32_t pins;
        uint32_t driveCurrent;
        uint32_t pinType;
        std::array<uint32_t, 8> pinConfig;
    };

} // namespace CFXS::HW::TM4C