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
#include "termbox/termbox.h"
#include "draw.h"
#include "argparse.hpp"


void read_wav(FILE *fd, Draw *draw, Track *track, uint32_t buf_len, bool &doread, int bit) {
    while (true) {
        uint8_t *buf;
        int ret;
        if (bit == 16) {
            int16_t *tmp = (int16_t *)calloc(buf_len, sizeof(int16_t));
            ret = fread(tmp, 2, buf_len, fd);
            buf = (uint8_t *)tmp;
            for (int i = 0; i < buf_len; i++) {
                int8_t ch = ((tmp[i] / 256) & 0xff);
                buf[i] = ch + 128;
            }
        } else if (bit == 8) {
            buf = (uint8_t *)calloc(buf_len, sizeof(uint8_t));
            ret = fread(buf, 1, buf_len, fd);
        }

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

void read_key(Draw *draw, bool &doread) {
    while (true) {
        struct tb_event ev;
        int ret = tb_poll_event(&ev);
        if (ret == TB_EVENT_KEY) {
            uint32_t key = ev.ch;
 
            switch (key) {
                case 'q':
                    tb_shutdown();
                    exit(0);
                    break;
                case 'i':
                    draw->zoomx(1);
                    break;
                case 'o':
                    draw->zoomx(-1);
                    break;
                case 'l':   // pan right
                    draw->pan(1);
                    break;
                case 'h':
                    draw->pan(-1);
                    break;
                case 'I':   // zoom vertically
                    draw->zoomy(1);
                    break;
                case 'O':
                    draw->zoomy(-1);
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
    argparse::ArgumentParser arg("wave view");
    arg.add_argument("-b", "--bit")
        .default_value(16)
        .action([](const std::string& value) {return std::stoi(value); })
        .help("set data as 8 bit or 16 bit");
    arg.add_argument("fname")
        .required()
        .help("file name");

    if (argc == 1) {
        std::cout << arg;
        return 0;
    }

    try {
        arg.parse_args(argc, argv);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::cout << arg;
        return 1;
    }

    std::string fname = arg.get<std::string>("fname");
    int bit = arg.get<int>("--bit");

    if (bit != 8 && bit != 16) {
        std::cerr << "Invalid bit depth: " << bit << ". Use 8 or 16." << std::endl;
        return 1;
    }

    tb_init();
    tb_select_input_mode(TB_INPUT_CURRENT);
    int win_w = tb_width();
    int win_h = tb_height();
    FILE *fd;
    if (fname == "-") {
        fd = stdin;
    } else {
        fd = fopen((char *)fname.c_str(), "rb");
    }
    if (fd == NULL) exit(-1);

    Track track(65536);
	Canvas canvas(win_w * 2, win_h * 4);
    Draw draw(&canvas, &track);

    bool doread = true;
    uint32_t buf_len = 1<<14;    // trade off between effiency and wave update rate
    std::thread th1(read_wav, fd, &draw, &track, buf_len, std::ref(doread), bit);
    std::thread th2(read_key, &draw, std::ref(doread));
    
	while (true) {
        sleep(1);
    }

    th1.join();
    th2.join();

    return 0;
}
