#include "board.h"
#include "compass.h"
#include "debounce.h"
#include "magnetometer.h"
#include <stdint.h>

static const uint8_t BUTTON_DEBOUNCE_SAMPLES = 3;
static const unsigned BUTTON_SAMPLE_DELAY_MS = 5;
// Hard-iron offsets in raw sensor counts; replace after calibration.
static const int32_t X_OFFSET = 0, Y_OFFSET = 0;
// Latest raw measurements and status, available for calibration in GDB.
volatile int16_t LAST_X = 0, LAST_Y = 0;
volatile uint32_t SENSOR_STATUS = 0;
extern uint32_t _estack, _sdata, _edata, _sidata, _sbss, _ebss;
[[noreturn]] void Reset_Handler(void);

// Cortex-M vector table: initial stack pointer followed by the reset vector.
__attribute__((section(".isr_vector"), used))
const uintptr_t vectors[] = {
    (uintptr_t)&_estack,
    (uintptr_t)&Reset_Handler};

[[noreturn]] static void binary_counter(void)
{
    uint8_t count = 0;
    // The press used to enter this mode is not a counter increment.
    Debouncer button = {true, 0, BUTTON_DEBOUNCE_SAMPLES};
    bool button_state;
    board_leds(count);
    for (;;)
    {
        board_delay_ms(BUTTON_SAMPLE_DELAY_MS);
        if (debounce_update(&button, board_user_button_pressed(), &button_state) && button_state)
        {
            ++count;
            board_leds(count);
        }
    }
}

[[noreturn]] static void application(void)
{
    board_init();
    board_led_test();
    board_greeting();
    if (board_user_button_pressed())
        binary_counter();
    Sensor sensor;
    bool have_sensor = false;
    int64_t north_x = 1, north_y = 0;
    Debouncer button = {false, 0, BUTTON_DEBOUNCE_SAMPLES};
    bool button_state;
    bool calibration_pending = false;
    for (;;)
    {
        if (debounce_update(&button, board_user_button_pressed(), &button_state) && button_state)
        {
            board_led_test();
            calibration_pending = true;
        }
        if (!have_sensor)
        {
            have_sensor = magnetometer_configure(&sensor);
            if (!have_sensor)
                SENSOR_STATUS = 0;
        }
        if (have_sensor)
        {
            Sample reading;
            if (magnetometer_sample(sensor, &reading))
            {
                LAST_X = reading.x;
                LAST_Y = reading.y;
                const int64_t x = (int64_t)reading.x - X_OFFSET;
                const int64_t y = (int64_t)reading.y - Y_OFFSET;
                SENSOR_STATUS = (x == 0 && y == 0) ? 2 : 3;
                if (calibration_pending && (x != 0 || y != 0))
                {
                    north_x = x;
                    north_y = y;
                    calibration_pending = false;
                    board_calibration_confirmation();
                }
                board_leds(compass_sector(x * north_x + y * north_y,
                                          y * north_x - x * north_y));
            }
            else
            {
                SENSOR_STATUS = 1;
                board_leds(0);
                have_sensor = false;
            }
        }
        else
            board_leds(0);
        // Separate button samples by at least 5 ms, even on I2C failures.
        board_delay_ms(BUTTON_SAMPLE_DELAY_MS);
    }
}

[[noreturn]] void Reset_Handler(void)
{
    uint32_t *source = &_sidata;
    for (uint32_t *dest = &_sdata; dest < &_edata; ++dest)
        *dest = *source++;
    for (uint32_t *dest = &_sbss; dest < &_ebss; ++dest)
        *dest = 0;
    application();
}
