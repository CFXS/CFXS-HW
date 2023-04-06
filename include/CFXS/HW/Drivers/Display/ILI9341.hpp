// [CFXS] //
#pragma once
#include <CFXS/Platform/CPU.hpp>

namespace CFXS::HW {

    class _ILI9341_Base {
    public:
        enum Command : uint8_t {
            NOP                       = 0x00,
            SOFTWARE_RESET            = 0x01,
            SLEEP_OUT                 = 0x11,
            DISPLAY_OFF               = 0x28,
            DISPLAY_ON                = 0x29,
            COLUMN_ADDRESS_SET        = 0x2A,
            PAGE_ADDRESS_SET          = 0x2B,
            MEMORY_WRITE              = 0x2C,
            MEMORY_READ               = 0x2E,
            TEARING_EFFECT_LINE       = 0x35,
            MEMORY_ACCESS_CONTROL     = 0x36,
            IDLE_MODE_OFF             = 0x38,
            PIXEL_FORMAT_SET          = 0x3A,
            FRAME_CONTROL_NORMAL      = 0xB1,
            DISPLAY_FUNCTION_CONTROL  = 0xB6,
            POWER_CONTROL_1           = 0xC0,
            POWER_CONTROL_2           = 0xC1,
            VCON_CONTROL_1            = 0xC5,
            VCON_CONTROL_2            = 0xC7,
            POSITIVE_GAMMA_CORRECTION = 0xE0,
            NEGATIVE_GAMMA_CORRECTION = 0xE1,
            INTERFACE_CONTROL         = 0xF6
        };

        enum Gamma : uint8_t {
            _2_2 = 0x01,
            _1_8 = 0x02,
            _2_5 = 0x04,
            _1_0 = 0x08,
        };

        enum FrameRate : uint8_t {
            _119,
            _111,
            _105,
            _99,
            _94,
            _90,
            _86,
            _82,
            _78,
            _75,
            _72,
            _69,
            _67,
            _64,
            _62,
            _60,
            _58,
            _57,
            _55,
            _53,
            _52,
            _50,
            _49,
            _48,
            _46,
            _45,
            _44,
            _43,
            _42,
            _41,
            _40,
            _39
        };
    };

    template<typename PIN_DATA,
             typename PIN_RS,
             typename PIN_WR,
             typename PIN_RESET,
             uint16_t W,
             uint16_t H,
             uint16_t X_PANEL_OFFSET = 0,
             uint16_t Y_PANEL_OFFSET = 0>
    struct Interface_ILI9341_Parallel {
        static constexpr uint16_t WIDTH  = W;
        static constexpr uint16_t HEIGHT = H;

        constexpr void Initialize([[maybe_unused]] size_t clock_speed = 0) {
            PIN_DATA{}.ConfigureAsOutput();
            PIN_RS{}.ConfigureAsOutput();
            PIN_WR{}.ConfigureAsOutput();
            PIN_RESET{}.ConfigureAsOutput();

            PIN_RESET{}.Write(true);
            PIN_DATA{}.Write(false);
            PIN_RS{}.Write(false);
            PIN_WR{}.Write(false);

            // Reset
            PIN_RESET{}.Write(true);
            CFXS::CPU::BlockingMicroseconds(15);
            PIN_RESET{}.Write(false);
            CFXS::CPU::BlockingMicroseconds(15);
            PIN_RESET{}.Write(true);
            CFXS::CPU::BlockingMilliseconds(120);

            SendCommand(_ILI9341_Base::Command::INTERFACE_CONTROL);
            SendData(0x01); // (default)
            SendData(0x00); // (default)
            SendData(0x00); // (default)

            SendCommand(_ILI9341_Base::Command::DISPLAY_OFF);
            SendCommand(_ILI9341_Base::Command::SLEEP_OUT);

            // 65K 16bit
            SendCommand(_ILI9341_Base::Command::PIXEL_FORMAT_SET);
            SendData(0b01010101);

            SendCommand(_ILI9341_Base::Command::DISPLAY_ON);
        }

        constexpr void SendCommand(_ILI9341_Base::Command command) {
            PIN_DATA{}.DirectWrite(static_cast<size_t>(command));
            PIN_WR{}.Write(false);
            PIN_RS{}.Write(false);
            PIN_WR{}.Write(true);
            PIN_RS{}.Write(true);
        }

        constexpr void SendData(uint8_t data) {
            PIN_DATA{}.DirectWrite(data);
            PIN_WR{}.Write(false);
            PIN_WR{}.Write(true);
        }

        constexpr void SendData(uint8_t* data, size_t count) {
            for (int i = 0; i < count; i++) {
                SendData(data[i]);
            }
        }

        constexpr void SendData16(uint16_t data) {
            PIN_DATA{}.DirectWrite_SHIFTED_BASE(data);
            PIN_WR{}.Write(false);
            PIN_WR{}.Write(true);
            PIN_DATA{}.DirectWrite(data);
            PIN_WR{}.Write(false);
            PIN_WR{}.Write(true);
        }

        constexpr void SetRegion(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
            x += X_PANEL_OFFSET;
            y += Y_PANEL_OFFSET;
            SendCommand(_ILI9341_Base::Command::PAGE_ADDRESS_SET);
            SendData16(x);
            x += w - 1;
            SendData16(x);
            SendCommand(_ILI9341_Base::Command::COLUMN_ADDRESS_SET);
            SendData16(y);
            y += h - 1;
            SendData16(y);
            SendCommand(_ILI9341_Base::Command::MEMORY_WRITE);
        }

        constexpr void ClearColorRegion16(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
            SetRegion(x, y, w, h);
            for (int i = 0; i < w * h; i++) {
                SendData16(color);
            }
        }

        constexpr void SendColorRegion16(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* color_data) {
        }
    };

    template<typename INTERFACE>
    class ILI9341 {
    public:
        using Base = _ILI9341_Base;

        static constexpr uint16_t BASE_WIDTH  = INTERFACE::WIDTH;
        static constexpr uint16_t BASE_HEIGHT = INTERFACE::HEIGHT;

    public:
        static void Initialize(uint32_t clock_speed = 0) {
            INTERFACE{}.Initialize(clock_speed);
        }

        static void SendFrame16(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* color_data) {
            INTERFACE{}.SendColorRegion16(x, y, w, h, color_data);
        }

        static void ClearFrame16(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
            INTERFACE{}.ClearColorRegion16(x, y, w, h, color);
        }

        static void SetOrientation(bool swap_xy,
                                   bool flip_x,
                                   bool flip_y,
                                   bool refresh_left_to_right = true,
                                   bool refresh_top_to_bottom = true) {
            INTERFACE{}.SendCommand(_ILI9341_Base::Command::MEMORY_ACCESS_CONTROL);
            INTERFACE{}.SendData((flip_y ? 0x80 : 0) |                // BOTTOM TO TOP
                                 (flip_x ? 0x40 : 0) |                // RIGHT TO LEFT
                                 (swap_xy ? 0x20 : 0) |               // XY REVERSE
                                 (refresh_top_to_bottom ? 0 : 0x10) | // REFRESH BOTTOM TO TOP
                                 // 0x08 | // BRG
                                 (refresh_left_to_right ? 0 : 0x04) | // REFRESH RIGHT TO LEFT
                                 0);
        }

        static void SetFrameRate(_ILI9341_Base::FrameRate rate) {
        }

        static void Enable_VSYNC_Output(uint16_t scanline) {
        }

        static uint16_t GetWidth() {
            return INTERFACE::WIDTH;
        }

        static uint16_t GetHeight() {
            return INTERFACE::HEIGHT;
        }
    };

} // namespace CFXS::HW