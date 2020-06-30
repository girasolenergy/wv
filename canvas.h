#ifndef __CANVAS_H__
#define __CANVAS_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "termbox.h"


class Canvas {
    public:
        Canvas(uint16_t width, uint16_t height);
        ~Canvas(void);
        void clear(void);
        void set(uint16_t x, uint16_t y);
        void unset(uint16_t x, uint16_t y);
        void draw(void);
        
        uint32_t width, height;
        uint8_t *min, *max;
        
    private:
	    uint32_t    *buff;
        uint32_t    braille = 0x2800;
        uint8_t     pixmap[4][2] = {
                        {0x01, 0x08},
                        {0x02, 0x10},
                        {0x04, 0x20},
                        {0x40, 0x80}
                        };
        
};

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

#endif
