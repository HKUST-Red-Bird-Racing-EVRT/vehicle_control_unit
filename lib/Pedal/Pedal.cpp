/**
 * @file Pedal.cpp
 * @author Planeson, Red Bird Racing
 * @brief Implementation of the Pedal class for handling throttle pedal inputs
 * @version 1.7
 * @date 2026-02-24
 */

#include "Pedal.hpp"
#include "SignalProcessing.hpp"
#include "CarState.hpp"
#include <stdint.h>
#include "Queue.hpp"
#include "Interp.hpp"
#include "Curves.hpp"

// ignore -Wunused-parameter warnings for Debug.h
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "Debug.hpp" 
#pragma GCC diagnostic pop

Pedal::Pedal(MCP2515 &motor_can_, CarState &car_, uint16_t &pedal_final_)
    : pedal_final(pedal_final_),
      car(car_),
      motor_can(motor_can_),
      fault_start_millis(0),
      last_motor_read_millis(0)
{
    while (sendCyclicRead(SPEED_IST, RPM_PERIOD) != MCP2515::ERROR_OK);
    while (sendCyclicRead(WARN_ERR, ERR_PERIOD) != MCP2515::ERROR_OK);
    while (motor_can.setFilter(MCP2515::RXF0, false, MOTOR_READ) != MCP2515::ERROR_OK);
}

void Pedal::update(uint16_t pedal_1, uint16_t pedal_2, uint16_t brake)
{
    pedal1_filter.addSample(pedal_1);
    pedal2_filter.addSample(pedal_2);
    brake_filter.addSample(brake);

    if (pedal_1 < APPS_5V_MIN) car.pedal.faults.bits.apps_5v_low = true;
    if (pedal_1 > APPS_5V_MAX) car.pedal.faults.bits.apps_5v_high = true;
    if (pedal_2 < APPS_3V3_MIN) car.pedal.faults.bits.apps_3v3_low = true;
    if (pedal_2 > APPS_3V3_MAX) car.pedal.faults.bits.apps_3v3_high = true;
    if (brake < brake_min) car.pedal.faults.bits.brake_low = true;
    if (brake > brake_max) car.pedal.faults.bits.brake_high = true;

    if (checkPedalFault())
    {
        if (car.pedal.faults.bits.fault_active)
        {
            if (car.millis - fault_start_millis > 100)
            {
                car.pedal.faults.bits.fault_exceeded = true;
                car.pedal.status.bits.force_stop = true; 
                DBGLN_GENERAL("FAULT: Pedal difference exceeded 100ms");
            }
        }
        else
        {
            fault_start_millis = car.millis;
            DBGLN_GENERAL("FAULT: Pedal difference detected");
        }
        car.pedal.faults.bits.fault_active = true;
    }
    else
    {
        if (car.pedal.faults.bits.fault_active)
        {
            DBGLN_GENERAL("FAULT: Pedal difference resolved");
        }
        car.pedal.faults.bits.fault_active = false;
    }

    if (car.pedal.faults.byte & FAULT_CHECK_HEX)
    {
        car.pedal.status.bits.force_stop = true; 
    }
}

void Pedal::sendFrame()
{
    car.pedal.apps_5v = pedal1_filter.getFiltered();
    car.pedal.apps_3v3 = pedal2_filter.getFiltered();
    car.pedal.brake = brake_filter.getFiltered();

    if (car.pedal.status.bits.force_stop)
    {
        DBGLN_GENERAL("Stopping motor: force_stop active");
        motor_can.sendMessage(&stop_frame);
        return;
    }

    if (car.pedal.status.bits.car_status != CarStatus::Drive)
    {
        motor_can.sendMessage(&stop_frame);
        return;
    }

    car.motor.torque_val = pedalTorqueMapping(pedal_final, car.pedal.brake, car.motor.motor_rpm, FLIP_MOTOR_DIR);

    torque_msg.data[1] = car.motor.torque_val & 0xFF;
    torque_msg.data[2] = (car.motor.torque_val >> 8) & 0xFF;
    motor_can.sendMessage(&torque_msg);
}

constexpr int16_t Pedal::pedalTorqueMapping(const uint16_t pedal, const uint16_t brake, const int16_t motor_rpm, const bool flip_dir)
{
    if (REGEN_ENABLED && brake > BRAKE_MAP.start() && !car.pedal.status.bits.motor_no_read)
    {
        if (pedal > THROTTLE_MAP.start())
        {
            car.pedal.status.bits.screenshot = true;
        }
        else if (flip_dir)
        {
            if (motor_rpm < PedalConstants::MIN_REGEN_RPM_VAL) return 0;
            else return -BRAKE_MAP.interp(brake);
        }
        else
        {
            if (motor_rpm > -PedalConstants::MIN_REGEN_RPM_VAL) return 0;
            else return BRAKE_MAP.interp(brake);
        }
    }

    if (flip_dir) return -THROTTLE_MAP.interp(pedal);
    else return THROTTLE_MAP.interp(pedal);
}

bool Pedal::checkPedalFault()
{
    const int16_t delta = (int16_t)car.pedal.apps_5v - (int16_t)APPS_3V3_SCALE_MAP.interp(car.pedal.apps_3v3);
    constexpr int16_t MAX_DELTA = THROTTLE_MAP.range() / 10; 
    
    if (delta > MAX_DELTA || delta < -MAX_DELTA)
    {
        return true;
    }
    return false;
}

MCP2515::ERROR Pedal::sendCyclicRead(uint8_t reg_id, uint8_t read_period)
{
    can_frame cyclic_request = { MOTOR_SEND, 3, REGID_READ, reg_id, read_period };
    return motor_can.sendMessage(&cyclic_request);
}

void Pedal::readMotor()
{
    can_frame rx_frame;
    if (motor_can.readMessage(&rx_frame) == MCP2515::ERROR_OK)
    {
        if (rx_frame.can_id == MOTOR_READ && rx_frame.can_dlc > 3)
        {
            if (rx_frame.data[0] == SPEED_IST)
            {
                last_motor_read_millis = car.millis;
                car.pedal.status.bits.motor_no_read = false;
                car.motor.motor_rpm = static_cast<int16_t>(rx_frame.data[1] | (rx_frame.data[2] << 8));
                return;
            }
            else if (rx_frame.data[0] == WARN_ERR)
            {
                car.motor.motor_error = static_cast<uint16_t>(rx_frame.data[1] | (rx_frame.data[2] << 8));
                car.motor.motor_warn = static_cast<uint16_t>(rx_frame.data[3] | (rx_frame.data[4] << 8));
                return;
            }
        }
    }
    if (car.millis - last_motor_read_millis > MAX_MOTOR_READ_MILLIS)
    {
        car.pedal.status.bits.motor_no_read = true;
    }
}