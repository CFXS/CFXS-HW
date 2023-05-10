#pragma once

#ifndef CFXS_HW_ETHERNET_PHY_ADDRESS
    #ifdef CFXS_HW_ETHERNET_INTERNAL_PHY
        #define CFXS_HW_ETHERNET_PHY_ADDRESS 0
    #else
        #define CFXS_HW_ETHERNET_PHY_ADDRESS 1
    #endif
#endif

#ifndef CFXS_HW_ETHERNET_TX_DESCRIPTOR_COUNT
    #define CFXS_HW_ETHERNET_TX_DESCRIPTOR_COUNT 16
#endif

#ifndef CFXS_HW_ETHERNET_RX_DESCRIPTOR_COUNT
    #define CFXS_HW_ETHERNET_RX_DESCRIPTOR_COUNT 24
#endif