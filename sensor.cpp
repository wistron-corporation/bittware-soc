#include "sensor.hpp"

#include <iostream>

namespace phosphor
{
namespace mpSOC
{
sensor::sensor(uint8_t busID) :
    busID(busID)
{
}

static inline temperature caculate(uint8_t high, uint8_t low)
{
    return {(high * TMP431_LOCAL_HIGH_STEP) + (low * TMP431_LOCAL_LOW_STEP), TMP431_TEMPERATURE_SCALE};
}

bool sensor::getTemp(temperature& tmp)
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
            tmp = caculate(high, low);
            //std::cout << "value = " << tmp.value << std::endl;
        }
        else
        {
            std::cerr << "Temperature sensor not exist" <<std::endl;
            return false;
        }
    }
    else
    {
        std::cerr << "smbusInit fail!" << std::endl;
        return false;
    }
    bus.smbusClose(busID);
    return true;
}
}
}