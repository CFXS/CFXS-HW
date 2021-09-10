#pragma once

namespace CFXS::HW {

    class GPIO {
    public:
        enum class ConfigParameter {
            INTERRUPT, // interrupts enabled/disabled, handler, priority, edge
        };

        enum class Direction { INPUT, OUTPUT, HIGH_Z, HARDWARE };

    public:
        template<typename T>
        constexpr GPIO(T* descriptor = nullptr) : m_Descriptor((const void*)descriptor) {}

        /// Initialize GPIO
        void Initialize(Direction dir = Direction::INPUT, size_t initialState = 0);

        /// Set descriptor
        template<typename T>
        void SetDescriptor(T* descriptor = nullptr) {
            m_Descriptor = (void*)descriptor;
        }

        /// Reconfigure specific GPIO settings
        void Reconfigure(ConfigParameter param, void* data);

        /// Set GPIO direction
        void SetDirection(Direction dir);

        /// Map to peripheral
        /// Hardware controlled
        void MakeHardwareControlled();

        /// Map to GPIO
        /// User controlled
        void MakeUserControlled();

        /// Read data from GPIO
        size_t Read() const;

        /// Write data to GPIO
        void Write(size_t data);
        void Write(bool state);

        /// Toggle data on GPIO (bit invert)
        void Toggle() { Write(~Read()); }

        template<typename T>
        const T* GetDescriptor() const {
            return static_cast<const T*>(m_Descriptor);
        }

    private:
        const void* m_Descriptor = nullptr;
        Direction m_Direction;
    };

} // namespace CFXS::HW