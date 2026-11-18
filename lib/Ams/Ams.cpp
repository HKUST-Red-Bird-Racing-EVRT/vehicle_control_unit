/**
 * @file Ams.cpp
 * @author Planeson (carson.cpk@proton.me)
 *         Chiho (nganchihon@gmail.com)
 * @brief Implementation of the Ams class for managing the Accumulator Management System (AMS) via CANbus
 * @version 2.0
 * @date 2026-11-10
 * @see Ams.hpp
 */

#include "Ams.hpp"
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
 * @brief Construct a new Ams object, initing car.pedal.status.bits.hv_ready to false
 * @param ams_can_ Reference to MCP2515 for AMS CAN bus
 * @param car_ Reference to CarState, for the status flags and setting AMS data
 */
Ams::Ams(MCP2515 &ams_can_, CarState &car_)
    : ams_can(ams_can_), car(car_)
{
    car.pedal.status.bits.hv_ready = false;
    car.pedal.status.bits.ams_no_msg = false;

}

/**
 * @brief Attempts to start HV.
 * First check BMS is in standby(3) state, then send the HV start command.
 * Keep sending the command until the BMS state changes to precharge(4).
 * Sets car.pedal.status.bits.hv_ready to true when BMS state changes to run(5).
 *
 */
void Ams::checkHv()
{
    if (car.pedal.status.bits.hv_ready)
        return; // already started, no need to check again
    ams_can.sendMessage(&start_hv_msg);
    car.pedal.status.bits.hv_ready = true; // fake start, suppose is listen for frame, if received then set true, else timeout
    return; // send start HV command, wait for BMS to change state
}

/**
 * @brief 
 * 
 */
void Ams::readAms()
{
    // read AMS CAN frame, set car.pedal.status.bits.ams_no_msg if no message received
    if (ams_can.readMessage(&rx_ams_msg) == MCP2515::ERROR::ERROR_NOMSG)
    {
        if (car.millis - last_ams_msg_millis > AMS_TIMEOUT_MILLIS)
        {
            car.pedal.status.bits.ams_no_msg = true;
        }
    }
    else
    {
        last_ams_msg_millis = car.millis;
        car.pedal.status.bits.ams_no_msg = false;
        switch (rx_ams_msg.can_id)
        {
            break;
        }
        return;
    }
}