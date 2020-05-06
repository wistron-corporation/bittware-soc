#include <iostream>
#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/elog.hpp>
#include <phosphor-logging/log.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/bus/match.hpp>
#include <sdbusplus/message.hpp>
#include <xyz/openbmc_project/Common/error.hpp>

namespace phosphor
{
namespace mpSOC
{
namespace util
{

class SDBusPlus
{
  public:
    template <typename T>
    static auto
        setProperty(sdbusplus::bus::bus& bus, const std::string& busName,
                    const std::string& objPath, const std::string& interface,
                    const std::string& property, const T& value)
    {
        sdbusplus::message::variant<T> data = value;

        try
        {
            auto methodCall = bus.new_method_call(
                busName.c_str(), objPath.c_str(), DBUS_PROPERTY_IFACE, "Set");

            methodCall.append(interface.c_str());
            methodCall.append(property);
            methodCall.append(data);

            auto reply = bus.call(methodCall);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Set properties fail. ERROR = " << e.what()
                      << std::endl;
            std::cerr << "Object path = " << objPath << std::endl;
            return;
        }
    }

    template <typename Property>
    static auto
        getProperty(sdbusplus::bus::bus& bus, const std::string& busName,
                    const std::string& objPath, const std::string& interface,
                    const std::string& property)
    {
        auto methodCall = bus.new_method_call(busName.c_str(), objPath.c_str(),
                                              DBUS_PROPERTY_IFACE, "Get");

        methodCall.append(interface.c_str());
        methodCall.append(property);

        sdbusplus::message::variant<Property> value;

        try
        {
            auto reply = bus.call(methodCall);
            reply.read(value);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Get properties fail.. ERROR = " << e.what()
                      << std::endl;
            std::cerr << "Object path = " << objPath << std::endl;
            return false;
        }

        return sdbusplus::message::variant_ns::get<Property>(value);
    }

    template <typename... Args>
    static auto CallMethod(sdbusplus::bus::bus& bus, const std::string& busName,
                           const std::string& objPath,
                           const std::string& interface,
                           const std::string& method, Args&&... args)
    {
        auto reqMsg = bus.new_method_call(busName.c_str(), objPath.c_str(),
                                          interface.c_str(), method.c_str());
        reqMsg.append(std::forward<Args>(args)...);
        try
        {
            auto respMsg = bus.call(reqMsg);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Call method fail. ERROR = " << e.what() << std::endl;
            std::cerr << "Object path = " << objPath << std::endl;
            return;
        }
    }
};
}
}
}
