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
#pragma once

namespace CFXS::HW {

    class GPIO {
    public:
        enum class ConfigParameter {
            INTERRUPT, // interrupts enabled/disabled, handler, priority, edge
        };

        enum class PinType { INPUT, OUTPUT, HIGH_Z, HARDWARE, ANALOG };

    public:
        template<typename T>
        constexpr GPIO(T* descriptor = nullptr) : m_Descriptor((const void*)descriptor) {
        }

        /// Initialize GPIO
        void Initialize(PinType dir = PinType::INPUT, size_t initialState = 0);

        /// Reconfigure specific GPIO settings
        void Reconfigure(ConfigParameter param, void* data);

        /// Set GPIO type
        void SetPinType(PinType dir);

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
        void Toggle() {
            Write(~Read());
        }

        /// Set descriptor
        template<typename T>
        void SetDescriptor(T* descriptor = nullptr) {
            m_Descriptor = (void*)descriptor;
        }

        /// Get descriptor as type T*
        template<typename T>
        const T* GetDescriptor() const {
            return static_cast<const T*>(m_Descriptor);
        }

    private:
        const void* m_Descriptor = nullptr;
        PinType m_PinType;
    };

} // namespace CFXS::HW