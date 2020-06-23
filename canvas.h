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
    buff = (cchar_t*)calloc(width * height, sizeof(cchar_t));
    clear();
}

Canvas::~Canvas(void) {
    if (buff)
        free(buff);
}

void Canvas::clear(void) {
    for (int i = 0; i < width * height; i++) {
        buff[i].chars[0] = braille;
    }
}

void Canvas::set(uint16_t x, uint16_t y) {
	int idx = y / 4 * width + x / 2;
	buff[idx].chars[0] |= pixmap[y % 4][x % 2];
}

void Canvas::unset(uint16_t x, uint16_t y) {
	int idx = y / 4 * width + x / 2;
	buff[idx].chars[0] &= ~pixmap[y % 4][x % 2];
}

void Canvas::draw(WINDOW *win) {
	for (int row = 0; row < height; row++) {
		mvwadd_wchnstr(win, row, 0, &buff[row * width], width);
	}
}

int main(int argc, char **argv) {
    setbuf(stdout, NULL);
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    int win_width;
    int win_height;
    getmaxyx(stdscr, win_height, win_width);
    int can_width = win_width * 2;
    int can_height = win_height * 4;
    WINDOW *win = newwin(win_height, win_width, 0, 0);
    Canvas can(can_width, can_height);

    for (int i = 0; i < can_width; i++) {
        can.set(i, i);
    }
    can.draw(win);
    wrefresh(win);

    for (int i = 0; i < can_width; i++) {
        can.unset(i, i);
        can.draw(win);
        wrefresh(win);
        sleep(1);
    }

    sleep(10);
    
    endwin();
    return 0;
}

//} // extern "C"

#endif
