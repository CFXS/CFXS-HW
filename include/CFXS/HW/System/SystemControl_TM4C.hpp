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