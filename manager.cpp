#include "manager.hpp"

#include "smbus.hpp"
#include "nlohmann/json.hpp"
#include "i2c-dev.h"

#include <fstream>
#include <iostream>

#define MONITOR_INTERVAL_SECONDS 1

static constexpr auto configFile = "/etc/bittware/bittware_config.json";
using Json = nlohmann::json;

namespace phosphor
{
namespace mpSOC
{
void bittwareManager::read()
{
    for (auto it = devs.begin(); it != devs.end(); it++)
    {
        if ((*it)->present)
        {
            (*it)->read();
        }
    }
}

void bittwareManager::run()
{
    init();
    
    std::function<void()> callback(std::bind(&bittwareManager::read, this));
    try
    {
        u_int64_t interval = MONITOR_INTERVAL_SECONDS * 1000000;
        _timer.restart(std::chrono::microseconds(interval));
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error in polling loop. ERROR = " << e.what() << std::endl;
    }
}

Json parseSensorConfig()
{
    std::ifstream jsonFile(configFile);
    if (!jsonFile.is_open())
    {
        std::cerr << "Bittware config JSON file not found" << std::endl;
    }

    auto data = Json::parse(jsonFile, nullptr, false);
    if (data.is_discarded())
    {
        std::cerr << "Bittware config readings JSON parser failure" << std::endl;
    }

    return data;
}

/** @brief Obtain the initial configuration value of NVMe  */
std::vector<phosphor::mpSOC::bittwareSOC::bittwareConfig> getNvmeConfig()
{
    phosphor::mpSOC::bittwareSOC::bittwareConfig bittwareConfig;
    std::vector<phosphor::mpSOC::bittwareSOC::bittwareConfig> bittwareConfigs;
    uint64_t criticalHigh = 0;
    uint64_t criticalLow = 0;
    uint64_t maxValue = 0;
    uint64_t minValue = 0;
    uint64_t warningHigh = 0;
    uint64_t warningLow = 0;

    try
    {
        auto data = parseSensorConfig();
        static const std::vector<Json> empty{};
        std::vector<Json> readings = data.value("config", empty);
        std::vector<Json> thresholds = data.value("threshold", empty);
        if (!thresholds.empty())
        {
            for (const auto& instance : thresholds)
            {
                criticalHigh = instance.value("criticalHigh", 0);
                criticalLow = instance.value("criticalLow", 0);
                maxValue = instance.value("maxValue", 0);
                minValue = instance.value("minValue", 0);
                warningHigh = instance.value("warningHigh", 0);
                warningLow = instance.value("warningLow", 0);
            }
        }
        else
        {
            std::cerr << "Invalid NVMe config file, thresholds dosen't exist"
                      << std::endl;
        }

        if (!readings.empty())
        {
            for (const auto& instance : readings)
            {
                uint8_t index = instance.value("bittwareIndex", 0);
                uint8_t busID = instance.value("bittwareBusID", 0);

                /*std::cout << "index = " << (int)index <<std::endl;
                std::cout << "busID = " << (int)busID <<std::endl;*/

                bittwareConfig.index = index;
                bittwareConfig.busID = busID;
                bittwareConfig.criticalHigh = criticalHigh;
                bittwareConfig.criticalLow = criticalLow;
                bittwareConfig.warningHigh = warningHigh;
                bittwareConfig.warningLow = warningLow;
                bittwareConfig.maxValue = maxValue;
                bittwareConfig.minValue = minValue;
                bittwareConfigs.push_back(bittwareConfig);
            }
        }
        else
        {
            std::cerr << "Invalid Bittware config file, config dosen't exist"
                      << std::endl;
        }
    }
    catch (const Json::exception& e)
    {
        std::cerr << "Json Exception caught. MSG: " << e.what() << std::endl;
    }

    return bittwareConfigs;
}

void bittwareManager::init()
{
    // read json file
    configs = getNvmeConfig();
    for (auto it = configs.begin(); it != configs.end(); it++)
    {
        std::cout << "Initializing Bittware " << (int)it->index << std::endl;
        auto dev = std::make_shared<phosphor::mpSOC::bittwareSOC>(
            it->index, bus, *it);
        devs.push_back(dev);
        std::cout << "Bittware " << (int)it->index << " initialized" << std::endl;
    }
}
}
}