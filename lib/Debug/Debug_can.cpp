/**
 * @file Debug_can.cpp
 * @author Planeson, Red Bird Racing
 * @brief Implementation of the Debug_CAN namespace for CAN debugging functions
 * @version 1.1
 * @date 2026-01-14
 * @see Debug_can.h
 */

#include "Debug_can.hpp"

// ignore -Wpedantic warnings for mcp2515.h
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <mcp2515.h>
#pragma GCC diagnostic pop

MCP2515 *Debug_CAN::can_interface = nullptr;

/**
 * @brief Initializes the Debug_CAN interface.
 * It should be called before using any other Debug_CAN functions.
 * 
 * @param can Pointer to the MCP2515 CAN controller instance.
 */
void Debug_CAN::initialize(MCP2515 *can)
{
    if (can == nullptr)
        return;
    can_interface = can;
}

void Debug_CAN::general(uint8_t event_code)
{
    if (!can_interface)
        return;

    can_frame tx_msg;
    tx_msg.can_id = GENERAL_DEBUG_MSG;
    tx_msg.can_dlc = 1;
    tx_msg.data[0] = event_code;
    tx_msg.data[1] = 0;
    tx_msg.data[2] = 0;
    tx_msg.data[3] = 0;
    tx_msg.data[4] = 0;
    tx_msg.data[5] = 0;
    tx_msg.data[6] = 0;
    tx_msg.data[7] = 0;

    can_interface->sendMessage(&tx_msg);
}

void Debug_CAN::general(uint8_t event_code, uint16_t value)
{
    if (!can_interface)
        return;

    can_frame tx_msg;
    tx_msg.can_id = GENERAL_DEBUG_MSG;
    tx_msg.can_dlc = 3;
    tx_msg.data[0] = event_code;
    tx_msg.data[1] = value & 0xFF;
    tx_msg.data[2] = (value >> 8) & 0xFF;
    tx_msg.data[3] = 0;
    tx_msg.data[4] = 0;
    tx_msg.data[5] = 0;
    tx_msg.data[6] = 0;
    tx_msg.data[7] = 0;

    can_interface->sendMessage(&tx_msg);
}
