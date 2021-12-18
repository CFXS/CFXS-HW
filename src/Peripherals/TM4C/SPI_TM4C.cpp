#ifdef CFXS_PLATFORM_TM4C
    #include <CFXS/HW/Peripherals/SPI.hpp>
    #include <CFXS/HW/Peripherals/Descriptors/TM4C/Desc_SPI_TM4C.hpp>
    #include <CFXS/HW/System/SystemControl_TM4C.hpp>
    #include <CFXS/Base/CPU.hpp>
    #include <driverlib/ssi.h>
    #include <inc/hw_ssi.h>
    #include <inc/hw_types.h>
    #include <CFXS/Base/Debug.hpp>

    #define _descriptor GetDescriptor<Desc_SPI>()

namespace CFXS::HW {

    using TM4C::Desc_SPI;
    using TM4C::SystemControl;

    /// Initialize SPI
    void SPI::Initialize() {
        GPIO clk(m_gpiodesc_CLK), miso(m_gpiodesc_MISO), mosi(m_gpiodesc_MOSI);
        clk.Initialize(GPIO::PinType::HARDWARE);
        miso.Initialize(GPIO::PinType::HARDWARE);
        mosi.Initialize(GPIO::PinType::HARDWARE);
        m_pin_CS.Initialize(GPIO::PinType::OUTPUT, 0xFFFFFFFF);

        SystemControl::EnablePeripheral(_descriptor->periph);
        Disable();
    }

    /// Configure SPI peripheral for master operation
    void SPI::ConfigureAsMaster(Mode mode, size_t bitrate, size_t dataWidth) {
        CFXS_ASSERT((CPU::CLOCK_FREQUENCY / bitrate) <= (254 * 256), "Invalid bitrate");
        CFXS_ASSERT((dataWidth >= 4) && (dataWidth <= 16), "invalid data width");

        uint32_t ssiProtocol;

        switch (mode) {
            case Mode::MODE_0: ssiProtocol = SSI_FRF_MOTO_MODE_0; break;
            case Mode::MODE_1: ssiProtocol = SSI_FRF_MOTO_MODE_1; break;
            case Mode::MODE_2: ssiProtocol = SSI_FRF_MOTO_MODE_2; break;
            case Mode::MODE_3: ssiProtocol = SSI_FRF_MOTO_MODE_3; break;
            default: CFXS_ASSERT(0, "Invalid mode"); return;
        }

        SSIConfigSetExpClk(_descriptor->base, CPU::CLOCK_FREQUENCY, ssiProtocol, SSI_MODE_MASTER, bitrate, dataWidth);
    }

    /// Enable SPI peripheral
    void SPI::Enable() {
        SSIEnable(_descriptor->base); //
    }

    /// Disable SPI peripheral
    void SPI::Disable() {
        SSIDisable(_descriptor->base); //
    }

    /// Write single data unit
    void SPI::Write(size_t data, bool waitUntilTransmitted) {
        SSIDataPut(_descriptor->base, data);
        if (waitUntilTransmitted)
            while (SSIBusy(_descriptor->base)) {}
    }

    /// Write multiple data units
    template<typename T>
    void SPI::Write(T* data, size_t count, bool waitUntilTransmitted) {
        while (count--) {
            SSIDataPut(_descriptor->base, *data++);
        }
        if (waitUntilTransmitted)
            while (SSIBusy(_descriptor->base)) {}
    }

    /// Read single data unit
    template<typename T>
    void SPI::Read(T* readTo) {
        while (!(HWREG(_descriptor->base + SSI_O_SR) & SSI_SR_RNE)) {}
        *readTo = HWREG(_descriptor->base + SSI_O_DR);
    }

    /// Is SPI busy
    bool SPI::IsBusy() { return SSIBusy(_descriptor->base); }

    /// Clear RX FIFO
    void SPI::Clear_RX_FIFO() {
        uint32_t temp;
        while (SSIDataGetNonBlocking(_descriptor->base, &temp)) {}
    }

    /// Is TX FIFO empty
    bool SPI::Is_TX_FIFO_Empty() {
        return (HWREG(_descriptor->base + SSI_O_SR) & SSI_SR_TFE); //
    }

    /// Is TX FIFO full
    bool SPI::Is_TX_FIFO_Full() {
        return !(HWREG(_descriptor->base + SSI_O_SR) & SSI_SR_TNF); //
    }

    /// Is RX FIFO empty
    bool SPI::Is_RX_FIFO_Empty() {
        return !(HWREG(_descriptor->base + SSI_O_SR) & SSI_SR_RNE); //
    }

    /// Is RX FIFO full
    bool SPI::Is_RX_FIFO_Full() {
        return (HWREG(_descriptor->base + SSI_O_SR) & SSI_SR_RFF); //
    }

    /// Get size of FIFO
    uint32_t SPI::Get_FIFO_Size() const {
        return 8; //
    }

    /// Set CS state
    void SPI::SetCS(bool state) {
        m_pin_CS.Write(state); //
    }

} // namespace CFXS::HW

    #undef _descriptor
#endif