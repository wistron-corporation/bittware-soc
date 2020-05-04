#include "smbus.hpp"
#include "config.h"

#include <string>
#include <map>
#include <utility>

namespace phosphor
{
namespace mpSOC
{
class vpd
{
public:
    vpd();
    vpd(uint8_t busID, uint8_t eepromAddr);
    vpd(const vpd&) = delete;
    vpd& operator=(const vpd&) = delete;
    vpd(vpd&&) = delete;
    vpd& operator=(vpd&&) = delete;
    virtual ~vpd() = default;
    std::map<std::string, std::string> vpdData;
    void verifyChecksum(uint8_t offset);
    void read();
    void parse();
private:
    bool parseField(const uint8_t fieldOffset, uint8_t& nextField);
    unsigned char rawData[I2C_DATA_MAX];
    uint8_t eepromAddr;
    bool idChecked;
    bool checksumVerified;
    uint8_t busID;
};
}
}