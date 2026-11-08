#include "magnetometer.h"
#include "i2c.h"

static const uint8_t AGR_WHO_AM_I = 0x4f;
static const uint8_t AGR_ID = 0x40;
static const uint8_t AGR_CONFIG_A = 0x60;
static const uint8_t AGR_CONFIG_C = 0x62;
static const uint8_t AGR_OUTPUT = 0xe8;
static const uint8_t DLHC_ID_REGISTER = 0x0a;
static const uint8_t DLHC_ID = 0x48;
static const uint8_t DLHC_CONFIG_A = 0x00;
static const uint8_t DLHC_CONFIG_B = 0x01;
static const uint8_t DLHC_MODE = 0x02;
static const uint8_t DLHC_OUTPUT = 0x03;
static const int16_t DLHC_OVERFLOW = -4096;

static int16_t signed_word(uint8_t high, uint8_t low)
{
    unsigned bits = ((unsigned)high << 8) | low;
    return (int16_t)(bits < 0x8000u ? (int32_t)bits : (int32_t)bits - 65536);
}

bool magnetometer_configure(Sensor *sensor)
{
    uint8_t id[1];
    if (i2c_read(AGR_WHO_AM_I, id, sizeof id) && id[0] == AGR_ID)
    {
        if (i2c_write(AGR_CONFIG_A, 0x00) && i2c_write(AGR_CONFIG_C, 0x10))
        {
            *sensor = SENSOR_AGR;
            return true;
        }
        return false;
    }
    if (i2c_read(DLHC_ID_REGISTER, id, sizeof id) && id[0] == DLHC_ID &&
        i2c_write(DLHC_CONFIG_A, 0x14) && i2c_write(DLHC_CONFIG_B, 0xe0) &&
        i2c_write(DLHC_MODE, 0x00))
    {
        *sensor = SENSOR_DLHC;
        return true;
    }
    return false;
}

bool magnetometer_sample(Sensor sensor, Sample *sample)
{
    uint8_t bytes[6];
    switch (sensor)
    {
    case SENSOR_DLHC:
    {
        if (!i2c_read(DLHC_OUTPUT, bytes, sizeof bytes))
            return false;
        int16_t x = signed_word(bytes[0], bytes[1]);
        int16_t z = signed_word(bytes[2], bytes[3]);
        int16_t y = signed_word(bytes[4], bytes[5]);
        if (x == DLHC_OVERFLOW || y == DLHC_OVERFLOW || z == DLHC_OVERFLOW)
            return false;
        *sample = (Sample){x, y};
        return true;
    }
    case SENSOR_AGR:
        if (!i2c_read(AGR_OUTPUT, bytes, sizeof bytes))
            return false;
        *sample = (Sample){signed_word(bytes[1], bytes[0]), signed_word(bytes[3], bytes[2])};
        return true;
    }
    return false;
}
