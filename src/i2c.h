#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
bool i2c_write(uint8_t reg, uint8_t value);
bool i2c_read(uint8_t reg, uint8_t *data, size_t length);
