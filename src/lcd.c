#include "lcd.h"
#include "i2c.h"
#include "timer.h"
#include <util/delay.h>

#define LCD_BL 0x08 // backlight pin
#define LCD_EN 0x04 // enable pin
#define LCD_RS 0x01 // register select pin (0=command, 1=data)


static void lcd_pulse(uint8_t data) {
    i2c_write(LCD_ADDR, data | LCD_EN);
    _delay_us(1);
    i2c_write(LCD_ADDR, data & ~LCD_EN);
    _delay_us(50);
}

static void lcd_nibble(uint8_t nib, uint8_t rs) {
    lcd_pulse((nib << 4) | LCD_BL | rs);
}

static void lcd_byte(uint8_t b, uint8_t rs) {
    lcd_nibble(b >> 4,   rs); // high nibble first
    lcd_nibble(b & 0x0F, rs); // then low nibble
}

static void lcd_cmd(uint8_t c) { 
    lcd_byte(c, 0);
}

void lcd_write_char(uint8_t c) {
    lcd_byte(c, LCD_RS);
}

void lcd_set_cursor(uint8_t col, uint8_t row) {
    // Row 0 starts at DDRAM 0x00, row 1 at 0x40
    lcd_cmd(0x80 | (col + (row ? 0x40 : 0x00)));
}

void lcd_print(const char *s) {
    while (*s) lcd_write_char((uint8_t)*s++);
}

void lcd_init(void) {
    ms_delay(50); // wait for LCD power to rise
    i2c_write(LCD_ADDR, LCD_BL);
    ms_delay(10);

    lcd_nibble(0x03, 0); ms_delay(5);
    lcd_nibble(0x03, 0); ms_delay(1);
    lcd_nibble(0x03, 0); ms_delay(1);
    lcd_nibble(0x02, 0); ms_delay(1); // switch to 4-bit mode

    lcd_cmd(0x28); // Function Set: 4-bit, 2 lines, 5x8 dots
    lcd_cmd(0x0C); // Display Control: display on, cursor off, blink off
    lcd_cmd(0x06); // Entry Mode: increment cursor, no display shift
    lcd_cmd(0x01); // Clear Display
    ms_delay(2);
}
