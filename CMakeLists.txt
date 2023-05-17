set(LIBRARY_NAME CFXS_HW)

# set(CMAKE_UNITY_BUILD true)
# set(CMAKE_UNITY_BUILD_BATCH_SIZE 16)
file(GLOB_RECURSE sources CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE asm_sources CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.S"
)

set_source_files_properties(${sources} PROPERTIES LANGUAGE CXX)
set_source_files_properties(${asm_sources} PROPERTIES LANGUAGE ASM)

add_library(${LIBRARY_NAME} ${sources})
target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${LIBRARY_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_precompile_headers(
  ${LIBRARY_NAME}
  PRIVATE
  <stddef.h>
  <stdint.h>
  <stdbool.h>
  <string.h>
)

if(${CFXS_HW_MODULE_ETHERNET})
  message("[CFXS Hardware] Ethernet module enabled")

  target_link_libraries(${LIBRARY_NAME} PRIVATE lwip)
  target_include_directories(lwip PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/modules/Ethernet/Common/lwip")

  if(NOT DEFINED CFXS_HW_MODULE_ETHERNET_PHY_TYPE)
    message(FATAL_ERROR "PHY type not set (CFXS_HW_MODULE_ETHERNET_PHY_TYPE) [INTERNAL/EXTERNAL]")
  endif()

  if(NOT DEFINED CFXS_HW_MODULE_ETHERNET_MODE)
    message(FATAL_ERROR "Ethernet mode not set (CFXS_HW_MODULE_ETHERNET_MODE) [INTERRUPT/POLL]")
  endif()

  if("${CFXS_HW_MODULE_ETHERNET_PHY_TYPE}" STREQUAL "INTERNAL")
    target_compile_definitions(${LIBRARY_NAME} PRIVATE "CFXS_HW_ETHERNET_INTERNAL_PHY")
  elseif("${CFXS_HW_MODULE_ETHERNET_PHY_TYPE}" STREQUAL "EXTERNAL")
    target_compile_definitions(${LIBRARY_NAME} PRIVATE "CFXS_HW_ETHERNET_EXTERNAL_PHY")
  else()
    message(FATAL_ERROR "Invalid PHY type: ${CFXS_HW_MODULE_ETHERNET_PHY_TYPE} != [INTERNAL/EXTERNAL]")
  endif()

  if("${CFXS_HW_MODULE_ETHERNET_MODE}" STREQUAL "INTERRUPT")
    target_compile_definitions(${LIBRARY_NAME} PRIVATE "CFXS_HW_ETHERNET_INTERRUPT_MODE")

    if(NOT DEFINED CFXS_HW_MODULE_ETHERNET_INTERRUPT_PRIORITY)
      message(FATAL_ERROR "Interrupt priority not set (CFXS_HW_MODULE_ETHERNET_INTERRUPT_PRIORITY)")
    endif()

    target_compile_definitions(${LIBRARY_NAME} PRIVATE "CFXS_HW_ETHERNET_INTERRUPT_PRIORITY=${CFXS_HW_MODULE_ETHERNET_INTERRUPT_PRIORITY}")

  elseif("${CFXS_HW_MODULE_ETHERNET_MODE}" STREQUAL "POLL")
    target_compile_definitions(${LIBRARY_NAME} PRIVATE "CFXS_HW_ETHERNET_POLL_MODE")
  else()
    message(FATAL_ERROR "Invalid ethernet mode: ${CFXS_HW_MODULE_ETHERNET_MODE} != [INTERRUPT/POLL]")
  endif()

  message(" - ${CFXS_HW_MODULE_ETHERNET_PHY_TYPE} PHY")

  if("${CFXS_HW_MODULE_ETHERNET_MODE}" STREQUAL "INTERRUPT")
    message(" - ${CFXS_HW_MODULE_ETHERNET_MODE} Mode [Priority ${CFXS_HW_MODULE_ETHERNET_INTERRUPT_PRIORITY}]")
  else()
    message(" - ${CFXS_HW_MODULE_ETHERNET_MODE} Mode")
  endif()

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

  set_source_files_properties(${eth_common_sources} PROPERTIES LANGUAGE CXX)
  set_source_files_properties(${eth_common_asm_sources} PROPERTIES LANGUAGE ASM)
  set_source_files_properties(${eth_sources} PROPERTIES LANGUAGE CXX)
  set_source_files_properties(${eth_asm_sources} PROPERTIES LANGUAGE ASM)

  if("${eth_sources}" STREQUAL "")
    message(WARNING "Ethernet platform specific C/C++ sources empty (modules/Ethernet/${CFXS_STARTUP_PLATFORM})")
  else()
  endif()

  if(NOT "${eth_common_sources}" STREQUAL "")
    target_sources(${LIBRARY_NAME} PRIVATE ${eth_common_sources})
  endif()

  if(NOT "${eth_common_asm_sources}" STREQUAL "")
    target_sources(${LIBRARY_NAME} PRIVATE ${eth_common_asm_sources})
  endif()

  if(NOT "${eth_sources}" STREQUAL "")
    target_sources(${LIBRARY_NAME} PRIVATE ${eth_sources})
  endif()

  if(NOT "${eth_asm_sources}" STREQUAL "")
    target_sources(${LIBRARY_NAME} PRIVATE ${eth_asm_sources})
  endif()

  target_include_directories(${LIBRARY_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/modules/Ethernet")

  if(${CFXS_PLATFORM} STREQUAL "TM4C")
    target_precompile_headers(
      ${LIBRARY_NAME}
      PRIVATE
      <driverlib/emac.h>
      <driverlib/rom.h>
      <driverlib/interrupt.h>
      <driverlib/debug.h>
      <driverlib/sysctl.h>
      <inc/hw_emac.h>
      <inc/hw_memmap.h>
      <inc/hw_ints.h>
      <inc/hw_types.h>
      <inc/hw_nvic.h>
    )

    target_link_libraries(${LIBRARY_NAME} PRIVATE tm4c_driverlib)
  else()
    message(FATAL_ERROR "CFXS_HW unsupported platform")
  endif()

  target_precompile_headers(
    ${LIBRARY_NAME}
    PRIVATE
    <lwip/pbuf.h>
    <lwip/opt.h>
    <lwip/def.h>
    <lwip/mem.h>
    <lwip/pbuf.h>
    <lwip/sys.h>
    <lwip/stats.h>
    <lwip/snmp.h>
    <lwip/tcpip.h>
    <netif/etharp.h>
  )
endif()

target_link_libraries(${LIBRARY_NAME} PRIVATE CFXS_Toolchain)
target_link_libraries(${LIBRARY_NAME} PRIVATE CFXS_Platform)
target_link_libraries(${LIBRARY_NAME} PRIVATE CFXS_Base)