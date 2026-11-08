#pragma once
#include <stdbool.h>
#include <stdint.h>
// Reports a transition only after the requested nonzero number of samples.
typedef struct
{
    bool stable;
    uint8_t count, samples;
} Debouncer;
static inline bool debounce_update(Debouncer *button, bool input, bool *state)
{
    if (input == button->stable)
        button->count = 0;
    else if (++button->count == button->samples)
    {
        button->stable = input;
        button->count = 0;
        *state = input;
        return true;
    }
    return false;
}
