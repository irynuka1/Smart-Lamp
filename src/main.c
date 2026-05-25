#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdio.h>

#include "timer.h"
#include "pwm.h"
#include "adc.h"
#include "i2c.h"
#include "lcd.h"

// RGB strip: Red=PD6 (OC0A), Green=PD5 (OC0B), Blue=PD3 (OC2B)
// Button:    PB7 — SW200, active-low, internal pull-up
// Pot:       PC1 (ADC1)
// LDR:       PC0 (ADC0)
// LCD:       16×2, I2C, SDA=PC4, SCL=PC5

#define POT_CH        1   // ADC channel for potentiometer
#define LDR_CH        0   // ADC channel for LDR
#define DEBOUNCE_MS   50
#define LCD_UPDATE_MS 150

typedef enum { OFF, MANUAL, AUTO } Mode;

static const char *mode_name(Mode mode) {
    switch (mode) {
        case OFF:
            return "OFF";
        case MANUAL:
            return "MANUAL";
        case AUTO:
            return "AUTO";
    }
    return "";
}

static void lcd_update(uint8_t brightness, Mode mode) {
    uint8_t pct = (uint16_t)brightness * 100 / 255;
    uint8_t filled = (uint16_t)brightness * 14  / 255;

    // Line 1: "MANUAL       75%"
    char line1[17];
    snprintf(line1, sizeof(line1), "%-6s      %3u%%",
             mode_name(mode), (unsigned)pct);

    // Line 2: "[##########    ]"
    char line2[17];
    line2[0] = '[';
    for (uint8_t i = 0; i < 14; i++)
        line2[i + 1] = (i < filled) ? (char)0xFF : ' ';
    line2[15] = ']';
    line2[16] = '\0';

    lcd_set_cursor(0, 0);
    lcd_print(line1);
    lcd_set_cursor(0, 1);
    lcd_print(line2);
}

int main(void) {
    // Button PB7: input + internal pull-up
    DDRB &= ~(1 << PB7);
    PORTB |= (1 << PB7);

    timer1_init();
    pwm_init();
    adc_init();
    i2c_init();
    sei(); // enable interrupts for ms_now / ms_delay

    lcd_init();

    Mode mode = OFF;
    uint8_t btn_last = 1;
    uint32_t debounce_ts = 0;
    uint32_t lcd_ts = 0;

    for (;;) {
        // Button: falling edge -> cycle OFF -> MANUAL -> AUTO
        uint8_t btn = (PINB >> PB7) & 1;
        if (!btn && btn_last) {
            uint32_t now = ms_now();
            if (now - debounce_ts > DEBOUNCE_MS) {
                switch (mode) {
                    case OFF:
                        mode = MANUAL;
                        break;
                    case MANUAL:
                        mode = AUTO;
                        break;
                    case AUTO:
                        mode = OFF;
                        break;
                }
                debounce_ts = now;
            }
        }
        btn_last = btn;

        // Brightness
        uint8_t brightness;
        switch (mode) {
            case MANUAL:
                brightness = adc_read(POT_CH) >> 2;
                break;
            case AUTO:
                brightness = 255 - (adc_read(LDR_CH) >> 2);
                break;
            default:
                brightness = 0;
                break;
        }
        set_color(brightness);

        // LCD update (rate-limited to avoid flicker)
        uint32_t now = ms_now();
        if (now - lcd_ts >= LCD_UPDATE_MS) {
            lcd_update(brightness, mode);
            lcd_ts = now;
        }
    }
}
