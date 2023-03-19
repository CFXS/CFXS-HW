// [CFXS] //
#pragma once
#include <CFXS/HW/Utils/TM4C/TemplateUtils.hpp>

namespace CFXS::HW::Static {

    template<size_t Peripheral, size_t Base, size_t Pins>
    class GPIO_TM4C {
    public:
        static constexpr size_t PERIPHERAL = Peripheral;
        static constexpr size_t BASE       = Base;
        static constexpr size_t PINS       = Pins;

    private:
        static constexpr size_t PIN_ACCESS_MASK = Pins << 2;
    };

} // namespace CFXS::HW::Static