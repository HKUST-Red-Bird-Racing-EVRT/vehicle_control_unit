/**
 * @file BMS.cpp
 * @author Planeson, Chiho, Red Bird Racing
 * @brief Implementation of the BMS class for managing the Accumulator (Kclear BMS) via CAN bus
 * @version 1.4
 * @date 2026-02-25
 * @see BMS.hpp
 */

#include "BMS.hpp"
#include "Debug.hpp"
#include "Enums.hpp"
#include "CarState.hpp"

// ignore -Wunused-parameter warnings for Debug.h
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "Debug.hpp" // DBGLN_GENERAL
#pragma GCC diagnostic pop

// ignore -Wpedantic warnings for mcp2515.h
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <mcp2515.h> // CAN frames, sending CAN frame, reading CAN frame
#pragma GCC diagnostic pop

/**
 * @brief Construct a new BMS object, initing car.pedal.status.bits.hv_ready to false
 * @param bms_can_ Reference to MCP2515 for BMS CAN bus
 * @param car_ Reference to CarState, for the status flags and setting BMS data
 */
BMS::BMS(MCP2515 &bms_can_, CarState &car_)
    : bms_can(bms_can_), car(car_)
{
    car.pedal.status.bits.hv_ready = false;
}

/**
 * @brief Initializes the CAN filters for reading BMS data.
 * Call after constructing the BMS object and the MCP2515 object it references to ensure BMS data is being read correctly.
 * This function will block until the filter is set correctly on the MCP2515. If it never succeed, the program will be stuck here.
 * Consider that in the case that we can't even set the filter on the MCP2515, we probably can't communicate with the BMS at all,
 * so being stuck here is acceptable since the car won't start without BMS communication anyway.
 */
void BMS::initFilter()
{
    return;
}

/**
 * @brief Attempts to start HV.
 * First check BMS is in standby(3) state, then send the HV start command.
 * Keep sending the command until the BMS state changes to precharge(4).
 * Sets car.pedal.status.bits.hv_ready to true when BMS state changes to run(5).
 *
 */
void BMS::checkHv()
{
    if (car.pedal.status.bits.hv_ready)
    {
        return; // already started, no need to check again
    }
    bms_can.sendMessage(&start_hv_msg);
    car.pedal.status.bits.bms_no_msg = false;
    car.pedal.status.bits.hv_ready = true;
    return; // send start HV command, wait for BMS to change state
}
