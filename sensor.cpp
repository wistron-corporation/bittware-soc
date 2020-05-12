#include "sensor.hpp"

#include <iostream>

#define TMP431_LOCAL_HIGH_COMMAND 0x00
#define TMP431_LOCAL_LOW_COMMAND 0x15
#define TMP431_LOCAL_HIGH_STEP 10000
#define TMP431_LOCAL_LOW_STEP 625
#define TMP431_TEMPERATURE_MULTIPLIER 10000
#define TMP431_TEMPERATURE_SCALE -4

namespace phosphor
{
namespace mpSOC
{
sensor::sensor(sdbusplus::bus::bus& bus, std::string path, uint8_t busID) :
    busID(busID),
    bittwareIfaces(bus, path.c_str())
{
    valueIface::scale(TMP431_TEMPERATURE_SCALE);
}

static inline temperature caculate(uint8_t high, uint8_t low)
{
    return {(high * TMP431_LOCAL_HIGH_STEP) +
        ((low >> 4) * TMP431_LOCAL_LOW_STEP), TMP431_TEMPERATURE_SCALE};
}

void sensor::setSensorThreshold(uint64_t criticalHigh, uint64_t criticalLow,
                                 uint64_t maxValue, uint64_t minValue,
                                 uint64_t warningHigh, uint64_t warningLow)
{
    criticalInterface::criticalHigh(criticalHigh * TMP431_TEMPERATURE_MULTIPLIER);
    criticalInterface::criticalLow(criticalLow * TMP431_TEMPERATURE_MULTIPLIER);

    warningInterface::warningHigh(warningHigh * TMP431_TEMPERATURE_MULTIPLIER);
    warningInterface::warningLow(warningLow * TMP431_TEMPERATURE_MULTIPLIER);

    valueIface::maxValue(maxValue * TMP431_TEMPERATURE_MULTIPLIER);
    valueIface::minValue(minValue * TMP431_TEMPERATURE_MULTIPLIER);
}

void sensor::setSensorValueToDbus(const u_int64_t value)
{
    valueIface::value(value);
}

void sensor::getTemp()
{
    auto bus = phosphor::smbus::Smbus();
    auto res = bus.smbusInit(busID);
    if (res != -1)
    {
        auto exist = bus.smbusCheckSlave(busID, TMP431_SLAVE_ADDR);
        if (exist)
        {
            auto high = bus.GetSmbusCmdByte(busID, TMP431_SLAVE_ADDR, TMP431_LOCAL_HIGH_COMMAND);
            auto low = bus.GetSmbusCmdByte(busID, TMP431_SLAVE_ADDR, TMP431_LOCAL_LOW_COMMAND);
            auto tmp = caculate(high, low);
            setSensorValueToDbus(tmp.value);
        }
        else
        {
            std::cerr << "Temperature sensor not exist" <<std::endl;
        }
    }
    else
    {
        std::cerr << "smbusInit fail!" << std::endl;
    }
    bus.smbusClose(busID);
}
}
}