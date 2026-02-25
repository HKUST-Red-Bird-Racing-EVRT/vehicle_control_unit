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

    void throttle_fault(PedalFault fault_status, uint16_t value);
    void throttle_fault(PedalFault fault_status);
    void brake_fault(PedalFault fault_status, uint16_t value);

    void status_brake(uint16_t brake_voltage);
    void status_bms(BmsStatus BMS_status);
    void general(canid_t id, uint8_t d0 = 0x00, uint8_t d1 = 0x00, uint8_t d2 = 0x00, uint8_t d3 = 0x00, uint8_t d4 = 0x00, uint8_t d5 = 0x00, uint8_t d6 = 0x00, uint8_t d7 = 0x00);


    constexpr canid_t THROTTLE_IN_MSG = 0x690;        /**< Debug: throttle input message */
    constexpr canid_t THROTTLE_OUT_MSG = 0x691;       /**< Debug: throttle output message */
    constexpr canid_t THROTTLE_FAULT_MSG = 0x692;     /**< Debug: throttle fault message */
    constexpr canid_t STATUS_CAR_MSG = 0x693;         /**< Debug: car status message */
    constexpr canid_t STATUS_CAR_CHANGE_MSG = 0x694;  /**< Debug: car status change message */
    constexpr canid_t STATUS_BRAKE_MSG = 0x695;       /**< Debug: brake status message */
    constexpr canid_t STATUS_BMS_MSG = 0x696;         /**< Debug: BMS status message */
    constexpr canid_t STATUS_HALL_SENSOR_MSG = 0x697; /**< Debug: Hall sensor message */
    
}

#endif // DEBUG_CAN_HPP