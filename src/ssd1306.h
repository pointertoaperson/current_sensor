#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>

#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 32
#define SSD1306_PAGES  (SSD1306_HEIGHT/8)
#define SSD1306_I2C_ADDR 0x3C

void ssd1306_init(void);
void ssd1306_clear(void);
void ssd1306_update(void);
void ssd1306_draw_pixel(uint8_t x, uint8_t y, uint8_t color);

/* Draw scaled digits, dot, minus. scale >=1 */
void ssd1306_draw_string_big(uint8_t x, uint8_t y, const char *s, uint8_t scale);

/* Print helpers: writes into framebuffer and updates display */
void ssd1306_print_big_text(uint8_t x, uint8_t y, const char *s, uint8_t scale);
void ssd1306_print_float_big(uint8_t x, uint8_t y, float v, uint8_t decimals, uint8_t scale);

#endif
