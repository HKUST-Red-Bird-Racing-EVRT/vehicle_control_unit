/**
 * @file Telemetry.cpp
 * @author Planeson (carson.cpk@proton.me)
 * @brief Implementation of the Telemetry class for sending telemetry data over CAN bus
 * @version 1.0.1
 * @date 2026-11-10
 * @see Telemetry.hpp
 * 
 * @copyright Copyright (c) 2026 Red Bird Racing
 */

#include "Telemetry.hpp"

/**
 * @brief Construct a new Telemetry object
 * @param mcp2515_ Reference to MCP2515 for sending CAN messages
 * @param car_ Reference to CarState
 */
Telemetry::Telemetry(MCP2515 &mcp2515_, CarState &car_)
    : mcp2515(mcp2515_), car(car_)
{
}

/**
 * @brief Internal helper to get and send the Pedal telemetry frame
 */
void Telemetry::sendPedal()
{
    can_frame pedal_frame = car.pedal.toCanFrame();
    mcp2515.sendMessage(&pedal_frame);
}

/**
 * @brief Internal helper to get and send the motor telemetry frame
 */
void Telemetry::sendMotor()
{
    can_frame motor_frame = car.motor.toCanFrame();
    mcp2515.sendMessage(&motor_frame);
}

/**
 * @brief Internal helper to get and send the BMS telemetry frame
 */
void Telemetry::sendBms()
{
    can_frame ams_frame = car.ams.toCanFrame();
    mcp2515.sendMessage(&ams_frame);
}