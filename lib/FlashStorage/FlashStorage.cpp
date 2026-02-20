
#include "Debug.hpp"
#include "FlashStorage.hpp"

FlashStorage::FlashStorage(const uint8_t pin, CarState &car_state) : car(car_state)
{
    car.pedal.status.bits.sd_ready = false;
    if (!SD.begin(pin))
    {
        DBG_GENERAL("SD card initialization failed!");
        return;
    }
    DBG_GENERAL("SD card initialized successfully.");

    int count = 0;
    if (!SD.exists("count.txt"))
    {
        File countFile = SD.open("count.txt", FILE_WRITE);
        if (countFile)
        {
            countFile.println(0); // Initialize count to 0
            countFile.close();
        }
        else
        {
            DBG_GENERAL("Error creating count file!");
        }
    }
    else
    {
        File countFile = SD.open("count.txt", FILE_READ);
        if (countFile)
        {
            count = countFile.parseInt();
            countFile.close();

            // Increment count and write back
            count++;
            countFile = SD.open("count.txt", FILE_WRITE);
            if (countFile)
            {
                countFile.println(count);
                countFile.close();
                DBG_GENERAL("Count updated.");
            }
            else
            {
                DBG_GENERAL("Error opening count file for writing!");
            }
        }
        else
        {
            DBG_GENERAL("Error opening count file for reading!");
        }
    }
    dataFile = SD.open("telemetry" + String(count) + ".txt", FILE_WRITE);
    if (dataFile)
    {
        car.pedal.status.bits.sd_ready = true;
        DBG_GENERAL("Telemetry file created successfully.");
    }
    else
    {
        DBG_GENERAL("Error creating telemetry file!");
    }
    // keep the telemetry file open for writing during the lifetime of the FlashStorage object
}

void FlashStorage::writeTelemetry()
{
    if (!dataFile)
    {
        DBG_GENERAL("Telemetry file not open for writing!");
        return;
    }
    else
    {
        // Write telemetry data in a simple CSV format: time, pedal, motor, bms
        dataFile.print(car.millis);
        dataFile.print(",");
        dataFile.print(car.pedal.apps_5v);
        dataFile.print(",");
        dataFile.print(car.pedal.apps_3v3);
        dataFile.print(",");
        dataFile.print(car.pedal.brake);
        dataFile.print(",");
        dataFile.print(car.pedal.hall_sensor);
        dataFile.print(",");
        dataFile.print(car.pedal.status.byte);
        dataFile.print(",");
        dataFile.print(car.pedal.faults.byte);
        dataFile.print(",");
        dataFile.print(car.motor.torque_val);
        dataFile.print(",");
        dataFile.print(car.motor.motor_rpm);
        dataFile.print(",");
        dataFile.print(car.motor.motor_error);
        dataFile.print(",");
        dataFile.print(car.motor.motor_warn);
        dataFile.print(",");
        dataFile.print(car.bms.bms_data[0]);
        dataFile.print(",");
        dataFile.print(car.bms.bms_data[1]);
        dataFile.print(",");
        dataFile.print(car.bms.bms_data[2]);
        dataFile.print(",");
        dataFile.print(car.bms.bms_data[3]);
        dataFile.print(",");
        dataFile.print(car.bms.bms_data[4]);
        dataFile.print(",");
        dataFile.print(car.bms.bms_data[5]);
        dataFile.print(",");
        dataFile.print(car.bms.bms_data[6]);
        dataFile.print(",");
        dataFile.println(car.bms.bms_data[7]);
        // don't flush for speed, we are still getting the "latest" data anyway
    }
}