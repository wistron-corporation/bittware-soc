#include "smbus.hpp"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <iostream>
#include <mutex>

#include "i2c-dev.h"

#define MAX_I2C_BUS 30

static int fd[MAX_I2C_BUS] = {0};

namespace phosphor
{
namespace smbus
{

std::mutex gMutex;

int phosphor::smbus::Smbus::open_i2c_dev(int i2cbus, char* filename,
                                         size_t size, int quiet)
{
    int file;

    snprintf(filename, size, "/dev/i2c/%d", i2cbus);
    filename[size - 1] = '\0';
    file = open(filename, O_RDWR);

    if (file < 0 && (errno == ENOENT || errno == ENOTDIR))
    {
        sprintf(filename, "/dev/i2c-%d", i2cbus);
        file = open(filename, O_RDWR);
    }

    if (file < 0 && !quiet)
    {
        if (errno == ENOENT)
        {
            fprintf(stderr,
                    "Error: Could not open file "
                    "`/dev/i2c-%d' or `/dev/i2c/%d': %s\n",
                    i2cbus, i2cbus, strerror(ENOENT));
        }
        else
        {
            fprintf(stderr,
                    "Error: Could not open file "
                    "`%s': %s\n",
                    filename, strerror(errno));
            if (errno == EACCES)
                fprintf(stderr, "Run as root?\n");
        }
    }

    return file;
}

int phosphor::smbus::Smbus::set_slave_addr(int file, int address, int force)
{
    /* With force, let the user read from/write to the registers
       even when a driver is also running */
    if (ioctl(file, force ? I2C_SLAVE_FORCE : I2C_SLAVE, address) < 0) {
        fprintf(stderr,
            "Error: Could not set address to 0x%02x: %s\n",
            address, strerror(errno));
        return -errno;
    }

    return 0;
}

int phosphor::smbus::Smbus::smbusInit(int smbus_num)
{
    int res = 0;
    char filename[20];

    gMutex.lock();

    fd[smbus_num] = open_i2c_dev(smbus_num, filename, sizeof(filename), 0);
    if (fd[smbus_num] < 0)
    {
        gMutex.unlock();

        return -1;
    }

    res = fd[smbus_num];

    gMutex.unlock();

    return res;
}

void phosphor::smbus::Smbus::smbusClose(int smbus_num)
{
    close(fd[smbus_num]);
}

int phosphor::smbus::Smbus::smbusSequentialRead(int smbus_num, int8_t device_addr, uint16_t length, unsigned char* buf)
{
    if (length > I2C_DATA_MAX)
    {
        fprintf(stderr, "length is over restriction\n");
        return -1;
    }
    
    int res;
    uint16_t byte_read = 0;

    gMutex.lock();
    if(fd[smbus_num] > 0) {
        res = set_slave_addr(fd[smbus_num], device_addr, I2C_SLAVE_FORCE);
        if(res < 0) {
            fprintf(stderr, "set PMBUS BUS%d to slave address 0x%02X failed (%s)\n", smbus_num, device_addr,strerror(errno));
                close(fd[smbus_num]);

                gMutex.unlock();
            return -1;
        }
    }
    
    res = i2c_smbus_read_byte_data(fd[smbus_num], 0);
    if (res < 0) {
        gMutex.unlock();
        return -1;
    }
    buf[0] = res;

    for (byte_read = 1; byte_read < length; byte_read++) {
        res = i2c_smbus_read_byte(fd[smbus_num]);
        if (res < 0) {
            gMutex.unlock();
            return -1;
        }
        buf[byte_read] = res;
    }

    gMutex.unlock();
    return byte_read;
}

bool phosphor::smbus::Smbus::smbusCheckSlave(int smbus_num, int8_t device_addr)
{
    int res;

    gMutex.lock();
    if(fd[smbus_num] > 0) {
        res = set_slave_addr(fd[smbus_num], device_addr, I2C_SLAVE_FORCE);
        if(res < 0) {
            fprintf(stderr, "set PMBUS BUS%d to slave address 0x%02X failed (%s)\n", smbus_num, device_addr,strerror(errno));
                close(fd[smbus_num]);

                gMutex.unlock();
            return false;
        }
    }

    res = i2c_smbus_write_quick(fd[smbus_num], I2C_SMBUS_WRITE);
    if (res < 0) {
        //fprintf(stderr, "Error: Read failed\n");
        gMutex.unlock();

        return false;
    }
    // printf("[SetSmbusCmdByte]0x%0*x\n",2, res);

    gMutex.unlock();
    return true;
}

int phosphor::smbus::Smbus::GetSmbusCmdByte(int smbus_num, int8_t device_addr, int8_t smbuscmd)
{
    int res;

    gMutex.lock();
    if(fd[smbus_num] > 0) {
        res = set_slave_addr(fd[smbus_num], device_addr, I2C_SLAVE_FORCE);
        if(res < 0) {
            fprintf(stderr, "set PMBUS BUS%d to slave address 0x%02X failed (%s)\n", smbus_num, device_addr,strerror(errno));
                close(fd[smbus_num]);

                gMutex.unlock();
            return -1;
        }
    }

    res = i2c_smbus_read_byte_data(fd[smbus_num], smbuscmd);
    if (res < 0) {
        //fprintf(stderr, "Error: Read failed\n");
        gMutex.unlock();

        return -1;
    }
    // printf("[SetSmbusCmdByte]0x%0*x\n",2, res);

    gMutex.unlock();
    return res;
}

int phosphor::smbus::Smbus::SetSmbusCmdByte(int smbus_num, int8_t device_addr, int8_t smbuscmd , int8_t data)
{
    int res;

    gMutex.lock();
    if(fd[smbus_num] > 0) {
        res = set_slave_addr(fd[smbus_num], device_addr, I2C_SLAVE_FORCE);
        if(res < 0) {
            fprintf(stderr, "set PMBUS BUS%d to slave address 0x%02X failed (%s)\n", smbus_num, device_addr,strerror(errno));
                close(fd[smbus_num]);

                gMutex.unlock();
            return -1;
        }
    }

    res = i2c_smbus_write_byte_data(fd[smbus_num], smbuscmd, data);
    if (res < 0) {
        //fprintf(stderr, "Error: Read failed\n");
        gMutex.unlock();

        return -1;
    }
    // printf("[SetSmbusCmdByte]0x%0*x\n",2, res);

    gMutex.unlock();
    return res;
}

int phosphor::smbus::Smbus::SendSmbusRWBlockCmdRAW(int smbus_num,
                                                   int8_t device_addr,
                                                   uint8_t* tx_data,
                                                   uint8_t tx_len,
                                                   uint8_t* rsp_data)
{
    int res, res_len;
    unsigned char Rx_buf[I2C_DATA_MAX] = {0};

    Rx_buf[0] = 1;

    gMutex.lock();

    res = i2c_read_after_write(fd[smbus_num], 0, device_addr, tx_len,
                               (unsigned char*)tx_data, I2C_DATA_MAX,
                               (const unsigned char*)Rx_buf);

    if (res < 0)
    {
       // fprintf(stderr, "Error: SendSmbusRWBlockCmdRAW failed\n");
    }

    res_len = Rx_buf[0] + 1;

    memcpy(rsp_data, Rx_buf, res_len);

    gMutex.unlock();

    return res;
}

} // namespace smbus
} // namespace phosphor