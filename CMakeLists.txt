# set(CMAKE_UNITY_BUILD true)
# set(CMAKE_UNITY_BUILD_BATCH_SIZE 16)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE asm_sources CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.S"
)

if(${CFXS_HW_MODULE_ETHERNET})
  message("[CFXS Hardware] Ethernet module enabled")

  if(NOT DEFINED CFXS_HW_MODULE_ETHERNET_PHY_TYPE)
    message(FATAL_ERROR "PHY type not set [INTERNAL/EXTERNAL]")
  endif()

  if("${CFXS_HW_MODULE_ETHERNET_PHY_TYPE}" STREQUAL "INTERNAL")
    add_compile_definitions("CFXS_HW_ETHERNET_INTERNAL_PHY")
  elseif("${CFXS_HW_MODULE_ETHERNET_PHY_TYPE}" STREQUAL "EXTERNAL")
    add_compile_definitions("CFXS_HW_ETHERNET_EXTERNAL_PHY")
  else()
    message(FATAL_ERROR "Invalid PHY type: ${CFXS_HW_MODULE_ETHERNET_PHY_TYPE} != [INTERNAL/EXTERNAL]")
  endif()

  message(" - ${CFXS_HW_MODULE_ETHERNET_PHY_TYPE} PHY")

  file(GLOB_RECURSE eth_common_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/Ethernet/Common/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/Ethernet/Common/*.cpp"
  )
  file(GLOB_RECURSE eth_common_asm_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/Ethernet/Common/*.S"
  )

  file(GLOB_RECURSE eth_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/Ethernet/${CFXS_STARTUP_PLATFORM}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/Ethernet/${CFXS_STARTUP_PLATFORM}/*.cpp"
  )
  file(GLOB_RECURSE eth_asm_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/Ethernet/${CFXS_STARTUP_PLATFORM}/*.S"
  )

  list(APPEND sources ${eth_common_sources})
  list(APPEND asm_sources ${eth_common_asm_sources})
  list(APPEND sources ${eth_sources})
  list(APPEND asm_sources ${eth_asm_sources})

  message(" - Adding files:")

  if(NOT "${eth_common_sources}" STREQUAL "")
    message("       [${eth_common_sources}]")
  endif()

  if(NOT "${eth_common_asm_sources}" STREQUAL "")
    message("       [${eth_common_asm_sources}]")
  endif()

  if("${eth_sources}" STREQUAL "")
    message(WARNING "Ethernet platform specific C/C++ sources empty (modules/Ethernet/${CFXS_STARTUP_PLATFORM})")
  else()
    message("       [${eth_sources}]")
  endif()

  if(NOT "${eth_asm_sources}" STREQUAL "")
    message("       [${eth_asm_sources}]")
  endif()

  set(ETH_INCLUDE_DIR_PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/modules/Ethernet")
  set(ETH_PRIVATE_PRECOMPILE_HEADERS
    <driverlib/emac.h>
    <driverlib/rom.h>
    <driverlib/interrupt.h>
    <inc/hw_ints.h>
    <lwip/pbuf.h>
    <inc/hw_emac.h>
    <inc/hw_memmap.h>
    <lwip/opt.h>
    <lwip/def.h>
    <lwip/mem.h>
    <lwip/pbuf.h>
    <lwip/sys.h>
    <lwip/stats.h>
    <lwip/snmp.h>
    <lwip/tcpip.h>
    <netif/etharp.h>
    <inc/hw_types.h>
    <driverlib/debug.h>
    <driverlib/sysctl.h>
    <inc/hw_nvic.h>
  )
endif()

set_source_files_properties(${sources} PROPERTIES LANGUAGE CXX)
set_source_files_properties(${asm_sources} PROPERTIES LANGUAGE ASM)

add_library(CFXS_HW ${sources})
target_include_directories(CFXS_HW PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(CFXS_HW PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(NOT "${ETH_INCLUDE_DIR_PRIVATE}" STREQUAL "")
  target_include_directories(CFXS_HW PRIVATE ${ETH_INCLUDE_DIR_PRIVATE})
endif()

target_precompile_headers(
  CFXS_HW
  PUBLIC
  <stddef.h>
  <stdint.h>
  <stdbool.h>
  <string.h>
  ${ETH_PRIVATE_PRECOMPILE_HEADERS}
)

if(${CFXS_HW_MODULE_ETHERNET})
  target_link_libraries(CFXS_HW PUBLIC lwip)
  target_include_directories(lwip PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/modules/Ethernet/Common/lwip")
endif()