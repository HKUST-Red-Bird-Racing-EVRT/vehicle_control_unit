/**
 * @file Pedal.cpp
 * @author Planeson, Chiho, Red Bird Racing
 * @brief Implementation of the Pedal class for handling throttle pedal inputs
 * @version 1.7
 * @date 2026-02-25
 * @see Pedal.hpp
 */

#include "Pedal.hpp"
#include "SignalProcessing.hpp"
#include "CarState.hpp"
#include <stdint.h>
#include "Queue.hpp"
#include "CarState.hpp"
#include "Interp.hpp"
#include "Curves.hpp"

// ignore -Wunused-parameter warnings for Debug.h
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "Debug.hpp" // DBGLN_GENERAL
#pragma GCC diagnostic pop

/**
 * @brief Constructor for the Pedal class.
 * Initializes the pedal state. fault is set to true initially,
 * so you must send update within 100ms of starting the car to clear it.
 * Call the initMotor function to set up the motor CAN filters and cyclic reads after constructing the Pedal object and the MCP2515 object it references.
 * @param motor_can_ Reference to the MCP2515 instance for motor CAN communication.
 * @param car_ Reference to the CarState structure.
 * @param pedal_final_ Reference to the pedal used as the final pedal value. Although not recommended, you can set another uint16 outside Pedal to be something like 0.3 APPS_1 + 0.7 APPS_2, then reference that here. If in future, this become a sustained need, should consider adding a function pointer to find the final pedal value to let Pedal class call it itself.
 */
Pedal::Pedal(MCP2515 &motor_can_, CarState &car_, uint16_t &pedal_final_)
    : pedal_final(pedal_final_),
      car(car_),
      motor_can(motor_can_),
      fault_start_millis(0),
      last_motor_read_millis(0),
      got_speed(false),
      got_error(false)
{
}

/**
 * @brief Initializes the CAN filters for reading motor data.
 * Call after constructing the Pedal object and the MCP2515 object it references to ensure motor data is being read correctly.
 * This function will block until the filter is set correctly on the MCP2515. If it never succeed, the program will be stuck here.
 * Consider that when we can't even set the filter on the MCP2515, we probably can't communicate with the motor controller at all,
 * so being stuck here is acceptable since the car won't be drivable without motor communication anyway.
 */
void Pedal::initFilter()
{
    //  set MCU CAN filter
    motor_can.setConfigMode();
    while (motor_can.setFilterMask(MCP2515::MASK0, false, 0x7FF) != MCP2515::ERROR_OK)
        ;
    while (motor_can.setFilter(MCP2515::RXF0, false, MOTOR_READ) != MCP2515::ERROR_OK)
        ;
    motor_can.setNormalMode();
}

/**
 * @brief Initializes the motor CAN communication by setting up cyclic reads for motor data and configuring CAN filters.
 * After the constructor of the Pedal class and the MCP2515 object it references are created,
 * first call initFilter to set up the filters,
 * then call this function to start the cyclic reads and ensure that motor data is being read correctly.
 * 
 * @return true if both motor speed and error data are being successfully read, false otherwise, can be used for looping
 * @see initFilter
 */
bool Pedal::initMotor()
{
    if (!got_speed)
    {
        while (sendCyclicRead(SPEED_IST, RPM_PERIOD) != MCP2515::ERROR_OK)
            ;
        got_speed = checkCyclicRead(SPEED_IST);
    }
    if (!got_error)
    {
        while (sendCyclicRead(WARN_ERR, ERR_PERIOD) != MCP2515::ERROR_OK)
            ;
        got_error = checkCyclicRead(WARN_ERR);
    }
    return got_speed && got_error;
}

/**
 * @brief Updates pedal sensor readings, applies filtering, and checks for faults.
 *
 * Stores new pedal readings, applies an average filter, and updates car state.
 * If a fault is detected between pedal sensors, sets fault flags and logs status.
 *
 * @param pedal_1 Raw value from pedal sensor 1.
 * @param pedal_2 Raw value from pedal sensor 2.
 * @param brake Raw value from brake sensor.
 */
void Pedal::update(uint16_t pedal_1, uint16_t pedal_2, uint16_t brake)
{
    // Add new samples to the filters
    pedal1_filter.addSample(pedal_1);
    pedal2_filter.addSample(pedal_2);
    brake_filter.addSample(brake);

    if (pedal_1 < APPS_5V_MIN)
        car.pedal.faults.bits.apps_5v_low = true;
    if (pedal_1 > APPS_5V_MAX)
        car.pedal.faults.bits.apps_5v_high = true;
    if (pedal_2 < APPS_3V3_MIN)
        car.pedal.faults.bits.apps_3v3_low = true;
    if (pedal_2 > APPS_3V3_MAX)
        car.pedal.faults.bits.apps_3v3_high = true;
    if (brake < brake_min)
        car.pedal.faults.bits.brake_low = true;
    if (brake > brake_max)
        car.pedal.faults.bits.brake_high = true;

    if (checkPedalFault())
    {
        // fault now
        if (car.pedal.faults.bits.fault_active)
        {
            // was faulty already, check time
            if (car.millis - fault_start_millis > 100)
            {
                car.pedal.faults.bits.fault_exceeded = true;
                car.pedal.status.bits.force_stop = true; // critical fault, force stop; since early return, need set here
                return;
            }
        }
        else
        {
            // new fault
            fault_start_millis = car.millis;
        }
        car.pedal.faults.bits.fault_active = true;
    }
    else
    {
        // no fault
        car.pedal.faults.bits.fault_active = false;
    }

    if (car.pedal.faults.byte & FAULT_CHECK_HEX)
    {
        car.pedal.status.bits.force_stop = true; // critical fault, force stop
    }

    return;
}

/**
 * @brief Sends the appropriate CAN frame to the motor based on pedal and car state.
 */
void Pedal::sendFrame()
{
    // Update Telemetry struct
    car.pedal.apps_5v = pedal1_filter.getFiltered();
    car.pedal.apps_3v3 = pedal2_filter.getFiltered();
    car.pedal.brake = brake_filter.getFiltered();

    if (false && car.pedal.status.bits.force_stop)
    {
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
    return;
}

/**
 * @brief Maps the pedal ADC to a torque value.
 * If no braking requested, maps throttle normally.
 * If braking requested and regen enabled,
 *      applies regen if motor RPM larger than minimum regen RPM,
 *      preventing reverse torque at low speeds.
 *      Regen is also disabled if motor rpm isn't read recently to prevent reverse power.
 *
 * @param pedal Pedal ADC in the range of 0-1023.
 * @param brake Brake ADC in the range of 0-1023.
 * @param motor_rpm Current motor RPM for regen logic, scaled to 0-32767.
 * @param flip_dir Boolean indicating whether to flip the motor direction.
 * @return Mapped torque value in the signed range of -TORQUE_MAX to TORQUE_MAX.
 */
constexpr int16_t Pedal::pedalTorqueMapping(const uint16_t pedal, const uint16_t brake, const int16_t motor_rpm, const bool flip_dir)
{
    if (REGEN_ENABLED && brake > BRAKE_MAP.start() && !car.pedal.status.bits.motor_no_read)
    {
        if (pedal > THROTTLE_MAP.start())
        {
            car.pedal.status.bits.screenshot = true;
            // to ensure BSPD can be tested, skip regen if both throttle and brake pressed
        }
        else if (!flip_dir)
        {
            if (motor_rpm < PedalConstants::MIN_REGEN_RPM_VAL)
                return 0;
            else

                return BRAKE_MAP.interp(brake);
        }
        else
        {
            if (motor_rpm > -PedalConstants::MIN_REGEN_RPM_VAL)
                return 0;
            else
                return -BRAKE_MAP.interp(brake);
        }
    }

    if (flip_dir)
        return -THROTTLE_MAP.interp(pedal);
    else
        return THROTTLE_MAP.interp(pedal);
}

/**
 * @brief Checks for a fault between two pedal sensor readings.
 *
 * Scales pedal_2 to match the range of pedal_1, then calculates the absolute difference.
 * If the difference exceeds 10% of the full-scale value (i.e., >102.4 for a 10-bit ADC),
 * the function considers this a fault and returns true. Otherwise, returns false.
 *
 * @return true if the difference exceeds the threshold (fault detected), false otherwise.
 */
bool Pedal::checkPedalFault()
{
    if (car.pedal.apps_5v < APPS_5V_PERCENT_TABLE[0].in)
    {
        return false;
    }
    const int16_t delta = (int16_t)car.pedal.apps_5v - (int16_t)APPS_3V3_SCALE_MAP.interp(car.pedal.apps_3v3);
    constexpr int16_t MAX_DELTA = THROTTLE_MAP.range() / 10; /**< MAX_DELTA is floor of 10% of APPS_5V valid range, later comparison will give rounding room */
    // if more than 10% difference between the two pedals, consider it a fault
    if (delta > MAX_DELTA || delta < -MAX_DELTA)
    {
        return true;
    }
    return false;
}

/**
 * @brief Sends a cyclic read request to the motor controller for speed (rpm).
 * @param reg_id Register ID to read from the motor controller.
 * @param read_period Period of reading motor data in ms.
 */
MCP2515::ERROR Pedal::sendCyclicRead(const uint8_t reg_id, const uint8_t read_period)
{
    const can_frame cyclic_request = {
        MOTOR_SEND, /**< can_id */
        3,          /**< can_dlc */
        REGID_READ, /**< data, register ID */
        reg_id,     /**< data, sub ID */
        read_period /**< data, read period in ms */
    };
    return motor_can.sendMessage(&cyclic_request);
}

bool Pedal::checkCyclicRead(const uint8_t reg_id)
{
    can_frame rx_frame;
    if (motor_can.readMessage(&rx_frame) == MCP2515::ERROR_OK &&
        rx_frame.can_id == MOTOR_READ &&
        rx_frame.can_dlc > 3 &&
        rx_frame.data[0] == reg_id)
    {
        return true;
    }
    return false;
}

/**
 * @brief Reads motor data from the CAN bus and updates the CarState.
 */
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
    return;
}