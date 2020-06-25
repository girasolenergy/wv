#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <curses.h>
#include <locale.h>
#include <canvas.h>


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

    FILE *fd;
    if (argc > 1)
	    fd = fopen(argv[1], "rb");
    else
	    fd = fopen("data.raw", "rb");
    if (fd == NULL) exit(-1);
    fseek(fd, 0, SEEK_END);
    long len = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    char *buff = (char *)calloc(len, sizeof(char));
    fread(buff, 1, len, fd);
    fclose(fd);
    
    track_t track = track_init(buff, len);

    int win_width;// = COLS;
    int win_height;// = LINES;
    getmaxyx(stdscr, win_height, win_width);
    int can_width = win_width * 2;
    int can_height = win_height * 4;
    WINDOW *win = newwin(win_height, win_width, 0, 0);
	Canvas canvas(win_width*2, win_height*4);
    

    int ch;
    float view_mid = 0.5;
    float view_size = 1;
    float vscale = 1;

	while (TRUE) {
        canvas.clear();
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
            int _min = (min - 128) * vscale + can_height / 2;
            int _max = (max - 128) * vscale + can_height / 2;
            for (int y = _min; y <= _max; y++) { // should use <= here otherwise missing points
                int _y = can_height - y;
                _y = MIN(_y, can_height-1);
                _y = MAX(_y, 0);
                canvas.set(i, _y);
            }
		}

		canvas.draw(win);
        mvwprintw(win, win_height-1, 0, "ppp=%.0f, wxh=%dx%d", ppp, win_width, win_height);
    	wrefresh(win);


        ch = wgetch(win);
        float min_size; 
        switch (ch) {
            case 'i':
                view_size *= 0.8;
                min_size = win_width * 2.0 / len;
                view_size = MAX(view_size, min_size);
                break;
            case 'o':
                view_size /= 0.8;
                //view_size = MIN(view_size, 1);
                break;
            case 'I':   // vertical zoom in
                vscale /= 0.8;
                break;
            case 'O':
                vscale *= 0.8;
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
