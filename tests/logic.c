#include "compass.h"
#include "debounce.h"
#include <assert.h>
#include <stddef.h>

int main(void)
{
    const struct { int64_t x, y; unsigned bit; } directions[] = {
        {1, 0, 0}, {1, -1, 1}, {0, -1, 2}, {-1, -1, 3},
        {-1, 0, 4}, {-1, 1, 5}, {0, 1, 6}, {1, 1, 7}
    };
    for (size_t i = 0; i < sizeof directions / sizeof directions[0]; ++i)
        assert(compass_sector(directions[i].x, directions[i].y) == (1u << directions[i].bit));
    assert(compass_sector(0, 0) == 0);
    assert(compass_sector(100, 20) == 1);
    assert(compass_sector(100, 40) == (1 | (1 << 7)));
    assert(compass_sector(100, 100) == (1 << 7));
    Debouncer button = {false, 0, 3};
    bool state;
    const bool first[] = {true, true, false, true, true};
    for (size_t i = 0; i < sizeof first / sizeof first[0]; ++i) assert(!debounce_update(&button, first[i], &state));
    assert(debounce_update(&button, true, &state) && state);
    assert(!debounce_update(&button, true, &state));
    const bool second[] = {false, true, false, false};
    for (size_t i = 0; i < sizeof second / sizeof second[0]; ++i) assert(!debounce_update(&button, second[i], &state));
    assert(debounce_update(&button, false, &state) && !state);
    assert(!debounce_update(&button, false, &state));
    Debouncer immediate = {true, 0, 1};
    assert(debounce_update(&immediate, false, &state) && !state);
}
