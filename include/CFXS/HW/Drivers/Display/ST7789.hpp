// [CFXS] //
#pragma once
#include <CFXS/HW/Peripherals/SPI.hpp>

namespace CFXS::HW {

    class ST7789 {
    public:
        enum Command : uint8_t {
            NOP                        = 0x00,
            RESET                      = 0x01,
            SLEEP_OFF                  = 0x11,
            DISPLAY_OFF                = 0x28,
            DISPLAY_ON                 = 0x29,
            INVERT_OFF                 = 0x20,
            INVERT_ON                  = 0x21,
            SET_GAMMA                  = 0x26,
            FRAMERATE_CONTROL_2        = 0xC6,
            COL_ADDRESS_SET            = 0x2A,
            ROW_ADDRESS_SET            = 0x2B,
            MEMORY_WRITE               = 0x2C,
            INTERFACE_PIXEL_FORMAT     = 0x3A,
            TEARING_EFFECT_OFF         = 0x34,
            TEARING_EFFECT_ON          = 0x35,
            MEMORY_DATA_ACCESS_CONTROL = 0x36,
            SET_TEAR_SCANLINE          = 0x44,
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

    template<typename SPI,
             typename RESET_PIN,
             typename DCSEL_PIN,
             uint16_t W,
             uint16_t H,
             uint16_t X_PANEL_OFFSET = 0,
             uint16_t Y_PANEL_OFFSET = 0>
    struct Interface_ST7899_4PIN {
        static constexpr uint16_t WIDTH  = W;
        static constexpr uint16_t HEIGHT = H;
        constexpr void Initialize(uint32_t clock_speed) {
            SPI{}.Initialize();
            SPI{}.ConfigureAsMaster(CFXS::HW::SPI::Mode::MODE_0, clock_speed, 8);
            SPI{}.Enable();
            SPI{}.SetCS(true);

            DCSEL_PIN{}.ConfigureAsOutput();
            DCSEL_PIN{}.Write(false);

            RESET_PIN{}.ConfigureAsOutput();
            RESET_PIN{}.Write(false);
            CFXS::CPU::BlockingMilliseconds(1);
            RESET_PIN{}.Write(true);
            CFXS::CPU::BlockingMilliseconds(1);
            SPI{}.SetCS(false);
            CFXS::CPU::BlockingMicroseconds(1);
            DCSEL_PIN{}.Write(false);

            SPI{}.Write(ST7789::Command::RESET);
            SPI{}.WaitForTransferFinished();
            SPI{}.SetCS(true);
            CFXS::CPU::BlockingMilliseconds(1);
            SPI{}.SetCS(false);
            SPI{}.WriteList(ST7789::Command::SLEEP_OFF,
                            ST7789::Command::NOP,
                            ST7789::Command::DISPLAY_ON,
                            ST7789::Command::NOP,
                            ST7789::Command::INVERT_ON,
                            ST7789::Command::NOP);
            SPI{}.WaitForTransferFinished();

            SendCommand(ST7789::Command::INTERFACE_PIXEL_FORMAT);
            SendData(0b01010101); // 65K 16bit

            SendCommand(ST7789::Command::SET_GAMMA);
            SendData(ST7789::Gamma::_2_2);
        }

        constexpr void SendCommand(ST7789::Command command) {
            SPI{}.WaitForTransferFinished();
            DCSEL_PIN{}.Write(false);
            SPI{}.Write(command);
        }

        constexpr void SendData(uint8_t data) {
            SPI{}.WaitForTransferFinished();
            DCSEL_PIN{}.Write(true);
            SPI{}.Write(data);
        }

        constexpr void SendData(uint8_t* data, size_t count) {
            DCSEL_PIN{}.Write(true);
            for (int i = 0; i < count; i++) {
                SPI{}.Write(*data++);
            }
        }

        constexpr void SetRegion(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
            x += X_PANEL_OFFSET;
            y += Y_PANEL_OFFSET;
            SendCommand(ST7789::Command::COL_ADDRESS_SET);
            SPI{}.WaitForTransferFinished();
            DCSEL_PIN{}.Write(true);
            SPI{}.WriteList(x >> 8, x);
            x += w - 1;
            SPI{}.WriteList(x >> 8, x);
            SendCommand(ST7789::Command::ROW_ADDRESS_SET);
            SPI{}.WaitForTransferFinished();
            DCSEL_PIN{}.Write(true);
            SPI{}.WriteList(y >> 8, y);
            y += h - 1;
            SPI{}.WriteList(y >> 8, y);
        }

        constexpr void ClearColorRegion16(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
            SetRegion(x, y, w, h);
            SendCommand(ST7789::Command::MEMORY_WRITE);

            uint8_t a = color >> 8;
            uint8_t b = color;
            SPI{}.WaitForTransferFinished();
            DCSEL_PIN{}.Write(true);
            for (size_t i = 0; i < w * h; i++) {
                SPI{}.Write(a);
                SPI{}.Write(b);
            }
        }

        constexpr void SendColorRegion16(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* color_data) {
            SetRegion(x, y, w, h);
            SendCommand(ST7789::Command::MEMORY_WRITE);

            SPI{}.WaitForTransferFinished();
            DCSEL_PIN{}.Write(true);
            uint8_t* data8 = (uint8_t*)color_data;
            SPI{}.Write(data8, w * h * 2);
        }
    };

    template<typename INTERFACE>
    class ST7789_Static {
    public:
        using Base = ST7789;

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
            INTERFACE{}.SendCommand(ST7789::Command::MEMORY_DATA_ACCESS_CONTROL);
            INTERFACE{}.SendData((flip_y ? 0x80 : 0) |                // BOTTOM TO TOP
                                 (flip_x ? 0x40 : 0) |                // RIGHT TO LEFT
                                 (swap_xy ? 0x20 : 0) |               // XY REVERSE
                                 (refresh_top_to_bottom ? 0 : 0x10) | // REFRESH BOTTOM TO TOP
                                 // 0x08 | // BRG
                                 (refresh_left_to_right ? 0 : 0x04) | // REFRESH RIGHT TO LEFT
                                 0);
        }

        static void SetFrameRate(ST7789::FrameRate rate) {
            INTERFACE{}.SendCommand(ST7789::Command::FRAMERATE_CONTROL_2);
            INTERFACE{}.SendData(rate);
        }

        static void Enable_VSYNC_Output(uint16_t scanline) {
            INTERFACE{}.SendCommand(ST7789::Command::TEARING_EFFECT_ON);
            INTERFACE{}.SendCommand(ST7789::Command::SET_TEAR_SCANLINE);
            INTERFACE{}.SendData(scanline >> 8);
            INTERFACE{}.SendData(scanline);
        }

        static uint16_t GetWidth() {
            return INTERFACE::WIDTH;
        }

        static uint16_t GetHeight() {
            return INTERFACE::HEIGHT;
        }
    };

} // namespace CFXS::HW