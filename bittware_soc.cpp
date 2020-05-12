#include "bittware_soc.hpp"
#include "smbus.hpp"

#include "i2c-dev.h"

#include <iostream>

#define IO_EXPANDER_SLAVE_ADDR 0x39
#define IO_EXPANDER_DIR_MASK (0x01 << 4)
#define IO_EXPANDER_VALUE_MASK (0x01 << 4)
#define IO_EXPANDER_COMMAND_1 0x01
#define IO_EXPANDER_COMMAND_3 0x03
#define I2C_VPD_SLAVE_ADDR 0x50

namespace phosphor
{
namespace mpSOC
{
static const std::unordered_map<std::string, keywordInfo>
    supportedKeywords = {
        {"ID", std::make_tuple("Model", ASSET_IFACE, 27)},
        {"PN", std::make_tuple("PartNumber", ASSET_IFACE, 7)},
        {"EC", std::make_tuple("EngineeringChangeLevel", BITTWARE_SOC_STATUS_IFACE, 6)},
        {"SN", std::make_tuple("SerialNumber", ASSET_IFACE, 12)},
        {"FN", std::make_tuple("FieldReplaceUnit", BITTWARE_SOC_STATUS_IFACE, 7)},
};

bittwareSOC::bittwareSOC(uint8_t index, sdbusplus::bus::bus& bus, bittwareConfig config) :
    index(index), bus(bus), _event(sdeventplus::Event::get_default()),
    _timer(_event, std::bind(&bittwareSOC::read, this)), config(config)
{
    init();
}

void bittwareSOC::read()
{
    if (present)
    {
        tmpSensor->getTemp();
    }
}

void bittwareSOC::createInventory()
{
    using Properties =
        std::map<std::string, sdbusplus::message::variant<std::string, bool>>;
    using Interfaces = std::map<std::string, Properties>;

    std::string inventoryPath;
    std::map<sdbusplus::message::object_path, Interfaces> obj;

    inventoryPath = "/system/chassis/motherboard/BittwareSOC" + std::to_string(index);
    obj = {{
        inventoryPath,
        {{ITEM_IFACE, {}}, {BITTWARE_SOC_STATUS_IFACE, {}}, {ASSET_IFACE, {}}},
    }};
    phosphor::mpSOC::util::SDBusPlus::CallMethod(bus, INVENTORY_BUSNAME,
        INVENTORY_NAMESPACE, INVENTORY_MANAGER_IFACE, "Notify", obj);
}

void bittwareSOC::setInventoryProperties(
            const bool& present, const vpd& vpdDev)
{
    std::string path = BITTWARE_SOC_INVENTORY_PATH + std::to_string(index);
    util::SDBusPlus::setProperty(bus, INVENTORY_BUSNAME,
        path, ITEM_IFACE, "Present", present);
    for (auto it = supportedKeywords.begin(); it != supportedKeywords.end(); it++)
    {
        auto data = vpdDev.vpdData.find(it->first);

        if (data != vpdDev.vpdData.end())
        {
            util::SDBusPlus::setProperty(bus, INVENTORY_BUSNAME, path,
                std::get<1>(it->second), std::get<0>(it->second),
                data->second.substr(0, std::get<2>(it->second)));
        }
        else
        {
            /* Keyword not found, filled with empty string */
            util::SDBusPlus::setProperty(bus, INVENTORY_BUSNAME, path,
                std::get<1>(it->second), std::get<0>(it->second),
                std::string());
        }
    }
}

/** @brief Make sure smbus on 250 SoC has been enabled */
bool bittwareSOC::smbusEnable(int busID, uint8_t addr)
{
    bool enabled = false;
    auto bus = phosphor::smbus::Smbus();

    auto res = bus.smbusInit(busID);
    if (res == -1)
    {
        std::cerr << "smbusInit fail!" << std::endl;
        return false;
    }

    auto exist = bus.smbusCheckSlave(busID, addr);
    if (exist)
    {
        uint8_t direction = bus.GetSmbusCmdByte(busID, addr, IO_EXPANDER_COMMAND_3);
        direction &= ~(IO_EXPANDER_DIR_MASK);
        res = bus.SetSmbusCmdByte(busID, addr, IO_EXPANDER_COMMAND_3, direction);
        if (res >= 0)
        {
            uint8_t value = bus.GetSmbusCmdByte(busID, addr, IO_EXPANDER_COMMAND_1);
            value |= IO_EXPANDER_VALUE_MASK;
            res = bus.SetSmbusCmdByte(busID, addr, IO_EXPANDER_COMMAND_1, value);
            if (res >= 0)
            {
                auto vpdExist = bus.smbusCheckSlave(busID, I2C_VPD_SLAVE_ADDR);
                auto sensorExist = bus.smbusCheckSlave(busID, TMP431_SLAVE_ADDR);
                enabled = (vpdExist & sensorExist);
            }
            else
            {
                std::cerr << "Failed to set IO expander direction.\n";
            }
        }
        else
        {
            std::cerr << "Failed to set IO expander direction.\n";
        }
    }
    else
    {
        std::cout << "Bittware " << (int)config.index << " not present." << std::endl;
    }

    bus.smbusClose(busID);

    return enabled;
}

void bittwareSOC::init()
{
    createInventory();
    present = smbusEnable(config.busID, IO_EXPANDER_SLAVE_ADDR);
    auto vpdDev = (present) ? vpd(config.busID, I2C_VPD_SLAVE_ADDR) : vpd();
    setInventoryProperties(present, vpdDev);
    if (present)
    {
        auto path = std::string(BITTWARE_SOC_OBJ_PATH + std::to_string(index));
        tmpSensor = std::make_shared<sensor>(bus, path, config.busID);
        tmpSensor->setSensorThreshold(config.criticalHigh, config.criticalLow,
            config.maxValue, config.minValue,
            config.warningHigh, config.warningLow);
    }
}
}
}