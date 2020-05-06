#include "config.h"
#include "bittware_soc.hpp"

namespace phosphor
{
namespace mpSOC
{
/** @class bittwareManager
 *  @brief bittwareManager manager implementation.
 */
class bittwareManager
{
  public:
    bittwareManager() = delete;
    bittwareManager(const bittwareManager&) = delete;
    bittwareManager& operator=(const bittwareManager&) = delete;
    bittwareManager(bittwareManager&&) = delete;
    bittwareManager& operator=(bittwareManager&&) = delete;
    virtual ~bittwareManager() = default;

    /** @brief Constructs bittwareManager
     *
     * @param[in] bus     - Handle to system dbus
     * @param[in] objPath - The dbus path of bittwareManager
     */
    bittwareManager(sdbusplus::bus::bus& bus) :
        bus(bus), _event(sdeventplus::Event::get_default()),
        _timer(_event, std::bind(&bittwareManager::read, this))
    {
    }
    /** @brief Setup polling timer in a sd event loop and attach to D-Bus
     *         event loop.
     */
    void run();
  private:
    /** @brief sdbusplus bus client connection. */
    sdbusplus::bus::bus& bus;
    /** @brief the Event Loop structure */
    sdeventplus::Event _event;
    /** @brief Read Timer */
    sdeventplus::utility::Timer<sdeventplus::ClockId::Monotonic> _timer;
    /** @brief Bittware informations parsed from Json file */
    std::vector<phosphor::mpSOC::bittwareSOC::bittwareConfig> configs;
    std::vector<std::shared_ptr<phosphor::mpSOC::bittwareSOC>> devs;
    /** @brief Set up initial configuration value of 250 SoC */
    void init();
    /** @brief Monitor Bittware 250 SoC every one second  */
    void read();
};
}
}