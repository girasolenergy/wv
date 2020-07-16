#ifndef __CANVAS_H__
#define __CANVAS_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "termbox/termbox.h"


class Canvas {
    public:
        Canvas(uint16_t width, uint16_t height);
        ~Canvas(void);
        void clear(void);
        void set(uint16_t x, uint16_t y);
        void unset(uint16_t x, uint16_t y);
        void draw(void);
        
        uint32_t width, height;
        uint8_t *min, *max;
        
    private:
	    uint32_t    *buff;
        uint32_t    braille = 0x2800;
        uint8_t     pixmap[4][2] = {
                        {0x01, 0x08},
                        {0x02, 0x10},
                        {0x04, 0x20},
                        {0x40, 0x80}
                    };
        
};

#endif
