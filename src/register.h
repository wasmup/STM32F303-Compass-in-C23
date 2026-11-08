#pragma once
#include <stdint.h>
static inline uint32_t reg_read(uintptr_t address) { return *(volatile uint32_t *)address; }
static inline void reg_write(uintptr_t address, uint32_t value) { *(volatile uint32_t *)address = value; }
static inline void reg_modify(uintptr_t address, uint32_t clear, uint32_t set) { reg_write(address, (reg_read(address) & ~clear) | set); }
