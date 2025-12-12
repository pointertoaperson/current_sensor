# AVR Bare-metal ADC sampler + SSD1306 display

This small project targets the ATmega328P (e.g. Arduino Uno MCU) and:
- Samples ADC0 at 1 kHz for 30 ms (30 samples)
- Finds the maximum ADC reading and converts it to voltage assuming AVcc = 5.0V
- Displays the result on an SSD1306 128x64 OLED via I2C with scaled/big digits

Files added:
- `src/main.c` - application logic, Timer1 + ADC setup
- `src/i2c.c`, `src/i2c.h` - basic TWI (I2C) routines
- `src/ssd1306.c`, `src/ssd1306.h` - SSD1306 framebuffer, init and big-digit draw
- `src/fonts.h` - 5x7 digit glyphs used for scaling

Wiring (example for I2C SSD1306):
- Connect `SDA` -> `PC4` (A4)
- Connect `SCL` -> `PC5` (A5)
- Connect GND and VCC (check module voltage: many modules are 3.3V tolerant or 5V tolerant)

Build (avr-gcc / avrdude):

```bash
avr-gcc -mmcu=atmega328p -DF_CPU=16000000UL -Os -I./src -o build/main.elf src/main.c src/i2c.c src/ssd1306.c
avr-objcopy -O ihex -R .eeprom build/main.elf build/main.hex
# then flash with avrdude (adjust programmer and port):
avrdude -c arduino -p m328p -P /dev/ttyUSB0 -b 115200 -U flash:w:build/main.hex
```

Notes:
- The code assumes `F_CPU = 16MHz` and AVcc as 5.0V. Adjust if your board differs.
- The SSD1306 driver here is minimal and uses the hardware TWI peripheral.
- The big-font rendering scales a 5x7 digits font by an integer scale. It only reliably handles digits, '.' and '-'.
- `dtostrf` (avr-libc) is used to format floats.

If you want: I can wire up a full Makefile that integrates compilation and avrdude flashing, or extend the font set to support arbitrary ASCII characters.
