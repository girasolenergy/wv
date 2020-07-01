#include "canvas.h"

Canvas::Canvas(uint16_t width, uint16_t height) {
    this->width = width;
    this->height = height;
    buff = (uint32_t *)calloc(width * height / 8, sizeof(uint32_t));
    clear();
}

Canvas::~Canvas(void) {
    if (buff)
        free(buff);
}

void Canvas::clear(void) {
    for (int i = 0; i < width * height / 8; i++) {
        buff[i] = braille;
    }
}

void Canvas::set(uint16_t x, uint16_t y) {
	int idx = y / 4 * width / 2 + x / 2;
	buff[idx] |= pixmap[y % 4][x % 2];
}

void Canvas::unset(uint16_t x, uint16_t y) {
	int idx = y / 4 * width / 2 + x / 2;
	buff[idx] &= ~pixmap[y % 4][x % 2];
}

void Canvas::draw(void) {
    for (int x = 0; x < width / 2; x++)
        for (int y = 0; y < height / 4; y++) {
            uint32_t ch = buff[y * width / 2 + x];
            tb_change_cell(x, y, ch, TB_DEFAULT, TB_DEFAULT);
        }
}
