#ifndef EXTERNALS_H
#define EXTERNALS_H

#include <stdint.h>

__attribute__((import_module("Fb"), import_name("clear")))
void fb_clear(void);

__attribute__((import_module("Fb"), import_name("color")))
void fb_color(uint16_t color);

__attribute__((import_module("Fb"), import_name("move")))
void fb_move(uint8_t x, uint8_t y);

__attribute__((import_module("Fb"), import_name("rectangle")))
void fb_rectangle(uint8_t width, uint8_t height);

__attribute__((import_module("Fb"), import_name("circle")))
void fb_circle(int x, int y, int r);

__attribute__((import_module("Fb"), import_name("filledCircle")))
void fb_filled_circle(int x, int y, int r);

__attribute__((import_module("Fb"), import_name("filledRectangle")))
void fb_filled_rectangle(uint8_t width, uint8_t height);

__attribute__((import_module("Fb"), import_name("roundedRectangle")))
void fb_rounded_rectangle(uint8_t width, uint8_t height, uint8_t stroke);

__attribute__((import_module("Fb"), import_name("swapBuffers")))
void fb_swap_buffers(void);

__attribute__((import_module("Palette"), import_name("drawGrid")))
void palette_draw_grid(uint8_t x, uint8_t y, uint8_t tileSize);

__attribute__((import_module("Palette"), import_name("getColor")))
uint16_t palette_get_color(uint8_t index);

__attribute__((import_module("Palette"), import_name("getColorFromIndex")))
uint16_t palette_get_color_from_index(uint8_t index);

__attribute__((import_module("App"), import_name("closeApp")))
void close_app(void);

#endif
