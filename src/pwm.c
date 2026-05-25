#include "pwm.h"
#include <avr/io.h>

void pwm_init(void) {
    // Timer0: Fast PWM, non-inverting on OC0A (PD6) and OC0B (PD5), prescaler 64
    TCCR0A = (1 << COM0A1) | (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
    TCCR0B = (1 << CS01) | (1 << CS00);
    OCR0A = 0;
    OCR0B = 0;
    DDRD |= (1 << PD6) | (1 << PD5);

    // Timer2: Fast PWM, non-inverting on OC2B (PD3), prescaler 64
    TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
    TCCR2B = (1 << CS22);
    OCR2B = 0;
    DDRD |= (1 << PD3);
}

void set_color(uint8_t brightness) {
    OCR0A = (uint16_t)BASE_R * brightness / 255; // red
    OCR0B = (uint16_t)BASE_G * brightness / 255; // green
    OCR2B = (uint16_t)BASE_B * brightness / 255; // blue
}
