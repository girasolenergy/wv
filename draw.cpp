#include "draw.h"

Draw::Draw(Canvas *canvas, Track *track) {
    this->canvas = canvas;
    this->track = track;
    start = 0;
    ppp = 4096*8;
    vscale = 1;
    num_pixel = 0;
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
