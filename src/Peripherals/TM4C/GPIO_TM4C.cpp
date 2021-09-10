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

    void GPIO::Initialize(Direction dir, size_t initialState) {
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

        m_Direction = dir;
        if (dir == Direction::INPUT) {
            GPIODirModeSet(GET_DESCRIPTOR()->base, GET_DESCRIPTOR()->pins, GPIO_DIR_MODE_IN);
        } else if (dir == Direction::OUTPUT) {
            GPIODirModeSet(GET_DESCRIPTOR()->base, GET_DESCRIPTOR()->pins, GPIO_DIR_MODE_OUT);
            Write(initialState);
        } else if (dir == Direction::HARDWARE) {
            MakeHardwareControlled();
        }
    }

    /// Reconfigure specific GPIO settings from descriptor
    void GPIO::Reconfigure(ConfigParameter param, void* data) { CFXS_ASSERT(0, "Not implemented"); }

    /// Set GPIO direction
    void GPIO::SetDirection(Direction dir) {
        m_Direction = dir;

        if (dir == Direction::INPUT) {
            GPIODirModeSet(GET_DESCRIPTOR()->base, GET_DESCRIPTOR()->pins, GPIO_DIR_MODE_IN);
        } else if (dir == Direction::OUTPUT) {
            GPIODirModeSet(GET_DESCRIPTOR()->base, GET_DESCRIPTOR()->pins, GPIO_DIR_MODE_OUT);
        } else if (dir == Direction::HARDWARE) {
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
        SetDirection(m_Direction); //
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
#endif