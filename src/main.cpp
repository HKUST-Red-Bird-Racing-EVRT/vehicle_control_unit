/**
 * @file main.cpp
 * @author Planeson, Red Bird Racing
 * @brief Main VCU program entry point
 * @version 2.2
 * @date 2026-02-24
 */

#include <Arduino.h>
#include "BoardConfig.h"
#include "Pedal.hpp"
#include "BMS.hpp"
#include "Enums.hpp"
#include "CarState.hpp"
#include "Scheduler.hpp"
#include "Curves.hpp"
#include "Telemetry.hpp"
#include "Debug.hpp"

// ignore -Wpedantic warnings for mcp2515.h
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <mcp2515.h>
#pragma GCC diagnostic pop

// ignore -Wunused-parameter warnings for Debug.h
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "Debug.hpp"
#pragma GCC diagnostic pop

// === Pin setup ===
constexpr uint8_t INPUT_COUNT = 5;
constexpr uint8_t OUTPUT_COUNT = 4;
constexpr uint8_t pins_in[INPUT_COUNT] = {DRIVE_MODE_BTN, BRAKE_IN, APPS_5V, APPS_3V3, HALL_SENSOR};
constexpr uint8_t pins_out[OUTPUT_COUNT] = {FRG, BRAKE_LIGHT, BUZZER, BMS_FAILED_LED};

// CAN interfaces
MCP2515 mcp2515_motor(CS_CAN_MOTOR); 
MCP2515 mcp2515_BMS(CS_CAN_BMS);     
MCP2515 mcp2515_DL(CS_CAN_DL);       

#define mcp2515_motor mcp2515_DL
#define mcp2515_BMS mcp2515_DL

constexpr uint8_t NUM_MCP = 3;
MCP2515 MCPS[NUM_MCP] = {mcp2515_motor, mcp2515_BMS, mcp2515_DL};

constexpr uint16_t BUSSIN_MILLIS = 2000;       
constexpr uint16_t BMS_OVERRIDE_MILLIS = 1000; 
constexpr uint16_t BRAKE_THRESHOLD = BRAKE_TABLE[0].in; 

bool brake_pressed = false; 

struct CarState car = {
    {}, // TelemetryFrameAdc
    {}, // TelemetryFrameDigital
    {}, // TelemetryFrameState
    0,  // millis
    0   // status_millis
};

// Global objects
Pedal pedal(mcp2515_motor, car, car.pedal.apps_5v);
BMS bms(mcp2515_BMS, car);
Telemetry telem(mcp2515_DL, car);

void scheduler_pedal() { pedal.sendFrame(); pedal.readMotor(); }
void scheduler_bms() { bms.checkHv(); }
void schedulerTelemetryPedal() { telem.sendPedal(); }
void schedulerTelemetryMotor() { telem.sendMotor(); }
void schedulerTelemetryBms() { telem.sendBms(); }

Scheduler<2, NUM_MCP> scheduler(10000, 500);

void setup()
{
#if DEBUG_SERIAL
    Debug_Serial::initialize();
    DBGLN_GENERAL("===== VCU STARTUP =====");
    DBGLN_GENERAL("Serial initialized");
#endif

    DBGLN_GENERAL("Initializing CAN interfaces...");
    for (uint8_t i = 0; i < NUM_MCP; ++i)
    {
        MCPS[i].reset();
        MCPS[i].setBitrate(CAN_RATE, MCP2515_CRYSTAL_FREQ);
        MCPS[i].setNormalMode();
    }

    for (uint8_t i = 0; i < INPUT_COUNT; ++i) pinMode(pins_in[i], INPUT);
    for (uint8_t i = 0; i < OUTPUT_COUNT; ++i)
    {
        pinMode(pins_out[i], OUTPUT);
        digitalWrite(pins_out[i], LOW);
    }

#if DEBUG_CAN
    Debug_CAN::initialize(&mcp2515_DL);
#endif

    scheduler.addTask(McpIndex::Motor, scheduler_pedal, 1);
    scheduler.addTask(McpIndex::Datalogger, schedulerTelemetryPedal, 1);
    scheduler.addTask(McpIndex::Datalogger, schedulerTelemetryMotor, 1);
    scheduler.addTask(McpIndex::Datalogger, schedulerTelemetryBms, 10);
    
    DBGLN_GENERAL("===== SETUP COMPLETE =====");
}

void loop()
{
    car.millis = millis();
    pedal.update(analogRead(APPS_5V), analogRead(APPS_3V3), analogRead(BRAKE_IN));

    brake_pressed = (car.pedal.brake >= BRAKE_THRESHOLD);
    digitalWrite(BRAKE_LIGHT, brake_pressed ? HIGH : LOW);
    scheduler.update(*micros);

    car.pedal.hall_sensor = analogRead(HALL_SENSOR);

    if (car.pedal.status.bits.force_stop)
    {
        car.pedal.status.bits.car_status = CarStatus::Init;
        digitalWrite(BUZZER, LOW);                          
        digitalWrite(FRG, LOW);                             
        return;                                             
    }

    switch (car.pedal.status.bits.car_status)
    {
    case CarStatus::Drive:
        return; 

    case CarStatus::Init:
        DBGLN_GENERAL("Motor State: INIT. Inhibiting drive.");

        if (digitalRead(DRIVE_MODE_BTN) == BUTTON_ACTIVE && brake_pressed)
        {
            car.pedal.status.bits.car_status = CarStatus::Startin;
            car.status_millis = car.millis;
            scheduler.addTask(McpIndex::Bms, scheduler_bms, 5);
        }
        break;

    case CarStatus::Startin:
        DBGLN_GENERAL("Motor State: STARTIN. Waiting for HV...");

        if (digitalRead(DRIVE_MODE_BTN) != BUTTON_ACTIVE || !brake_pressed)
        {
            car.pedal.status.bits.car_status = CarStatus::Init;
            car.status_millis = car.millis;
            scheduler.removeTask(McpIndex::Bms, scheduler_bms);
            break;
        }
        if (car.pedal.status.bits.hv_ready || (car.millis - car.status_millis >= BMS_OVERRIDE_MILLIS))
        {
            car.pedal.status.bits.car_status = CarStatus::Bussin;
            car.status_millis = car.millis;
            digitalWrite(BUZZER, HIGH);
            scheduler.removeTask(McpIndex::Bms, scheduler_bms);
            break;
        }
        break;

    case CarStatus::Bussin:
        if (car.millis - car.status_millis >= BUSSIN_MILLIS)
        {
            digitalWrite(BUZZER, LOW);
            digitalWrite(FRG, HIGH);
            car.pedal.status.bits.car_status = CarStatus::Drive;
        }
        break;

    default:
        car.pedal.status.bits.state_unknown = true;
        car.pedal.status.bits.car_status = CarStatus::Init;
        car.status_millis = car.millis;
        break;
    }

    if (pedal.pedal_final > THROTTLE_TABLE[0].in) 
    {
        car.pedal.status.bits.car_status = CarStatus::Init;
        car.status_millis = car.millis;
    }
}