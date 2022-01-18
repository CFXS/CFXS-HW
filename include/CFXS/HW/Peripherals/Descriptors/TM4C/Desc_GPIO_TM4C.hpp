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

#include <array>
#include <inc/hw_memmap.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>

namespace CFXS::HW::TM4C {

#pragma pack(1)
    class Desc_GPIO {
        friend class CFXS::HW::GPIO;

    public:
        /// \param gpioPeriph SYSCTL_PERIPH_GPIOx
        /// \param gpioBase GPIO_PORTx_BASE
        /// \param pins GPIO_PIN_x or GPIO_PIN_x | GPIO_PIN_y
        /// \param gpioDriveCurrent GPIO_STRENGTH_xMA
        /// \param gpioPinType GPIO_PIN_TYPE_x
        /// \param gpioPinConfig !null terminated! array of GPIO_Pxy_z (GPIO_PA0_U0RX)
        constexpr Desc_GPIO(uint32_t gpioPeriph           = 0,
                            uint32_t gpioBase             = 0,
                            uint32_t gpioPins             = 0,
                            uint32_t gpioDriveCurrent     = 0,
                            uint32_t gpioPinType          = 0,
                            const uint32_t* gpioPinConfig = nullptr) :
            base(gpioBase),
            pins(gpioPins),
            periph(gpioPeriph),
            driveCurrent(gpioDriveCurrent),
            pinType(gpioPinType),
            pinConfig(gpioPinConfig) {
        }

        /// \param pin "PA0" / "PA0,1,2,..."
        /// \param gpioDriveCurrent GPIO_STRENGTH_xMA
        /// \param gpioPinType GPIO_PIN_TYPE_x
        /// \param gpioPinConfig !null terminated! array of GPIO_Pxy_z (GPIO_PA0_U0RX)
        template<std::size_t N>
        constexpr Desc_GPIO(const char (&pin)[N],
                            uint32_t gpioDriveCurrent     = 0,
                            uint32_t gpioPinType          = 0,
                            const uint32_t* gpioPinConfig = nullptr) :
            driveCurrent(gpioDriveCurrent), pinType(gpioPinType), pinConfig(gpioPinConfig) {
            static_assert((N - 1) >= 3, "Invalid pin (format: \"PA0\")");
            switch (pin[1]) {
                case 'A':
                    periph = SYSCTL_PERIPH_GPIOA & 0xFFFF;
                    base   = GPIO_PORTA_BASE;
                    break;
                case 'B':
                    periph = SYSCTL_PERIPH_GPIOB & 0xFFFF;
                    base   = GPIO_PORTB_BASE;
                    break;
                case 'C':
                    periph = SYSCTL_PERIPH_GPIOC & 0xFFFF;
                    base   = GPIO_PORTC_BASE;
                    break;
                case 'D':
                    periph = SYSCTL_PERIPH_GPIOD & 0xFFFF;
                    base   = GPIO_PORTD_BASE;
                    break;
                case 'E':
                    periph = SYSCTL_PERIPH_GPIOE & 0xFFFF;
                    base   = GPIO_PORTE_BASE;
                    break;
                case 'F':
                    periph = SYSCTL_PERIPH_GPIOF & 0xFFFF;
                    base   = GPIO_PORTF_BASE;
                    break;
                case 'G':
                    periph = SYSCTL_PERIPH_GPIOG & 0xFFFF;
                    base   = GPIO_PORTG_BASE;
                    break;
                case 'H':
                    periph = SYSCTL_PERIPH_GPIOH & 0xFFFF;
                    base   = GPIO_PORTH_BASE;
                    break;
                case 'J':
                    periph = SYSCTL_PERIPH_GPIOJ & 0xFFFF;
                    base   = GPIO_PORTJ_BASE;
                    break;
                case 'K':
                    periph = SYSCTL_PERIPH_GPIOK & 0xFFFF;
                    base   = GPIO_PORTK_BASE;
                    break;
                case 'L':
                    periph = SYSCTL_PERIPH_GPIOL & 0xFFFF;
                    base   = GPIO_PORTL_BASE;
                    break;
                case 'M':
                    periph = SYSCTL_PERIPH_GPIOM & 0xFFFF;
                    base   = GPIO_PORTM_BASE;
                    break;
                case 'N':
                    periph = SYSCTL_PERIPH_GPION & 0xFFFF;
                    base   = GPIO_PORTN_BASE;
                    break;
                case 'P':
                    periph = SYSCTL_PERIPH_GPIOP & 0xFFFF;
                    base   = GPIO_PORTP_BASE;
                    break;
                case 'Q':
                    periph = SYSCTL_PERIPH_GPIOQ & 0xFFFF;
                    base   = GPIO_PORTQ_BASE;
                    break;
                case 'R':
                    periph = SYSCTL_PERIPH_GPIOR & 0xFFFF;
                    base   = GPIO_PORTR_BASE;
                    break;
                case 'S':
                    periph = SYSCTL_PERIPH_GPIOS & 0xFFFF;
                    base   = GPIO_PORTS_BASE;
                    break;
                case 'T':
                    periph = SYSCTL_PERIPH_GPIOT & 0xFFFF;
                    base   = GPIO_PORTT_BASE;
                    break;
                default: periph = 0; base = 0;
            };

            pins = 0;
            for (size_t i = 2; i < N; i++) {
                switch (pin[i]) {
                    case '0': pins |= GPIO_PIN_0; break;
                    case '1': pins |= GPIO_PIN_1; break;
                    case '2': pins |= GPIO_PIN_2; break;
                    case '3': pins |= GPIO_PIN_3; break;
                    case '4': pins |= GPIO_PIN_4; break;
                    case '5': pins |= GPIO_PIN_5; break;
                    case '6': pins |= GPIO_PIN_6; break;
                    case '7': pins |= GPIO_PIN_7; break;
                }
            }
        }

    private:
        uint32_t base;
        uint16_t pins;
        uint16_t periph; // lower 2 bytes of periph (high bytes are always 0xF000)
        uint16_t driveCurrent;
        uint16_t pinType;
        const uint32_t* pinConfig;
    };
#pragma pack()

} // namespace CFXS::HW::TM4C