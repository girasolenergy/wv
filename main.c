#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED // pour utilisÃ© les fonction wide character et cchar_t
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <curses.h>
#include <locale.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

int braille = 0x2800;
uint8_t pixmap[4][2] = {
    {0x01, 0x08},
    {0x02, 0x10},
    {0x04, 0x20},
    {0x40, 0x80}
};

typedef struct {
	cchar_t*	buff;
	int 		width;
	int			height;
} canvas_t;

canvas_t canvas_init(int width, int height) {
	canvas_t canvas;
	canvas.width = width;
	canvas.height = height;
    canvas.buff = (cchar_t*)calloc(width * height, sizeof(cchar_t));
    canvas_clear(canvas);
    return canvas;
}

void canvas_set(canvas_t canvas, int x, int y) {
	int idx = y / 4 * canvas.width + x / 2;
	canvas.buff[idx].chars[0] |= pixmap[y % 4][x % 2];
}

void canvas_draw(canvas_t canvas, WINDOW *win) {
	for (int row = 0; row < canvas.height; row++) {
		mvwadd_wchnstr(win, row, 0, &canvas.buff[row * canvas.width], canvas.width);
	}
}

void canvas_clear(canvas_t canvas) {
	for (int i = 0; i < canvas.width * canvas.height; i++) {
		canvas.buff[i].chars[0] = braille;
	}
}


int main(int argc, char **argv) {
    setbuf(stdout, NULL);
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

	//FILE *fd = fopen(argv[1], "rb");
	FILE *fd = fopen("d.raw", "rb");
	if (fd == NULL) {
		exit(-1);
	}
	char buff[1000];
	fread(buff, 1, 1000, fd);

    int win_width = 80;
    int win_height = 50;
    WINDOW *win = newwin(win_height, win_width, 0, 0);

	canvas_t canvas = canvas_init(win_height, win_width);

    int j = 10;
    int ch;
	while (TRUE) {
        ch = getch();
        if (ch == 'l')
            j++;
        
        canvas_clear(canvas);
		for (int i = 0; i < 100; i++) {
            int y = buff[i + j];
            y = MIN(y, canvas.height * 4);
			canvas_set(canvas, i, buff[i+j]);
		}
		canvas_draw(canvas, win);
    	wrefresh(win);
        //sleep(1);
	}

    sleep(999);
    endwin();
    return 0;
}
