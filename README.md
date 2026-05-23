# Smart Lamp

A smart lamp controller built on the **ATmega328P Xplained Mini** development board. The firmware is written in C using **avr-libc** - no Arduino framework, no third-party libraries.

---

## Features


| Feature                   | Description                                                |
| ------------------------- | ---------------------------------------------------------- |
| **Three operating modes** | Cycled via onboard button SW200: OFF -> MANUAL -> AUTO     |
| **Manual brightness**     | Potentiometer on ADC1 sets LED intensity (0–100%)          |
| **Automatic brightness**  | LDR on ADC0 adjusts intensity inversely to ambient light   |
| **RGB LED strip**         | 8-bit PWM on three independent channels (configurable hue) |
| **LCD display**           | 16×2 I2C display showing current mode and a brightness bar |
| **Button debounce**       | Software debounce with 50 ms using the ms counter          |


---

## Hardware

### Components


| Component       | Part                                              |
| --------------- | ------------------------------------------------- |
| Microcontroller | ATmega328P Xplained Mini (16 MHz, 5 V)            |
| RGB LED strip   | Common-cathode, 3-channel                         |
| Potentiometer   | 10 kΩ, voltage divider to ADC                     |
| LDR             | Photoresistor + pull-down resistor to ADC         |
| LCD             | 16×2 HD44780-compatible with PCF8574 I2C backpack |


### Pin Connections


| Signal        | ATmega328P Port | Notes                                 |
| ------------- | --------------- | ------------------------------------- |
| LED Red       | PD6             | PWM - OC0A (Timer0)                   |
| LED Green     | PD5             | PWM - OC0B (Timer0)                   |
| LED Blue      | PD3             | PWM - OC2B (Timer2)                   |
| Potentiometer | PC1             | ADC channel 1                         |
| LDR           | PC0             | ADC channel 0                         |
| Button SW200  | PB7             | Onboard, active-low, internal pull-up |
| LCD SDA       | PC4             | TWI hardware pin                      |
| LCD SCL       | PC5             | TWI hardware pin                      |


---

## Software Architecture

The firmware is split into five independent driver modules plus `main.c` for application logic.

```
src/
├── main.c        # Application: mode FSM, brightness, LCD update
├── timer.h / .c  # Timer1 CTC - 1 ms system tick
├── pwm.h   / .c  # Timer0 + Timer2 Fast PWM - RGB output
├── adc.h   / .c  # ADC - blocking single-shot conversion
├── i2c.h   / .c  # TWI/I2C - master polling driver
└── lcd.h   / .c  # HD44780 4-bit driver over PCF8574 backpack
```

---

## Technical Implementation

### Timing — `timer.c`

Timer1 is configured in **CTC mode** to generate a 1 ms interrupt, providing a `ms_now()` / `ms_delay()` facility similar to Arduino's `millis()`.


| Register | Bits set            | Effect                                            |
| -------- | ------------------- | ------------------------------------------------- |
| `TCCR1A` | `0x00`              | No output compare pin action; WGM bits [1:0] = 00 |
| `TCCR1B` | `WGM12, CS11, CS10` | CTC mode; prescaler /64                           |
| `OCR1A`  | `249`               | 16 MHz / 64 / 250 = **1 kHz tick** (1 ms per ISR) |
| `TIMSK1` | `OCIE1A`            | Enable Output Compare A match interrupt           |


The ISR atomically increments a `volatile uint32_t` counter. `ms_now()` reads it with interrupts briefly disabled to avoid a torn read across the 4-byte value.

---

### PWM — `pwm.c`

All three LED channels use hardware PWM at ~976 Hz (16 MHz / 64 / 256). Brightness is scaled per-channel against a base color constant (`BASE_R/G/B` in `pwm.h`) to allow independent hue tuning.

**Timer0** drives the red (PD6/OC0A) and green (PD5/OC0B) channels:


| Register | Bits set                       | Effect                                   |
| -------- | ------------------------------ | ---------------------------------------- |
| `TCCR0A` | `COM0A1, COM0B1, WGM01, WGM00` | Fast PWM, non-inverting on OC0A and OC0B |
| `TCCR0B` | `CS01, CS00`                   | Prescaler 64                             |
| `OCR0A`  | 0–255                          | Red duty cycle                           |
| `OCR0B`  | 0–255                          | Green duty cycle                         |


**Timer2** drives the blue (PD3/OC2B) channel:


| Register | Bits set               | Effect                          |
| -------- | ---------------------- | ------------------------------- |
| `TCCR2A` | `COM2B1, WGM21, WGM20` | Fast PWM, non-inverting on OC2B |
| `TCCR2B` | `CS22`                 | Prescaler 64                    |
| `OCR2B`  | 0–255                  | Blue duty cycle                 |


Brightness scaling uses integer arithmetic to avoid floating-point:

```c
OCR0A = (uint16_t)BASE_R * brightness / 255;
```

---

### ADC — `adc.c`

The ADC is configured for **single-ended conversions** against the AVcc reference (5 V). A prescaler of 128 gives an ADC clock of 125 kHz, within the 50–200 kHz recommended range for 10-bit accuracy.


| Register | Bits set                    | Effect                                        |
| -------- | --------------------------- | --------------------------------------------- |
| `ADMUX`  | `REFS0, MUX[2:0]`           | AVcc reference; channel selected per call     |
| `ADCSRA` | `ADEN, ADPS2, ADPS1, ADPS0` | ADC enable; prescaler 128 → 125 kHz ADC clock |


Each call to `adc_read(ch)` performs a blocking conversion: sets `ADSC`, then polls until the hardware clears it, then reads the 10-bit result from the `ADC` register (ADCL/ADCH).

The 10-bit result (0–1023) is right-shifted by 2 to produce an 8-bit brightness value (0–255).

---

### TWI / I2C — `i2c.c`

The hardware TWI peripheral is used in **master transmit mode** with polling (no interrupts).


| Register | Value / bits set | Effect                                    |
| -------- | ---------------- | ----------------------------------------- |
| `TWSR`   | `0x00`           | Prescaler bits TWPS[1:0] = 00 -> factor 1 |
| `TWBR`   | `72`             | SCL = 16 MHz / (16 + 2*72) = 100 kHz      |
| `TWCR`   | `TWEN`           | Enable TWI peripheral                     |


Each `i2c_write()` call performs: START -> address+W -> data byte -> STOP. The driver waits for `TWINT` to be set after each operation, which signals that the hardware has completed the current step.

---

### HD44780 LCD — `lcd.c`

The LCD is controlled in **4-bit mode** through a PCF8574 I2C I/O expander. Each byte sent to the PCF8574 maps to the following LCD signals:

```
 7  6  5  4 | 3   2   1   0
D7 D6 D5 D4 | BL  EN  RW  RS
```

Sending a byte to the LCD requires two nibble transfers (high nibble first). Each nibble transfer pulses the EN pin high then low - the HD44780 latches data on the falling edge of EN.

**Initialization sequence** (per HD44780 datasheet):

```
Wait >= 50 ms after Vcc rise
Send nibble 0x3  ->  wait 5 ms
Send nibble 0x3  ->  wait 1 ms
Send nibble 0x3  ->  wait 1 ms
Send nibble 0x2  ->  switch to 4-bit mode
0x28  Function Set: 4-bit, 2 lines, 5×8 font
0x0C  Display on, cursor off
0x06  Entry mode: auto-increment, no shift
0x01  Clear display (wait 2 ms)
```

---

## Building & Flashing

### Prerequisites

- [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/) (CLI or VS Code extension)
- ATmega328P Xplained Mini connected via USB

### Commands

```bash
# Clone the repository
git clone <repo-url>
cd <repo-dir>

# Compile only
platformio run

# Compile and upload
platformio run --target upload
```

### `platformio.ini`

```ini
[env:avr]
platform = atmelavr
board = atmega328p_xplained_mini

upload_protocol = xplainedmini
upload_flags =
  -P
  usb
  -e
```

---

## Customisation

**Hue adjustment** — edit `BASE_R`, `BASE_G`, `BASE_B` in `src/pwm.h`.

**LCD I2C address** — change `LCD_ADDR` in `src/lcd.h`.

**Debounce / LCD rate** — adjust `DEBOUNCE_MS` and `LCD_UPDATE_MS` in `src/main.c`.