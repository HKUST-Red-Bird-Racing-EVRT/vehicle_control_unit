/**
 * @file Enums.hpp
 * @author Planeson, Red Bird Racing
 * @brief Enumeration definitions for the VCU
 * @version 1.3.1
 * @date 2026-02-09
 */

#ifndef ENUMS_HPP
#define ENUMS_HPP
#include <stdint.h>
#include <can.h> // canid_t

/**
 * @brief Main car status state machine.
 *
 * Represents the current state of the car in the startup and drive sequence.
 */
enum class CarStatus : uint8_t
{
    Init = 0,    /**< Just started the car */
    Startin = 1, /**< Driver holds "Start" button and full brakes (transition state) */
    Bussin = 2,  /**< Buzzer active, driver can release "Start" and brakes (transition state) */
    Drive = 3    /**< Ready to drive, Drive mode LED on, throttle enabled */
};

/**
 * @brief AMS status
 *
 * Represents the current state of the Accumulator Management System (AMS).
 */
enum class AmsStatus : uint8_t
{
    Init = 0,      /**< BDUCU waiting for VCU/charger signal */
    Precharge = 1, /**< Precharging, AIR- and PRE closed */
    Active = 2,    /**< TS running, AIR- and AIR+ closed */
    Discharge = 3, /**< SDC tripped, BDUCU loses control of relays, all relays open */
};

/**
 * @brief MCP2515 instance indices.
 *
 * Used to identify different MCP2515 CAN controller instances.
 */
enum class McpIndex : uint8_t
{
    Motor = 0,     /**< Motor CAN MCP2515 instance */
    Bms = 1,       /**< BMS CAN MCP2515 instance */
    Datalogger = 2 /**< Datalogger CAN MCP2515 instance */
};

// === CAN IDs ===

/**
 * @brief CAN message IDs for status and brake debug.
 *
 * Used for sending car status and brake messages over CAN bus.
 */
enum class StatusCanId : canid_t
{
    CarMsg = 0x693,          /**< Debug: car status message */
    StaCarChangeMsg = 0x694, /**< Debug: car status change message */
    BrakeMsg = 0x695,        /**< Debug: brake status message */
    BmsMsg = 0x696,          /**< Debug: BMS status message */
    HallSensorMsg = 0x697    /**< Debug: Hall sensor message */
};

#endif // ENUMS_HPP