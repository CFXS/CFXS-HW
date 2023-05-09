#include <CFXS/HW/System/SystemControl_TM4C.hpp>
#include <Common/ETH_Debug.hpp>
#include <CFXS/Base/MAC_Address.hpp>
// driverlib includes in precompiled header (CFXS Hardware CMakeLists)

using CFXS::DebugLevel;
using CFXS::HW::TM4C::SystemControl;

#ifndef CFXS_HW_ETHERNET_PHY_ADDRESS
    #define CFXS_HW_ETHERNET_PHY_ADDRESS 0
#endif

#ifndef CFXS_HW_ETHERNET_DEFAULT_MAC_ADDRESS
    #define CFXS_HW_ETHERNET_DEFAULT_MAC_ADDRESS "02:00:00:00:00:01"
#endif

void __cfxs_initialize_module_Ethernet() {
    CFXS_ETH_printf(DebugLevel::INFO, "Initialize module (TM4C129)\n");

    // Enable EMAC
    SystemControl::EnablePeripheral(SystemControl::Peripheral::EMAC0);

#ifdef CFXS_HW_ETHERNET_INTERNAL_PHY
    // Enable PHY if in INTERNAL PHY mode
    SystemControl::EnablePeripheral(SystemControl::Peripheral::EPHY0);
#endif

    EMACAddrSet(EMAC0_BASE, CFXS_HW_ETHERNET_PHY_ADDRESS, CFXS::MAC_Address{CFXS_HW_ETHERNET_DEFAULT_MAC_ADDRESS}.GetDataPointer());

    char buf[32];
    CFXS::MAC_Address{CFXS_HW_ETHERNET_DEFAULT_MAC_ADDRESS}.PrintTo(buf, 32);
    CFXS_ETH_printf(DebugLevel::TRACE, "%s\n", buf);
}