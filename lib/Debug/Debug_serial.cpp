/**
 * @file Debug_serial.cpp
 * @author Planeson, Red Bird Racing
 * @brief Implementation of the Debug_Serial namespace for serial debugging functions
 * @version 1.1
 * @date 2026-01-14
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
 * @brief Prints a throttle fault message to the serial console.
 * This function formats and sends the throttle fault status and value to the serial console.
 * 
 * @param fault_status The status of the throttle fault as defined in PedalFault enum.
 * @param value Optional uint16_t value associated with the fault
 */
void Debug_Serial::throttle_fault(PedalFault fault_status, uint16_t value)
{
    switch (fault_status)
    {
    case PedalFault::None:
        break;
    case PedalFault::DiffContinuing:
        Serial.print("Pedal mismatch continuing. Difference: ");
        Serial.println(value);
        break;
    case PedalFault::ThrottleLow:
        Serial.print("Throttle input too low. Value: ");
        Serial.println(value);
        break;
    case PedalFault::ThrottleHigh:
        Serial.print("Throttle too high. Value: ");
        Serial.println(value);
        break;
    default:
        Serial.println("Unknown fault status");
        break;
    }
}

/**
 * @brief Prints the current BMS status to the serial console.
 * This function formats and sends the current BMS status to the serial console.
 * 
 * @param BMS_status The current status of the BMS as defined in BmsStatus enum.
 */
void Debug_Serial::status_bms(BmsStatus BMS_status)
{
    static BmsStatus last_status = BmsStatus::Unused;
    if (BMS_status == last_status)
        return;
    last_status = BMS_status;

    switch (BMS_status)
    {
    case BmsStatus::NoMsg:
        Serial.println("BMS Status: No message received");
        break;
    case BmsStatus::WrongId:
        Serial.println("BMS Status: Wrong ID");
        break;
    case BmsStatus::Waiting:
        Serial.println("BMS Status: Waiting to start");
        break;
    case BmsStatus::Starting:
        Serial.println("BMS Status: Starting");
        break;
    case BmsStatus::Started:
        Serial.println("BMS Status: Started");
        break;
    default:
        Serial.println("BMS Status: UNKNOWN");
        break;
    }
}