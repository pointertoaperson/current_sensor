#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>  
#include "ssd1306.h"
#include "i2c.h"
#include "fonts.h"


// framebuffer for 128x32 => 128 * 4 pages = 512 bytes
static uint8_t ssd1306_buffer[SSD1306_WIDTH * SSD1306_PAGES];

// low-level I2C command
static void ssd1306_write_command(uint8_t cmd) {
    twi_start();
    twi_write((SSD1306_I2C_ADDR << 1) | 0); // write address
    twi_write(0x00); // control byte: Co=0, D/C#=0 (command)
    twi_write(cmd);
    twi_stop();
}

void ssd1306_init(void) {
    twi_init();

    // Init sequence for 128x32
    ssd1306_write_command(0xAE);
    ssd1306_write_command(0x20); ssd1306_write_command(0x00);
    ssd1306_write_command(0xB0);
    ssd1306_write_command(0xC8);
    ssd1306_write_command(0x00);
    ssd1306_write_command(0x10);
    ssd1306_write_command(0x40);
    ssd1306_write_command(0x81); ssd1306_write_command(0x7F);
    ssd1306_write_command(0xA1);
    ssd1306_write_command(0xA6);
    ssd1306_write_command(0xA8); ssd1306_write_command(0x1F);
    ssd1306_write_command(0xA4);
    ssd1306_write_command(0xD3); ssd1306_write_command(0x00);
    ssd1306_write_command(0xD5); ssd1306_write_command(0x80);
    ssd1306_write_command(0xD9); ssd1306_write_command(0xF1);
    ssd1306_write_command(0xDA); ssd1306_write_command(0x02);
    ssd1306_write_command(0xDB); ssd1306_write_command(0x40);
    ssd1306_write_command(0x8D); ssd1306_write_command(0x14);
    ssd1306_write_command(0xAF);

    memset(ssd1306_buffer, 0, sizeof(ssd1306_buffer));
    ssd1306_update();
}

void ssd1306_clear(void) {
    memset(ssd1306_buffer, 0x00, sizeof(ssd1306_buffer));
}

void ssd1306_update(void) {
    for (uint8_t page = 0; page < SSD1306_PAGES; page++) {
        ssd1306_write_command(0xB0 | page);
        ssd1306_write_command(0x00);
        ssd1306_write_command(0x10);

        twi_start();
        twi_write((SSD1306_I2C_ADDR << 1) | 0);
        twi_write(0x40);
        for (uint8_t col = 0; col < SSD1306_WIDTH; col++) {
            twi_write(ssd1306_buffer[page * SSD1306_WIDTH + col]);
        }
        twi_stop();
    }
}

void ssd1306_draw_pixel(uint8_t x, uint8_t y, uint8_t color) {
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;
    uint16_t index = (y / 8) * SSD1306_WIDTH + x;
    uint8_t bit = 1 << (y & 7);
    if (color) ssd1306_buffer[index] |= bit;
    else ssd1306_buffer[index] &= ~bit;
}



static void ssd1306_draw_char_big_internal(uint8_t x, uint8_t y, char c, uint8_t scale) {
    const uint8_t spacing = 1;
    const uint8_t *glyph = font5x7_glyph(c);
    if (!glyph) return; // unsupported character, skip

    for (uint8_t col = 0; col < 5; col++) {
        uint8_t colbits = glyph[col];
        for (uint8_t sx = 0; sx < scale; sx++) {
            for (uint8_t row = 0; row < 8; row++) {
                uint8_t pixel = (colbits >> row) & 1;
                for (uint8_t sy = 0; sy < scale; sy++) {
                    ssd1306_draw_pixel(x + col * scale + sx, y + row * scale + sy, pixel);
                }
            }
        }
    }
}

void ssd1306_draw_string_big(uint8_t x, uint8_t y, const char *s, uint8_t scale) {
    uint8_t cursor_x = x;
    while (*s) {
        ssd1306_draw_char_big_internal(cursor_x, y, *s, scale);
        cursor_x += (5 + 1) * scale;
        s++;
    }
}

void ssd1306_print_float_big(uint8_t x, uint8_t y, float v, uint8_t decimals, uint8_t scale) {
    char buf[16];
    if (v < 0.0f) {
        buf[0] = '-';
        v = -v;
        dtostrf(v, 0, decimals, buf + 1);
    } else {
        dtostrf(v, 0, decimals, buf);
    }
    ssd1306_draw_string_big(x, y, buf, scale);
    ssd1306_update();
}
