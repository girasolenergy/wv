#ifndef __CANVAS_H__
#define __CANVAS_H__

//extern "C" {

// for cchar_t
#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <curses.h>
#include <locale.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

class Canvas {
    public:
        Canvas(uint16_t width, uint16_t height);
        ~Canvas(void);
        void clear(void);
        void set(uint16_t x, uint16_t y);
        void unset(uint16_t x, uint16_t y);
        void draw(WINDOW *win);
        
        int width, height;
        
    private:
	    cchar_t*	buff;
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
    buff = (cchar_t*)calloc(width * height / 8, sizeof(cchar_t));
    clear();
}

Canvas::~Canvas(void) {
    if (buff)
        free(buff);
}

void Canvas::clear(void) {
    for (int i = 0; i < width * height / 8; i++) {
        buff[i].chars[0] = braille;
    }
}

void Canvas::set(uint16_t x, uint16_t y) {
	int idx = y / 4 * width / 2 + x / 2;
	buff[idx].chars[0] |= pixmap[y % 4][x % 2];
}

void Canvas::unset(uint16_t x, uint16_t y) {
	int idx = y / 4 * width / 2 + x / 2;
	buff[idx].chars[0] &= ~pixmap[y % 4][x % 2];
}

void Canvas::draw(WINDOW *win) {
	for (int row = 0; row < height / 4; row++) {
		mvwadd_wchnstr(win, row, 0, &buff[row * width / 2], width / 2);
	}
}
//} // extern "C"

#endif
