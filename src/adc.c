#include "adc.h"
#include <avr/io.h>

void adc_init(void) {
    ADMUX = (1 << REFS0);  // AVcc reference
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // enable, prescaler 128
}

uint16_t adc_read(uint8_t ch) {
    ADMUX  = (1 << REFS0) | (ch & 0x07); // select channel, keep AVcc reference
    ADCSRA |= (1 << ADSC);               // start conversion
    while (ADCSRA & (1 << ADSC));        // wait until ADSC cleared by hardware
    return ADC;                          // read 10-bit result
}
