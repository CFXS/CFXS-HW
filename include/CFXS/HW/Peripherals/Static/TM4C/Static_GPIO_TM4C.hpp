// [CFXS] //
#pragma once
#include <CFXS/HW/Utils/TM4C/TemplateUtils.hpp>

#define DEF_STATIC_GPIO_TM4C(str) \
    CFXS::HW::Static::GPIO_TM4C<CFXS::HW::Utils::TM4C::GPIO::StringToBase(str), CFXS::HW::Utils::TM4C::GPIO::StringToPins(str)>

namespace CFXS::HW::Static {

    /// PinDefinitions - "A0"/"A0,1,2"
    template<uint32_t BASE, uint8_t PINS>
    class GPIO_TM4C {
        static constexpr uint32_t PIN_ACCESS_MASK = PINS << 2;
        static constexpr uint32_t GPIO_O_DATA     = 0x00000000; // GPIO Data
        static constexpr uint32_t GPIO_O_DIR      = 0x00000400; // GPIO Direction
        static constexpr uint32_t GPIO_O_IS       = 0x00000404; // GPIO Interrupt Sense
        static constexpr uint32_t GPIO_O_IBE      = 0x00000408; // GPIO Interrupt Both Edges
        static constexpr uint32_t GPIO_O_IEV      = 0x0000040C; // GPIO Interrupt Event
        static constexpr uint32_t GPIO_O_IM       = 0x00000410; // GPIO Interrupt Mask
        static constexpr uint32_t GPIO_O_RIS      = 0x00000414; // GPIO Raw Interrupt Status
        static constexpr uint32_t GPIO_O_MIS      = 0x00000418; // GPIO Masked Interrupt Status
        static constexpr uint32_t GPIO_O_ICR      = 0x0000041C; // GPIO Interrupt Clear
        static constexpr uint32_t GPIO_O_AFSEL    = 0x00000420; // GPIO Alternate Function Select
        static constexpr uint32_t GPIO_O_DR2R     = 0x00000500; // GPIO 2-mA Drive Select
        static constexpr uint32_t GPIO_O_DR4R     = 0x00000504; // GPIO 4-mA Drive Select
        static constexpr uint32_t GPIO_O_DR8R     = 0x00000508; // GPIO 8-mA Drive Select
        static constexpr uint32_t GPIO_O_ODR      = 0x0000050C; // GPIO Open Drain Select
        static constexpr uint32_t GPIO_O_PUR      = 0x00000510; // GPIO Pull-Up Select
        static constexpr uint32_t GPIO_O_PDR      = 0x00000514; // GPIO Pull-Down Select
        static constexpr uint32_t GPIO_O_SLR      = 0x00000518; // GPIO Slew Rate Control Select
        static constexpr uint32_t GPIO_O_DEN      = 0x0000051C; // GPIO Digital Enable
        static constexpr uint32_t GPIO_O_LOCK     = 0x00000520; // GPIO Lock
        static constexpr uint32_t GPIO_O_CR       = 0x00000524; // GPIO Commit
        static constexpr uint32_t GPIO_O_AMSEL    = 0x00000528; // GPIO Analog Mode Select
        static constexpr uint32_t GPIO_O_PCTL     = 0x0000052C; // GPIO Port Control
        static constexpr uint32_t GPIO_O_ADCCTL   = 0x00000530; // GPIO ADC Control
        static constexpr uint32_t GPIO_O_DMACTL   = 0x00000534; // GPIO DMA Control
        static constexpr uint32_t GPIO_O_SI       = 0x00000538; // GPIO Select Interrupt
        static constexpr uint32_t GPIO_O_DR12R    = 0x0000053C; // GPIO 12-mA Drive Select
        static constexpr uint32_t GPIO_O_WAKEPEN  = 0x00000540; // GPIO Wake Pin Enable
        static constexpr uint32_t GPIO_O_WAKELVL  = 0x00000544; // GPIO Wake Level
        static constexpr uint32_t GPIO_O_WAKESTAT = 0x00000548; // GPIO Wake Status
        static constexpr uint32_t GPIO_O_PP       = 0x00000FC0; // GPIO Peripheral Property
        static constexpr uint32_t GPIO_O_PC       = 0x00000FC4; // GPIO Peripheral Configuration

    public:
        /// @brief Set or clear configured pin mask to configured port
        /// @param s true = set, false = clear
        constexpr void Write(bool s) {
            if (s) {
                __mem32(BASE + GPIO_O_DATA + PIN_ACCESS_MASK) = PINS;
            } else {
                __mem32(BASE + GPIO_O_DATA + PIN_ACCESS_MASK) = 0;
            }
        }

        /// @brief Write pin mask to configured port
        /// @param val
        constexpr void DirectWrite(uint32_t mask) {
            // TODO: check if 8bit access is faster than 32bit
            __mem32(BASE + GPIO_O_DATA + PIN_ACCESS_MASK) = mask;
        }

        /// @brief Read configured GPIO port with pin mask
        /// @return GPIO port reading
        constexpr uint32_t DirectRead() {
            // TODO: check if 8bit access is faster than 32bit
            return __mem32(BASE + GPIO_O_DATA + PIN_ACCESS_MASK);
        }

        /// @brief Configure pins as inputs
        constexpr void ConfigureAsInput() {
            __mem32(BASE + GPIO_O_DIR)   = (__mem32(BASE + GPIO_O_DIR) & ~(PINS));   // in
            __mem32(BASE + GPIO_O_AFSEL) = (__mem32(BASE + GPIO_O_AFSEL) & ~(PINS)); // i/o
        }

        /// @brief Configure pins as outputs
        constexpr void ConfigureAsOutput() {
            __mem32(BASE + GPIO_O_DIR)   = (__mem32(BASE + GPIO_O_DIR) | PINS);      // out
            __mem32(BASE + GPIO_O_AFSEL) = (__mem32(BASE + GPIO_O_AFSEL) & ~(PINS)); // i/o
        }

        /// @brief Configure pins to be hardware/peripheral controlled
        constexpr void ConfigureAsHardware() {
            __mem32(BASE + GPIO_O_DIR)   = (__mem32(BASE + GPIO_O_DIR) & ~(PINS)); // in
            __mem32(BASE + GPIO_O_AFSEL) = (__mem32(BASE + GPIO_O_AFSEL) | PINS);  // hw
        }
    };

} // namespace CFXS::HW::Static