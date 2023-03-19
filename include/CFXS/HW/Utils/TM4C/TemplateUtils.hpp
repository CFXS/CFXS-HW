// [CFXS] //
#pragma once

namespace CFXS::HW::Utils::TM4C {

    namespace GPIO {

        template<size_t N>
        constexpr uint32_t StringToPins(const char (&pin)[N]) {
            static_assert((N - 1) >= 2, "Invalid pin (format: \"A0\")");
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

        template<size_t N>
        constexpr uint32_t StringToBase(const char (&pin)[N]) {
            static_assert((N - 1) >= 2, "Invalid pin (format: \"A0\")");
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

        template<size_t N>
        constexpr uint32_t StringToPeripheral(const char (&pin)[N]) {
            static_assert((N - 1) >= 2, "Invalid pin (format: \"A0\")");
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