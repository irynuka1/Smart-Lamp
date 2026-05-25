#pragma once
#include <stdint.h>

/*
 * Timer1 CTC - 1 ms system tick
 *
 * TCCR1B: WGM12=1 (CTC), CS11|CS10 (prescaler 64)
 * OCR1A:  249  ->  16 MHz / 64 / 250 = 1 kHz tick
 * TIMSK1: OCIE1A=1 (compare-match interrupt)
 */

void timer1_init(void);
uint32_t ms_now(void);
void ms_delay(uint32_t ms);
