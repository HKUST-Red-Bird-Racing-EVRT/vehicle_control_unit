/**
 * @file can_test.cpp
 * @author Chiho, Red Bird Racing
 * @brief Standalone CAN bring-up test for VCU v3.3, built only by `pio run -e can_test`
 * @details
 *
 * Reading the result on a PCAN:
 * - nothing at all on a bus     -> spi_ok/loopback fail (check OUT_4) or wrong bitrate/crystal
 * - TEC climbs to 128, no frames -> no other node ACKing, or wrong MCP2515_CRYSTAL_FREQ
 * - int_pin stays 1 while sending frames to the VCU -> INT line not wired to that pin
 * OUT_4 (output walk off): solid = both chips passed SPI + loopback, fast blink = something failed.
 */

#include <Arduino.h>
#include "BoardConfig.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <mcp2515.h>
#pragma GCC diagnostic pop

#ifndef MOTOR_INT
#error "can_test targets VCU v3.3, select USE_VCU_V3_3 in BoardConfig.h"
#endif

constexpr uint8_t NUM_BUS = 2;
constexpr uint16_t STATUS_ID = 0x7F0;
constexpr uint16_t ECHO_ID = 0x7F8;
constexpr uint16_t LOOPBACK_ID = 0x123;
constexpr uint16_t ANALOG_ID = 0x7F2;
constexpr uint16_t DIGITAL_ID = 0x7F3;

#ifndef CAN_TEST_OUTPUTS
#define CAN_TEST_OUTPUTS 0 // 1 = walk OUT_1-4, drives FRG!
#endif
constexpr uint8_t outputs[] = {OUT_1, OUT_2, OUT_3, OUT_4};
constexpr uint8_t dins[] = {DIN5V_1, DIN5V_2, DIN5V_3, DIN5V_4};
constexpr uint8_t analogs[] = {APPS_5V, APPS_3V3, BRAKE_IN, HALL_SENSOR};

MCP2515 mcps[NUM_BUS] = {MCP2515(CS_CAN_MOTOR), MCP2515(CS_CAN_DL)};
constexpr uint8_t int_pins[NUM_BUS] = {MOTOR_INT, DEBUG_INT};

struct BusState
{
    bool spi_ok;
    bool loopback_ok;
    uint8_t last_tx_err;
    uint8_t rx_count;
};
BusState bus[NUM_BUS] = {};

bool all_ok = true;
uint8_t counter = 0;
uint32_t last_send_ms = 0;
uint8_t reset_cause = 0;
uint8_t out_step = 0; // 0 = all off, n = OUT_n on

bool loopbackTest(MCP2515 &mcp)
{
    if (mcp.setLoopbackMode() != MCP2515::ERROR_OK)
        return false;

    can_frame tx = {};
    tx.can_id = LOOPBACK_ID;
    tx.can_dlc = 4;
    tx.data[0] = 0xDE;
    tx.data[1] = 0xAD;
    tx.data[2] = 0xBE;
    tx.data[3] = 0xEF;
    if (mcp.sendMessage(&tx) != MCP2515::ERROR_OK)
        return false;

    can_frame rx = {};
    const uint32_t start = millis();
    while (millis() - start < 20)
    {
        if (mcp.readMessage(&rx) == MCP2515::ERROR_OK)
            return rx.can_id == tx.can_id && rx.can_dlc == tx.can_dlc && memcmp(rx.data, tx.data, tx.can_dlc) == 0;
    }
    return false;
}

void sendIo(MCP2515 &mcp)
{
    can_frame tx = {};
    tx.can_id = ANALOG_ID;
    tx.can_dlc = 8;
    for (uint8_t i = 0; i < sizeof(analogs); ++i)
    {
        const uint16_t v = analogRead(analogs[i]);
        tx.data[2 * i] = v & 0xFF;
        tx.data[2 * i + 1] = v >> 8;
    }
    mcp.sendMessage(&tx);

    uint8_t din_mask = 0;
    for (uint8_t i = 0; i < sizeof(dins); ++i)
        din_mask |= digitalRead(dins[i]) << i;

    const uint16_t current_lo = analogRead(CURRENT_LO);
    tx.can_id = DIGITAL_ID;
    tx.can_dlc = 6;
    tx.data[0] = current_lo & 0xFF;
    tx.data[1] = current_lo >> 8;
    tx.data[2] = din_mask;
    tx.data[3] = digitalRead(AMS_INT);
    tx.data[4] = reset_cause;
    tx.data[5] = out_step;
    mcp.sendMessage(&tx);
}

void setup()
{
    reset_cause = MCUSR;
    MCUSR = 0;

    // keep real loads off: brake light, buzzer, FRG
    for (uint8_t pin : outputs)
    {
        digitalWrite(pin, LOW);
        pinMode(pin, OUTPUT);
    }
    delay(500);
    for (uint8_t pin : outputs)
    {
        digitalWrite(pin, HIGH);
        pinMode(pin, OUTPUT);
    }
    for (uint8_t i = 0; i < NUM_BUS; ++i)
        pinMode(int_pins[i], INPUT); // MCP2515 INT is push-pull active low
    for (uint8_t pin : dins)
        pinMode(pin, INPUT);
    pinMode(AMS_INT, INPUT);

    for (uint8_t i = 0; i < NUM_BUS; ++i)
    {
        mcps[i].reset();
        bus[i].spi_ok = mcps[i].setBitrate(CAN_RATE, MCP2515_CRYSTAL_FREQ) == MCP2515::ERROR_OK;
        bus[i].loopback_ok = bus[i].spi_ok && loopbackTest(mcps[i]);
        mcps[i].setNormalMode();
        all_ok = all_ok && bus[i].spi_ok && bus[i].loopback_ok;
    }

}

void loop()
{
    for (uint8_t i = 0; i < NUM_BUS; ++i)
    {
        can_frame rx;
        while (mcps[i].readMessage(&rx) == MCP2515::ERROR_OK)
        {
            ++bus[i].rx_count;
            if ((rx.can_id & CAN_SFF_MASK) >= STATUS_ID && (rx.can_id & CAN_SFF_MASK) <= 0x7FF)
                continue; // don't echo our own IDs, avoids ping-pong if both chips share a bus
            rx.can_id = ECHO_ID + i;
            mcps[i].sendMessage(&rx);
        }
    }

    const uint32_t now = millis();
    if (now - last_send_ms < 100)
        return;
    last_send_ms = now;
    ++counter;

    if (CAN_TEST_OUTPUTS)
    {
        if (counter % 10 == 0) // 1 s per step
        {
            out_step = (out_step + 1) % (sizeof(outputs) + 1);
            for (uint8_t i = 0; i < sizeof(outputs); ++i){
                digitalWrite(outputs[i], out_step == i + 1 ? HIGH : LOW);
            }
        }
    }
    else if (!all_ok)
        digitalWrite(OUT_4, counter & 1);

    for (uint8_t i = 0; i < NUM_BUS; ++i)
    {
        can_frame tx = {};
        tx.can_id = STATUS_ID + i;
        tx.can_dlc = 8;
        tx.data[0] = counter;
        tx.data[1] = bus[i].spi_ok;
        tx.data[2] = bus[i].loopback_ok;
        tx.data[3] = bus[i].last_tx_err;
        tx.data[4] = bus[i].rx_count;
        tx.data[5] = digitalRead(int_pins[i]);
        tx.data[6] = mcps[i].errorCountTX();
        tx.data[7] = mcps[i].errorCountRX();
        bus[i].last_tx_err = mcps[i].sendMessage(&tx);
        sendIo(mcps[i]);
    }
    delay(500);
}
