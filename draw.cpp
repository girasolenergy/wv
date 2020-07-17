#include "draw.h"
#include <limits.h>

Draw::Draw(Canvas *canvas, Track *track) {
    this->canvas = canvas;
    this->track = track;
    start = 0;
    ppp = 4096*8;
    vscale = 1;
    num_pixel = 0;
    pan_step = canvas->width * 0.1;
    min = (uint8_t *)calloc(canvas->width, sizeof(uint8_t));
    max = (uint8_t *)calloc(canvas->width, sizeof(uint8_t));
}

void Draw::dodraw(void) {
    mutex.lock();
    tb_clear();
    canvas->clear();
    num_pixel = track->get_disp_data(start, ppp, canvas->width, min, max);
    int height = canvas->height;
    for (int i = 0; i < num_pixel; i++) {
        int _min = (min[i] - 128) * vscale + height / 2;
        int _max = (max[i] - 128) * vscale + height / 2;
        for (int y = _min; y <= _max; y++) { // should use <= here otherwise missing points
            int _y = height - y;
            _y = std::min(_y, height-1);
            _y = std::max(_y, 0);
            canvas->set(i, _y);
        }
	}
	canvas->draw();
    tb_present();
    mutex.unlock();
}

void Draw::pan(int direction) {
    if (direction == 1) {
        if (this->start > INT_MAX - this->pan_step * this->ppp)
            return;
        this->start += this->pan_step * this->ppp;
    } else if (direction == -1) {
        this->start -= this->pan_step * this->ppp;
        this->start = std::max(this->start, (uint32_t)0);
    }
}

void Draw::zoomx(int direction) {
    if (direction == 1) {   // zoom in
        if (ppp / 2 < 1)
            return;
        if (this->num_pixel >= this->canvas->width)
            this->start += (this->num_pixel * 0.5 /2) * this->ppp;
        this->ppp *= 0.5;
    } else if (direction == -1) {   // zoom out
        if (ppp >= INT_MAX / 2)
            return;
        ppp *= 2;
        start -= (canvas->width * 0.5 / 2) * ppp;
        start = std::max(start, (uint32_t)0);
    }
}


void Draw::zoomy(int direction) {
    if (direction == 1)
        vscale *= 1.2;
    else if (direction == -1)
        vscale /= 1.2;
}
