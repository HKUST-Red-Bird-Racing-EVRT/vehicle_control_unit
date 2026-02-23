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

inline void DBG_GENERAL(const char *x)
{
#if DEBUG_GENERAL && DEBUG_SERIAL
    Debug_Serial::print(x);
#endif
}

inline void DBGLN_GENERAL(const char *x) 
{
#if DEBUG_GENERAL && DEBUG_SERIAL
    Debug_Serial::println(x);
#endif
}

#endif // DEBUG_HPP