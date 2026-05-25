#include "timer.h"
#include <avr/io.h>
#include <avr/interrupt.h>

static volatile uint32_t g_ms = 0;

ISR(TIMER1_COMPA_vect) {
    g_ms++;
}

uint32_t ms_now(void) {
    uint32_t v;
    cli(); v = g_ms; sei();
    return v;
}

void ms_delay(uint32_t ms) {
    uint32_t t = ms_now();
    while (ms_now() - t < ms);
}

void timer1_init(void) {
    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10); // CTC, prescaler 64
    OCR1A = 249;  // 16 MHz / 64 / 250 = 1 kHz -> 1 ms per tick
    TIMSK1 = (1 << OCIE1A);
}
