/**
 * @file Debug_serial.cpp
 * @author Planeson, Red Bird Racing
 * @brief Implementation of the Debug_Serial namespace for serial debugging functions
 * @version 1.1.1
 * @date 2026-02-09
 * @see Debug_serial.h
 */

#include "Debug_serial.hpp"
#include "Enums.hpp"

/**
 * @brief Initializes the Debug_Serial interface.
 * It should be called before using any other Debug_Serial functions.
 * 
 */
void Debug_Serial::initialize()
{
    Serial.begin(115200);
}

/**
 * @brief Prints a string to the serial console.
 * 
 * @param msg The message to print.
 * @note Serial exclusive
 */
void Debug_Serial::print(const char *msg) { Serial.print(msg); }

/**
 * @brief Prints a string AND a newline to the serial console.
 * 
 * @param msg The message to print.
 * @note Serial exclusive
 */
void Debug_Serial::println(const char *msg) { Serial.println(msg); }

