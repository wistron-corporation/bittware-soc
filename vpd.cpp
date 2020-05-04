#include "smbus.hpp"
#include "vpd.hpp"

#include "i2c-dev.h"

#include <iostream>

#define PCI_VPD_ID_STRING_TAG 0x02
#define PCI_VPD_VPD_RO_TAG 0x10
#define PCI_VPD_VPD_RW_TAG 0x11
#define PCI_VPD_END_TAG 0x0f

#define PCI_VPD_SRDT_TIN_MASK 0x78
#define PCI_VPD_LRDT_TIN_MASK 0x7f

#define PCI_VPD_LEN_LSB_OFFSET 0x01
#define PCI_VPD_LEN_MSB_OFFSET 0x02
#define PCI_VPD_DATA_LEN_OFFSET 0x02

/* The three-byte header contains a two-byte keyword and a one-byte length. */
#define PCI_VPD_KEYWORD_LEN 2
#define PCI_VPD_HEADER_LEN 3

namespace phosphor
{
namespace mpSOC
{
vpd::vpd() :
    idChecked(false), checksumVerified(false)
{
    vpdData.clear();
}

vpd::vpd(uint8_t busID, uint8_t eepromAddr) :
    busID(busID), eepromAddr(eepromAddr), idChecked(false),
    checksumVerified(false)
{
    memset(rawData, 0, I2C_DATA_MAX);
    read();
    parse();
}

void vpd::read()
{
    auto bus = phosphor::smbus::Smbus();
    
    auto res = bus.smbusInit(busID);
    if (res != -1)
    {
        /* Dump all data from eeprom, for detail, please refer to atmel-8719 datasheet,
         * Sequential Read section.
         */
        res = bus.smbusSequentialRead(busID, eepromAddr, I2C_DATA_MAX, rawData);
        if (res < 0) {
            std::cerr << "Read VPD data failed" << std::endl;
        }
    }
    else
    {
        std::cerr << "smbusInit fail!" << std::endl;
    }

    bus.smbusClose(busID);
}

static inline uint8_t pci_vpd_lrdt_tag(uint8_t lrdt)
{
	return (lrdt & PCI_VPD_LRDT_TIN_MASK);
}

static inline uint8_t pci_vpd_srdt_tag(uint8_t srdt)
{
	return (srdt & PCI_VPD_SRDT_TIN_MASK) >> 3;
}

static inline uint16_t combineByte(uint8_t msb, uint8_t lsb)
{
    return (msb << 8) | lsb;
}

void vpd::verifyChecksum(uint8_t offset)
{
    uint8_t checksum = 0;
    for(int i = 0; i <= offset; i++)
    {
        checksum += rawData[i];
    }
    checksumVerified = (checksum == 0) ? true : false;
}

bool vpd::parseField(const uint8_t fieldOffset, uint8_t& nextField)
{
    auto name = pci_vpd_srdt_tag(rawData[fieldOffset]);
    if (name == PCI_VPD_END_TAG)
    {
        std::cout << "Reach end of VPD." << std::endl;
        if (checksumVerified != true)
        {
            std::cerr << "Invalid VPD data, checksum incorrect." << std::endl;
            vpdData.clear();
        }
        return false;
    }

    int res = 0;
    uint16_t byteRead = 0;
    uint8_t dataOffset = 0;
    uint8_t dataLen = 0;
    std::string keyword;
    std::string data;
    std::string str;
    name = pci_vpd_lrdt_tag(rawData[fieldOffset]);
    uint16_t len = combineByte(rawData[PCI_VPD_LEN_MSB_OFFSET + fieldOffset], 
                rawData[PCI_VPD_LEN_LSB_OFFSET + fieldOffset]);
    if ((fieldOffset + len + PCI_VPD_HEADER_LEN) > I2C_DATA_MAX)
    {
        std::cerr << "Length of this field exceed buffer size." << std::endl;
        return false;
    }

    switch(name)
    {
        case PCI_VPD_ID_STRING_TAG :
            str = std::string(reinterpret_cast<char*>(rawData + fieldOffset + PCI_VPD_HEADER_LEN), len);
            res = str.compare(VPD_ID);
            if (res != 0)
            {
                std::cerr << "Wrong device, this module supports " << VPD_ID << std::endl;
                return false;
            }
            vpdData.insert(std::pair<std::string, std::string>("ID", str));
            byteRead = len;
            idChecked = true;
            break;
        case PCI_VPD_VPD_RO_TAG :
        case PCI_VPD_VPD_RW_TAG :
            if (idChecked == false)
            {
                return false;
            }
            dataOffset = fieldOffset + PCI_VPD_HEADER_LEN;
            while (byteRead < len)
            {
                dataLen = rawData[dataOffset + PCI_VPD_DATA_LEN_OFFSET];
                if (dataLen + byteRead + PCI_VPD_HEADER_LEN > len)
                {
                    return false;
                }
                keyword = std::string(
                    reinterpret_cast<char*>(rawData + dataOffset), PCI_VPD_KEYWORD_LEN);
                data = std::string(
                    reinterpret_cast<char*>(rawData + dataOffset + PCI_VPD_HEADER_LEN), dataLen);
                if (keyword.compare("RV") == 0)
                {
                    verifyChecksum(dataOffset + PCI_VPD_HEADER_LEN);
                }
                dataOffset += dataLen + PCI_VPD_HEADER_LEN;
                byteRead += PCI_VPD_HEADER_LEN + dataLen;
                vpdData.insert(std::pair<std::string, std::string>(keyword, data));
            }
            break;
        default :
            return false;
    }

    if (byteRead != len)
    {
        std::cerr << "Data length doesn't match with Field length." << std::endl;
        return false;
    }
    
    nextField = PCI_VPD_HEADER_LEN + len + fieldOffset;
    return true;
}

void vpd::parse()
{
    uint8_t fieldOffset = 0;
    uint8_t nextField = 0;
    bool remain;
    while (fieldOffset < I2C_DATA_MAX)
    {
        remain = parseField(fieldOffset, nextField);
        if (remain)
        {
            fieldOffset = nextField;
        }
        else
        {
            break;
        }
    }
}
}
}