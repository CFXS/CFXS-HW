// [CFXS] //
#pragma once
#include <CFXS/HW/Peripherals/SPI.hpp>
#include <CFXS/HW/Utils/TM4C/TemplateUtils.hpp>
#include <CFXS/HW/System/SystemControl_TM4C.hpp>
#include <initializer_list>

namespace CFXS::HW::TM4C {

    class SPI {
    public:
        struct BaseOffset {
            static constexpr uint32_t CR0    = 0x00000000; // SSI Control 0
            static constexpr uint32_t CR1    = 0x00000004; // SSI Control 1
            static constexpr uint32_t DR     = 0x00000008; // SSI Data
            static constexpr uint32_t SR     = 0x0000000C; // SSI Status
            static constexpr uint32_t CPSR   = 0x00000010; // SSI Clock Prescale
            static constexpr uint32_t IM     = 0x00000014; // SSI Interrupt Mask
            static constexpr uint32_t RIS    = 0x00000018; // SSI Raw Interrupt Status
            static constexpr uint32_t MIS    = 0x0000001C; // SSI Masked Interrupt Status
            static constexpr uint32_t ICR    = 0x00000020; // SSI Interrupt Clear
            static constexpr uint32_t DMACTL = 0x00000024; // SSI DMA Control
            static constexpr uint32_t PP     = 0x00000FC0; // SSI Peripheral Properties
            static constexpr uint32_t CC     = 0x00000FC8; // SSI Clock Configuration
        };

        struct Register {
            struct CR0 {
                static constexpr uint32_t SCR_M    = 0x0000FF00; // SSI Serial Clock Rate
                static constexpr uint32_t SPH      = 0x00000080; // SSI Serial Clock Phase
                static constexpr uint32_t SPO      = 0x00000040; // SSI Serial Clock Polarity
                static constexpr uint32_t FRF_M    = 0x00000030; // SSI Frame Format Select
                static constexpr uint32_t FRF_MOTO = 0x00000000; // Freescale SPI Frame Format
                static constexpr uint32_t FRF_TI   = 0x00000010; // Synchronous Serial Frame Format
                static constexpr uint32_t FRF_NMW  = 0x00000020; // MICROWIRE Frame Format
                static constexpr uint32_t DSS_M    = 0x0000000F; // SSI Data Size Select
                static constexpr uint32_t DSS_4    = 0x00000003; // 4-bit data
                static constexpr uint32_t DSS_5    = 0x00000004; // 5-bit data
                static constexpr uint32_t DSS_6    = 0x00000005; // 6-bit data
                static constexpr uint32_t DSS_7    = 0x00000006; // 7-bit data
                static constexpr uint32_t DSS_8    = 0x00000007; // 8-bit data
                static constexpr uint32_t DSS_9    = 0x00000008; // 9-bit data
                static constexpr uint32_t DSS_10   = 0x00000009; // 10-bit data
                static constexpr uint32_t DSS_11   = 0x0000000A; // 11-bit data
                static constexpr uint32_t DSS_12   = 0x0000000B; // 12-bit data
                static constexpr uint32_t DSS_13   = 0x0000000C; // 13-bit data
                static constexpr uint32_t DSS_14   = 0x0000000D; // 14-bit data
                static constexpr uint32_t DSS_15   = 0x0000000E; // 15-bit data
                static constexpr uint32_t DSS_16   = 0x0000000F; // 16-bit data
                static constexpr uint32_t SCR_S    = 8;
            };

            struct CR1 {
                static constexpr uint32_t EOM           = 0x00000800; // Stop Frame (End of Message)
                static constexpr uint32_t FSSHLDFRM     = 0x00000400; // FSS Hold Frame
                static constexpr uint32_t HSCLKEN       = 0x00000200; // High Speed Clock Enable
                static constexpr uint32_t DIR           = 0x00000100; // SSI Direction of Operation
                static constexpr uint32_t MODE_M        = 0x000000C0; // SSI Mode
                static constexpr uint32_t MODE_LEGACY   = 0x00000000; // Legacy SSI mode
                static constexpr uint32_t MODE_BI       = 0x00000040; // Bi-SSI mode
                static constexpr uint32_t MODE_QUAD     = 0x00000080; // Quad-SSI Mode
                static constexpr uint32_t MODE_ADVANCED = 0x000000C0; // Advanced SSI Mode with 8-bit packet size
                static constexpr uint32_t EOT           = 0x00000010; // End of Transmission
                static constexpr uint32_t MS            = 0x00000004; // SSI Master/Slave Select
                static constexpr uint32_t SSE           = 0x00000002; // SSI Synchronous Serial Port Enable
                static constexpr uint32_t LBM           = 0x00000001; // SSI Loopback Mode
            };

            struct SR {
                static constexpr uint32_t BSY = 0x00000010; // SSI Busy Bit
                static constexpr uint32_t RFF = 0x00000008; // SSI Receive FIFO Full
                static constexpr uint32_t RNE = 0x00000004; // SSI Receive FIFO Not Empty
                static constexpr uint32_t TNF = 0x00000002; // SSI Transmit FIFO Not Full
                static constexpr uint32_t TFE = 0x00000001; // SSI Transmit FIFO Empty
            };
        };
    };

    template<size_t PERIPH_INDEX, typename CLOCK, typename MOSI, typename MISO, typename CS>
    class Static_SPI {
        static_assert(PERIPH_INDEX < 4, "Peripheral index too big");

    public:
        constexpr Static_SPI() = default;

        constexpr void Initialize() const {
            switch (PERIPH_INDEX) {
                case 0: SystemControl::EnablePeripheral(0xF0001C00); break;
                case 1: SystemControl::EnablePeripheral(0xF0001C01); break;
                case 2: SystemControl::EnablePeripheral(0xF0001C02); break;
                case 3: SystemControl::EnablePeripheral(0xF0001C03); break;
            }

            CLOCK{}.ConfigureAsHardware();
            MOSI{}.ConfigureAsHardware();
            MISO{}.ConfigureAsHardware();
            CS{}.ConfigureAsOutput();
            CS{}.Write(true);
            Disable();
        }

        /// @brief Configure SPI in master mode
        /// @param mode phase/polarity
        /// @param speed bitrate
        /// @param bits bits per transfer word
        void ConfigureAsMaster(CFXS::HW::SPI::Mode mode, size_t speed, size_t bits) const {
            bool is_enabled = IsEnabled();
            if (is_enabled)
                Disable();
            uint32_t ui32MaxBitRate;
            uint32_t ui32RegVal;
            uint32_t ui32PreDiv;
            uint32_t ui32SCR;
            uint32_t ui32SPH_SPO;

            // CFXS_ASSERT((CFXS::CPU::CLOCK_FREQUENCY / speed) <= (254 * 256), "Bit rate too high");
            // CFXS_ASSERT((bits >= 4) && (bits <= 16), "Invalid data size");

            // Set mode
            ui32RegVal                                = 0; // Master mode
            __mem32(GetBase() + SPI::BaseOffset::CR1) = ui32RegVal;

            // Set clock predivider
            ui32MaxBitRate = CFXS::CPU::CLOCK_FREQUENCY / speed;
            ui32PreDiv     = 0;
            do {
                ui32PreDiv += 2;
                ui32SCR = (ui32MaxBitRate / ui32PreDiv) - 1;
            } while (ui32SCR > 255);
            __mem32(GetBase() + SPI::BaseOffset::CPSR) = ui32PreDiv;

            // Set protocol and clock rate
            ui32SPH_SPO = (((size_t)mode) & 3) << 6;
            ui32RegVal  = (ui32SCR << 8) | ui32SPH_SPO | (((size_t)mode) & SPI::Register::CR0::FRF_M) | (bits - 1);
            __mem32(GetBase() + SPI::BaseOffset::CR0) = ui32RegVal;
            if (is_enabled)
                Enable();
        }

        /// @brief Enable SPI
        constexpr void Enable() const { __mem32(GetBase() + SPI::BaseOffset::CR1) |= SPI::Register::CR1::SSE; }

        /// @brief Disable SPI
        constexpr void Disable() const { __mem32(GetBase() + SPI::BaseOffset::CR1) &= ~SPI::Register::CR1::SSE; }

        constexpr bool IsEnabled() const { return __mem32(GetBase() + SPI::BaseOffset::CR1) & SPI::Register::CR1::SSE; }

        /// @brief Set chipselect level
        /// @param state true = high
        constexpr void SetCS(bool state) const { CS{}.Write(state); }

        /// @brief Read all data from RX FIFO
        constexpr void Clear_RX_FIFO() const {
            uint32_t temp;
            while (NonBlockingRead(&temp)) {
            }
        }

        /// @brief Write data to SPI
        /// @param data data to write
        /// @param waitUntilTransmitted block until transfer finished
        constexpr void Write(uint32_t data) const {
            while (Is_TX_FIFO_Full()) {
            }

            __mem32(GetBase() + SPI::BaseOffset::DR) = data;
        }

        /// @brief Write multiple bytes to SPI interface
        /// @param data data array to write
        /// @param len number of bytes to write
        /// @param waitUntilTransmitted block until transfer finished
        constexpr void Write(uint8_t* data, size_t len) const {
            for (size_t i = 0; i < len; i++) {
                Write(data[i]);
            }
        }

        /// @brief Write multiple words to SPI interface, unrolled at compile time
        /// @param ...args words to write
        template<class... Args>
        constexpr void WriteList(Args... args) {
            _WriteList(
                [this](auto arg) constexpr { Write(arg); }, args...);
        }

        /// @brief Read data from FIFO
        /// @param read write data to here
        constexpr void Read(uint8_t* readTo) const {
            while (Is_RX_FIFO_Empty()) {
            }
            *readTo = __mem32(GetBase() + SPI::BaseOffset::DR);
        }

        /// @brief Attempt to read data from FIFO
        /// @param read write data to here
        /// @return true if data read, false if no data was available
        constexpr bool NonBlockingRead(uint32_t* read) const {
            if (__mem32(GetBase() + SPI::BaseOffset::SR) & SPI::Register::SR::RNE) {
                *read = __mem32(GetBase() + SPI::BaseOffset::DR);
                return true;
            } else {
                return false;
            }
        }

        /// @brief Check if SPI TX FIFO is full
        /// @return true if FIFO full
        constexpr bool Is_TX_FIFO_Full() const { return !(__mem32(GetBase() + SPI::BaseOffset::SR) & SPI::Register::SR::TNF); }

        /// @brief Check if SPI RX FIFO is empty
        /// @return true if empty
        constexpr bool Is_RX_FIFO_Empty() const { return !(__mem32(GetBase() + SPI::BaseOffset::SR) & SPI::Register::SR::RNE); }

        /// @brief Check if SPI busy (transfer in progress)
        /// @return true if busy
        constexpr bool IsBusy() const { return (__mem32(GetBase() + SPI::BaseOffset::SR) & SPI::Register::SR::BSY) ? true : false; }

        /// @brief Wait for transfer to finish
        constexpr void WaitForTransferFinished() {
            while (IsBusy()) {
            }
        }

    private:
        /// @brief Get SPI peripheral base address
        /// @return SPI peripheral base address
        constexpr uint32_t GetBase() const {
            switch (PERIPH_INDEX) {
                case 0: return 0x40008000;
                case 1: return 0x40009000;
                case 2: return 0x4000A000;
                case 3: return 0x4000B000;
            }
        }

        /////////////////////////////////////
        // Write list unroll helpers

        template<class F, class First, class... Rest>
        constexpr void _WriteList(F f, First first, Rest... rest) {
            f(first);
            _WriteList(f, rest...);
        }

        template<class F>
        constexpr void _WriteList(F f) {}
    };

} // namespace CFXS::HW::TM4C