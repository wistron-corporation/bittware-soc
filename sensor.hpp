#include "smbus.hpp"
#include "i2c-dev.h"

#include <xyz/openbmc_project/Sensor/Value/server.hpp>
#include <xyz/openbmc_project/Sensor/Threshold/Critical/server.hpp>
#include <xyz/openbmc_project/Sensor/Threshold/Warning/server.hpp>

#define TMP431_SLAVE_ADDR 0x4c

typedef struct
{
    u_int64_t value;
    int scale;
} temperature;

namespace phosphor
{
namespace mpSOC
{
using valueIface = sdbusplus::xyz::openbmc_project::Sensor::server::Value;
using criticalInterface =
    sdbusplus::xyz::openbmc_project::Sensor::Threshold::server::Critical;

using warningInterface =
    sdbusplus::xyz::openbmc_project::Sensor::Threshold::server::Warning;

using bittwareIfaces =
    sdbusplus::server::object::object<valueIface, criticalInterface,
                                      warningInterface>;

class sensor : public bittwareIfaces
{
  public:
    sensor() = delete;
    sensor(const sensor&) = delete;
    sensor& operator=(const sensor&) = delete;
    sensor(sensor&&) = delete;
    sensor& operator=(sensor&&) = delete;
    virtual ~sensor() = default;
    sensor(sdbusplus::bus::bus& bus, std::string path, uint8_t busID);
    void getTemp();
    void setSensorThreshold(uint64_t criticalHigh, uint64_t criticalLow,
                             uint64_t maxValue, uint64_t minValue,
                             uint64_t warningHigh, uint64_t warningLow);
    void setSensorValueToDbus(const u_int64_t value);
  private:
    uint8_t busID;
};
}
}