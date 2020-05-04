#pragma once

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "i2c-dev.h"

namespace phosphor
{
namespace smbus
{

class Smbus
{
  public:
    Smbus(){};

    int open_i2c_dev(int i2cbus, char* filename, size_t size, int quiet);

    int set_slave_addr(int file, int address, int force);

    int smbusInit(int smbus_num);

    void smbusClose(int smbus_num);

    int smbusSequentialRead(int smbus_num, int8_t device_addr, uint16_t length, unsigned char* buf);

    int GetSmbusCmdByte(int smbus_num, int8_t device_addr, int8_t smbuscmd);

    bool smbusCheckSlave(int smbus_num, int8_t device_addr);

    int SetSmbusCmdByte(int smbus_num, int8_t device_addr, int8_t smbuscmd , int8_t data);

    int SendSmbusRWBlockCmdRAW(int smbus_num, int8_t device_addr,
                               uint8_t* tx_data, uint8_t tx_len,
                               uint8_t* rsp_data);
};

} // namespace smbus
} // namespace phosphor