#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef enum
{
    SENSOR_DLHC,
    SENSOR_AGR
} Sensor;
typedef struct
{
    int16_t x, y;
} Sample;
bool magnetometer_configure(Sensor *sensor);
bool magnetometer_sample(Sensor sensor, Sample *sample);
