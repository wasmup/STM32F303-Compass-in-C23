#pragma once
#include <stdbool.h>
#include <stdint.h>
void board_init(void);
void board_delay_ms(unsigned milliseconds);
bool board_user_button_pressed(void);
void board_leds(uint8_t mask);
void board_led_test(void);
void board_greeting(void);
void board_calibration_confirmation(void);
