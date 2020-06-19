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

void canvas_clear(canvas_t canvas) {
	for (int i = 0; i < canvas.width * canvas.height; i++) {
		canvas.buff[i].chars[0] = braille;
	}
}

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

typedef struct {
    int  buff_size;
    char *buff;
    int  group_size;
    char *max;
    char *min;
} track_t;
    
track_t track_init(char *buff, int size) {
    track_t track;
    track.buff_size = size;
    track.buff = buff;
    track.group_size = 1 << 10; // must be the power of 2 since we use mask later
    int num_group = track.buff_size / track.group_size;
    //printf("buffsize=%d,group_size=%d,num_group=%d\n", size, track.group_size, num_group);
    track.min = (char*)calloc(num_group, sizeof(char));
    track.max = (char*)calloc(num_group, sizeof(char));
    
    char *_buff = track.buff;
    for (int i = 0; i < num_group; i++) {
        char _min = 255;
        char _max = 0;
        for (int j = 0; j < track.group_size; j++) {
            _min = MIN(*_buff, _min);
            _max = MAX(*_buff, _max);
            _buff++;
        }
        track.min[i] = _min;
        track.max[i] = _max;
    }

    return track;
}

void track_get_minmax(track_t track, int start, int end, char *min, char *max) {
    char _min = 255;
    char _max = 0;
    uint32_t _mask = track.group_size - 1;
    uint32_t num_total_samples = end - start + 1;
    // ahead of group boundary, iteration one by one slowly
    int num_slow_samples = (track.group_size - start) & _mask;
    if (num_slow_samples > num_total_samples)
        num_slow_samples = num_total_samples;
    int _end = start + num_slow_samples;
    int _idx = start;
    while (_idx < _end) {
        _min = MIN(track.buff[_idx], _min);
        _max = MAX(track.buff[_idx], _max);
        _idx++;
    }
    num_total_samples -= num_slow_samples;
    // contains at least one group
    while (num_total_samples > track.group_size) {
        int group_idx = _idx / track.group_size;
        _min = MIN(track.min[group_idx], _min);
        _max = MAX(track.max[group_idx], _max);
        _idx += track.group_size;
        num_total_samples -= track.group_size;
    }
    // behind group boundary, interate one by one slowly
    int end_idx = _idx + num_total_samples;
    //printf("%d\n", num_total_samples);
    while (_idx < end_idx) {
        _min = MIN(track.buff[_idx], _min);
        _max = MAX(track.buff[_idx], _max);
        _idx++;
    }

    *min = _min;
    *max = _max;
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
    if (fd == NULL) exit(-1);
    fseek(fd, 0, SEEK_END);
    long len = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    char *buff = (char *)calloc(len, sizeof(char));
    fread(buff, 1, len, fd);
    fclose(fd);
    
    track_t track = track_init(buff, len);

    int win_width = 80;
    int win_height = 65;
    WINDOW *win = newwin(win_height, win_width, 0, 0);
	canvas_t canvas = canvas_init(win_width*2, win_height*4);
    

    int ch;
    float view_mid = 0.5;
    float view_size = 1;

	while (TRUE) {
        canvas_clear(canvas);
        int view_size_px = view_size * len;
        int view_mid_px = view_mid * len;
        int start = view_mid_px - view_size_px / 2;
        float ppp = view_size_px / (win_width * 2);
		for (int i = 0; i < win_width*2; i++) {
            int st = start + i * ppp;
            if (st < 0)
                continue;
            int ed = st + ppp;
            if (ed > len)
                break;
            char min, max;
            track_get_minmax(track, st, ed, &min, &max);
            for (int y = min; y < max; y++)
                canvas_set(canvas, i, y);
		}

		canvas_draw(canvas, win);
    	wrefresh(win);


        ch = wgetch(win);
        switch (ch) {
            case 'i':
                view_size *= 0.8;
                break;
            case 'o':
                view_size /= 0.8;
                //view_size = MIN(view_size, 1);
                break;
            case 'l':
                view_mid += view_size * 0.1;
                break;
            case 'h':
                view_mid -= view_size * 0.1;
                break;
            default:
                break;
        }
    }

    endwin();
    return 0;
}
