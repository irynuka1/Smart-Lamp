#pragma once
#include <stdint.h>

/*
 * ADC — single-ended, AVcc reference, blocking conversion
 *
 * ADMUX:  REFS0=1 (AVcc ref), MUX[3:0] = channel
 * ADCSRA: ADEN=1 (enable), ADPS[2:0]=111 (prescaler 128 -> 125 kHz ADC clock)
 *         ADSC=1 starts a conversion; cleared by hardware when done
 *
 * Channels used:
 *   ADC0 (PC0) - LDR
 *   ADC1 (PC1) - potentiometer
 */

void adc_init(void);
uint16_t adc_read(uint8_t ch);
