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
#ifdef CFXS_PLATFORM_TM4C
    #include <CFXS/HW/Peripherals/GPIO.hpp>
    #include <CFXS/HW/Peripherals/Descriptors/TM4C/Desc_GPIO_TM4C.hpp>
    #include <CFXS/HW/System/SystemControl_TM4C.hpp>
    #include <CFXS/Base/Debug.hpp>
    #include <driverlib/gpio.h>
    #include <inc/hw_types.h>

    #define _descriptor GetDescriptor<Desc_GPIO>()

namespace CFXS::HW {

    using TM4C::Desc_GPIO;
    using TM4C::SystemControl;

    void GPIO::Initialize(PinType dir, size_t initialState) {
        CFXS_ASSERT(_descriptor, "Descriptor not set");
        CFXS_ASSERT(_descriptor->periph, "Invalid peripheral");
        CFXS_ASSERT(_descriptor->base, "Invalid base");
        CFXS_ASSERT(_descriptor->pins, "Invalid pins");

        // Descriptor holds only lower 2 bytes of periph (high bytes are always 0xF000)
        SystemControl::EnablePeripheral(0xF0000000 | _descriptor->periph, true);

        GPIOPadConfigSet(_descriptor->base,
                         _descriptor->pins,
                         _descriptor->driveCurrent ? _descriptor->driveCurrent : GPIO_STRENGTH_2MA,
                         _descriptor->pinType ? _descriptor->pinType : GPIO_PIN_TYPE_STD);

        if (_descriptor->pinConfig) {
            for (int i = 0; i < 8; i++) {
                GPIOPinConfigure(_descriptor->pinConfig[i]);
            }
        }

        m_PinType = dir;
        if (dir == PinType::INPUT || dir == PinType::ANALOG) {
            GPIODirModeSet(_descriptor->base, _descriptor->pins, GPIO_DIR_MODE_IN);
        } else if (dir == PinType::OUTPUT) {
            GPIODirModeSet(_descriptor->base, _descriptor->pins, GPIO_DIR_MODE_OUT);
            Write(initialState);
        } else if (dir == PinType::HARDWARE) {
            MakeHardwareControlled();
        }
    }

    /// Reconfigure specific GPIO settings from descriptor
    void GPIO::Reconfigure(ConfigParameter param, void* data) {
        CFXS_ASSERT(0, "Not implemented");
    }

    /// Set GPIO type
    void GPIO::SetPinType(PinType dir) {
        m_PinType = dir;

        if (dir == PinType::INPUT || dir == PinType::ANALOG) {
            GPIODirModeSet(_descriptor->base, _descriptor->pins, GPIO_DIR_MODE_IN);
        } else if (dir == PinType::OUTPUT) {
            GPIODirModeSet(_descriptor->base, _descriptor->pins, GPIO_DIR_MODE_OUT);
        } else if (dir == PinType::HARDWARE) {
            MakeHardwareControlled();
        }
    }

    /// Map to peripheral
    /// Hardware controlled
    void GPIO::MakeHardwareControlled() {
        GPIODirModeSet(_descriptor->base, _descriptor->pins, GPIO_DIR_MODE_HW); //
    }

    /// Map to GPIO
    /// User controlled
    void GPIO::MakeUserControlled() {
        SetPinType(m_PinType); //
    }

    /// Read data from GPIO
    size_t GPIO::Read() const {
        return HWREGB(_descriptor->accessAddress); //
    }

    /// Write data to GPIO
    void GPIO::Write(size_t data) {
        HWREGB(_descriptor->accessAddress) = data; //
    }

    /// Write data to GPIO
    void GPIO::Write(bool data) {
        HWREGB(_descriptor->accessAddress) = data ? 0xFFFFFFFF : 0; //
    }

} // namespace CFXS::HW

    #undef _descriptor
#endif