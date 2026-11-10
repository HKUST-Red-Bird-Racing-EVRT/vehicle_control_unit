/**
 * @file BMS.hpp
 * @author Planeson, Red Bird Racing
 * @brief Declaration of the BMS class for managing the Accumulator (Kclear BMS) via CAN bus
 * @version 1.2
 * @date 2026-02-04
 * @see BMS.cpp
 * @dir BMS @brief The BMS library contains the BMS class for managing the Accumulator (Kclear BMS) via CAN bus, including starting HV and checking BMS status.
 */

#ifndef BMS_HPP
#define BMS_HPP

#include "Scheduler.hpp"
#include "CarState.hpp"

// ignore -Wpedantic warnings for mcp2515.h
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <mcp2515.h>
#pragma GCC diagnostic pop

constexpr uint16_t BDUCU_ID = 0x500;

/** Start HV command frame */
constexpr can_frame start_hv_msg = {
    BDUCU_ID, /**< can_id */
    1,        /**< can_dlc */
    {0x01}    /**< data: VCU is host */
};

/**
 * @brief BMS class for managing the Accumulator (Kclear BMS) via CAN bus
 */
class BMS
{
public:
    BMS(MCP2515 &bms_can_, CarState &car_);
    /**
     * @brief Returns true if HV has been started
     * @return true if HV started, false otherwise
     */
    bool hvReady() const { return car.pedal.status.bits.hv_ready; };
    void initFilter();
    void checkHv();

private:
    MCP2515 &bms_can; /**< Reference to MCP2515 for BMS CAN bus */
    /** Local storage for received BMS CAN frame */
    can_frame rx_bms_msg = {
        0,   /**< can_id */
        0,   /**< can_dlc */
        {0}, /**< data */
    };
    CarState &car; /**< Reference to CarState, for the status flags and setting BMS data */
};
#endif // BMS_HPP