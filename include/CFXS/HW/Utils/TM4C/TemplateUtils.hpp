// [CFXS] //
#pragma once

// Register base and peripheral addresses are the same for all TM4C series devices

namespace CFXS::HW::Utils::TM4C {

    namespace GPIO {

        /// @brief Convert port string to pin mask
        /// @param pin port string "A0" / "B1,2,3"
        /// @return Pin mask
        template<size_t N>
        constexpr uint32_t PortStringToPins(const char (&pin)[N]) {
            uint32_t pins = 0;
            for (size_t i = 1; i < N; i++) {
                switch (pin[i]) {
                    case '0': pins |= 0x00000001; break;
                    case '1': pins |= 0x00000002; break;
                    case '2': pins |= 0x00000004; break;
                    case '3': pins |= 0x00000008; break;
                    case '4': pins |= 0x00000010; break;
                    case '5': pins |= 0x00000020; break;
                    case '6': pins |= 0x00000040; break;
                    case '7': pins |= 0x00000080; break;
                    default: break;
                }
            }
            return pins;
        }

        /// @brief Convert port string to GPIO port base register address
        /// @param pin port string "A0" / "B1,2,3"
        /// @return GPIO port base register
        template<size_t N>
        constexpr uint32_t PortStringToBase(const char (&pin)[N]) {
            switch (pin[0]) {
                case 'A': return 0x40004000;
                case 'B': return 0x40005000;
                case 'C': return 0x40006000;
                case 'D': return 0x40007000;
                case 'E': return 0x40024000;
                case 'F': return 0x40025000;
                case 'G': return 0x40026000;
                case 'H': return 0x40027000;
                case 'J': return 0x4003D000;
                case 'K': return 0x40061000;
                case 'L': return 0x40062000;
                case 'M': return 0x40063000;
                case 'N': return 0x40064000;
                case 'P': return 0x40065000;
                case 'Q': return 0x40066000;
                case 'R': return 0x40067000;
                case 'S': return 0x40068000;
                case 'T': return 0x40069000;
                default: return 0;
            };
        }

        /// @brief Convert port string to SystemControl GPIO port register
        /// @param pin port string "A0" / "B1,2,3"
        /// @return SystemControl address of GPIO base
        template<size_t N>
        constexpr uint32_t PortStringToPeripheral(const char (&pin)[N]) {
            switch (pin[0]) {
                case 'A': return 0xF0000800;
                case 'B': return 0xF0000801;
                case 'C': return 0xF0000802;
                case 'D': return 0xF0000803;
                case 'E': return 0xF0000804;
                case 'F': return 0xF0000805;
                case 'G': return 0xF0000806;
                case 'H': return 0xF0000807;
                case 'J': return 0xF0000808;
                case 'K': return 0xF0000809;
                case 'L': return 0xF000080A;
                case 'M': return 0xF000080B;
                case 'N': return 0xF000080C;
                case 'P': return 0xF000080D;
                case 'Q': return 0xF000080E;
                case 'R': return 0xF000080F;
                case 'S': return 0xF0000810;
                case 'T': return 0xF0000811;
                default: return 0;
            };
        }

    } // namespace GPIO

} // namespace CFXS::HW::Utils::TM4C