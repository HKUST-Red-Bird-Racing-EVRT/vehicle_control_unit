/**
 * @file Debug.hpp
 * @author Planeson, Chiho, Red Bird Racing
 * @brief Debugging macros and functions for serial and CAN output
 * @version 2.0
 * @date 2026-02-25
 * @see Debug_serial, Debug_can
 * @dir Debug @brief The Debug library contains debugging macros and functions for serial and CAN output, allowing for easy toggling of debug messages and separation of concerns between different types of debug information.
 */

#ifndef DEBUG_HPP
#define DEBUG_HPP

#include "Enums.hpp"

// ignore -Wpedantic warnings for mcp2515.h
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <mcp2515.h>
#pragma GCC diagnostic pop

// === Debug Flags ===
#define DEBUG 1                  // if 0, all debug messages are ignored
#define DEBUG_SERIAL (1 && DEBUG) // if 0, all serial debug messages are ignored
#define DEBUG_CAN (1 && DEBUG)    // if 0, all CAN debug messages are ignored

#if DEBUG_SERIAL
#include <Debug_serial.hpp>
#endif

#if DEBUG_CAN
#include <Debug_can.hpp>
#endif

#define DEBUG_GENERAL 1

// ===== Debug Macros for Serial Output =====

/**
 * @brief Prints a general debug message to the serial console.
 * @param x The message to print.
 * @note Serial exclusive
 */
inline void DBG_GENERAL(const char *x)
{
#if DEBUG_GENERAL && DEBUG_SERIAL
    Debug_Serial::print(x);
#endif
}

/**
 * @brief Prints a line to the serial console for general debug.
 * @param x The message to print.
 * @note Serial exclusive
 */
inline void DBGLN_GENERAL(const char *x)
{
#if DEBUG_GENERAL && DEBUG_SERIAL
    Debug_Serial::println(x);
#endif
}



/**
 * @brief Sends arbitrary debug data via CAN with specified message ID and data bytes.
 * @param id CAN message ID
 * @param data0 Byte 0 of CAN data (default 0x00)
 * @param data1 Byte 1 of CAN data (default 0x00)
 * @param data2 Byte 2 of CAN data (default 0x00)
 * @param data3 Byte 3 of CAN data (default 0x00)
 * @param data4 Byte 4 of CAN data (default 0x00)
 * @param data5 Byte 5 of CAN data (default 0x00)
 * @param data6 Byte 6 of CAN data (default 0x00)
 * @param data7 Byte 7 of CAN data (default 0x00)
 * @note CAN exclusive
 */
inline void DBG_GENERAL_CAN(canid_t id, uint8_t data0 = 0x00, uint8_t data1 = 0x00,
                            uint8_t data2 = 0x00, 
                            uint8_t data3 = 0x00, 
                            uint8_t data4 = 0x00,
                            uint8_t data5 = 0x00, 
                            uint8_t data6 = 0x00, 
                            uint8_t data7 = 0x00)
{
#if DEBUG && DEBUG_CAN
    Debug_CAN::send_message(id, data0, data1, data2, data3, data4, data5, data6, data7);
#endif
}

#endif // DEBUG_HPP