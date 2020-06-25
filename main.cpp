#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <curses.h>
#include <locale.h>
#include <canvas.h>
#include "block.h"
#include "track.h"


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
    //char *buff = (char *)calloc(len, sizeof(char));
    //fread(buff, 1, len, fd);
    //fclose(fd);
    
    uint32_t buf_len = 8192;    // trade off between effiency and wave update rate
    

    int win_width;
    int win_height;// = LINES;
    getmaxyx(stdscr, win_height, win_width);
    int can_width = win_width * 2;
    int can_height = win_height * 4;
    WINDOW *win = newwin(win_height, win_width, 0, 0);
	Canvas canvas(win_width*2, win_height*4);
    Track track(65536);

    uint8_t *min = (uint8_t *)calloc(can_width, sizeof(uint8_t));
    uint8_t *max = (uint8_t *)calloc(can_width, sizeof(uint8_t));

    int ch;
    float view_mid = 0.5;
    float view_size = 1;
    float vscale = 1;

    uint32_t num_pixel;
	while (TRUE) {
        canvas.clear();
        //int view_size_px = view_size * len;
        //int view_mid_px = view_mid * len;
        //int start = view_mid_px - view_size_px / 2;
        //float ppp = view_size_px / (win_width * 2);
        int ppp = 4800;
        
        uint8_t *buf = (uint8_t *)calloc(buf_len, sizeof(uint8_t));
        fread(buf, 1, buf_len, fd);
        Block *block = new Block(buf, buf_len);
        track.append_block(block);
        
        num_pixel = track.get_disp_data(0, ppp, can_width, min, max);

		for (int i = 0; i < num_pixel; i++) {
            int _min = (min[i] - 128) * vscale + can_height / 2;
            int _max = (max[i] - 128) * vscale + can_height / 2;
            for (int y = _min; y <= _max; y++) { // should use <= here otherwise missing points
                int _y = can_height - y;
                _y = std::min(_y, can_height-1);
                _y = std::max(_y, 0);
                canvas.set(i, _y);
            }
		}

		canvas.draw(win);
        mvwprintw(win, win_height-1, 0, "ppp=%d, wxh=%dx%d, num_pixel=%d", ppp, can_width, can_height, num_pixel);
    	wrefresh(win);
        
        usleep(1e5);
        continue;


        ch = wgetch(win);
        float min_size; 
        switch (ch) {
            case 'i':
                view_size *= 0.8;
                min_size = win_width * 2.0 / len;
                view_size = std::max(view_size, min_size);
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
