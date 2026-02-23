/**
 * @file BMS.cpp
 * @author Planeson, Red Bird Racing
 * @brief Implementation of the BMS class for managing the Accumulator (Kclear BMS) via CAN bus
 * @version 1.3.1
 * @date 2026-02-12
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
    bms_can.setConfigMode();
    while (bms_can.setFilterMask(MCP2515::MASK0, true, 0x7FF) != MCP2515::ERROR_OK)
        ;
    while (bms_can.setFilter(MCP2515::RXF0, true, BMS_INFO_EXT) != MCP2515::ERROR_OK)
        ;
    bms_can.setNormalMode();
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
    car.pedal.status.bits.bms_no_msg = false;
    if (car.pedal.status.bits.hv_ready)
    return; // already started
    car.pedal.status.bits.hv_ready = false;
    if (bms_can.readMessage(&rx_bms_msg) == MCP2515::ERROR_NOMSG)
    {
        DBG_BMS_STATUS(BmsStatus::NoMsg);
        car.pedal.status.bits.bms_no_msg = true;
        return;
    }

    /* now that we set filter on MCP2515, it's impossible to get wrong ID
    // Check if the BMS is in standby state (0x3 in upper 4 bits)
    if (rx_bms_msg.can_id != BMS_INFO_EXT)
    {
        DBG_BMS_STATUS(BmsStatus::WrongId);
        car.pedal.status.bits.bms_wrong_id = true;
        return;
    } // Not a BMS info frame, retry
    */

    switch (rx_bms_msg.data[6] & 0xF0)
    {
    case 0x30: // Standby state
        DBG_BMS_STATUS(BmsStatus::Waiting);
        bms_can.sendMessage(&start_hv_msg);
        DBGLN_GENERAL("BMS in standby state, sent start HV cmd");
        // sent start HV cmd, wait for BMS to change state
        return;
    case 0x40: // Precharge state
        DBG_BMS_STATUS(BmsStatus::Starting);
        bms_can.sendMessage(&start_hv_msg);
        DBGLN_GENERAL("BMS in precharge state, HV starting");
        return; // BMS is in precharge state, wait
    case 0x50:  // Run state
        DBG_BMS_STATUS(BmsStatus::Started);
        DBGLN_GENERAL("BMS in run state, HV started");
        car.pedal.status.bits.hv_ready = true; // BMS is in run state
        return;
    default:
        DBG_BMS_STATUS(BmsStatus::Unused);
        DBGLN_GENERAL("BMS in unknown state, retrying...");
        return; // Unknown state, retry
    }
}
