/**
 * @file Debug.hpp
 * @author Planeson, Red Bird Racing
 * @brief Debugging macros and functions for serial and CAN output
 * @version 1.1
 * @date 2026-01-15
 * @see Debug_serial, Debug_can
 * @dir Debug @brief The Debug library contains debugging macros and functions for serial and CAN output, allowing for easy toggling of debug messages and separation of concerns between different types of debug information.
 */

#ifndef DEBUG_HPP
#define DEBUG_HPP

#include "Enums.hpp"

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
#define DEBUG_STATUS (1 && DEBUG_SERIAL) // Serial only
#define DEBUG_STATUS_CAR (1 && DEBUG_STATUS)
#define DEBUG_STATUS_BRAKE (1 && DEBUG_STATUS)

// ===== Simple Serial-Only Debug Functions =====


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
 * @brief Prints a status message to the serial console.
 * @param x The message to print.
 * @note Serial exclusive
 */
inline void DBG_STATUS(const char *x)
{
#if DEBUG_STATUS && DEBUG_SERIAL
    Debug_Serial::print(x);
#endif
}

/**
 * @brief Prints a line to the serial console for status debug.
 * @param x The message to print.
 * @note Serial exclusive
 */
inline void DBGLN_STATUS(const char *x)
{
#if DEBUG_STATUS && DEBUG_SERIAL
    Debug_Serial::println(x);
#endif
}

/**
 * @brief Sends throttle fault debug info via CAN or serial (if enabled).
 * Overloads for fault status with or without value.
 * @param fault_status The fault status enum.
 * @param value Optional float value for fault.
 */
inline void DBG_THROTTLE_FAULT(PedalFault fault_status, uint16_t value)
{
#if DEBUG_THROTTLE_FAULT && DEBUG_SERIAL
#if DEBUG_SERIAL
    Debug_Serial::throttle_fault(fault_status, value);
#endif
#endif
}

inline void DBG_THROTTLE_FAULT(PedalFault fault_status)
{
#if DEBUG_THROTTLE_FAULT && DEBUG_SERIAL
#if DEBUG_SERIAL
    Debug_Serial::throttle_fault(fault_status);
#endif
#endif
}

/**
 * @brief Sends BMS debug info via CAN or serial (if enabled).
 * @param BMS_status The BMS status enum.
 */
inline void DBG_BMS_STATUS(BmsStatus BMS_status)
{
#if DEBUG_BRAKE_FAULT && DEBUG_SERIAL
#if DEBUG_SERIAL
    Debug_Serial::status_bms(BMS_status);
#endif
#endif
}

/**
 * @brief Sends a generic 8-byte message over CAN for ad-hoc debugging.
 * * Allows quick injection of arbitrary data onto the CAN bus during testing. 
 * Any trailing data bytes not explicitly provided will automatically default to 0x00.
 * * @param id The CAN ID to transmit the message on.
 * @param d0 Data byte 0 (defaults to 0x00).
 * @param d1 Data byte 1 (defaults to 0x00).
 * @param d2 Data byte 2 (defaults to 0x00).
 * @param d3 Data byte 3 (defaults to 0x00).
 * @param d4 Data byte 4 (defaults to 0x00).
 * @param d5 Data byte 5 (defaults to 0x00).
 * @param d6 Data byte 6 (defaults to 0x00).
 * @param d7 Data byte 7 (defaults to 0x00).
 * @note CAN exclusive
 */
inline void DBG_GENERAL_CAN(canid_t id, uint8_t d0 = 0x00, uint8_t d1 = 0x00, uint8_t d2 = 0x00, uint8_t d3 = 0x00, uint8_t d4 = 0x00, uint8_t d5 = 0x00, uint8_t d6 = 0x00, uint8_t d7 = 0x00)
{
#if DEBUG_CAN
    Debug_CAN::general(id, d0, d1, d2, d3, d4, d5, d6, d7);
#endif
}
#endif // DEBUG_HPP