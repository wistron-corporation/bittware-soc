#include "smbus.hpp"

#include "i2c-dev.h"

#define TMP431_SLAVE_ADDR 0x4c
#define TMP431_LOCAL_HIGH_COMMAND 0x00
#define TMP431_LOCAL_LOW_COMMAND 0x15
#define TMP431_LOCAL_HIGH_STEP 10000
#define TMP431_LOCAL_LOW_STEP 625
#define TMP431_TEMPERATURE_WARNING_HIGH 55 * 10000
#define TMP431_TEMPERATURE_CRITICAL_HIGH 60 * 10000
#define TMP431_TEMPERATURE_SCALE -4

typedef struct
{
    u_int64_t value;
    int scale;
} temperature;

namespace phosphor
{
namespace mpSOC
{
class sensor
{
public:
    sensor() = delete;
    sensor(const sensor&) = delete;
    sensor& operator=(const sensor&) = delete;
    sensor(sensor&&) = delete;
    sensor& operator=(sensor&&) = delete;
    virtual ~sensor() = default;
    sensor(uint8_t busID);
    bool getTemp(temperature& tmp);
private:
    uint8_t busID;
};
}
}