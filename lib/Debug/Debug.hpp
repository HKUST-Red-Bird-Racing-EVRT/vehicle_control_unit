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

#include <stdint.h>

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

#define DEBUG_THROTTLE (1 && DEBUG)
#define DEBUG_GENERAL (1 && DEBUG)
#define DEBUG_GENERAL_CAN (1 && DEBUG_CAN)

// ===== Simple Serial-Only Debug Functions =====

/**
 * @brief Prints a throttle debug message to the serial console.
 * @param x The message to print.
 * @note Serial exclusive
 */
inline void DBG_THROTTLE(const char *x)
{
#if DEBUG_THROTTLE && DEBUG_SERIAL
    Debug_Serial::print(x);
#endif
}

/**
 * @brief Prints a line to the serial console for throttle debug.
 * @param x The message to print.
 * @note Serial exclusive
 */
inline void DBGLN_THROTTLE(const char *x)
{
#if DEBUG_THROTTLE && DEBUG_SERIAL
    Debug_Serial::println(x);
#endif
}

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

inline void DBG_GENERAL_CAN(uint8_t event_code)
{
#if DEBUG_GENERAL_CAN && DEBUG_CAN
    Debug_CAN::general(event_code);
#endif
}

inline void DBG_GENERAL_CAN(uint8_t event_code, uint16_t value)
{
#if DEBUG_GENERAL_CAN && DEBUG_CAN
    Debug_CAN::general(event_code, value);
#endif
}

#endif // DEBUG_HPP