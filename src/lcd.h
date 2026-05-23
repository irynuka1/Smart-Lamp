#pragma once
#include <stdint.h>

/*
 * HD44780 16x2 LCD driver - 4-bit mode I2C backpack
 *
 * I2C address: 0x27
 *
 * Initialization follows the HD44780 datasheet reset sequence
 * for 4-bit mode, sending nibble 0x3 three times then 0x2.
 */

#define LCD_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS  2

void lcd_init(void);
void lcd_set_cursor(uint8_t col, uint8_t row);
void lcd_print(const char *s);
void lcd_write_char(uint8_t c);
