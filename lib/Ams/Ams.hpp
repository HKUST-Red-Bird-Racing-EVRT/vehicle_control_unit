/**
 * @file Ams.hpp
 * @author Planeson (carson.cpk@proton.me)
 * @brief Declaration of the Ams class for managing the Accumulator Management System (AMS) via CANbus
 * @version 2.0
 * @date 2026-11-10
 * @see Ams.cpp
 * 
 * @copyright Copyright (c) 2026 Red Bird Racing
 * 
 * @dir Ams @brief The AMS library contains the Ams class for managing the Accumulator Management System (AMS) via CANbus, including starting HV and checking AMS status.
 */

#ifndef AMS_HPP
#define AMS_HPP

#include "Scheduler.hpp"
#include "CarState.hpp"

// ignore -Wpedantic warnings for mcp2515.h
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <mcp2515.h>
#pragma GCC diagnostic pop

constexpr uint32_t AMS_TIMEOUT_MILLIS = 1000; /**< Timeout for AMS response in milliseconds */

constexpr canid_t AMS_SEND_CMD = 0x500; /**< CAN ID for sending commands to the AMS */
constexpr uint8_t AMD_CMD_START_HV = 0x01; /**< Command to start HV */

/** Start HV command frame */
constexpr can_frame start_hv_msg = {
    AMS_SEND_CMD, /**< can_id */
    1,            /**< can_dlc */
    {AMD_CMD_START_HV}  /**< data: MainRlyCmd = 1 (output HV), ShutDownCmd = 1 (do not shutdown) */
};

/**
 * @brief Ams class for managing the Accumulator Management System (AMS) via CANbus
 */
class Ams
{
public:
    Ams(MCP2515 &ams_can_, CarState &car_);
    /**
     * @brief Returns true if HV has been started
     * @return true if HV started, false otherwise
     */
    bool hvReady() const { return car.pedal.status.bits.hv_ready; };
    void checkHv();
    void readAms();

private:
    MCP2515 &ams_can; /**< Reference to MCP2515 for AMS CAN bus */
    /** Local storage for received AMS CAN frame */
    can_frame rx_ams_msg = {
        0,   /**< can_id */
        0,   /**< can_dlc */
        {0}, /**< data */
    };
    CarState &car;       /**< Reference to CarState, for the status flags and setting AMS data */
    uint32_t last_ams_msg_millis = 0; /**< Millisecond timestamp of the last received AMS message */
};
#endif // AMS_HPP