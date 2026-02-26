/**
 * @file Debug_can.hpp
 * @author Planeson, Chiho, Red Bird Racing
 * @brief Declaration of the Debug_CAN namespace for CAN debugging functions
 * @version 2.0
 * @date 2026-02-25
 * @see Debug_can.cpp
 */

#ifndef DEBUG_CAN_HPP
#define DEBUG_CAN_HPP

// ignore -Wpedantic warnings for mcp2515.h
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <mcp2515.h>
#pragma GCC diagnostic pop

#include "Enums.hpp"

/**
 * @brief Namespace for CAN debugging functions
 */
namespace Debug_CAN
{
    extern MCP2515 *can_interface; /**< Pointer to the MCP2515 CAN controller instance. */

    void initialize(MCP2515 *can_interface);
// deleted repeated throttle fault as it is been declared in telemetry already
    void send_message(
        canid_t id,
        uint8_t data0 = 0x00, 
        uint8_t data1 = 0x00, 
        uint8_t data2 = 0x00,
        uint8_t data3 = 0x00,
        uint8_t data4 = 0x00, 
        uint8_t data5 = 0x00,
        uint8_t data6 = 0x00, 
        uint8_t data7 = 0x00);

    constexpr canid_t THROTTLE_FAULT_MSG = 0x692;     /**< Debug: throttle fault message */
}

#endif // DEBUG_CAN_HPP
