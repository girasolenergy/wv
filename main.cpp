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
#include "limits.h"
#include "termbox.h"


void read_key(WINDOW *win, std::queue<int> &queue) {
    while (TRUE) {
        int ch = wgetch(win);
        queue.push(ch);
    }
}

void read_wav(FILE *fd, Track *track, uint32_t buf_len, bool &doread) {
    int delay = buf_len / 48000.0 * 1000000; 
    while (1) {
        uint8_t *buf = (uint8_t *)calloc(buf_len, sizeof(uint8_t));
        int ret = fread(buf, 1, buf_len, fd);
        if (ret == 0)
            break;
        //if (ret < buf_len) {
        //    fseek(fd, 0, SEEK_SET);
        //    continue;
        //}
        if (doread) {
            Block *block = new Block(buf, buf_len);
            track->append_block(block);
        }
        //usleep(delay); // 333ms
    }
    
}

int main(int argc, char **argv) {
    setbuf(stdout, NULL);

    int ret = tb_init();
    tb_select_input_mode(TB_INPUT_CURRENT);
    int win_w, win_h;
    win_w = tb_width();
    win_h = tb_height();
    tb_change_cell(0, 0, 0x28ff, TB_DEFAULT, TB_DEFAULT);
    tb_present();
    //sleep(2);
    //tb_shutdown();
    //return 0;

    FILE *fd;
    if (argc == 1) {
        endwin();
        return 0;
    }
	fd = fopen(argv[1], "rb");
    if (fd == NULL) exit(-1);


    Track track(65536);
    int can_width = win_w * 2;
    int can_height = win_h * 4;
	Canvas canvas(can_width, can_height);
    uint8_t *min = (uint8_t *)calloc(can_width, sizeof(uint8_t));
    uint8_t *max = (uint8_t *)calloc(can_width, sizeof(uint8_t));

    std::queue<int> queue;
    //std::thread th1(read_key, win, std::ref(queue));
    bool doread = true;
    uint32_t buf_len = 1<<14;    // trade off between effiency and wave update rate
    std::thread th2(read_wav, fd, &track, buf_len, std::ref(doread));

    float vscale = 1;
    uint32_t num_pixel = 0;
    int ppp = buf_len / 4;
    int32_t start = 0;

	while (TRUE) {
        canvas.clear();

        //std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
        uint32_t view_mid = start + ppp * can_width / 2;
        uint32_t view_len = ppp * can_width;
        num_pixel = track.get_disp_data(start, ppp, can_width, min, max);
        //std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();

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
		canvas.draw();
        tb_present();


        int key = -1;
        struct tb_event ev;
        int ret = tb_poll_event(&ev);

        if (ret == TB_EVENT_KEY) {
            uint32_t key = ev.ch;
            if (key == 'q') {
                tb_shutdown();
                return 0;
            }
 
            uint32_t pan_step = can_width * 0.1;
            switch (key) {
                case 'i':
                    if (ppp / 2 < 1)
                        break;
                    // if waveform is less than one screen, the zoom center is not the center of the screen
                    if (num_pixel >= can_width) 
                        start += (num_pixel * 0.5 / 2) * ppp;
                    ppp *= 0.5;
                    break;
                case 'o':
                    if (ppp >= INT_MAX / 2)
                        break;
                    ppp *= 2;
                    start -= (can_width * 0.5 / 2) * ppp;
                    start = std::max(start, 0);
                    break;
                case 'l':   // pan right
                    if (start > INT_MAX - pan_step * ppp)
                        break;
                    start += pan_step * ppp;
                    break;
                case 'h':
                    start -= pan_step * ppp;
                    start = std::max(start, 0);
                    break;
                case 'I':   // zoom vertically
                    vscale *= 1.2;
                    break;
                case 'O':
                    vscale /= 1.2;
                    break;
                case 'p':   // pause
                    doread = !doread;
                    break;
                default:
                    break;
            }
        }
        continue;

        if (!queue.empty()) {
            key = queue.front();
            queue.pop();
            uint32_t pan_step = can_width * 0.1;

            switch (key) {
                case 'i':
                    if (ppp / 2 < 1)
                        break;
                    // if waveform is less than one screen, the zoom center is not the center of the screen
                    if (num_pixel >= can_width) 
                        start += (num_pixel * 0.5 / 2) * ppp;
                    ppp *= 0.5;
                    break;
                case 'o':
                    if (ppp >= INT_MAX / 2)
                        break;
                    ppp *= 2;
                    start -= (can_width * 0.5 / 2) * ppp;
                    start = std::max(start, 0);
                    break;
                case 'l':   // pan right
                    if (start > INT_MAX - pan_step * ppp)
                        break;
                    start += pan_step * ppp;
                    break;
                case 'h':
                    start -= pan_step * ppp;
                    start = std::max(start, 0);
                    break;
                case 'I':   // zoom vertically
                    vscale *= 1.2;
                    break;
                case 'O':
                    vscale /= 1.2;
                    break;
                case 'p':   // pause
                    doread = !doread;
                    break;
                default:
                    break;
            }
        }

        //uint64_t duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        //mvwprintw(win, win_height-1, 0, "ppp=%d, wxh=%dx%d, num_pixel=%d, duration=%ldms, data=%.2fMB, start=%u", ppp, can_width, can_height, num_pixel, duration, num_pixel * ppp * 1.0 / (1<<20), start);
        
        usleep(20e3); // 20ms
        continue;
    }

    //th1.join();
    th2.join();

    endwin();
    return 0;
}
