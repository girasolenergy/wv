#ifndef __DRAW_H__
#define __DRAW_H__

#include <mutex>
#include "canvas.h"
#include "track.h"
#include "termbox/termbox.h"

class Draw {
    public:
        Draw(Canvas *canvas, Track *track);
        void dodraw(void);
        void pan(int);
        void zoomx(int);
        void zoomy(int);
        Canvas *canvas;
        Track *track;
        uint32_t start, ppp, pan_step;
        float vscale;
        uint32_t num_pixel;
        uint8_t *min;
        uint8_t *max;
        std::mutex mutex;
};


#endif
