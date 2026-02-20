

#include <SD.h>
#include "CarState.hpp"

class FlashStorage
{
public:
    FlashStorage(const uint8_t pin, CarState &car_state);
    void writeTelemetry();

private:
    CarState &car;
    File dataFile;
};