#include "i2c.h"
#include "register.h"

static const uintptr_t GPIOB = 0x48000400;
static const uint32_t SCL = 1u << 6, SDA = 1u << 7;
static const uint8_t SENSOR_ADDRESS = 0x1e;
static void pause(void)
{
    for (unsigned i = 0; i < 40; ++i)
        __asm__ volatile("nop");
}
static void line(uint32_t pin, bool high)
{
    reg_write(GPIOB + 0x18, high ? pin : pin << 16);
    pause();
}
static bool clock_high(void)
{
    line(SCL, true);
    for (unsigned i = 0; i < 1000; ++i)
        if (reg_read(GPIOB + 0x10) & SCL)
            return true;
    return false;
}
static void stop(void)
{
    line(SDA, false);
    (void)clock_high();
    line(SDA, true);
}
static bool start(void)
{
    line(SDA, true);
    if (!clock_high())
        return false;
    if ((reg_read(GPIOB + 0x10) & SDA) == 0)
    {
        // A reset during a transfer can leave a slave waiting for more clocks.
        for (unsigned i = 0; i < 9; ++i)
        {
            line(SCL, false);
            if (!clock_high())
                return false;
            if (reg_read(GPIOB + 0x10) & SDA)
                break;
        }
        stop();
        if (!clock_high() || (reg_read(GPIOB + 0x10) & SDA) == 0)
            return false;
    }
    line(SDA, false);
    line(SCL, false);
    return true;
}
static bool send(uint8_t byte)
{
    for (unsigned i = 0; i < 8; ++i)
    {
        line(SDA, (byte & 0x80) != 0);
        if (!clock_high())
            return false;
        line(SCL, false);
        byte <<= 1;
    }
    line(SDA, true);
    if (!clock_high())
        return false;
    const bool ack = (reg_read(GPIOB + 0x10) & SDA) == 0;
    line(SCL, false);
    return ack;
}
static bool receive(bool more, uint8_t *value)
{
    uint8_t byte = 0;
    line(SDA, true);
    for (unsigned i = 0; i < 8; ++i)
    {
        if (!clock_high())
            return false;
        byte = (byte << 1) | ((reg_read(GPIOB + 0x10) & SDA) != 0);
        line(SCL, false);
    }
    line(SDA, !more);
    if (!clock_high())
        return false;
    line(SCL, false);
    line(SDA, true);
    *value = byte;
    return true;
}
bool i2c_write(uint8_t address, uint8_t value)
{
    const bool ok = start() && send(SENSOR_ADDRESS << 1) && send(address) && send(value);
    stop();
    return ok;
}
bool i2c_read(uint8_t address, uint8_t *data, size_t length)
{
    bool ok = start() && send(SENSOR_ADDRESS << 1) && send(address) && start() && send((SENSOR_ADDRESS << 1) | 1);
    for (size_t i = 0; ok && i < length; ++i)
    {
        if (!receive(i + 1 < length, &data[i]))
            ok = false;
    }
    stop();
    return ok;
}
