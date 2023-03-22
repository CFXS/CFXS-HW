// [CFXS] //
#pragma once
#include <CFXS/HW/Utils/TM4C/TemplateUtils.hpp>
#include <CFXS/HW/System/SystemControl_TM4C.hpp>

#define DEF_STATIC_GPIO_TM4C(str)                                                   \
    CFXS::HW::TM4C::Static_GPIO<CFXS::HW::Utils::TM4C::GPIO::PortStringToBase(str), \
                                CFXS::HW::Utils::TM4C::GPIO::PortStringToPins(str), \
                                CFXS::HW::Utils::TM4C::GPIO::PortStringToPeripheral(str)>

namespace CFXS::HW::TM4C {

    class GPIO {
    public:
        struct BaseOffset {
            static constexpr uint32_t DATA     = 0x00000000; // GPIO Data
            static constexpr uint32_t DIR      = 0x00000400; // GPIO Direction
            static constexpr uint32_t IS       = 0x00000404; // GPIO Interrupt Sense
            static constexpr uint32_t IBE      = 0x00000408; // GPIO Interrupt Both Edges
            static constexpr uint32_t IEV      = 0x0000040C; // GPIO Interrupt Event
            static constexpr uint32_t IM       = 0x00000410; // GPIO Interrupt Mask
            static constexpr uint32_t RIS      = 0x00000414; // GPIO Raw Interrupt Status
            static constexpr uint32_t MIS      = 0x00000418; // GPIO Masked Interrupt Status
            static constexpr uint32_t ICR      = 0x0000041C; // GPIO Interrupt Clear
            static constexpr uint32_t AFSEL    = 0x00000420; // GPIO Alternate Function Select
            static constexpr uint32_t DR2R     = 0x00000500; // GPIO 2-mA Drive Select
            static constexpr uint32_t DR4R     = 0x00000504; // GPIO 4-mA Drive Select
            static constexpr uint32_t DR8R     = 0x00000508; // GPIO 8-mA Drive Select
            static constexpr uint32_t ODR      = 0x0000050C; // GPIO Open Drain Select
            static constexpr uint32_t PUR      = 0x00000510; // GPIO Pull-Up Select
            static constexpr uint32_t PDR      = 0x00000514; // GPIO Pull-Down Select
            static constexpr uint32_t SLR      = 0x00000518; // GPIO Slew Rate Control Select
            static constexpr uint32_t DEN      = 0x0000051C; // GPIO Digital Enable
            static constexpr uint32_t LOCK     = 0x00000520; // GPIO Lock
            static constexpr uint32_t CR       = 0x00000524; // GPIO Commit
            static constexpr uint32_t AMSEL    = 0x00000528; // GPIO Analog Mode Select
            static constexpr uint32_t PCTL     = 0x0000052C; // GPIO Port Control
            static constexpr uint32_t ADCCTL   = 0x00000530; // GPIO ADC Control
            static constexpr uint32_t DMACTL   = 0x00000534; // GPIO DMA Control
            static constexpr uint32_t SI       = 0x00000538; // GPIO Select Interrupt
            static constexpr uint32_t DR12R    = 0x0000053C; // GPIO 12-mA Drive Select
            static constexpr uint32_t WAKEPEN  = 0x00000540; // GPIO Wake Pin Enable
            static constexpr uint32_t WAKELVL  = 0x00000544; // GPIO Wake Level
            static constexpr uint32_t WAKESTAT = 0x00000548; // GPIO Wake Status
            static constexpr uint32_t PP       = 0x00000FC0; // GPIO Peripheral Property
            static constexpr uint32_t PC       = 0x00000FC4; // GPIO Peripheral Configuration
        };

        struct Base {
            static constexpr uint32_t PORT_A     = 0x40004000; // GPIO Port A
            static constexpr uint32_t PORT_B     = 0x40005000; // GPIO Port B
            static constexpr uint32_t PORT_C     = 0x40006000; // GPIO Port C
            static constexpr uint32_t PORT_D     = 0x40007000; // GPIO Port D
            static constexpr uint32_t PORT_E     = 0x40024000; // GPIO Port E
            static constexpr uint32_t PORT_F     = 0x40025000; // GPIO Port F
            static constexpr uint32_t PORT_G     = 0x40026000; // GPIO Port G
            static constexpr uint32_t PORT_H     = 0x40027000; // GPIO Port H
            static constexpr uint32_t PORT_J     = 0x4003D000; // GPIO Port J
            static constexpr uint32_t PORT_A_AHB = 0x40058000; // GPIO Port A (high speed)
            static constexpr uint32_t PORT_B_AHB = 0x40059000; // GPIO Port B (high speed)
            static constexpr uint32_t PORT_C_AHB = 0x4005A000; // GPIO Port C (high speed)
            static constexpr uint32_t PORT_D_AHB = 0x4005B000; // GPIO Port D (high speed)
            static constexpr uint32_t PORT_E_AHB = 0x4005C000; // GPIO Port E (high speed)
            static constexpr uint32_t PORT_F_AHB = 0x4005D000; // GPIO Port F (high speed)
            static constexpr uint32_t PORT_G_AHB = 0x4005E000; // GPIO Port G (high speed)
            static constexpr uint32_t PORT_H_AHB = 0x4005F000; // GPIO Port H (high speed)
            static constexpr uint32_t PORT_J_AHB = 0x40060000; // GPIO Port J (high speed)
            static constexpr uint32_t PORT_K     = 0x40061000; // GPIO Port K
            static constexpr uint32_t PORT_L     = 0x40062000; // GPIO Port L
            static constexpr uint32_t PORT_M     = 0x40063000; // GPIO Port M
            static constexpr uint32_t PORT_N     = 0x40064000; // GPIO Port N
            static constexpr uint32_t PORT_P     = 0x40065000; // GPIO Port P
            static constexpr uint32_t PORT_Q     = 0x40066000; // GPIO Port Q
            static constexpr uint32_t PORT_R     = 0x40067000; // General-Purpose Input/Outputs
            static constexpr uint32_t PORT_S     = 0x40068000; // General-Purpose Input/Outputs
            static constexpr uint32_t PORT_T     = 0x40069000; // General-Purpose Input/Outputs
        };

        struct Strength {
            static constexpr uint32_t _2MA    = 0x00000001; // 2mA drive strength
            static constexpr uint32_t _4MA    = 0x00000002; // 4mA drive strength
            static constexpr uint32_t _6MA    = 0x00000065; // 6mA drive strength
            static constexpr uint32_t _8MA    = 0x00000066; // 8mA drive strength
            static constexpr uint32_t _8MA_SC = 0x0000006E; // 8mA drive with slew rate control
            static constexpr uint32_t _10MA   = 0x00000075; // 10mA drive strength
            static constexpr uint32_t _12MA   = 0x00000077; // 12mA drive strength
        };

        struct PinType {
            static constexpr uint32_t STD       = 0x00000008; // Push-pull
            static constexpr uint32_t STD_WPU   = 0x0000000A; // Push-pull with weak pull-up
            static constexpr uint32_t STD_WPD   = 0x0000000C; // Push-pull with weak pull-down
            static constexpr uint32_t OD        = 0x00000009; // Open-drain
            static constexpr uint32_t ANALOG    = 0x00000000; // Analog comparator
            static constexpr uint32_t WAKE_HIGH = 0x00000208; // Hibernate wake, high
            static constexpr uint32_t WAKE_LOW  = 0x00000108; // Hibernate wake, low
        };

        static constexpr uint32_t BASE_ADDRESSES[] = {
            Base::PORT_A, Base::PORT_A_AHB, Base::PORT_B, Base::PORT_B_AHB, Base::PORT_C, Base::PORT_C_AHB, Base::PORT_D, Base::PORT_D_AHB,
            Base::PORT_E, Base::PORT_E_AHB, Base::PORT_F, Base::PORT_F_AHB, Base::PORT_G, Base::PORT_G_AHB, Base::PORT_H, Base::PORT_H_AHB,
            Base::PORT_J, Base::PORT_J_AHB, Base::PORT_K, Base::PORT_K,     Base::PORT_L, Base::PORT_L,     Base::PORT_M, Base::PORT_M,
            Base::PORT_N, Base::PORT_N,     Base::PORT_P, Base::PORT_P,     Base::PORT_Q, Base::PORT_Q,     Base::PORT_R, Base::PORT_R,
            Base::PORT_S, Base::PORT_S,     Base::PORT_T, Base::PORT_T,
        };

        /// @brief Configure pin matrix routing
        /// @param route Pin configuration
        template<class... Args>
        static void ConfigurePinRouting(Args... routes) {
            _ConfigurePinRouting(
                [](uint32_t route) {
                    uint32_t base = (route >> 16) & 0xff;
                    if (__mem32(SystemControl::Register::GPIOHBCTL) & (1 << base)) {
                        base = BASE_ADDRESSES[(base << 1) + 1];
                    } else {
                        base = BASE_ADDRESSES[base << 1];
                    }
                    uint32_t shift                   = (route >> 8) & 0xff;
                    __mem32(base + BaseOffset::PCTL) = ((__mem32(base + BaseOffset::PCTL) & ~(0xf << shift)) | ((route & 0xf) << shift));
                },
                routes...);
        }

    private:
        /////////////////////////////////////
        // ConfigurePinRouting unroll helpers

        template<class F, class First, class... Rest>
        static void _ConfigurePinRouting(F f, First first, Rest... rest) {
            f(first);
            _ConfigurePinRouting(f, rest...);
        }

        template<class F>
        static void _ConfigurePinRouting(F f) {
        }
    };

    template<uint32_t BASE = 0, uint8_t PINS = 0, uint32_t SYSCTL = 0>
    class Static_GPIO {
        static constexpr uint32_t PIN_ACCESS_MASK = PINS << 2;

    public:
        constexpr Static_GPIO() = default;

        /// @brief Get GPIO port SystemControl address
        /// @return GPIO port SystemControl address
        constexpr uint32_t GetSystemControlAddress() const {
            return SYSCTL;
        }

        /// @brief Get GPIO port register base address
        /// @return GPIO port register base address
        constexpr uint32_t GetBase() const {
            return BASE;
        }

        /// @brief Get GPIO pins
        /// @return GPIO pins
        constexpr uint32_t GetPins() const {
            return PINS;
        }

        /// @brief Set or clear configured pin mask to configured port
        /// @param s true = set, false = clear
        constexpr void Write(bool s) const {
            if (s) {
                __mem32(BASE + GPIO::BaseOffset::DATA + PIN_ACCESS_MASK) = PINS;
            } else {
                __mem32(BASE + GPIO::BaseOffset::DATA + PIN_ACCESS_MASK) = 0;
            }
        }

        /// @brief Write pin mask to configured port
        /// @param val
        constexpr void DirectWrite(uint32_t mask) const {
            // TODO: check if 8bit access is faster than 32bit
            __mem32(BASE + GPIO::BaseOffset::DATA + PIN_ACCESS_MASK) = mask;
        }

        /// @brief Read configured GPIO port with pin mask
        /// @return GPIO port reading
        constexpr uint32_t DirectRead() const {
            // TODO: check if 8bit access is faster than 32bit
            return __mem32(BASE + GPIO::BaseOffset::DATA + PIN_ACCESS_MASK);
        }

        /// @brief Set pad configuration
        /// @param drive_strength output drive strength
        /// @param pin_type [GPIO::PinType] type of pad (normal, pull-up, pull-down, ...)
        void SetConfig(uint32_t drive_strength, uint32_t pin_type) const {
            // Set GPIO peripheral configuration register first as required
            for (uint8_t ui8Bit = 0; ui8Bit < 8; ui8Bit++) {
                if (PINS & (1 << ui8Bit)) {
                    __mem32(BASE + GPIO::BaseOffset::PC) = (__mem32(BASE + GPIO::BaseOffset::PC) & ~(0x3 << (2 * ui8Bit)));
                    __mem32(BASE + GPIO::BaseOffset::PC) |= (((drive_strength >> 5) & 0x3) << (2 * ui8Bit));
                }
            }

            // Set output drive strength
            __mem32(BASE + GPIO::BaseOffset::DR2R) = ((drive_strength & 1) ? (__mem32(BASE + GPIO::BaseOffset::DR2R) | PINS) :
                                                                             (__mem32(BASE + GPIO::BaseOffset::DR2R) & ~(PINS)));
            __mem32(BASE + GPIO::BaseOffset::DR4R) = ((drive_strength & 2) ? (__mem32(BASE + GPIO::BaseOffset::DR4R) | PINS) :
                                                                             (__mem32(BASE + GPIO::BaseOffset::DR4R) & ~(PINS)));
            __mem32(BASE + GPIO::BaseOffset::DR8R) = ((drive_strength & 4) ? (__mem32(BASE + GPIO::BaseOffset::DR8R) | PINS) :
                                                                             (__mem32(BASE + GPIO::BaseOffset::DR8R) & ~(PINS)));
            __mem32(BASE + GPIO::BaseOffset::SLR) =
                ((drive_strength & 8) ? (__mem32(BASE + GPIO::BaseOffset::SLR) | PINS) : (__mem32(BASE + GPIO::BaseOffset::SLR) & ~(PINS)));

            // Set 12-mA drive select register
            __mem32(BASE + GPIO::BaseOffset::DR12R) = ((drive_strength & 0x10) ? (__mem32(BASE + GPIO::BaseOffset::DR12R) | PINS) :
                                                                                 (__mem32(BASE + GPIO::BaseOffset::DR12R) & ~(PINS)));

            // Set pin type
            __mem32(BASE + GPIO::BaseOffset::ODR) =
                ((pin_type & 1) ? (__mem32(BASE + GPIO::BaseOffset::ODR) | PINS) : (__mem32(BASE + GPIO::BaseOffset::ODR) & ~(PINS)));
            __mem32(BASE + GPIO::BaseOffset::PUR) =
                ((pin_type & 2) ? (__mem32(BASE + GPIO::BaseOffset::PUR) | PINS) : (__mem32(BASE + GPIO::BaseOffset::PUR) & ~(PINS)));
            __mem32(BASE + GPIO::BaseOffset::PDR) =
                ((pin_type & 4) ? (__mem32(BASE + GPIO::BaseOffset::PDR) | PINS) : (__mem32(BASE + GPIO::BaseOffset::PDR) & ~(PINS)));
            __mem32(BASE + GPIO::BaseOffset::DEN) =
                ((pin_type & 8) ? (__mem32(BASE + GPIO::BaseOffset::DEN) | PINS) : (__mem32(BASE + GPIO::BaseOffset::DEN) & ~(PINS)));

            // Set wake pin enable register and wake level register
            __mem32(BASE + GPIO::BaseOffset::WAKELVL) = ((pin_type & 0x200) ? (__mem32(BASE + GPIO::BaseOffset::WAKELVL) | PINS) :
                                                                              (__mem32(BASE + GPIO::BaseOffset::WAKELVL) & ~(PINS)));
            __mem32(BASE + GPIO::BaseOffset::WAKEPEN) = ((pin_type & 0x300) ? (__mem32(BASE + GPIO::BaseOffset::WAKEPEN) | PINS) :
                                                                              (__mem32(BASE + GPIO::BaseOffset::WAKEPEN) & ~(PINS)));

            // Set analog mode select register
            __mem32(BASE + GPIO::BaseOffset::AMSEL) =
                ((pin_type == GPIO::PinType::ANALOG) ? (__mem32(BASE + GPIO::BaseOffset::AMSEL) | PINS) :
                                                       (__mem32(BASE + GPIO::BaseOffset::AMSEL) & ~(PINS)));
        }

        /// @brief Configure pins as inputs
        constexpr void ConfigureAsInput() const {
            __mem32(BASE + GPIO::BaseOffset::DIR) &= ~PINS;   // in
            __mem32(BASE + GPIO::BaseOffset::AFSEL) &= ~PINS; // i/o
            SetConfig(GPIO::Strength::_2MA, GPIO::PinType::STD);
        }

        /// @brief Configure pins as outputs
        constexpr void ConfigureAsOutput() const {
            SetConfig(GPIO::Strength::_2MA, GPIO::PinType::STD);
            __mem32(BASE + GPIO::BaseOffset::DIR) |= PINS;    // out
            __mem32(BASE + GPIO::BaseOffset::AFSEL) &= ~PINS; // i/o
        }

        /// @brief Configure pins to be hardware/peripheral controlled
        constexpr void ConfigureAsHardware() const {
            __mem32(BASE + GPIO::BaseOffset::DIR)   = __mem32(BASE + GPIO::BaseOffset::DIR) & ~PINS;
            __mem32(BASE + GPIO::BaseOffset::AFSEL) = __mem32(BASE + GPIO::BaseOffset::AFSEL) | PINS;
            SetConfig(GPIO::Strength::_2MA, GPIO::PinType::STD);
        }

        /// @brief Configure pins to be hardware/peripheral controlled
        constexpr void ConfigureAsAnalog() const {
            ConfigureAsInput();
            SetConfig(GPIO::Strength::_2MA, GPIO::PinType::ANALOG);
        }
    };

} // namespace CFXS::HW::TM4C