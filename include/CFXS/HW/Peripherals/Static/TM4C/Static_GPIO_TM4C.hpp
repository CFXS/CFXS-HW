// [CFXS] //
#pragma once
#include <CFXS/HW/Utils/TM4C/TemplateUtils.hpp>

#define DEF_STATIC_GPIO_TM4C(str) \
    CFXS::HW::Static::GPIO_TM4C<CFXS::HW::Utils::TM4C::GPIO::StringToBase(str), CFXS::HW::Utils::TM4C::GPIO::StringToPins(str)>

namespace CFXS::HW::Static {

    /// PinDefinitions - "A0"/"A0,1,2"
    template<uint32_t BASE, uint32_t PINS>
    class GPIO_TM4C {
        static constexpr size_t PIN_ACCESS_MASK = PINS << 2;

    public:
        /// @brief Set or clear configured pin mask to configured port
        /// @param s true = set, false = clear
        constexpr void Write(bool s) {
            if (s) {
                __mem32(BASE + PIN_ACCESS_MASK) = PINS;
            } else {
                __mem32(BASE + PIN_ACCESS_MASK) = 0;
            }
        }

        /// @brief Write pin mask to configured port
        /// @param val
        constexpr void DirectWrite(uint32_t mask) {
            __mem32(BASE + PIN_ACCESS_MASK) = mask;
        }

        /// @brief Read configured GPIO port with pin mask
        /// @return GPIO port reading
        constexpr uint32_t DirectRead() {
            return __mem32(BASE + PIN_ACCESS_MASK);
        }
    };

} // namespace CFXS::HW::Static