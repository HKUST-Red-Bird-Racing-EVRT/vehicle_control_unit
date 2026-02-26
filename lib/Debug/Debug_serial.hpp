/**
 * @file Debug_serial.hpp
 * @author Planeson, Chiho, Red Bird Racing
 * @brief Declaration of the Debug_Serial namespace for serial debugging functions
 * @version 2.0
 * @date 2026-02-25
 * @see Debug_serial.cpp
 */

#ifndef DEBUG_SERIAL_HPP
#define DEBUG_SERIAL_HPP

#include <Arduino.h>
#include "Enums.hpp"

/**
 * @brief Namespace for serial debugging functions
 */
namespace Debug_Serial {
    void initialize();

    void print(const char* msg);
    void println(const char* msg);
    
   // telemetry repeated
}

#endif // DEBUG_SERIAL_HPP
