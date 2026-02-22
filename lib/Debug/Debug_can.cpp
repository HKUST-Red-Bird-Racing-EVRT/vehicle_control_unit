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