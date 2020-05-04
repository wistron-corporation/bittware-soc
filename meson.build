project(
    'bittware-250-soc',
    'cpp',
    version: '1.0',
    default_options: [
        'cpp_std=c++17',
    ],
)

executable(
    'bittware-250-soc',
    [
        'main.cpp',
        'bittware_soc.cpp',
        'manager.cpp',
        'smbus.cpp',
        'vpd.cpp',
        'sensor.cpp',
    ],
    dependencies: [
        dependency('phosphor-logging'),
        dependency('sdbusplus'),
        dependency('phosphor-dbus-interfaces'),
        dependency('sdeventplus'),
    ],
    install: true,
    install_dir: get_option('bindir')
)

install_data(sources : 'bittware_config.json', install_dir : '/etc/bittware')

conf_data = configuration_data()
conf_data.set('BITTWARE_SOC_REQUEST_NAME', '"xyz.openbmc_project.Bittware.manager"')
conf_data.set('BITTWARE_SOC_OBJ_PATH_ROOT', '"/xyz/openbmc_project/sensors/temperature"')
conf_data.set('BITTWARE_SOC_OBJ_PATH', '"/xyz/openbmc_project/sensors/temperature/Bittware"')
conf_data.set('DBUS_PROPERTY_IFACE', '"org.freedesktop.DBus.Properties"')
conf_data.set('BITTWARE_SOC_STATUS_IFACE', '"xyz.openbmc_project.Bittware.Status"')
conf_data.set('VPD_ID', '"250SoC OpenCAPI Accelerator"')
conf_data.set('ITEM_IFACE', '"xyz.openbmc_project.Inventory.Item"')
conf_data.set('ASSET_IFACE', '"xyz.openbmc_project.Inventory.Decorator.Asset"')
conf_data.set('INVENTORY_BUSNAME', '"xyz.openbmc_project.Inventory.Manager"')
conf_data.set('BITTWARE_SOC_INVENTORY_PATH', '"/xyz/openbmc_project/inventory/system/chassis/motherboard/BittwareSOC"')
conf_data.set('INVENTORY_NAMESPACE', '"/xyz/openbmc_project/inventory"')
conf_data.set('INVENTORY_MANAGER_IFACE', '"xyz.openbmc_project.Inventory.Manager"')
configure_file(output : 'config.h', configuration : conf_data)