#include "compass.h"
uint8_t compass_sector(int64_t x, int64_t y)
{
    if (x == 0 && y == 0)
        return 0;
    const int64_t ax = x < 0 ? -x : x;
    const int64_t ay = y < 0 ? -y : y;
    const unsigned x_led = x >= 0 ? 0 : 4;
    const unsigned y_led = y >= 0 ? 6 : 2;
    const unsigned diagonal = x >= 0 ? (y >= 0 ? 7 : 1) : (y >= 0 ? 5 : 3);
    const int64_t near = ax >= ay ? ay : ax;
    const int64_t far = ax >= ay ? ax : ay;
    const int64_t axis_led = ax >= ay ? x_led : y_led;
    // tan(15 degrees) approximately 274/1024; tan(30 degrees) 591/1024.
    if (near * 1024 <= far * 274)
        return 1u << axis_led;
    if (near * 1024 < far * 591)
        return (1u << axis_led) | (1u << diagonal);
    return 1u << diagonal;
}
