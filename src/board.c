#include "board.h"
#include "register.h"
#include <stddef.h>

static const uintptr_t RCC_AHBENR = 0x40021014, SYST_CSR = 0xe000e010,
                       SYST_RVR = 0xe000e014, SYST_CVR = 0xe000e018,
                       GPIOA = 0x48000000, GPIOB = 0x48000400, GPIOE = 0x48001000;
static const uint32_t SCL = 1u << 6, SDA = 1u << 7;
static const unsigned LED_PINS[] = {13u, 14u, 15u, 8u, 9u, 10u, 11u, 12u};
void board_delay_ms(unsigned milliseconds)
{
    // SysTick runs from the reset-default 8 MHz HSI system clock.
    // One millisecond per reload keeps the 24-bit reload value in range.
    for (unsigned i = 0; i < milliseconds; ++i)
    {
        reg_write(SYST_RVR, 8000 - 1);
        reg_write(SYST_CVR, 0);
        reg_write(SYST_CSR, 0x5);
        while ((reg_read(SYST_CSR) & (1u << 16)) == 0)
        {
        }
        reg_write(SYST_CSR, 0);
    }
}

void board_init()
{
    reg_modify(RCC_AHBENR, 0, (1u << 17) | (1u << 18) | (1u << 21));
    // The blue user button is PA0, active high with an external pull-down.
    reg_modify(GPIOA, 3, 0);
    reg_modify(GPIOA + 0x0c, 3, 0);
    reg_modify(GPIOE, 0xffff0000, 0x55550000);
    board_leds(0);
    reg_write(GPIOB + 0x18, SCL | SDA);
    reg_modify(GPIOB + 0x04, 0, SCL | SDA);
    reg_modify(GPIOB + 0x0c, (3u << 12) | (3u << 14), (1u << 12) | (1u << 14));
    reg_modify(GPIOB, (3u << 12) | (3u << 14), (1u << 12) | (1u << 14));
}
bool board_user_button_pressed() { return (reg_read(GPIOA + 0x10) & 1u) != 0; }
void board_leds(uint8_t mask)
{
    uint32_t set = 0;
    for (size_t i = 0; i < sizeof LED_PINS / sizeof LED_PINS[0]; ++i)
    {
        if (mask & (1u << i))
            set |= 1u << LED_PINS[i];
    }
    reg_write(GPIOE + 0x18, set | ((0xff00u & ~set) << 16));
}
void board_led_test()
{
    board_leds(0);
    for (size_t bit = 0; bit < sizeof LED_PINS / sizeof LED_PINS[0]; ++bit)
    {
        board_leds(1u << bit);
        board_delay_ms(100);
        board_leds(0);
    }
}
void board_greeting(void)
{
    board_leds((1u << 0) | (1u << 4) | (1u << 5) | (1u << 6) | (1u << 7));
    board_delay_ms(500);
    board_leds(0);
}
void board_calibration_confirmation()
{
    board_leds(0xff);
    board_delay_ms(100);
    board_leds(0);
}
