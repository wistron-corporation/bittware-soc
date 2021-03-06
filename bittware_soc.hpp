#include "vpd.hpp"
#include "sensor.hpp"

namespace phosphor
{
namespace mpSOC
{
using keywordInfo = std::tuple<std::string, std::string, uint8_t>;

/** @class bittwareSOC
 *  @brief bittwareSOC manager implementation.
 */
class bittwareSOC
{
  public:
    bittwareSOC() = delete;
    bittwareSOC(const bittwareSOC&) = delete;
    bittwareSOC& operator=(const bittwareSOC&) = delete;
    bittwareSOC(bittwareSOC&&) = delete;
    bittwareSOC& operator=(bittwareSOC&&) = delete;
    virtual ~bittwareSOC() = default;

    struct bittwareConfig
    {
        uint8_t index;
        uint8_t busID;
        uint64_t criticalHigh;
        uint64_t criticalLow;
        uint64_t maxValue;
        uint64_t minValue;
        uint64_t warningHigh;
        uint64_t warningLow;
    };

    /** @brief Constructs bittwareSOC
     *
     * @param[in] bus     - Handle to system dbus
     * @param[in] objPath - The dbus path of bittwareSOC
     */
    bittwareSOC(uint8_t index, sdbusplus::bus::bus& bus, bittwareConfig config);
    void createInventory();
    void setInventoryProperties(const bool& present, const vpd& vpdDev);
    /** @brief Reading temperature of Bittware 250 SoC  */
    void read();
    bool present;
  private:
    uint8_t index;
    /** @brief sdbusplus bus client connection. */
    sdbusplus::bus::bus& bus;
    /** @brief the temperature sensor on bittware SoC */
    std::shared_ptr<sensor> tmpSensor;
    bittwareConfig config;
    /** @brief Set up initial configuration value of 250 SoC */
    void init();
    bool smbusEnable(int busID, uint8_t addr);
};
}
}