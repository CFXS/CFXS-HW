#pragma once

namespace CFXS::HW {

#pragma pack(1)
    struct CommandHeader_ADAU146X {
        bool read : 1;
        uint8_t chipAddress : 7;
        uint16_t subAddress;
        uint8_t data[];
    };
#pragma pack()

    enum class Regs_ADAU146X : uint16_t {
        DATA_SAFELOAD0     = 0x6000, // Safeload Data Slot 0
        DATA_SAFELOAD1     = 0x6001, // Safeload Data Slot 1
        DATA_SAFELOAD2     = 0x6002, // Safeload Data Slot 2
        DATA_SAFELOAD3     = 0x6003, // Safeload Data Slot 3
        DATA_SAFELOAD4     = 0x6004, // Safeload Data Slot 4
        ADDRESS_SAFELOAD   = 0x6005, // Target address for safeload transfer
        NUM_SAFELOAD_LOWER = 0x6006,
        NUM_SAFELOAD_UPPER = 0x6007,

        PLL_CTRL0              = 0xF000, // PLL Feedback Divider
        PLL_CTRL1              = 0xF001, // PLL Prescale Divider
        PLL_CLK_SRC            = 0xF002, // PLL Clock Source
        PLL_ENABLE             = 0xF003, // PLL Enable
        PLL_LOCK               = 0xF004, // PLL Lock
        MCLK_OUT               = 0xF005, // CLKOUT Control
        PLL_WATCHDOG           = 0xF006, // Analog PLL Watchdog Control
        DISABLE_AUTOLOCK       = 0xF00A, // Disable PLL autolock?
        CLK_GEN1_M             = 0xF020, // Denominator (M) for Clock Generator 1
        CLK_GEN1_N             = 0xF021, // Numerator (N) for Clock Generator 1
        CLK_GEN2_M             = 0xF022, // Denominator (M) for Clock Generator 2
        CLK_GEN2_N             = 0xF023, // Numerator (N) for Clock Generator 2
        CLK_GEN3_M             = 0xF024, // Denominator (M) for Clock Generator 3
        CLK_GEN3_N             = 0xF025, // Numerator (N) for Clock Generator 3
        CLK_GEN3_SRC           = 0xF026, // Input Reference for Clock Generator 3
        CLK_GEN3_LOCK          = 0xF027, // Lock Bit for Clock Generator 3 Input Reference
        POWER_ENABLE0          = 0xF050, // Power Enable 0
        POWER_ENABLE1          = 0xF051, // Power Enable 1
        ASRC_INPUTx            = 0xF100, // ASRC Input Selector
        ASRC_OUT_RATEx         = 0xF140, // ASRC Output Rate Selector
        SOUT_SOURCEx           = 0xF180, // Source of Data for Serial Output Ports
        SPDIFTX_INPUT          = 0xF1C0, // S/PDIF Transmitter Data Selector
        SERIAL_BYTE_x_0        = 0xF200, // Serial Port Control 0
        SERIAL_BYTE_x_1        = 0xF201, // Serial Port Control 1
        FTDM_INx               = 0xF300, // FTDM Mapping for the Serial Inputs
        FTDM_OUTx              = 0xF380, // FTDM Mapping for the Serial Outputs
        HIBERNATE              = 0xF400, // Hibernate Setting
        START_PULSE            = 0xF401, // Start Pulse Selection
        START_CORE             = 0xF402, // Instruction to Start the Core
        KILL_CORE              = 0xF403, // Instruction to Stop the Core
        START_ADDRESS          = 0xF404, // Start Address of the Program
        CORE_STATUS            = 0xF405, // Core Status
        DEBUG_MODE             = 0xF420, // Debug Mode
        PANIC_CLEAR            = 0xF421, // Clear the Panic Manager
        PANIC_PARITY_MASK      = 0xF422, // Panic Parity
        PANIC_SOFTWARE_MASK    = 0xF423, // Software Panic
        PANIC_WD_MASK          = 0xF424, // Watchdog Panic
        PANIC_STACK_MASK       = 0xF425, // Stack Panic
        PANIC_LOOP_MASK        = 0xF426, // Loop Panic
        PANIC_FLAG             = 0xF427, // Panic Flag
        PANIC_CODE             = 0xF428, // Panic Code
        DECODE_OP0             = 0xF429, //?
        DECODE_OP1             = 0xF42A, //?
        DECODE_OP2             = 0xF42B, //?
        DECODE_OP3             = 0xF42C, //?
        EXECUTE_OP0            = 0xF42D, //?
        EXECUTE_OP1            = 0xF42E, //?
        EXECUTE_OP2            = 0xF42F, //?
        EXECUTE_OP3            = 0xF430, //?
        DECODE_COUNT           = 0xF431, //?
        EXECUTE_COUNT          = 0xF432, // Execute Stage Error Program Count
        SOFTWARE_VALUE_0       = 0xF433, // Software Panic Value 0
        SOFTWARE_VALUE_1       = 0xF434, // Software Panic Value 1
        WATCHDOG_MAXCOUNT      = 0xF443, // Watchdog Maximum Count
        WATCHDOG_PRESCALE      = 0xF444, // Watchdog Prescale
        BLOCKINT_EN            = 0xF450, // Enable Block Interrupts
        BLOCKINT_VALUE         = 0xF451, // Value for the Block Interrupt Counter
        PROG_CNTR0             = 0xF460, // Program Counter, Bits[23:16]
        PROG_CNTR1             = 0xF461, // Program Counter, Bits[15:0]
        PROG_CNTR_CLEAR        = 0xF462, // Program Counter Clear
        PROG_CNTR_LENGTH0      = 0xF463, // Program Counter Length, Bits[23:16]
        PROG_CNTR_LENGTH1      = 0xF464, // Program Counter Length, Bits[15:0]
        PROG_CNTR_MAXLENGTH0   = 0xF465, // Program Counter Maximum Length, Bits[23:16]
        PROG_CNTR_MAXLENGTH1   = 0xF466, // Program Counter Maximum Length, Bits[15:0]
        PANIC_PARITY_MASK1     = 0xF467, // Panic Mask Parity DM0 Bank [1:0]
        PANIC_PARITY_MASK2     = 0xF468, // Panic Mask Parity DM0 Bank [3:2]
        PANIC_PARITY_MASK3     = 0xF469, // Panic Mask Parity DM1 Bank [1:0]
        PANIC_PARITY_MASK4     = 0xF46A, // Panic Mask Parity DM1 Bank [3:2]
        PANIC_PARITY_MASK5     = 0xF46B, // Panic Mask Parity PM Bank [1:0]
        PANIC_CODE1            = 0xF46C, // Panic Parity Error DM0 Bank [1:0]
        PANIC_CODE2            = 0xF46D, // Panic Parity Error DM0 Bank [3:2]
        PANIC_CODE3            = 0xF46E, // Panic Parity Error DM1 Bank [1:0]
        PANIC_CODE4            = 0xF46F, // Panic Parity Error DM1 Bank [3:2]
        PANIC_CODE5            = 0xF470, // Panic Parity Error PM Bank [1:0]
        MPx_MODE               = 0xF510, // Multipurpose Pin Mode
        MPx_WRITE              = 0xF520, // Multipurpose Pin Write Value
        MPx_READ               = 0xF530, // Multipurpose Pin Read Value
        DMIC_CTRL0             = 0xF560, // Digital PDM Microphone Control 0
        DMIC_CTRL1             = 0xF561, // Digital PDM Microphone Control 1
        ASRC_LOCK              = 0xF580, // ASRC Lock Status
        ASRC_MUTE              = 0xF581, // ASRC Mute
        ASRCx_RATIO            = 0xF582, // ASRC Ratio
        ASRC_RAMPMAX_OVR       = 0xF590, // RAMPMAX Override
        ASRCx_RAMPMAX          = 0xF591, // ASRCx RAMPMAX
        ADC_READx              = 0xF5A0, // Auxiliary ADC Read Value
        SPDIF_LOCK_DET         = 0xF600, // S/PDIF Receiver Lock Bit Detection
        SPDIF_RX_CTRL          = 0xF601, // S/PDIF Receiver Control
        SPDIF_RX_DECODE        = 0xF602, // Decoded Signals From the S/PDIF Receiver
        SPDIF_RX_COMPTRMODE    = 0xF603, // Compression Mode From the S/PDIF Receiver
        SPDIF_RESTART          = 0xF604, // Automatically Resume S/PDIF Receiver Audio Input
        SPDIF_LOSS_OF_LOCK     = 0xF605, // S/PDIF Receiver Loss of Lock Detection
        SPDIF_RX_MCLKSPEED     = 0xF606, //?
        SPDIF_TX_MCLKSPEED     = 0xF607, //?
        SPDIF_AUX_EN           = 0xF608, // S/PDIF Receiver Auxiliary Outputs Enable
        SPDIF_RX_AUXBIT_READY  = 0xF60F, // S/PDIF Receiver Auxiliary Bits Ready Flag
        SPDIF_RX_CS_LEFT_x     = 0xF610, // S/PDIF Receiver Channel Status Bits (Left)
        SPDIF_RX_CS_RIGHT_x    = 0xF620, // S/PDIF Receiver Channel Status Bits (Right)
        SPDIF_RX_UD_LEFT_x     = 0xF630, // S/PDIF Receiver User Data Bits (Left)
        SPDIF_RX_UD_RIGHT_x    = 0xF640, // S/PDIF Receiver User Data Bits (Right)
        SPDIF_RX_VB_LEFT_x     = 0xF650, // S/PDIF Receiver Validity Bits (Left)
        SPDIF_RX_VB_RIGHT_x    = 0xF660, // S/PDIF Receiver Validity Bits (Right)
        SPDIF_RX_PB_LEFT_x     = 0xF670, // S/PDIF Receiver Parity Bits (Left)
        SPDIF_RX_PB_RIGHT_x    = 0xF680, // S/PDIF Receiver Parity Bits (Right)
        SPDIF_TX_EN            = 0xF690, // S/PDIF Transmitter Enable
        SPDIF_TX_CTRL          = 0xF691, // S/PDIF Transmitter Control
        SPDIF_TX_AUXBIT_SOURCE = 0xF69F, // S/PDIF Transmitter Auxiliary Bits Source Select
        SPDIF_TX_CS_LEFT_x     = 0xF6A0, // S/PDIF Transmitter Channel Status Bits (Left)
        SPDIF_TX_CS_RIGHT_x    = 0xF6B0, // S/PDIF Transmitter Channel Status Bits (Right)
        SPDIF_TX_UD_LEFT_x     = 0xF6C0, // S/PDIF Transmitter User Data Bits (Left)
        SPDIF_TX_UD_RIGHT_x    = 0xF6D0, // S/PDIF Transmitter User Data Bits (Right)
        SPDIF_TX_VB_LEFT_x     = 0xF6E0, // S/PDIF Transmitter Validity Bits (Left)
        SPDIF_TX_VB_RIGHT_x    = 0xF6F0, // S/PDIF Transmitter Validity Bits (Right)
        SPDIF_TX_PB_LEFT_x     = 0xF700, // S/PDIF Transmitter Parity Bits (Left)
        SPDIF_TX_PB_RIGHT_x    = 0xF710, // S/PDIF Transmitter Parity Bits (Right)
        BCLK_INx_PIN           = 0xF780, // BCLK Input Pins Drive Strength and Slew Rate
        BCLK_OUTx_PIN          = 0xF784, // BCLK Output Pins Drive Strength and Slew Rate
        LRCLK_INx_PIN          = 0xF788, // LRCLK Input Pins Drive Strength and Slew Rate
        LRCLK_OUTx_PIN         = 0xF78C, // LRCLK Output Pins Drive Strength and Slew Rate
        SDATA_INx_PIN          = 0xF790, // SDATA Input Pins Drive Strength and Slew Rate
        SDATA_OUTx_PIN         = 0xF794, // SDATA Output Pins Drive Strength and Slew Rate
        SPDIF_TX_PIN           = 0xF798, // S/PDIF Transmitter Pin Drive Strength and Slew Rate
        SCLK_SCL_PIN           = 0xF799, // SCLK/SCL Pin Drive Strength and Slew Rate
        MISO_SDA_PIN           = 0xF79A, // MISO/SDA Pin Drive Strength and Slew Rate
        SS_PIN                 = 0xF79B, // SS/ADDR0 Pin Drive Strength and Slew Rate
        MOSI_ADDR1_PIN         = 0xF79C, // MOSI/ADDR1 Pin Drive Strength and Slew Rate
        SCLK_SCL_M_PIN         = 0xF79D, // SCL_M/SCLK_M/MP2 Pin Drive Strength and Slew Rate
        MISO_SDA_M_PIN         = 0xF79E, // SDA_M/MISO_M/MP3 Pin Drive Strength and Slew Rate
        SS_M_PIN               = 0xF79F, // SS_M/MP0 Pin Drive Strength and Slew Rate
        MOSI_M_PIN             = 0xF7A0, // MOSI_M/MP1 Pin Drive Strength and Slew Rate
        MP6_PIN                = 0xF7A1, // MP6 Pin Drive Strength and Slew Rate
        MP7_PIN                = 0xF7A2, // MP7 Pin Drive Strength and Slew Rate
        CLKOUT_PIN             = 0xF7A3, // CLKOUT Pin Drive Strength and Slew Rate
        SECONDPAGE_ENABLE      = 0xF899, //?
        SOFT_RESET             = 0xF890, // Soft Reset
    };

} // namespace CFXS::HW
