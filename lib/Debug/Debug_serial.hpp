/**
 * @file Debug_serial.hpp
 * @author Planeson, Red Bird Racing
 * @brief Declaration of the Debug_Serial namespace for serial debugging functions
 * @version 1.1
 * @date 2026-01-14
 * @see Debug_serial.cpp
 */

#ifndef DEBUG_SERIAL_HPP
#define DEBUG_SERIAL_HPP

#include <Arduino.h>

/**
 * @brief Namespace for serial debugging functions
 */
namespace Debug_Serial {
    void initialize();

    void print(const char* msg);
    void println(const char* msg);
}

#endif // DEBUG_SERIAL_HPP