#ifdef CFXS_PLATFORM_TM4C
    #include <CFXS/HW/Peripherals/GPIO.hpp>
    #include <CFXS/HW/Peripherals/Descriptors/TM4C/Desc_GPIO_TM4C.hpp>
    #include <CFXS/HW/System/SystemControl_TM4C.hpp>
    #include <CFXS/Base/Debug.hpp>
    #include <driverlib/gpio.h>

    #define _descriptor GetDescriptor<Desc_GPIO>()

namespace CFXS::HW {

    using TM4C::Desc_GPIO;
    using TM4C::SystemControl;

    void GPIO::Initialize(PinType dir, size_t initialState) {
        CFXS_ASSERT(_descriptor, "Descriptor not set");
        CFXS_ASSERT(_descriptor->periph, "Invalid peripheral");
        CFXS_ASSERT(_descriptor->base, "Invalid base");
        CFXS_ASSERT(_descriptor->pins, "Invalid pins");

        // Descriptor hold only lower 2 bytes of periph (high bytes are always 0xF000)
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
    void GPIO::Reconfigure(ConfigParameter param, void* data) { CFXS_ASSERT(0, "Not implemented"); }

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
        return GPIOPinRead(_descriptor->base, _descriptor->pins); //
    }

    /// Write data to GPIO
    void GPIO::Write(size_t data) {
        GPIOPinWrite(_descriptor->base, _descriptor->pins, data); //
    }

    /// Write data to GPIO
    void GPIO::Write(bool data) {
        GPIOPinWrite(_descriptor->base, _descriptor->pins, data ? _descriptor->pins : 0); //
    }

} // namespace CFXS::HW

    #undef _descriptor
#endif