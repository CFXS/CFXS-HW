// [CFXS] //
#pragma once

namespace CFXS::HW::Utils::TM4C {

    namespace GPIO {

        template<size_t N>
        constexpr uint32_t DescriptionStringToPins(const char (&pin)[N]) {
            static_assert((N - 1) >= 2, "Invalid pin (format: \"A0\")");
            uint32_t pins = 0;
            for (size_t i = 1; i < N; i++) {
                switch (pin[i]) {
                    case '0': pins |= GPIO_PIN_0; break;
                    case '1': pins |= GPIO_PIN_1; break;
                    case '2': pins |= GPIO_PIN_2; break;
                    case '3': pins |= GPIO_PIN_3; break;
                    case '4': pins |= GPIO_PIN_4; break;
                    case '5': pins |= GPIO_PIN_5; break;
                    case '6': pins |= GPIO_PIN_6; break;
                    case '7': pins |= GPIO_PIN_7; break;
                    default: break;
                }
            }
            return pins;
        }

        template<size_t N>
        constexpr uint32_t DescriptionStringToBase(const char (&pin)[N]) {
            static_assert((N - 1) >= 2, "Invalid pin (format: \"A0\")");
            switch (pin[0]) {
                case 'A': return GPIO_PORTA_BASE;
                case 'B': return GPIO_PORTB_BASE;
                case 'C': return GPIO_PORTC_BASE;
                case 'D': return GPIO_PORTD_BASE;
                case 'E': return GPIO_PORTE_BASE;
                case 'F': return GPIO_PORTF_BASE;
                case 'G': return GPIO_PORTG_BASE;
                case 'H': return GPIO_PORTH_BASE;
                case 'J': return GPIO_PORTJ_BASE;
                case 'K': return GPIO_PORTK_BASE;
                case 'L': return GPIO_PORTL_BASE;
                case 'M': return GPIO_PORTM_BASE;
                case 'N': return GPIO_PORTN_BASE;
                case 'P': return GPIO_PORTP_BASE;
                case 'Q': return GPIO_PORTQ_BASE;
                case 'R': return GPIO_PORTR_BASE;
                case 'S': return GPIO_PORTS_BASE;
                case 'T': return GPIO_PORTT_BASE;
                default: return 0;
            };
        }

        template<size_t N>
        constexpr uint32_t DescriptionStringToPeripheral(const char (&pin)[N]) {
            static_assert((N - 1) >= 2, "Invalid pin (format: \"A0\")");
            switch (pin[0]) {
                case 'A': return SYSCTL_PERIPH_GPIOA;
                case 'B': return SYSCTL_PERIPH_GPIOB;
                case 'C': return SYSCTL_PERIPH_GPIOC;
                case 'D': return SYSCTL_PERIPH_GPIOD;
                case 'E': return SYSCTL_PERIPH_GPIOE;
                case 'F': return SYSCTL_PERIPH_GPIOF;
                case 'G': return SYSCTL_PERIPH_GPIOG;
                case 'H': return SYSCTL_PERIPH_GPIOH;
                case 'J': return SYSCTL_PERIPH_GPIOJ;
                case 'K': return SYSCTL_PERIPH_GPIOK;
                case 'L': return SYSCTL_PERIPH_GPIOL;
                case 'M': return SYSCTL_PERIPH_GPIOM;
                case 'N': return SYSCTL_PERIPH_GPION;
                case 'P': return SYSCTL_PERIPH_GPIOP;
                case 'Q': return SYSCTL_PERIPH_GPIOQ;
                case 'R': return SYSCTL_PERIPH_GPIOR;
                case 'S': return SYSCTL_PERIPH_GPIOS;
                case 'T': return SYSCTL_PERIPH_GPIOT;
                default: return 0;
            };
        }

    } // namespace GPIO

} // namespace CFXS::HW::Utils::TM4C