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

#define __LOCALDEF_HWREGBITW(x, b) __mem32(((uint32_t)(x)&0xF0000000) | 0x02000000 | (((uint32_t)(x)&0x000FFFFF) << 5) | ((b) << 2))
#define __LOCALDEF_HWREGBITH(x, b) __mem16(((uint32_t)(x)&0xF0000000) | 0x02000000 | (((uint32_t)(x)&0x000FFFFF) << 5) | ((b) << 2))
#define __LOCALDEF_HWREGBITB(x, b) __mem8(((uint32_t)(x)&0xF0000000) | 0x02000000 | (((uint32_t)(x)&0x000FFFFF) << 5) | ((b) << 2))

namespace CFXS::HW::TM4C {

    class SystemControl {
    public:
        struct Register {
            static constexpr uint32_t PPBASE    = 0x400FE300;
            static constexpr uint32_t SRBASE    = 0x400FE500;
            static constexpr uint32_t RCGCBASE  = 0x400FE600;
            static constexpr uint32_t SCGCBASE  = 0x400FE700;
            static constexpr uint32_t DCGCBASE  = 0x400FE800;
            static constexpr uint32_t PCBASE    = 0x400FE900;
            static constexpr uint32_t PRBASE    = 0x400FEA00;
            static constexpr uint32_t RESC      = 0x400FE05C;
            static constexpr uint32_t PWRTC     = 0x400FE060;
            static constexpr uint32_t RCC       = 0x400FE060;
            static constexpr uint32_t NMIC      = 0x400FE064;
            static constexpr uint32_t GPIOHBCTL = 0x400FE06C;
        };

        struct Peripheral {
            static constexpr uint32_t ADC0      = 0xf0003800;
            static constexpr uint32_t ADC1      = 0xf0003801;
            static constexpr uint32_t CAN0      = 0xf0003400;
            static constexpr uint32_t CAN1      = 0xf0003401;
            static constexpr uint32_t COMP0     = 0xf0003c00;
            static constexpr uint32_t EMAC0     = 0xf0009c00;
            static constexpr uint32_t EPHY0     = 0xf0003000;
            static constexpr uint32_t EPI0      = 0xf0001000;
            static constexpr uint32_t GPIOA     = 0xf0000800;
            static constexpr uint32_t GPIOB     = 0xf0000801;
            static constexpr uint32_t GPIOC     = 0xf0000802;
            static constexpr uint32_t GPIOD     = 0xf0000803;
            static constexpr uint32_t GPIOE     = 0xf0000804;
            static constexpr uint32_t GPIOF     = 0xf0000805;
            static constexpr uint32_t GPIOG     = 0xf0000806;
            static constexpr uint32_t GPIOH     = 0xf0000807;
            static constexpr uint32_t GPIOJ     = 0xf0000808;
            static constexpr uint32_t HIBERNATE = 0xf0001400;
            static constexpr uint32_t CCM0      = 0xf0007400;
            static constexpr uint32_t EEPROM0   = 0xf0005800;
            static constexpr uint32_t FAN0      = 0xf0005400;
            static constexpr uint32_t FAN1      = 0xf0005401;
            static constexpr uint32_t GPIOK     = 0xf0000809;
            static constexpr uint32_t GPIOL     = 0xf000080a;
            static constexpr uint32_t GPIOM     = 0xf000080b;
            static constexpr uint32_t GPION     = 0xf000080c;
            static constexpr uint32_t GPIOP     = 0xf000080d;
            static constexpr uint32_t GPIOQ     = 0xf000080e;
            static constexpr uint32_t GPIOR     = 0xf000080f;
            static constexpr uint32_t GPIOS     = 0xf0000810;
            static constexpr uint32_t GPIOT     = 0xf0000811;
            static constexpr uint32_t I2C0      = 0xf0002000;
            static constexpr uint32_t I2C1      = 0xf0002001;
            static constexpr uint32_t I2C2      = 0xf0002002;
            static constexpr uint32_t I2C3      = 0xf0002003;
            static constexpr uint32_t I2C4      = 0xf0002004;
            static constexpr uint32_t I2C5      = 0xf0002005;
            static constexpr uint32_t I2C6      = 0xf0002006;
            static constexpr uint32_t I2C7      = 0xf0002007;
            static constexpr uint32_t I2C8      = 0xf0002008;
            static constexpr uint32_t I2C9      = 0xf0002009;
            static constexpr uint32_t LCD0      = 0xf0009000;
            static constexpr uint32_t ONEWIRE0  = 0xf0009800;
            static constexpr uint32_t PWM0      = 0xf0004000;
            static constexpr uint32_t PWM1      = 0xf0004001;
            static constexpr uint32_t QEI0      = 0xf0004400;
            static constexpr uint32_t QEI1      = 0xf0004401;
            static constexpr uint32_t SSI0      = 0xf0001c00;
            static constexpr uint32_t SSI1      = 0xf0001c01;
            static constexpr uint32_t SSI2      = 0xf0001c02;
            static constexpr uint32_t SSI3      = 0xf0001c03;
            static constexpr uint32_t TIMER0    = 0xf0000400;
            static constexpr uint32_t TIMER1    = 0xf0000401;
            static constexpr uint32_t TIMER2    = 0xf0000402;
            static constexpr uint32_t TIMER3    = 0xf0000403;
            static constexpr uint32_t TIMER4    = 0xf0000404;
            static constexpr uint32_t TIMER5    = 0xf0000405;
            static constexpr uint32_t TIMER6    = 0xf0000406;
            static constexpr uint32_t TIMER7    = 0xf0000407;
            static constexpr uint32_t UART0     = 0xf0001800;
            static constexpr uint32_t UART1     = 0xf0001801;
            static constexpr uint32_t UART2     = 0xf0001802;
            static constexpr uint32_t UART3     = 0xf0001803;
            static constexpr uint32_t UART4     = 0xf0001804;
            static constexpr uint32_t UART5     = 0xf0001805;
            static constexpr uint32_t UART6     = 0xf0001806;
            static constexpr uint32_t UART7     = 0xf0001807;
            static constexpr uint32_t UDMA      = 0xf0000c00;
            static constexpr uint32_t USB0      = 0xf0002800;
            static constexpr uint32_t WDOG0     = 0xf0000000;
            static constexpr uint32_t WDOG1     = 0xf0000001;
            static constexpr uint32_t WTIMER0   = 0xf0005c00;
            static constexpr uint32_t WTIMER1   = 0xf0005c01;
            static constexpr uint32_t WTIMER2   = 0xf0005c02;
            static constexpr uint32_t WTIMER3   = 0xf0005c03;
            static constexpr uint32_t WTIMER4   = 0xf0005c04;
            static constexpr uint32_t WTIMER5   = 0xf0005c05;
        };

    public:
        /// @brief Check if peripheral is present in the system
        /// @param periph Peripheral SystemControl address
        /// @return true if peripheral is present
        static bool IsPeripheralPresent(uint32_t periph) {
            return __LOCALDEF_HWREGBITW(Register::PPBASE + ((periph & 0xff00) >> 8), periph & 0xff);
        }

        /// @brief Check if peripheral is enabled
        /// @param periph Peripheral SystemControl address
        /// @return true if peripheral is enabled
        static bool IsPeripheralEnabled(uint32_t periph) {
            return __LOCALDEF_HWREGBITW(Register::PRBASE + ((periph & 0xff00) >> 8), periph & 0xff);
        }

        /// @brief Check if peripheral is ready/initialized
        /// @param periph Peripheral SystemControl address
        /// @return true if peripheral is ready/initialized
        static bool IsPeripheralReady(uint32_t periph) {
            return __LOCALDEF_HWREGBITW(Register::PRBASE + ((periph & 0xff00) >> 8), periph & 0xff);
        }

        /// @brief Block until peripheral is ready/initialized
        /// @param periph Peripheral SystemControl address
        static void WaitForPeripheralReady(uint32_t periph) {
            while (!IsPeripheralReady(periph)) {
            }
        }

        /// @brief Enable peripheral
        /// @param periph Peripheral SystemControl address
        /// @param waitForReady Block until peripheral ready after enabling
        static void EnablePeripheral(uint32_t periph, bool waitForReady = true) {
            if (!IsPeripheralEnabled(periph))
                __LOCALDEF_HWREGBITW(Register::RCGCBASE + ((periph & 0xff00) >> 8), periph & 0xff) = 1;
            if (waitForReady)
                WaitForPeripheralReady(periph);
        }

        /// @brief Disable peripheral
        /// @param periph Peripheral SystemControl address
        static void DisablePeripheral(uint32_t periph) {
            if (IsPeripheralEnabled(periph))
                __LOCALDEF_HWREGBITW(Register::RCGCBASE + ((periph & 0xff00) >> 8), periph & 0xff) = 0;
        }

        /// @brief Reset peripheral
        /// @param periph Peripheral SystemControl address
        static void ResetPeripheral(uint32_t periph) {
            // Enter reset state
            __LOCALDEF_HWREGBITW(Register::SRBASE + ((periph & 0xff00) >> 8), periph & 0xff) = 1;
            // Wait
            for (int i = 0; i < 16; i++)
                asm volatile("nop");
            // Exit reset state
            __LOCALDEF_HWREGBITW(Register::SRBASE + ((periph & 0xff00) >> 8), periph & 0xff) = 0;
        }
    };

} // namespace CFXS::HW::TM4C

#undef __LOCALDEF_HWREGBITW
#undef __LOCALDEF_HWREGBITH
#undef __LOCALDEF_HWREGBITB