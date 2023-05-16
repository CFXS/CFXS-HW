#include <lwipopts.h>
#include "lwip/priv/tcp_priv.h"
#include "lwip/priv/raw_priv.h"
#include "lwip/priv/mem_priv.h"
#include "lwiplib.h"
#include "netif/etharp.h"
#include "lwip/igmp.h"
#include "lwip/init.h"
#include "lwip/ip4_frag.h"
#include "lwip/ip_addr.h"
#include "lwip/dhcp.h"
#include "_defines.hpp"

//*****************************************************************************
//
// Ensure that ICMP checksum offloading is enabled; otherwise the TM4C129
// driver will not operate correctly.
//
//*****************************************************************************
#ifndef LWIP_OFFLOAD_ICMP_CHKSUM
    #define LWIP_OFFLOAD_ICMP_CHKSUM 1
#endif

extern struct netif e_Main_Network_Interface;
//*****************************************************************************
//
// Include lwIP high-level API code.
//
//*****************************************************************************
//#include "lwip/src/api/api_lib.c"
//#include "lwip/src/api/api_msg.c"
//#include "lwip/src/api/err.c"
//#include "lwip/src/api/netbuf.c"
//#include "lwip/src/api/netdb.c"
//#include "lwip/src/api/netifapi.c"
//#include "lwip/src/api/sockets.c"
//#include "lwip/src/api/tcpip.c"

//*****************************************************************************
//
// Include the core lwIP TCP/IP stack code.
//
//*****************************************************************************
//#include "lwip/src/core/def.c"
//#include "lwip/src/core/dhcp.c"
//#include "lwip/src/core/dns.c"
//#include "lwip/src/core/init.c"
//#include "lwip/src/core/mem.c"
//#include "lwip/src/core/memp.c"
//#include "lwip/src/core/netif.c"
//#include "lwip/src/core/pbuf.c"
//#include "lwip/src/core/raw.c"
//#include "lwip/src/core/stats.c"
//#include "lwip/src/core/sys.c"
//#include "lwip/src/core/tcp.c"
//#include "lwip/src/core/tcp_in.c"
//#include "lwip/src/core/tcp_out.c"
//#include "lwip/src/core/timers.c"
//#include "lwip/src/core/udp.c"

//*****************************************************************************
//
// Include the IPV4 code.
//
//*****************************************************************************
//#include "lwip/src/core/ipv4/autoip.c"
//#include "lwip/src/core/ipv4/icmp.c"
//#include "lwip/src/core/ipv4/igmp.c"
//#include "lwip/src/core/ipv4/inet.c"
//#include "lwip/src/core/ipv4/inet_chksum.c"
//#include "lwip/src/core/ipv4/ip.c"
//#include "lwip/src/core/ipv4/ip_addr.c"
//#include "lwip/src/core/ipv4/ip_frag.c"

//*****************************************************************************
//
// Include the network interface code.
//
//*****************************************************************************
//#include "lwip/src/netif/etharp.c"

//*****************************************************************************
//
// Include the network interface PPP code.
//
//*****************************************************************************
//#include "lwip/src/netif/ppp/auth.c"
//#include "lwip/src/netif/ppp/chap.c"
//#include "lwip/src/netif/ppp/chpms.c"
//#include "lwip/src/netif/ppp/fsm.c"
//#include "lwip/src/netif/ppp/ipcp.c"
//#include "lwip/src/netif/ppp/lcp.c"
//#include "lwip/src/netif/ppp/magic.c"
//#include "lwip/src/netif/ppp/md5.c"
//#include "lwip/src/netif/ppp/pap.c"
//#include "lwip/src/netif/ppp/ppp.c"
//#include "lwip/src/netif/ppp/ppp_oe.c"
//#include "lwip/src/netif/ppp/randm.c"
//#include "lwip/src/netif/ppp/vj.c"

//*****************************************************************************
//
// Include Tiva-specific lwIP interface/porting layer code.
//
//*****************************************************************************
//#include "lwip/ports/tiva-tm4c129/perf.c"
//#include "lwip/ports/tiva-tm4c129/sys_arch.c"
//#include "lwip/ports/tiva-tm4c129/netif/tiva-tm4c129.c"

//*****************************************************************************
//
//! \addtogroup lwiplib_api
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// The lwIP Library abstration layer provides for a host callback function to
// be called periodically in the lwIP context.  This is the timer interval, in
// ms, for this periodic callback.  If the timer interval is defined to 0 (the
// default value), then no periodic host callback is performed.
//
//*****************************************************************************
#ifndef HOST_TMR_INTERVAL
    #define HOST_TMR_INTERVAL 0
#else
extern void lwIPHostTimerHandler(void);
#endif

//*****************************************************************************
//
// The link detect polling interval.
//
//*****************************************************************************
#define LINK_TMR_INTERVAL 10

//*****************************************************************************
//
// Set the PHY configuration to the default (internal) option if necessary.
//
//*****************************************************************************
#ifndef EMAC_PHY_CONFIG
    #define EMAC_PHY_CONFIG (EMAC_PHY_TYPE_INTERNAL | EMAC_PHY_INT_MDIX_EN | EMAC_PHY_AN_100B_T_FULL_DUPLEX)
#endif

//*****************************************************************************
//
// Driverlib headers needed for this library module.
//
//*****************************************************************************
#if !NO_SYS
    #if RTOS_FREERTOS
        #include "FreeRTOS.h"
        #include "task.h"
        #include "queue.h"
        #include "semphr.h"
    #endif
    #if ((RTOS_FREERTOS) < 1)
        #error No RTOS is defined.  Please define an RTOS.
    #endif
    #if ((RTOS_FREERTOS) > 1)
        #error More than one RTOS defined.  Please define only one RTOS at a time.
    #endif
#endif

//*****************************************************************************
//
// The lwIP network interface structure for the Tiva Ethernet MAC.
//
//*****************************************************************************

//*****************************************************************************
//
// The application's interrupt handler for hardware timer events from the MAC.
//
//*****************************************************************************
tHardwareTimerHandler g_pfnTimerHandler;

//*****************************************************************************
//
// The local time for the lwIP Library Abstraction layer, used to support the
// Host and lwIP periodic callback functions.
//
//*****************************************************************************
#if NO_SYS
uint32_t g_ui32LocalTimer = 0;
#endif

//*****************************************************************************
//
// The local time when the TCP timer was last serviced.
//
//*****************************************************************************
#if NO_SYS
static uint32_t g_ui32TCPTimer = 0;
#endif

//*****************************************************************************
//
// The local time when the HOST timer was last serviced.
//
//*****************************************************************************
#if NO_SYS && HOST_TMR_INTERVAL
static uint32_t g_ui32HostTimer = 0;
#endif

//*****************************************************************************
//
// The local time when the ARP timer was last serviced.
//
//*****************************************************************************
#if NO_SYS && LWIP_ARP
static uint32_t g_ui32ARPTimer = 0;
#endif

//*****************************************************************************
//
// The local time when the AutoIP timer was last serviced.
//
//*****************************************************************************
#if NO_SYS && LWIP_AUTOIP
static uint32_t g_ui32AutoIPTimer = 0;
#endif

//*****************************************************************************
//
// The local time when the DHCP Coarse timer was last serviced.
//
//*****************************************************************************
#if NO_SYS && LWIP_DHCP
static uint32_t g_ui32DHCPCoarseTimer = 0;
#endif

//*****************************************************************************
//
// The local time when the DHCP Fine timer was last serviced.
//
//*****************************************************************************
#if NO_SYS && LWIP_DHCP
static uint32_t g_ui32DHCPFineTimer = 0;
#endif

//*****************************************************************************
//
// The local time when the IP Reassembly timer was last serviced.
//
//*****************************************************************************
#if NO_SYS && IP_REASSEMBLY
static uint32_t g_ui32IPReassemblyTimer = 0;
#endif

//*****************************************************************************
//
// The local time when the IGMP timer was last serviced.
//
//*****************************************************************************
#if NO_SYS && LWIP_IGMP
static uint32_t g_ui32IGMPTimer = 0;
#endif

//*****************************************************************************
//
// The local time when the DNS timer was last serviced.
//
//*****************************************************************************
#if NO_SYS && LWIP_DNS
static uint32_t g_ui32DNSTimer = 0;
#endif

//*****************************************************************************
//
// The local time when the link detect timer was last serviced.
//
//*****************************************************************************
#if NO_SYS && (LWIP_AUTOIP || LWIP_DHCP)
static uint32_t g_ui32LinkTimer = 0;
#endif

//*****************************************************************************
//
// The default IP address acquisition mode.
//
//*****************************************************************************
static uint32_t g_ui32IPMode = IPADDR_USE_STATIC;

//*****************************************************************************
//
// The most recently detected link state.
//
//*****************************************************************************
#if LWIP_AUTOIP || LWIP_DHCP
static bool g_bLinkActive = false;
#endif

//*****************************************************************************
//
// The IP address to be used.  This is used during the initialization of the
// stack and when the interface configuration is changed.
//
//*****************************************************************************
static uint32_t g_ui32IPAddr;

//*****************************************************************************
//
// The netmask to be used.  This is used during the initialization of the stack
// and when the interface configuration is changed.
//
//*****************************************************************************
static uint32_t g_ui32NetMask;

//*****************************************************************************
//
// The gateway address to be used.  This is used during the initialization of
// the stack and when the interface configuration is changed.
//
//*****************************************************************************
static uint32_t g_ui32GWAddr;

//*****************************************************************************
//
// The stack size for the interrupt task.
//
//*****************************************************************************
#if !NO_SYS
    #define STACKSIZE_LWIPINTTASK 128
#endif

//*****************************************************************************
//
// The handle for the "queue" (semaphore) used to signal the interrupt task
// from the interrupt handler.
//
//*****************************************************************************
#if !NO_SYS
static xQueueHandle g_pInterrupt;
#endif

//*****************************************************************************
//
// This task handles reading packets from the Ethernet controller and supplying
// them to the TCP/IP thread.
//
//*****************************************************************************
#if !NO_SYS
static void lwIPInterruptTask(void *pvArg) {
    //
    // Loop forever.
    //
    while (1) {
        //
        // Wait until the semaphore has been signaled.
        //
        while (xQueueReceive(g_pInterrupt, &pvArg, portMAX_DELAY) != pdPASS) {
        }

        //
        // Processes any packets waiting to be sent or received.
        //
        tivaif_interrupt(&e_Main_Network_Interface, (uint32_t)pvArg);

        //
        // Re-enable the Ethernet interrupts.
        //
        MAP_EMACIntEnable(
            EMAC0_BASE,
            (EMAC_INT_RECEIVE | EMAC_INT_TRANSMIT | EMAC_INT_TX_STOPPED | EMAC_INT_RX_NO_BUFFER | EMAC_INT_RX_STOPPED | EMAC_INT_PHY));
    }
}
#endif

//*****************************************************************************
//
// This function performs a periodic check of the link status and responds
// appropriately if it has changed.
//
//*****************************************************************************
#if LWIP_AUTOIP || LWIP_DHCP
static void lwIPLinkDetect(void) {
    bool bHaveLink;
    struct ip4_addr ip_addr;
    struct ip4_addr net_mask;
    struct ip4_addr gw_addr;

    //
    // See if there is an active link.
    //
    bHaveLink = EMACPHYRead(EMAC0_BASE, CFXS_HW_ETHERNET_PHY_ADDRESS, EPHY_BMSR) & EPHY_BMSR_LINKSTAT;

    //
    // Return without doing anything else if the link state hasn't changed.
    //
    if (bHaveLink == g_bLinkActive) {
        return;
    }

    //
    // Save the new link state.
    //
    g_bLinkActive = bHaveLink;

    //
    // Clear any address information from the network interface.
    //
    ip_addr.addr  = 0;
    net_mask.addr = 0;
    gw_addr.addr  = 0;
    netif_set_addr(&e_Main_Network_Interface, &ip_addr, &net_mask, &gw_addr);

    //
    // See if there is a link now.
    //
    if (bHaveLink) {
        //
        // Start DHCP, if enabled.
        //
    #if LWIP_DHCP
        if (g_ui32IPMode == IPADDR_USE_DHCP) {
            dhcp_start(&e_Main_Network_Interface);
        }
    #endif

        //
        // Start AutoIP, if enabled and DHCP is not.
        //
    #if LWIP_AUTOIP
        if (g_ui32IPMode == IPADDR_USE_AUTOIP) {
            autoip_start(&e_Main_Network_Interface);
        }
    #endif
    } else {
        //
        // Stop DHCP, if enabled.
        //
    #if LWIP_DHCP
        if (g_ui32IPMode == IPADDR_USE_DHCP) {
            dhcp_stop(&e_Main_Network_Interface);
        }
    #endif

        //
        // Stop AutoIP, if enabled and DHCP is not.
        //
    #if LWIP_AUTOIP
        if (g_ui32IPMode == IPADDR_USE_AUTOIP) {
            autoip_stop(&e_Main_Network_Interface);
        }
    #endif
    }
}
#endif

//*****************************************************************************
//
// This function services all of the lwIP periodic timers, including TCP and
// Host timers.  This should be called from the lwIP context, which may be
// the Ethernet interrupt (in the case of a non-RTOS system) or the lwIP
// thread, in the event that an RTOS is used.
//
//*****************************************************************************
#if NO_SYS
static void lwIPServiceTimers(void) {
    //
    // Service the host timer.
    //
    #if HOST_TMR_INTERVAL
    if ((g_ui32LocalTimer - g_ui32HostTimer) >= HOST_TMR_INTERVAL) {
        g_ui32HostTimer = g_ui32LocalTimer;
        lwIPHostTimerHandler();
    }
    #endif

    //
    // Service the ARP timer.
    //
    #if LWIP_ARP
    if ((g_ui32LocalTimer - g_ui32ARPTimer) >= ARP_TMR_INTERVAL) {
        g_ui32ARPTimer = g_ui32LocalTimer;
        etharp_tmr();
    }
    #endif

    //
    // Service the TCP timer.
    //
    #if LWIP_TCP
    if ((g_ui32LocalTimer - g_ui32TCPTimer) >= TCP_TMR_INTERVAL) {
        g_ui32TCPTimer = g_ui32LocalTimer;
        tcp_tmr();
    }
    #endif

    //
    // Service the AutoIP timer.
    //
    #if LWIP_AUTOIP
    if ((g_ui32LocalTimer - g_ui32AutoIPTimer) >= AUTOIP_TMR_INTERVAL) {
        g_ui32AutoIPTimer = g_ui32LocalTimer;
        autoip_tmr();
    }
    #endif

    //
    // Service the DCHP Coarse Timer.
    //
    #if LWIP_DHCP
    if ((g_ui32LocalTimer - g_ui32DHCPCoarseTimer) >= DHCP_COARSE_TIMER_MSECS) {
        g_ui32DHCPCoarseTimer = g_ui32LocalTimer;
        dhcp_coarse_tmr();
    }
    #endif

    //
    // Service the DCHP Fine Timer.
    //
    #if LWIP_DHCP
    if ((g_ui32LocalTimer - g_ui32DHCPFineTimer) >= DHCP_FINE_TIMER_MSECS) {
        g_ui32DHCPFineTimer = g_ui32LocalTimer;
        dhcp_fine_tmr();
    }
    #endif

    //
    // Service the IP Reassembly Timer
    //
    #if IP_REASSEMBLY
    if ((g_ui32LocalTimer - g_ui32IPReassemblyTimer) >= IP_TMR_INTERVAL) {
        g_ui32IPReassemblyTimer = g_ui32LocalTimer;
        ip_reass_tmr();
    }
    #endif

    //
    // Service the IGMP Timer
    //
    #if LWIP_IGMP
    if ((g_ui32LocalTimer - g_ui32IGMPTimer) >= IGMP_TMR_INTERVAL) {
        g_ui32IGMPTimer = g_ui32LocalTimer;
        igmp_tmr();
    }
    #endif

    //
    // Service the DNS Timer
    //
    #if LWIP_DNS
    if ((g_ui32LocalTimer - g_ui32DNSTimer) >= DNS_TMR_INTERVAL) {
        g_ui32DNSTimer = g_ui32LocalTimer;
        dns_tmr();
    }
    #endif

    //
    // Service the link timer.
    //
    #if LWIP_AUTOIP || LWIP_DHCP
    if ((g_ui32LocalTimer - g_ui32LinkTimer) >= LINK_TMR_INTERVAL) {
        g_ui32LinkTimer = g_ui32LocalTimer;
        lwIPLinkDetect();
    }
    #endif
}
#endif

//*****************************************************************************
//
// Handles the timeout for the host callback function timer when using a RTOS.
//
//*****************************************************************************
#if !NO_SYS && HOST_TMR_INTERVAL
static void lwIPPrivateHostTimer(void *pvArg) {
    //
    // Call the application-supplied host timer callback function.
    //
    lwIPHostTimerHandler();

    //
    // Re-schedule the host timer callback function timeout.
    //
    sys_timeout(HOST_TMR_INTERVAL, lwIPPrivateHostTimer, NULL);
}
#endif

//*****************************************************************************
//
// Handles the timeout for the link detect timer when using a RTOS.
//
//*****************************************************************************
#if !NO_SYS && (LWIP_AUTOIP || LWIP_DHCP)
static void lwIPPrivateLinkTimer(void *pvArg) {
    //
    // Perform the link detection.
    //
    lwIPLinkDetect();

    //
    // Re-schedule the link detect timer timeout.
    //
    sys_timeout(LINK_TMR_INTERVAL, lwIPPrivateLinkTimer, NULL);
}
#endif

//*****************************************************************************
//
// Completes the initialization of lwIP.  This is directly called when not
// using a RTOS and provided as a callback to the TCP/IP thread when using a
// RTOS.
//
//*****************************************************************************
extern err_t __cfxs_ethernet_lwip_network_interface_init(struct netif *netif);
static void lwIPPrivateInit(void *pvArg) {
    struct ip4_addr ip_addr;
    struct ip4_addr net_mask;
    struct ip4_addr gw_addr;

    //
    // If not using a RTOS, initialize the lwIP stack.
    //
#if NO_SYS
    lwip_init();
#endif

    //
    // If using a RTOS, create a queue (to be used as a semaphore) to signal
    // the Ethernet interrupt task from the Ethernet interrupt handler.
    //
#if !NO_SYS
    #if RTOS_FREERTOS
    g_pInterrupt = xQueueCreate(1, sizeof(void *));
    #endif
#endif

    //
    // If using a RTOS, create the Ethernet interrupt task.
    //
#if !NO_SYS
    #if RTOS_FREERTOS
    xTaskCreate(lwIPInterruptTask, (signed portCHAR *)"eth_int", STACKSIZE_LWIPINTTASK, 0, tskIDLE_PRIORITY + 1, 0);
    #endif
#endif

    //
    // Setup the network address values.
    //
    if (g_ui32IPMode == IPADDR_USE_STATIC) {
        ip_addr.addr  = g_ui32IPAddr;
        net_mask.addr = g_ui32NetMask;
        gw_addr.addr  = g_ui32GWAddr;
    } else {
        ip_addr.addr  = 0;
        net_mask.addr = 0;
        gw_addr.addr  = 0;
    }

    //
    // Create, configure and add the Ethernet controller interface with
    // default settings.  ip_input should be used to send packets directly to
    // the stack when not using a RTOS and tcpip_input should be used to send
    // packets to the TCP/IP thread's queue when using a RTOS.
    //
#if NO_SYS
    netif_add(&e_Main_Network_Interface, &ip_addr, &net_mask, &gw_addr, nullptr, __cfxs_ethernet_lwip_network_interface_init, ip_input);
#else
    netif_add(&e_Main_Network_Interface, &ip_addr, &net_mask, &gw_addr, nullptr, tivaif_init, tcpip_input);
#endif
    netif_set_default(&e_Main_Network_Interface);

    //
    // Bring the interface up.
    //
    netif_set_up(&e_Main_Network_Interface);

    //
    // Setup a timeout for the host timer callback function if using a RTOS.
    //
#if !NO_SYS && HOST_TMR_INTERVAL
    sys_timeout(HOST_TMR_INTERVAL, lwIPPrivateHostTimer, NULL);
#endif

    //
    // Setup a timeout for the link detect callback function if using a RTOS.
    //
#if !NO_SYS && (LWIP_AUTOIP || LWIP_DHCP)
    sys_timeout(LINK_TMR_INTERVAL, lwIPPrivateLinkTimer, NULL);
#endif
}

//*****************************************************************************
//
//! Initializes the lwIP TCP/IP stack.
//!
//! \param ui32SysClkHz is the current system clock rate in Hz.
//! \param pui8MAC is a pointer to a six byte array containing the MAC
//! address to be used for the interface.
//! \param ui32IPAddr is the IP address to be used (static).
//! \param ui32NetMask is the network mask to be used (static).
//! \param ui32GWAddr is the Gateway address to be used (static).
//! \param ui32IPMode is the IP Address Mode.  \b IPADDR_USE_STATIC will force
//! static IP addressing to be used, \b IPADDR_USE_DHCP will force DHCP with
//! fallback to Link Local (Auto IP), while \b IPADDR_USE_AUTOIP will force
//! Link Local only.
//!
//! This function performs initialization of the lwIP TCP/IP stack for the
//! Ethernet MAC, including DHCP and/or AutoIP, as configured.
//!
//! \return None.
//
//*****************************************************************************
void lwIPInit(uint32_t ui32SysClkHz,
              const uint8_t *pui8MAC,
              uint32_t ui32IPAddr,
              uint32_t ui32NetMask,
              uint32_t ui32GWAddr,
              uint32_t ui32IPMode) {
    //
    // Check the parameters.
    //
#if LWIP_DHCP && LWIP_AUTOIP
    ASSERT((ui32IPMode == IPADDR_USE_STATIC) || (ui32IPMode == IPADDR_USE_DHCP) || (ui32IPMode == IPADDR_USE_AUTOIP));
#elif LWIP_DHCP
    ASSERT((ui32IPMode == IPADDR_USE_STATIC) || (ui32IPMode == IPADDR_USE_DHCP));
#elif LWIP_AUTOIP
    ASSERT((ui32IPMode == IPADDR_USE_STATIC) || (ui32IPMode == IPADDR_USE_AUTOIP));
#else
    ASSERT(ui32IPMode == IPADDR_USE_STATIC);
#endif

    g_ui32IPMode  = ui32IPMode;
    g_ui32IPAddr  = ui32IPAddr;
    g_ui32NetMask = ui32NetMask;
    g_ui32GWAddr  = ui32GWAddr;

    //
    // Initialize lwIP.  The remainder of initialization is done immediately if
    // not using a RTOS and it is deferred to the TCP/IP thread's context if
    // using a RTOS.
    //
#if NO_SYS
    lwIPPrivateInit(0);
#else
    tcpip_init(lwIPPrivateInit, 0);
#endif
}

//*****************************************************************************
//
//! Registers an interrupt callback function to handle the IEEE-1588 timer.
//!
//! \param pfnTimerFunc points to a function which is called whenever the
//! Ethernet MAC reports an interrupt relating to the IEEE-1588 hardware timer.
//!
//! This function allows an application to register a handler for all
//! interrupts generated by the IEEE-1588 hardware timer in the Ethernet MAC.
//! To allow minimal latency timer handling, the callback function provided
//! will be called in interrupt context, regardless of whether or not lwIP is
//! configured to operate with an RTOS.  In an RTOS environment, the callback
//! function is responsible for ensuring that all processing it performs is
//! compatible with the low level interrupt context it is called in.
//!
//! The callback function takes two parameters.  The first is the base address
//! of the MAC reporting the timer interrupt and the second is the timer
//! interrupt status as reported by EMACTimestampIntStatus().  Note that
//! EMACTimestampIntStatus() causes the timer interrupt sources to be cleared
//! so the application should not call EMACTimestampIntStatus() within the
//! handler.
//!
//! \return None.
//
//*****************************************************************************
void lwIPTimerCallbackRegister(tHardwareTimerHandler pfnTimerFunc) {
    //
    // Remember the callback function address passed.
    //
    g_pfnTimerHandler = pfnTimerFunc;
}

//*****************************************************************************
//
//! Handles periodic timer events for the lwIP TCP/IP stack.
//!
//! \param ui32TimeMS is the incremental time for this periodic interrupt.
//!
//! This function will update the local timer by the value in \e ui32TimeMS.
//! If the system is configured for use without an RTOS, an Ethernet interrupt
//! will be triggered to allow the lwIP periodic timers to be serviced in the
//! Ethernet interrupt.
//!
//! \return None.
//
//*****************************************************************************
#if NO_SYS
    #include <CFXS/Base/Time.hpp>
CFXS::Time_t s_LWIPLIB_TimerLastCall = 0;
void lwIPTimer(void *) {
    //
    // Increment the lwIP Ethernet timer.
    //
    g_ui32LocalTimer += CFXS::Time::ms - s_LWIPLIB_TimerLastCall;
    s_LWIPLIB_TimerLastCall = CFXS::Time::ms;

    //
    // Generate an Ethernet interrupt.  This will perform the actual work
    // of checking the lwIP timers and taking the appropriate actions.  This is
    // needed since lwIP is not re-entrant, and this allows all lwIP calls to
    // be placed inside the Ethernet interrupt handler ensuring that all calls
    // into lwIP are coming from the same context, preventing any reentrancy
    // issues.  Putting all the lwIP calls in the Ethernet interrupt handler
    // avoids the use of mutexes to avoid re-entering lwIP.
    //

    lwIPServiceTimers();

    HWREG(NVIC_SW_TRIG) |= INT_EMAC0 - 16;
}
#endif

//*****************************************************************************
//
//! Returns the IP address for this interface.
//!
//! This function will read and return the currently assigned IP address for
//! the Stellaris Ethernet interface.
//!
//! \return Returns the assigned IP address for this interface.
//
//*****************************************************************************
uint32_t lwIPLocalIPAddrGet(void) {
#if LWIP_AUTOIP || LWIP_DHCP
    if (g_bLinkActive) {
        return ((uint32_t)e_Main_Network_Interface.ip_addr.addr);
    } else {
        return (0xffffffff);
    }
#else
    return ((uint32_t)e_Main_Network_Interface.ip_addr.addr);
#endif
}

//*****************************************************************************
//
//! Returns the network mask for this interface.
//!
//! This function will read and return the currently assigned network mask for
//! the Stellaris Ethernet interface.
//!
//! \return the assigned network mask for this interface.
//
//*****************************************************************************
uint32_t lwIPLocalNetMaskGet(void) {
    return ((uint32_t)e_Main_Network_Interface.netmask.addr);
}

//*****************************************************************************
//
//! Returns the gateway address for this interface.
//!
//! This function will read and return the currently assigned gateway address
//! for the Stellaris Ethernet interface.
//!
//! \return the assigned gateway address for this interface.
//
//*****************************************************************************
uint32_t lwIPLocalGWAddrGet(void) {
    return ((uint32_t)e_Main_Network_Interface.gw.addr);
}

//*****************************************************************************
//
//! Returns the local MAC/HW address for this interface.
//!
//! \param pui8MAC is a pointer to an array of bytes used to store the MAC
//! address.
//!
//! This function will read the currently assigned MAC address into the array
//! passed in \e pui8MAC.
//!
//! \return None.
//
//*****************************************************************************
void lwIPLocalMACGet(uint8_t *pui8MAC) {
    EMACAddrGet(EMAC0_BASE, 0, pui8MAC);
}

//*****************************************************************************
//
// Completes the network configuration change.  This is directly called when
// not using a RTOS and provided as a callback to the TCP/IP thread when using
// a RTOS.
//
//*****************************************************************************
static void lwIPPrivateNetworkConfigChange(void *pvArg) {
    uint32_t ui32IPMode;
    struct ip4_addr ip_addr;
    struct ip4_addr net_mask;
    struct ip4_addr gw_addr;

    //
    // Get the new address mode.
    //
    ui32IPMode = (uint32_t)pvArg;

    //
    // Setup the network address values.
    //
    if (ui32IPMode == IPADDR_USE_STATIC) {
        ip_addr.addr  = g_ui32IPAddr;
        net_mask.addr = g_ui32NetMask;
        gw_addr.addr  = g_ui32GWAddr;
    }
#if LWIP_DHCP || LWIP_AUTOIP
    else {
        ip_addr.addr  = 0;
        net_mask.addr = 0;
        gw_addr.addr  = 0;
    }
#endif

    //
    // Switch on the current IP Address Aquisition mode.
    //
    switch (g_ui32IPMode) {
        //
        // Static IP
        //
        case IPADDR_USE_STATIC: {
            //
            // Set the new address parameters.  This will change the address
            // configuration in lwIP, and if necessary, will reset any links
            // that are active.  This is valid for all three modes.
            //
            netif_set_addr(&e_Main_Network_Interface, &ip_addr, &net_mask, &gw_addr);

            //
            // If we are going to DHCP mode, then start the DHCP server now.
            //
#if LWIP_DHCP
            if ((ui32IPMode == IPADDR_USE_DHCP) && g_bLinkActive) {
                dhcp_start(&e_Main_Network_Interface);
            }
#endif

            //
            // If we are going to AutoIP mode, then start the AutoIP process
            // now.
            //
#if LWIP_AUTOIP
            if ((ui32IPMode == IPADDR_USE_AUTOIP) && g_bLinkActive) {
                autoip_start(&e_Main_Network_Interface);
            }
#endif

            //
            // And we're done.
            //
            break;
        }

        //
        // DHCP (with AutoIP fallback).
        //
#if LWIP_DHCP
        case IPADDR_USE_DHCP: {
            //
            // If we are going to static IP addressing, then disable DHCP and
            // force the new static IP address.
            //
            if (ui32IPMode == IPADDR_USE_STATIC) {
                dhcp_stop(&e_Main_Network_Interface);
                netif_set_addr(&e_Main_Network_Interface, &ip_addr, &net_mask, &gw_addr);
            }
            //
            // If we are going to AUTO IP addressing, then disable DHCP, set
            // the default addresses, and start AutoIP.
            //
    #if LWIP_AUTOIP
            else if (ui32IPMode == IPADDR_USE_AUTOIP) {
                dhcp_stop(&e_Main_Network_Interface);
                netif_set_addr(&e_Main_Network_Interface, &ip_addr, &net_mask, &gw_addr);
                if (g_bLinkActive) {
                    autoip_start(&e_Main_Network_Interface);
                }
            }
    #endif
            break;
        }
#endif

        //
        // AUTOIP
        //
#if LWIP_AUTOIP
        case IPADDR_USE_AUTOIP: {
            //
            // If we are going to static IP addressing, then disable AutoIP and
            // force the new static IP address.
            //
            if (ui32IPMode == IPADDR_USE_STATIC) {
                autoip_stop(&e_Main_Network_Interface);
                netif_set_addr(&e_Main_Network_Interface, &ip_addr, &net_mask, &gw_addr);
            }
            //
            // If we are going to DHCP addressing, then disable AutoIP, set the
            // default addresses, and start dhcp.
            //
    #if LWIP_DHCP
            else if (ui32IPMode == IPADDR_USE_DHCP) {
                autoip_stop(&e_Main_Network_Interface);
                netif_set_addr(&e_Main_Network_Interface, &ip_addr, &net_mask, &gw_addr);
                if (g_bLinkActive) {
                    dhcp_start(&e_Main_Network_Interface);
                }
            }
    #endif
            break;
        }
#endif
    }

    //
    // Bring the interface up.
    //
    netif_set_up(&e_Main_Network_Interface);

    //
    // Save the new mode.
    //
    g_ui32IPMode = ui32IPMode;
}

//*****************************************************************************
//
//! Change the configuration of the lwIP network interface.
//!
//! \param ui32IPAddr is the new IP address to be used (static).
//! \param ui32NetMask is the new network mask to be used (static).
//! \param ui32GWAddr is the new Gateway address to be used (static).
//! \param ui32IPMode is the IP Address Mode.  \b IPADDR_USE_STATIC 0 will
//! force static IP addressing to be used, \b IPADDR_USE_DHCP will force DHCP
//! with fallback to Link Local (Auto IP), while \b IPADDR_USE_AUTOIP will
//! force Link Local only.
//!
//! This function will evaluate the new configuration data.  If necessary, the
//! interface will be brought down, reconfigured, and then brought back up
//! with the new configuration.
//!
//! \return None.
//
//*****************************************************************************
void lwIPNetworkConfigChange(uint32_t ui32IPAddr, uint32_t ui32NetMask, uint32_t ui32GWAddr, uint32_t ui32IPMode) {
    //
    // Check the parameters.
    //
#if LWIP_DHCP && LWIP_AUTOIP
    ASSERT((ui32IPMode == IPADDR_USE_STATIC) || (ui32IPMode == IPADDR_USE_DHCP) || (ui32IPMode == IPADDR_USE_AUTOIP));
#elif LWIP_DHCP
    ASSERT((ui32IPMode == IPADDR_USE_STATIC) || (ui32IPMode == IPADDR_USE_DHCP));
#elif LWIP_AUTOIP
    ASSERT((ui32IPMode == IPADDR_USE_STATIC) || (ui32IPMode == IPADDR_USE_AUTOIP));
#else
    ASSERT(ui32IPMode == IPADDR_USE_STATIC);
#endif

    //
    // Save the network configuration for later use by the private network
    // configuration change.
    //
    g_ui32IPAddr  = ui32IPAddr;
    g_ui32NetMask = ui32NetMask;
    g_ui32GWAddr  = ui32GWAddr;

    //
    // Complete the network configuration change.  The remainder is done
    // immediately if not using a RTOS and it is deferred to the TCP/IP
    // thread's context if using a RTOS.
    //
#if NO_SYS
    lwIPPrivateNetworkConfigChange((void *)ui32IPMode);
#else
    tcpip_callback(lwIPPrivateNetworkConfigChange, (void *)ui32IPMode);
#endif
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
