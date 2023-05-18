// driverlib includes in precompiled header (CFXS Hardware CMakeLists)
#include <CFXS/HW/System/SystemControl_TM4C.hpp>
#include <Common/ETH_Debug.hpp>
#include <CFXS/Base/MAC_Address.hpp>
#include <CFXS/Platform/CPU.hpp>
#include <CFXS/Platform/Task.hpp>
#include <CFXS/Base/Memory.hpp>
#include <CFXS/Base/Time.hpp>
#include <CFXS/Base/IPv4.hpp>
#include "_defines.hpp"

extern "C" {
extern void lwIPTimer(void *);

extern void lwIPInit(uint32_t ui32SysClkHz,
                     const uint8_t *pui8MAC,
                     uint32_t ui32IPAddr,
                     uint32_t ui32NetMask,
                     uint32_t ui32GWAddr,
                     uint32_t ui32IPMode);
}

struct netif e_Main_Network_Interface;

uint32_t s_Ethernet_DataRateCounter_TX = 0;
uint32_t s_Ethernet_DataRateCounter_RX = 0;
uint32_t s_Ethernet_DataRate_TX        = 0;
uint32_t s_Ethernet_DataRate_RX        = 0;
uint64_t s_Ethernet_PacketCount_TX     = 0;
uint64_t s_Ethernet_PacketCount_RX     = 0;

using CFXS::DebugLevel;
using CFXS::HW::TM4C::SystemControl;

////////////////////////////////////////////////////////////

static int s_Eth_ProtectLevel = 0;
extern bool __cfxs_is_initialization_complete();

__c_func uint32_t sys_now() {
    return CFXS::Time::ms;
}

__c_func sys_prot_t sys_arch_protect() {
    if (s_Eth_ProtectLevel == 0)
        CFXS::CPU::DisableInterrupts();
    s_Eth_ProtectLevel++;
    return s_Eth_ProtectLevel;
}
__c_func void sys_arch_unprotect(sys_prot_t level) {
    s_Eth_ProtectLevel--;
    if (s_Eth_ProtectLevel == 0 && __cfxs_is_initialization_complete())
        CFXS::CPU::EnableInterrupts();
}

////////////////////////////////////////////////////////////

struct PacketDescriptor {
    tEMACDMADescriptor dma_descriptor;
    struct pbuf *packet_buffer;
};

struct PacketDescriptorList {
    PacketDescriptor *descriptors;
    uint16_t count;
    uint16_t read_index;
    uint16_t write_index;
};

PacketDescriptor e_Ethernet_TX_Descriptors[CFXS_HW_ETHERNET_TX_DESCRIPTOR_COUNT];
PacketDescriptor e_Ethernet_RX_Descriptors[CFXS_HW_ETHERNET_RX_DESCRIPTOR_COUNT];
PacketDescriptorList e_Ethernet_TX_Descriptor_List = {e_Ethernet_TX_Descriptors, CFXS_HW_ETHERNET_TX_DESCRIPTOR_COUNT, 0, 0};
PacketDescriptorList e_Ethernet_RX_Descriptor_List = {e_Ethernet_RX_Descriptors, CFXS_HW_ETHERNET_RX_DESCRIPTOR_COUNT, 0, 0};

static void InitializeDescriptors() {
    CFXS_ETH_printf(DebugLevel::TRACE, "Initialize descriptors\n");

    /* Transmit list -  mark all descriptors as not owned by the hardware */
    for (int i = 0; i < CFXS_HW_ETHERNET_TX_DESCRIPTOR_COUNT; i++) {
        e_Ethernet_TX_Descriptors[i].packet_buffer            = (struct pbuf *)0;
        e_Ethernet_TX_Descriptors[i].dma_descriptor.ui32Count = 0;
        e_Ethernet_TX_Descriptors[i].dma_descriptor.pvBuffer1 = 0;
        e_Ethernet_TX_Descriptors[i].dma_descriptor.DES3.pLink =
            ((i == (CFXS_HW_ETHERNET_TX_DESCRIPTOR_COUNT - 1)) ? &e_Ethernet_TX_Descriptors[0].dma_descriptor :
                                                                 &e_Ethernet_TX_Descriptors[i + 1].dma_descriptor);
        e_Ethernet_TX_Descriptors[i].dma_descriptor.ui32CtrlStatus =
            DES0_TX_CTRL_INTERRUPT | DES0_TX_CTRL_CHAINED | DES0_TX_CTRL_IP_ALL_CKHSUMS;
    }

    e_Ethernet_TX_Descriptor_List.read_index  = 0;
    e_Ethernet_TX_Descriptor_List.write_index = 0;

    /* Receive list -  tag each descriptor with a pbuf and set all fields to
    * allow packets to be received.
    */
    for (int i = 0; i < CFXS_HW_ETHERNET_RX_DESCRIPTOR_COUNT; i++) {
        e_Ethernet_RX_Descriptors[i].packet_buffer            = pbuf_alloc(PBUF_RAW, PBUF_POOL_BUFSIZE, PBUF_POOL);
        e_Ethernet_RX_Descriptors[i].dma_descriptor.ui32Count = DES1_RX_CTRL_CHAINED;
        if (e_Ethernet_RX_Descriptors[i].packet_buffer) {
            /* Set the DMA to write directly into the pbuf payload. */
            e_Ethernet_RX_Descriptors[i].dma_descriptor.pvBuffer1 = e_Ethernet_RX_Descriptors[i].packet_buffer->payload;
            e_Ethernet_RX_Descriptors[i].dma_descriptor.ui32Count |=
                (e_Ethernet_RX_Descriptors[i].packet_buffer->len << DES1_RX_CTRL_BUFF1_SIZE_S);
            e_Ethernet_RX_Descriptors[i].dma_descriptor.ui32CtrlStatus = DES0_RX_CTRL_OWN;
        } else {
            CFXS_ETH_printf(
                DebugLevel::WARNING, "InitializeDescriptors failed to allocate pbuf %u/%u\n", i + 1, CFXS_HW_ETHERNET_RX_DESCRIPTOR_COUNT);

            /* No pbuf available so leave the buffer pointer empty. */
            e_Ethernet_RX_Descriptors[i].dma_descriptor.pvBuffer1      = 0;
            e_Ethernet_RX_Descriptors[i].dma_descriptor.ui32CtrlStatus = 0;
        }
        e_Ethernet_RX_Descriptors[i].dma_descriptor.DES3.pLink =
            ((i == (CFXS_HW_ETHERNET_RX_DESCRIPTOR_COUNT - 1)) ? &e_Ethernet_RX_Descriptors[0].dma_descriptor :
                                                                 &e_Ethernet_RX_Descriptors[i + 1].dma_descriptor);
    }

    e_Ethernet_TX_Descriptor_List.read_index  = 0;
    e_Ethernet_TX_Descriptor_List.write_index = 0;

    //
    // Set the descriptor pointers in the hardware.
    //
    EMACRxDMADescriptorListSet(EMAC0_BASE, &e_Ethernet_RX_Descriptors[0].dma_descriptor);
    EMACTxDMADescriptorListSet(EMAC0_BASE, &e_Ethernet_TX_Descriptors[0].dma_descriptor);
}

////////////////////////////////////////////////////////////////////////////////////
struct EthernetHeapStatus {
    size_t alloc_count = 0;
    size_t free_count  = 0;
};

EthernetHeapStatus s_EthernetHeapStats;
#include <CFXS/Platform/Heap/MemoryManager.hpp>
extern CFXS::Heap s_MainHeap;

// #define CFXS_HW_ETHERNET_MEMORY_CHECKS

#ifdef CFXS_HW_ETHERNET_MEMORY_CHECKS
eastl::unordered_map<void *, bool> s_alloc_map;
eastl::unordered_map<void *, uint32_t> s_release_map;
volatile __used uint32_t s_release_location;
#endif

__c_func void __cfxs_eth_free(void *data) {
#ifdef CFXS_HW_ETHERNET_MEMORY_CHECKS
    register uint32_t lr;
    asm volatile("mov %0, LR\n" : "=r"(lr));
#endif

    s_EthernetHeapStats.free_count++;

#ifdef CFXS_HW_ETHERNET_MEMORY_CHECKS
    if ((s_alloc_map.find(data) == s_alloc_map.end())) {
        CFXS_ERROR("free unknown ptr %p\n", data);
    } else if (s_alloc_map[data] == false) {
        s_release_location = s_release_map[data];
        CFXS_ERROR("free released ptr %p (released from %p)\n", data, s_release_location);
    } else {
        s_alloc_map[data]   = false;
        s_release_map[data] = lr;
    }
#endif

    s_MainHeap.Deallocate(data);
}
__c_func void *__cfxs_eth_malloc(size_t size) {
    s_EthernetHeapStats.alloc_count++;
    auto p = s_MainHeap.Allocate(size);
#ifdef CFXS_HW_ETHERNET_MEMORY_CHECKS
    s_alloc_map[p] = true;
#endif
    return p;
}
__c_func void *__cfxs_eth_calloc(size_t nitems, size_t size) {
    s_EthernetHeapStats.alloc_count++;
    auto p = s_MainHeap.AllocateAndZero(nitems * size);
#ifdef CFXS_HW_ETHERNET_MEMORY_CHECKS
    s_alloc_map[p] = true;
#endif
    return p;
}

__always_inline void Process_Ethernet_PHY_Interrupt() {
#if EEE_SUPPORT
    uint16_t ui16EEEStatus;
#endif
    auto misr1 = EMACPHYRead(EMAC0_BASE, CFXS_HW_ETHERNET_PHY_ADDRESS, EPHY_MISR1);

    EMACPHYRead(EMAC0_BASE, CFXS_HW_ETHERNET_PHY_ADDRESS, EPHY_BMSR);
    auto status = EMACPHYRead(EMAC0_BASE, CFXS_HW_ETHERNET_PHY_ADDRESS, EPHY_STS);

#if EEE_SUPPORT
    ui16EEEStatus = EMACPHYMMDRead(EMAC0_BASE, CFXS_HW_ETHERNET_PHY_ADDRESS, 0x703D);
#endif

    if (misr1 & EPHY_MISR1_LINKSTAT) {
        if (status & EPHY_STS_LINK) {
#if NO_SYS
            netif_set_link_up(&e_Main_Network_Interface);
#else
            tcpip_callback((tcpip_callback_fn)netif_set_link_up, psNetif);
#endif

#if EEE_SUPPORT
            if (ui16EEEStatus & 0x2) {
                EMACLPIConfig(EMAC0_BASE, true, 1000, 36);
                EMACLPILinkSet(EMAC0_BASE);
                g_bEEELinkActive = true;
            }
#endif
        } else {
#if NO_SYS
            netif_set_link_down(&e_Main_Network_Interface);
#else
            tcpip_callback((tcpip_callback_fn)netif_set_link_down, psNetif);
#endif
#if EEE_SUPPORT
            g_bEEELinkActive = false;
            EMACLPILinkClear(EMAC0_BASE);
            EMACLPIConfig(EMAC0_BASE, false, 1000, 0);
#endif
        }
    }
    if (misr1 & (EPHY_MISR1_SPEED | EPHY_MISR1_SPEED | EPHY_MISR1_ANC)) {
        uint32_t config;
        uint32_t mode;
        uint32_t max_rx_frame_size;
        EMACConfigGet(EMAC0_BASE, &config, &mode, &max_rx_frame_size);

        if (status & EPHY_STS_SPEED) {
            config &= ~EMAC_CONFIG_100MBPS;
        } else {
            config |= EMAC_CONFIG_100MBPS;
        }

        if (status & EPHY_STS_DUPLEX) {
            config |= EMAC_CONFIG_FULL_DUPLEX;
        } else {
            config &= ~EMAC_CONFIG_FULL_DUPLEX;
        }

        EMACConfigSet(EMAC0_BASE, config, mode, max_rx_frame_size);
    }
}

__always_inline void Process_Ethernet_Transmit() {
    auto &tx_desc_list = e_Ethernet_TX_Descriptor_List;

    for (auto i = 0; i < tx_desc_list.count; i++) {
        auto &desc_ref = tx_desc_list.descriptors[tx_desc_list.read_index];

        if (desc_ref.dma_descriptor.ui32CtrlStatus & DES0_TX_CTRL_OWN) {
            break;
        }

        if (desc_ref.packet_buffer) {
            if (!((uint32_t)(desc_ref.packet_buffer) & 1)) {
                s_Ethernet_DataRateCounter_TX += desc_ref.packet_buffer->tot_len;
                s_Ethernet_PacketCount_TX++;
                pbuf_free(desc_ref.packet_buffer);
            }
            desc_ref.packet_buffer = nullptr;
        } else {
            break;
        }
        tx_desc_list.read_index++;
        if (tx_desc_list.read_index == tx_desc_list.count) {
            tx_desc_list.read_index = 0;
        }
    }
}

__always_inline void Process_Ethernet_Receive() {
    static struct pbuf *packet_buffer = nullptr;
    auto descriptor_end               = e_Ethernet_RX_Descriptor_List.read_index ? (e_Ethernet_RX_Descriptor_List.read_index - 1) :
                                                                                   (e_Ethernet_RX_Descriptor_List.count - 1);

    auto &ref_read_index = e_Ethernet_RX_Descriptor_List.read_index;
    auto &descriptors    = e_Ethernet_RX_Descriptor_List.descriptors;

    while (ref_read_index != descriptor_end) {
        auto &desc_ref = descriptors[ref_read_index];

        if (desc_ref.packet_buffer) {
            if (desc_ref.dma_descriptor.ui32CtrlStatus & DES0_RX_CTRL_OWN) {
                break;
            }

            if (desc_ref.dma_descriptor.ui32CtrlStatus & DES0_RX_STAT_LAST_DESC) {
                desc_ref.packet_buffer->len =
                    (desc_ref.dma_descriptor.ui32CtrlStatus & DES0_RX_STAT_FRAME_LENGTH_M) >> DES0_RX_STAT_FRAME_LENGTH_S;
                desc_ref.packet_buffer->tot_len = desc_ref.packet_buffer->len;
            }

            if (packet_buffer) {
                pbuf_cat(packet_buffer, desc_ref.packet_buffer);
                desc_ref.packet_buffer = packet_buffer;
            }

            packet_buffer = desc_ref.packet_buffer;

            if (desc_ref.dma_descriptor.ui32CtrlStatus & DES0_RX_STAT_LAST_DESC) {
                if (desc_ref.dma_descriptor.ui32CtrlStatus & DES0_RX_STAT_ERR) {
                    LWIP_DEBUGF(NETIF_DEBUG, ("tivaif_receive: packet error\n"));
                    pbuf_free(packet_buffer);
                    LINK_STATS_INC(link.drop);
                    packet_buffer = nullptr;
                } else {
                    LINK_STATS_INC(link.recv);

#if LWIP_PTPD
                    pBuf->time_s  = pDescList->pDescriptors[pDescList->ui32Read].dma_descriptor.ui32IEEE1588TimeHi;
                    pBuf->time_ns = pDescList->pDescriptors[pDescList->ui32Read].dma_descriptor.ui32IEEE1588TimeLo;
#endif

                    s_Ethernet_DataRateCounter_RX += packet_buffer->tot_len;
                    s_Ethernet_PacketCount_RX++;
#if NO_SYS
                    auto error_reason = ethernet_input(packet_buffer, &e_Main_Network_Interface);
#else
                    auto error_reason = tcpip_input(pBuf, psNetif);
#endif
                    if (error_reason != ERR_OK) {
                        CFXS_ETH_printf(DebugLevel::WARNING, "[RX] ethernet_input error (%d)\n", error_reason);
                        pbuf_free(packet_buffer);
                        LINK_STATS_INC(link.memerr);
                        LINK_STATS_INC(link.drop);
                    }

                    packet_buffer = nullptr;
                }
            }
        }

        desc_ref.packet_buffer            = pbuf_alloc(PBUF_RAW, PBUF_POOL_BUFSIZE, PBUF_POOL);
        desc_ref.dma_descriptor.ui32Count = DES1_RX_CTRL_CHAINED;
        if (desc_ref.packet_buffer) {
            desc_ref.dma_descriptor.pvBuffer1 = desc_ref.packet_buffer->payload;
            desc_ref.dma_descriptor.ui32Count |= (desc_ref.packet_buffer->len << DES1_RX_CTRL_BUFF1_SIZE_S);

            desc_ref.dma_descriptor.ui32CtrlStatus = DES0_RX_CTRL_OWN;
        } else {
            // LWIP_DEBUGF(NETIF_DEBUG, ("tivaif_receive: pbuf_alloc error\n"));
            CFXS_ETH_printf(DebugLevel::ERROR, "[RX] Failed to allocate packet buffer\n");

            desc_ref.dma_descriptor.pvBuffer1 = 0;

            LINK_STATS_INC(link.memerr);
            break;
        }

        ref_read_index++;
        if (ref_read_index == e_Ethernet_RX_Descriptor_List.count) {
            ref_read_index = 0;
        }
    }
}

__always_inline void Process_Ethernet_Interrupt(uint32_t status) {
    // if (status & EMAC_INT_ABNORMAL_INT) {
    //     g_ui32AbnormalInts++;
    // }

    if (status & EMAC_INT_PHY) {
        Process_Ethernet_PHY_Interrupt();
    }

    if (status & EMAC_INT_TRANSMIT) {
#if EEE_SUPPORT
        if (g_bEEELinkActive) {
            EMACLPIEnter(EMAC0_BASE);
        }
#endif
        Process_Ethernet_Transmit();
    }

    if (status & (EMAC_INT_RECEIVE | EMAC_INT_RX_NO_BUFFER | EMAC_INT_RX_STOPPED)) {
        Process_Ethernet_Receive();
    }
}

void interrupt_Ethernet() {
#if !NO_SYS
    portBASE_TYPE xWake;
#endif
    auto status = EMACIntStatus(EMAC0_BASE, true);

#if EEE_SUPPORT
    if (ui32Status & EMAC_INT_LPI) {
        EMACLPIStatus(EMAC0_BASE);
    }
#endif

    if (status & EMAC_INT_POWER_MGMNT) {
        EMACTxEnable(EMAC0_BASE);
        EMACRxEnable(EMAC0_BASE);

        EMACPowerManagementStatusGet(EMAC0_BASE);

        status &= ~(EMAC_INT_POWER_MGMNT);
    }

    if (status == 0) {
        return;
    }

    EMACIntClear(EMAC0_BASE, status);

    // if (status & EMAC_INT_TIMESTAMP) {
    //     //
    //     // Yes - read and clear the timestamp interrupt status.
    //     //
    //    auto timer_status = EMACTimestampIntStatus(EMAC0_BASE);

    //     //
    //     // If a timer interrupt handler has been registered, call it.
    //     //
    //     if (g_pfnTimerHandler) {
    //         g_pfnTimerHandler(EMAC0_BASE, timer_status);
    //     }
    // }

#if NO_SYS
    Process_Ethernet_Interrupt(status);
#else
    xQueueSendFromISR(g_pInterrupt, (void *)&ui32Status, &xWake);
    MAP_EMACIntDisable(
        EMAC0_BASE,
        (EMAC_INT_RECEIVE | EMAC_INT_TRANSMIT | EMAC_INT_TX_STOPPED | EMAC_INT_RX_NO_BUFFER | EMAC_INT_RX_STOPPED | EMAC_INT_PHY));
    #if RTOS_FREERTOS
    if (xWake == pdTRUE) {
        portYIELD_FROM_ISR(true);
    }
    #endif
#endif
}

#define PTR_SAFE_FOR_EMAC_DMA(ptr) (((uint32_t)(ptr) >= 0x2000000) && ((uint32_t)(ptr) < 0x20070000))
static struct pbuf *tivaif_check_pbuf(struct pbuf *p) {
    struct pbuf *pBuf;
    err_t Err;

    pBuf = p;

    /* Walk the list of buffers in the pbuf checking each. */
    do {
        /* Does this pbuf's payload reside in memory that the Ethernet DMA
         * can access?
         */
        if (!PTR_SAFE_FOR_EMAC_DMA(pBuf->payload)) {
            /* This buffer is outside the DMA-able memory space so we need
             * to copy the pbuf.
             */
            pBuf = pbuf_alloc(PBUF_RAW, p->tot_len, PBUF_POOL);

            /* If we got a new pbuf... */
            if (pBuf) {
                /* ...copy the old pbuf into the new one. */
                Err = pbuf_copy(pBuf, p);

                /* If we failed to copy the pbuf, free the newly allocated one
                 * and make sure we return a NULL to show a problem.
                 */
                if (Err != ERR_OK) {
                    pbuf_free(pBuf);
                    pBuf = NULL;
                } else {
                }
            }

            /* Reduce the reference count on the original pbuf since we're not
             * going to hold on to it after returning from tivaif_transmit.
             * Note that we already bumped the reference count at the top of
             * tivaif_transmit.
             */
            pbuf_free(p);

            /* Send back the new pbuf pointer or NULL if an error occurred. */
            return (pBuf);
        }

        /* Move on to the next buffer in the queue */
        pBuf = pBuf->next;
    } while (pBuf);

    /**
     * If we get here, the passed pbuf can be safely used without needing to
     * be copied.
     */
    return (p);
}

static err_t ethernet_transmit(struct netif *psNetif, struct pbuf *p) {
    struct pbuf *pBuf;
    bool bFirst;
    SYS_ARCH_DECL_PROTECT(lev);

    LWIP_DEBUGF(NETIF_DEBUG, ("ethernet_transmit %p, len %d\n", p, p->tot_len));

    /**
   * This entire function must run within a "critical section" to preserve
   * the integrity of the transmit pbuf queue.
   */
    SYS_ARCH_PROTECT(lev);

    /**
   * Increase the reference count on the packet provided so that we can
   * hold on to it until we are finished transmitting its content.
   */
    pbuf_ref(p);

    /**
   * Determine whether all buffers passed are within SRAM and, if not, copy
   * the pbuf into SRAM-resident buffers so that the Ethernet DMA can access
   * the data.
   */
    p = tivaif_check_pbuf(p);

    /* Make sure we still have a valid buffer (it may have been copied) */
    if (!p) {
        LINK_STATS_INC(link.memerr);
        SYS_ARCH_UNPROTECT(lev);
        return (ERR_MEM);
    }

    /* Get our state data from the netif structure we were passed. */

    /* Make sure that the transmit descriptors are not all in use */
    auto pDesc = &(e_Ethernet_TX_Descriptor_List.descriptors[e_Ethernet_TX_Descriptor_List.write_index]);
    if (pDesc->packet_buffer) {
        /**
       * The current write descriptor has a pbuf attached to it so this
       * implies that the ring is full. Reject this transmit request with a
       * memory error since we can't satisfy it just now.
       */
        pbuf_free(p);
        LINK_STATS_INC(link.memerr);
        SYS_ARCH_UNPROTECT(lev);
        return (ERR_MEM);
    }

    /* How many pbufs are in the chain passed? */
    auto ui32NumChained = (uint32_t)pbuf_clen(p);

    /* How many free transmit descriptors do we have? */
    auto ui32NumDescs =
        (e_Ethernet_TX_Descriptor_List.read_index > e_Ethernet_TX_Descriptor_List.write_index) ?
            (e_Ethernet_TX_Descriptor_List.read_index - e_Ethernet_TX_Descriptor_List.write_index) :
            ((CFXS_HW_ETHERNET_TX_DESCRIPTOR_COUNT - e_Ethernet_TX_Descriptor_List.write_index) + e_Ethernet_TX_Descriptor_List.read_index);

    /* Do we have enough free descriptors to send the whole packet? */
    if (ui32NumDescs < ui32NumChained) {
        /* No - we can't transmit this whole packet so return an error. */
        pbuf_free(p);
        LINK_STATS_INC(link.memerr);
        SYS_ARCH_UNPROTECT(lev);
        return (ERR_MEM);
    }

    /* Tag the first descriptor as the start of the packet. */
    bFirst                               = true;
    pDesc->dma_descriptor.ui32CtrlStatus = DES0_TX_CTRL_FIRST_SEG;

    /* Here, we know we can send the packet so write it to the descriptors */
    pBuf = p;

    while (ui32NumChained) {
        /* Get a pointer to the descriptor we will write next. */
        pDesc = &(e_Ethernet_TX_Descriptor_List.descriptors[e_Ethernet_TX_Descriptor_List.write_index]);

        /* Fill in the buffer pointer and length */
        pDesc->dma_descriptor.ui32Count = (uint32_t)pBuf->len;
        pDesc->dma_descriptor.pvBuffer1 = pBuf->payload;

        /* Tag the first descriptor as the start of the packet. */
        if (bFirst) {
            bFirst                               = false;
            pDesc->dma_descriptor.ui32CtrlStatus = DES0_TX_CTRL_FIRST_SEG;
        } else {
            pDesc->dma_descriptor.ui32CtrlStatus = 0;
        }

        pDesc->dma_descriptor.ui32CtrlStatus |= (DES0_TX_CTRL_IP_ALL_CKHSUMS | DES0_TX_CTRL_CHAINED);

        /* Decrement our descriptor counter, move on to the next buffer in the
       * pbuf chain. */
        ui32NumChained--;
        pBuf = pBuf->next;

        /* Update the descriptor list write index. */
        e_Ethernet_TX_Descriptor_List.write_index++;
        if (e_Ethernet_TX_Descriptor_List.write_index == CFXS_HW_ETHERNET_TX_DESCRIPTOR_COUNT) {
            e_Ethernet_TX_Descriptor_List.write_index = 0;
        }

        /* If this is the last descriptor, mark it as the end of the packet. */
        if (!ui32NumChained) {
            pDesc->dma_descriptor.ui32CtrlStatus |= (DES0_TX_CTRL_LAST_SEG | DES0_TX_CTRL_INTERRUPT);

            /* Tag the descriptor with the original pbuf pointer. */
            pDesc->packet_buffer = p;
        } else {
            /* Set the lsb of the pbuf pointer.  We use this as a signal that
           * we should not free the pbuf when we are walking the descriptor
           * list while processing the transmit interrupt.  We only free the
           * pbuf when processing the last descriptor used to transmit its
           * chain.
           */
            pDesc->packet_buffer = (struct pbuf *)((uint32_t)p + 1);
        }

        /* Hand the descriptor over to the hardware. */
        pDesc->dma_descriptor.ui32CtrlStatus |= DES0_TX_CTRL_OWN;
    }

    /* Tell the transmitter to start (in case it had stopped). */
    EMACTxDMAPollDemand(EMAC0_BASE);

    LINK_STATS_INC(link.xmit);

    SYS_ARCH_UNPROTECT(lev);

    return (ERR_OK);
}

#ifdef CFXS_HW_ETHERNET_INTERRUPT_MODE
void InitializeInterruptMode() {
    #ifdef CFXS_HW_ETHERNET_INTERRUPT_PRIORITY
        #if (CFXS_HW_ETHERNET_PRIORITY >= 0) && ((CFXS_HW_ETHERNET_INTERRUPT_PRIORITY & (~__NVIC_PRIO_BITS)) == 0)
    ROM_IntPrioritySet(INT_EMAC0, CFXS_HW_ETHERNET_INTERRUPT_PRIORITY << 5);
        #else
            #error Ethernet interrupt priority out of range
        #endif
    #else
        #error Ethernet interrupt priority not defined
    #endif
    IntRegister(INT_EMAC0, interrupt_Ethernet);
    ROM_IntEnable(INT_EMAC0);
}
#endif

#ifdef CFXS_HW_ETHERNET_POLL_MODE
void InitializePollingMode() {
    CFXS::Task::Create(
        HIGH_PRIORITY,
        "Ethernet Poll Task",
        [](auto...) {
            interrupt_Ethernet();
        },
        0)
        ->Start();
}
#endif

err_t __cfxs_ethernet_lwip_network_interface_init(struct netif *interface) {
#if LWIP_NETIF_HOSTNAME
    interface->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

    NETIF_INIT_SNMP(psNetif, snmp_ifType_ethernet_csmacd, 1000000);

    interface->name[0]    = 'N';
    interface->name[1]    = 'A';
    interface->output     = etharp_output;
    interface->linkoutput = ethernet_transmit;
    interface->mtu        = 1500;
    interface->hwaddr_len = ETH_HWADDR_LEN;
    interface->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP;

    return ERR_OK;
}

void __cfxs_module_Ethernet_Initialize(const CFXS::MAC_Address &default_mac) {
    CFXS_ETH_printf(DebugLevel::INFO, "Initialize module (TM4C129)\n");

    char buf[32];
    default_mac.PrintTo(buf, 32);
    CFXS_ETH_printf(DebugLevel::INFO, " - MAC: %s\n", buf);

    // Enable EMAC
    CFXS_ETH_printf(DebugLevel::TRACE, "Enable EMAC0\n");
    SystemControl::EnablePeripheral(SystemControl::Peripheral::EMAC0);

#ifdef CFXS_HW_ETHERNET_INTERNAL_PHY
    // Enable PHY if in INTERNAL PHY mode
    CFXS_ETH_printf(DebugLevel::TRACE, "Enable EPHY0\n");
    SystemControl::EnablePeripheral(SystemControl::Peripheral::EPHY0);
#endif

//
// Configure for use with whichever PHY the user requires.
//
#if CFXS_HW_ETHERNET_INTERNAL_PHY
    CFXS_ETH_printf(DebugLevel::TRACE, "Configure internal PHY\n");
    ROM_EMACPHYConfigSet(EMAC0_BASE, (EMAC_PHY_TYPE_INTERNAL | EMAC_PHY_INT_MDIX_EN | EMAC_PHY_AN_100B_T_FULL_DUPLEX));
#else
    #if defined(CFXS_HW_ETHERNET_PHY_INTERFACE_MII)
    CFXS_ETH_printf(DebugLevel::TRACE, "Configure external MII PHY\n");
    ROM_EMACPHYConfigSet(EMAC0_BASE, (EMAC_PHY_TYPE_EXTERNAL_MII | EMAC_PHY_INT_MDIX_EN | EMAC_PHY_AN_100B_T_FULL_DUPLEX));
    #elif defined(CFXS_HW_ETHERNET_PHY_INTERFACE_RMII)
    CFXS_ETH_printf(DebugLevel::TRACE, "Configure external RMII PHY\n");
    ROM_EMACPHYConfigSet(EMAC0_BASE, (EMAC_PHY_TYPE_EXTERNAL_RMII | EMAC_PHY_INT_MDIX_EN | EMAC_PHY_AN_100B_T_FULL_DUPLEX));
    #else
        #error External PHY type not defined (CFXS_HW_ETHERNET_PHY_INTERFACE_MII/CFXS_HW_ETHERNET_PHY_INTERFACE_RMII)
    #endif
#endif

    //
    // Initialize the MAC and set the DMA mode.
    //
    ROM_EMACInit(EMAC0_BASE, CFXS::CPU::CLOCK_FREQUENCY, EMAC_BCONFIG_MIXED_BURST | EMAC_BCONFIG_PRIORITY_FIXED, 4, 4, 0);

    ROM_EMACAddrSet(EMAC0_BASE, CFXS_HW_ETHERNET_PHY_ADDRESS, default_mac.GetRawData());

    //
    // Set MAC configuration options.
    //
    ROM_EMACConfigSet(
        EMAC0_BASE,
        (EMAC_CONFIG_FULL_DUPLEX | EMAC_CONFIG_CHECKSUM_OFFLOAD | EMAC_CONFIG_7BYTE_PREAMBLE | EMAC_CONFIG_IF_GAP_96BITS |
         EMAC_CONFIG_USE_MACADDR0 | EMAC_CONFIG_SA_FROM_DESCRIPTOR | EMAC_CONFIG_BO_LIMIT_1024),
        (EMAC_MODE_RX_STORE_FORWARD | EMAC_MODE_TX_STORE_FORWARD | EMAC_MODE_TX_THRESHOLD_64_BYTES | EMAC_MODE_RX_THRESHOLD_64_BYTES),
        0);

    InitializeDescriptors();

#ifdef CFXS_HW_ETHERNET_EXTERNAL_PHY
    ROM_EMACPHYWrite(EMAC0_BASE, CFXS_HW_ETHERNET_PHY_ADDRESS, EPHY_BMCR, EPHY_BMCR_MIIRESET);
    CFXS_ETH_printf(DebugLevel::TRACE, "Resetting external PHY\n");
    while ((ROM_EMACPHYRead(EMAC0_BASE, CFXS_HW_ETHERNET_PHY_ADDRESS, EPHY_BMCR) & EPHY_BMCR_MIIRESET) == EPHY_BMCR_MIIRESET) {
    }
    CFXS_ETH_printf(DebugLevel::TRACE, " - done\n");
#endif

    /* Clear any stray PHY interrupts that may be set. */
    volatile auto tmp_0 = ROM_EMACPHYRead(EMAC0_BASE, CFXS_HW_ETHERNET_PHY_ADDRESS, EPHY_MISR1);
    volatile auto tmp_1 = ROM_EMACPHYRead(EMAC0_BASE, CFXS_HW_ETHERNET_PHY_ADDRESS, EPHY_MISR2);

    /* Configure and enable the link status change interrupt in the PHY. */
    auto scr = ROM_EMACPHYRead(EMAC0_BASE, CFXS_HW_ETHERNET_PHY_ADDRESS, EPHY_SCR);
    scr |= (EPHY_SCR_INTEN_EXT | EPHY_SCR_INTOE_EXT);
    ROM_EMACPHYWrite(EMAC0_BASE, CFXS_HW_ETHERNET_PHY_ADDRESS, EPHY_SCR, scr);
    ROM_EMACPHYWrite(EMAC0_BASE,
                     CFXS_HW_ETHERNET_PHY_ADDRESS,
                     EPHY_MISR1,
                     (EPHY_MISR1_LINKSTATEN | EPHY_MISR1_SPEEDEN | EPHY_MISR1_DUPLEXMEN | EPHY_MISR1_ANCEN));

    /* Read the PHY interrupt status to clear any stray events. */
    volatile auto tmp_2 = ROM_EMACPHYRead(EMAC0_BASE, CFXS_HW_ETHERNET_PHY_ADDRESS, EPHY_MISR1);

    /**
   * Set MAC filtering options.  We receive all broadcast and mui32ticast
   * packets along with those addressed specifically for us.
   */
    ROM_EMACFrameFilterSet(EMAC0_BASE, (EMAC_FRMFILTER_HASH_AND_PERFECT | EMAC_FRMFILTER_PASS_MULTICAST));

    /* Clear any pending MAC interrupts. */
    ROM_EMACIntClear(EMAC0_BASE, ROM_EMACIntStatus(EMAC0_BASE, false));

    /* Enable the Ethernet MAC transmitter and receiver. */
    ROM_EMACTxEnable(EMAC0_BASE);
    ROM_EMACRxEnable(EMAC0_BASE);

    /* Enable the Ethernet RX and TX interrupt source. */
    ROM_EMACIntEnable(
        EMAC0_BASE,
        (EMAC_INT_RECEIVE | EMAC_INT_TRANSMIT | EMAC_INT_TX_STOPPED | EMAC_INT_RX_NO_BUFFER | EMAC_INT_RX_STOPPED | EMAC_INT_PHY));

/* Enable all processor interrupts. */
//IntMasterEnable(); //TODO(KRISTS): MODOFIED BY ME because wtf does lwip have to do with interrupts?

/* Tell the PHY to start an auto-negotiation cycle. */
#ifdef CFXS_HW_ETHERNET_EXTERNAL_PHY
    ROM_EMACPHYWrite(EMAC0_BASE, PHY_PHYS_ADDR, EPHY_BMCR, (EPHY_BMCR_SPEED | EPHY_BMCR_DUPLEXM | EPHY_BMCR_ANEN | EPHY_BMCR_RESTARTAN));
#else
    ROM_EMACPHYWrite(EMAC0_BASE, CFXS_HW_ETHERNET_PHY_ADDRESS, EPHY_BMCR, (EPHY_BMCR_ANEN | EPHY_BMCR_RESTARTAN));
#endif

#if defined(CFXS_HW_ETHERNET_INTERRUPT_MODE)
    InitializeInterruptMode();
#elif defined(CFXS_HW_ETHERNET_POLL_MODE)
    InitializePollingMode();
#else
    #error Ethernet mode not defined (CFXS_HW_ETHERNET_INTERRUPT_MODE/CFXS_HW_ETHERNET_POLL_MODE)
#endif

    memcpy(e_Main_Network_Interface.hwaddr, default_mac.GetRawData(), 6);

    lwIPInit(CFXS::CPU::CLOCK_FREQUENCY,
             default_mac.GetRawData(),
             CFXS::IPv4{"2.0.0.1"}.GetValue(),
             CFXS::IPv4{"255.0.0.0"}.GetValue(),
             CFXS::IPv4::BROADCAST().GetValue(),
             0);

    CFXS::Task::Create(HIGH_PRIORITY, "lwIP Timer", lwIPTimer, 100)->Start();

    CFXS::Task::Create(
        LOW_PRIORITY,
        "Network Rate",
        [](auto...) {
            static CFXS::Time_t lastTime = 0;
            auto diff                    = CFXS::Time::ms - lastTime;
            if (diff == 0)
                diff = 1;
            auto scale = (1000.0f / diff);

            auto cnt_tx                   = s_Ethernet_DataRateCounter_TX;
            s_Ethernet_DataRateCounter_TX = 0;
            auto cnt_rx                   = s_Ethernet_DataRateCounter_RX;
            s_Ethernet_DataRateCounter_RX = 0;
            s_Ethernet_DataRate_TX        = cnt_tx * scale;
            s_Ethernet_DataRate_RX        = cnt_rx * scale;

            lastTime = CFXS::Time::ms;
        },
        1000)
        ->Start();
}
