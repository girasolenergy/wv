#ifndef __BLOCK_H__
#define __BLOCK_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <algorithm>


class Block {
    public:
        Block(uint8_t *buff, uint32_t len=65536);
        ~Block(void);
        void update_minmax(void);

        uint8_t *buff;
        uint32_t len;
        uint8_t *max;
        uint8_t *min;
        uint32_t bin_size;
        uint32_t num_bin;

    private:
};

#endif
