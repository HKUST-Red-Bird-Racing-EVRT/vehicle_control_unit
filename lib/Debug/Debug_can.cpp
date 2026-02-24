/**
 * @file Debug_can.cpp
 * @author Planeson, Red Bird Racing
 * @brief Implementation of the Debug_CAN namespace for CAN debugging functions
 * @version 1.2
 * @date 2026-02-17
 * @see Debug_can.hpp
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

/**
 * @brief Sends arbitrary debug data via CAN with specified message ID and data bytes.
 * 
 * @param id CAN message ID
 * @param data0 Byte 0 of CAN data (default 0x00)
 * @param data1 Byte 1 of CAN data (default 0x00)
 * @param data2 Byte 2 of CAN data (default 0x00)
 * @param data3 Byte 3 of CAN data (default 0x00)
 * @param data4 Byte 4 of CAN data (default 0x00)
 * @param data5 Byte 5 of CAN data (default 0x00)
 * @param data6 Byte 6 of CAN data (default 0x00)
 * @param data7 Byte 7 of CAN data (default 0x00)
 */
void Debug_CAN::send_message(
    canid_t id,
    uint8_t data0, 
    uint8_t data1, 
    uint8_t data2, 
    uint8_t data3,
    uint8_t data4, 
    uint8_t data5, 
    uint8_t data6, 
    uint8_t data7)
{
    if (!can_interface)
        return;

    can_frame tx_msg;
    tx_msg.can_id = id;
    tx_msg.can_dlc = 8;

    tx_msg.data[0] = data0;
    tx_msg.data[1] = data1;
    tx_msg.data[2] = data2;
    tx_msg.data[3] = data3;
    tx_msg.data[4] = data4;
    tx_msg.data[5] = data5;
    tx_msg.data[6] = data6;
    tx_msg.data[7] = data7;

    can_interface->sendMessage(&tx_msg);
}