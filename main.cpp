#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <curses.h>
#include <locale.h>
#include <canvas.h>
#include <chrono>
#include <queue>
#include <thread>
#include "block.h"
#include "track.h"


void read_key(WINDOW *win, std::queue<int> &queue) {
    while (TRUE) {
        int ch = wgetch(win);
        queue.push(ch);
    }
}

void read_wav(FILE *fd, Track *track, uint32_t buf_len) {
    int delay = buf_len / 48000.0 * 1000000; 
    while (1) {
        uint8_t *buf = (uint8_t *)calloc(buf_len, sizeof(uint8_t));
        int ret = fread(buf, 1, buf_len, fd);
        if (ret < buf_len) {
            fseek(fd, 0, SEEK_SET);
            continue;
        }
        Block *block = new Block(buf, buf_len);
        track->append_block(block);

        usleep(delay); // 333ms
    }
    
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
    //long len = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    int win_width, win_height;
    getmaxyx(stdscr, win_height, win_width);
    int can_width = win_width * 2;
    int can_height = win_height * 4;
    WINDOW *win = newwin(win_height, win_width, 0, 0);

    uint32_t buf_len = 1<<12;    // trade off between effiency and wave update rate
	Canvas canvas(win_width*2, win_height*4);
    Track track(65536);
    uint8_t *min = (uint8_t *)calloc(can_width, sizeof(uint8_t));
    uint8_t *max = (uint8_t *)calloc(can_width, sizeof(uint8_t));
    std::queue<int> queue;
    std::thread th1(read_key, win, std::ref(queue));
    std::thread th2(read_wav, fd, &track, buf_len);

    int ch;
    float view_mid = 0.5;
    float view_size = 1;
    float vscale = 1;

    uint32_t num_pixel = 0;
    int ppp = 4000;

    uint32_t start = 0;
	while (TRUE) {
        canvas.clear();
        //int view_size_px = view_size * len;
        //int view_mid_px = view_mid * len;
        //int start = view_mid_px - view_size_px / 2;
        //float ppp = view_size_px / (win_width * 2);
        //if (num_pixel == can_width)
        //    ppp *= 2;
        
        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
       
        //uint32_t view_size_px = 100000;
        //int32_t len = track.get_len() - view_size_px;
        //if (len < 0)
        //    start = 0;
        //else
        //    start = len;
        //ppp = view_size_px / can_width;

        
        num_pixel = track.get_disp_data(start, ppp, can_width, min, max);
        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();

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

        int key = -1;
        if (!queue.empty()) {
            key = queue.front();
            queue.pop();

            switch (key) {
                case 'i':
                    ppp *= 0.5;
                    ppp = std::max(ppp, 1);
                    break;
                case 'o':
                    ppp *= 2;
                    //ppp = std::max(ppp, 1);
                    break;
                case 'l':   // pan right
                    start += can_width * ppp * 0.2;
                    break;
                case 'h':
                    start -= can_width * ppp * 0.2;
                    start = std::max(start, (uint32_t)0);
                default:
                    break;
            }
        } else {
        }
        uint64_t duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        mvwprintw(win, win_height-1, 0, "ppp=%d, wxh=%dx%d, num_pixel=%d, duration=%ldms, data=%.2fMB, key=%d", ppp, can_width, can_height, num_pixel, duration, num_pixel * ppp * 1.0 / (1<<20), key);
    	wrefresh(win);
        
        usleep(50e3); // 10ms
        continue;


        ch = wgetch(win);
        float min_size; 
        switch (ch) {
            case 'i':
                view_size *= 0.8;
                //min_size = win_width * 2.0 / len;
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

    th1.join();
    th2.join();

    endwin();
    return 0;
}
