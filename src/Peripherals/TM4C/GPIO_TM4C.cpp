#ifdef CFXS_PLATFORM_TM4C
    #include <CFXS/HW/Peripherals/GPIO.hpp>
    #include <CFXS/HW/Peripherals/Descriptors/TM4C/Desc_GPIO_TM4C.hpp>
    #include <CFXS/HW/System/SystemControl_TM4C.hpp>
    #include <CFXS/Base/Debug.hpp>
    #include <driverlib/gpio.h>

    #define GET_DESCRIPTOR() GetDescriptor<Desc_GPIO>()

namespace CFXS::HW {

    using TM4C::Desc_GPIO;
    using TM4C::SystemControl;

    void GPIO::Initialize(PinType dir, size_t initialState) {
        CFXS_ASSERT(GET_DESCRIPTOR(), "Descriptor not set");
        CFXS_ASSERT(GET_DESCRIPTOR()->periph, "Invalid peripheral");
        CFXS_ASSERT(GET_DESCRIPTOR()->base, "Invalid base");
        CFXS_ASSERT(GET_DESCRIPTOR()->pins, "Invalid pins");

        SystemControl::EnablePeripheral(GET_DESCRIPTOR()->periph, true);

        GPIOPadConfigSet(GET_DESCRIPTOR()->base,
                         GET_DESCRIPTOR()->pins,
                         GET_DESCRIPTOR()->driveCurrent ? GET_DESCRIPTOR()->driveCurrent : GPIO_STRENGTH_2MA,
                         GET_DESCRIPTOR()->pinType ? GET_DESCRIPTOR()->pinType : GPIO_PIN_TYPE_STD);

        for (int i = 0; i < 8; i++) {
            if (GET_DESCRIPTOR()->pinConfig[i])
                GPIOPinConfigure(GET_DESCRIPTOR()->pinConfig[i]);
            else
                break;
        }

        m_PinType = dir;
        if (dir == PinType::INPUT || dir == PinType::ANALOG) {
            GPIODirModeSet(GET_DESCRIPTOR()->base, GET_DESCRIPTOR()->pins, GPIO_DIR_MODE_IN);
        } else if (dir == PinType::OUTPUT) {
            GPIODirModeSet(GET_DESCRIPTOR()->base, GET_DESCRIPTOR()->pins, GPIO_DIR_MODE_OUT);
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
            GPIODirModeSet(GET_DESCRIPTOR()->base, GET_DESCRIPTOR()->pins, GPIO_DIR_MODE_IN);
        } else if (dir == PinType::OUTPUT) {
            GPIODirModeSet(GET_DESCRIPTOR()->base, GET_DESCRIPTOR()->pins, GPIO_DIR_MODE_OUT);
        } else if (dir == PinType::HARDWARE) {
            MakeHardwareControlled();
        }
    }

    /// Map to peripheral
    /// Hardware controlled
    void GPIO::MakeHardwareControlled() {
        GPIODirModeSet(GET_DESCRIPTOR()->base, GET_DESCRIPTOR()->pins, GPIO_DIR_MODE_HW); //
    }

    /// Map to GPIO
    /// User controlled
    void GPIO::MakeUserControlled() {
        SetPinType(m_PinType); //
    }

    /// Read data from GPIO
    size_t GPIO::Read() const {
        return GPIOPinRead(GET_DESCRIPTOR()->base, GET_DESCRIPTOR()->pins); //
    }

    /// Write data to GPIO
    void GPIO::Write(size_t data) {
        GPIOPinWrite(GET_DESCRIPTOR()->base, GET_DESCRIPTOR()->pins, data); //
    }

    /// Write data to GPIO
    void GPIO::Write(bool data) {
        GPIOPinWrite(GET_DESCRIPTOR()->base, GET_DESCRIPTOR()->pins, data ? GET_DESCRIPTOR()->pins : 0); //
    }

} // namespace CFXS::HW

    #undef GET_DESCRIPTOR
#endif