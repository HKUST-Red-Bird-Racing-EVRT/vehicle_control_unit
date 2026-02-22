/**
 * @file Debug_can.hpp
 * @author Planeson, Red Bird Racing
 * @brief Declaration of the Debug_CAN namespace for CAN debugging functions
 * @version 1.1
 * @date 2026-01-14
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
}

#endif // DEBUG_CAN_HPP