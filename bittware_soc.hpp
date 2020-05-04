#include "config.h"

#include "sdbusplus.hpp"
#include "vpd.hpp"
#include "sensor.hpp"

#include <sdbusplus/bus.hpp>
#include <sdbusplus/server.hpp>
#include <sdbusplus/server/object.hpp>
#include <sdeventplus/clock.hpp>
#include <sdeventplus/event.hpp>
#include <sdeventplus/utility/timer.hpp>
#include <xyz/openbmc_project/Sensor/Value/server.hpp>
#include <xyz/openbmc_project/Sensor/Threshold/Critical/server.hpp>
#include <xyz/openbmc_project/Sensor/Threshold/Warning/server.hpp>

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

using keywordInfo = std::tuple<std::string, std::string, uint8_t>;

/** @class bittwareSOC
 *  @brief bittwareSOC manager implementation.
 */
class bittwareSOC : public bittwareIfaces
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
        void setSensorValueToDbus(const u_int64_t value);
        /** @brief Reading temperature of Bittware 250 SoC  */
        void read();
        bool present;
    private:
        uint8_t index;
        /** @brief sdbusplus bus client connection. */
        sdbusplus::bus::bus& bus;
        /** @brief the Event Loop structure */
        sdeventplus::Event _event;
        /** @brief Read Timer */
        sdeventplus::utility::Timer<sdeventplus::ClockId::Monotonic> _timer;
        /** @brief the temperature sensor on bittware SoC */
        sensor tmpSensor;
        bittwareConfig config;
        /** @brief Set up initial configuration value of 250 SoC */
        void init();
        bool smbusEnable(int busID, uint8_t addr);
};
}
}
