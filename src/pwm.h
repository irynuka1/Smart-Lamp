#pragma once
#include <stdint.h>

/*
 * Timer0 (8-bit), Fast PWM, prescaler 64 -> 976 Hz
 * Timer2 (8-bit), Fast PWM, prescaler 64 -> 976 Hz
 *
 * Base color (tune to adjust hue at full brightness)
 */
#define BASE_R 230
#define BASE_G 55
#define BASE_B 35

void pwm_init(void);
void set_color(uint8_t brightness);
