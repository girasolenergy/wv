#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include <chrono>
#include <queue>
#include <thread>
#include "canvas.h"
#include "block.h"
#include "track.h"
#include "termbox.h"
#include "draw.h"


void read_wav(FILE *fd, Draw *draw, Track *track, uint32_t buf_len, bool &doread) {
    while (true) {
        uint8_t *buf = (uint8_t *)calloc(buf_len, sizeof(uint8_t));
        int ret = fread(buf, 1, buf_len, fd);
        if (doread) {
            Block *block = new Block(buf, ret);
            track->append_block(block);
            draw->dodraw();
        }
        if (ret < buf_len) {
            break;
        }
    }
    
}

void handle_event(Draw *draw, bool &doread) {
    Canvas *canvas = draw->canvas;
    Track *track = draw->track;
    while (true) {
        struct tb_event ev;
        int ret = tb_poll_event(&ev);

        if (ret == TB_EVENT_KEY) {
            uint32_t key = ev.ch;
            if (key == 'q') {
                tb_shutdown();
                exit(0);
            }
 
            uint32_t pan_step = canvas->width * 0.1;
            switch (key) {
                case 'i':
                    if (draw->ppp / 2 < 1)
                        break;
                    // if waveform is less than one screen, the zoom center is not the center of the screen
                    if (draw->num_pixel >= canvas->width) 
                        draw->start += (draw->num_pixel * 0.5 / 2) * draw->ppp;
                    draw->ppp *= 0.5;
                    break;
                case 'o':
                    if (draw->ppp >= INT_MAX / 2)
                        break;
                    draw->ppp *= 2;
                    draw->start -= (canvas->width * 0.5 / 2) * draw->ppp;
                    draw->start = std::max(draw->start, (uint32_t)0);
                    break;
                case 'l':   // pan right
                    if (draw->start > INT_MAX - pan_step * draw->ppp)
                        break;
                    draw->start += pan_step * draw->ppp;
                    break;
                case 'h':
                    draw->start -= pan_step * draw->ppp;
                    draw->start = std::max(draw->start, (uint32_t)0);
                    break;
                case 'I':   // zoom vertically
                    draw->vscale *= 1.2;
                    break;
                case 'O':
                    draw->vscale /= 1.2;
                    break;
                case 'p':   // pause
                    doread = !doread;
                    break;
                default:
                    break;
            }
            draw->dodraw();
        }
    }
}


int main(int argc, char **argv) {
    setbuf(stdout, NULL);
    int ret = tb_init();
    tb_select_input_mode(TB_INPUT_CURRENT);
    int win_w, win_h;
    win_w = tb_width();
    win_h = tb_height();

    if (argc == 1) {
        tb_shutdown();
        return 0;
    }
	FILE *fd = fopen(argv[1], "rb");
    if (fd == NULL) exit(-1);

    Track track(65536);
    int can_width = win_w * 2;
    int can_height = win_h * 4;
	Canvas canvas(can_width, can_height);
    Draw draw(&canvas, &track);

    bool doread = true;
    uint32_t buf_len = 1<<17;    // trade off between effiency and wave update rate
    std::thread th1(read_wav, fd, &draw, &track, buf_len, std::ref(doread));
    std::thread th2(handle_event, &draw, std::ref(doread));
    
	while (true) {
        sleep(1);
    }

    th1.join();
    th2.join();

    return 0;
}
