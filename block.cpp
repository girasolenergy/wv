#include "block.h"

Block::Block(uint8_t *buff, uint32_t len) {
    this->buff = buff;
    this->len = len;
    bin_size = 1 << 10;    // need to be the power of 2
    num_bin = ceil((float)len / bin_size);
    
    min = (uint8_t *)calloc(num_bin, sizeof(uint8_t));
    max = (uint8_t *)calloc(num_bin, sizeof(uint8_t));
    update_minmax();
}

Block::~Block(void) {
    if (buff) {
        free(buff);
    }

    if (min) {
        free(min);
    }

    if (max) {
        free(max);
    }
}

void Block::update_minmax(void) {
    uint8_t *p = buff;
    for (uint32_t i = 0; i < num_bin; i++) {
        uint8_t _min = 255;
        uint8_t _max = 0;
        for (uint32_t j = 0; j < bin_size; j++) {
            _min = std::min(_min, *p);
            _max = std::max(_max, *p);
            p++;
        }
        min[i] = _min;
        max[i] = _max;
    }
}


