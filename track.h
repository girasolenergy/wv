#ifndef __TRACK_H__
#define __TRACK_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <math.h>
#include <vector>
#include <algorithm>
#include <mutex>
#include "block.h"


class Track {
    public:
        Track(uint32_t num_block=65536);  // reserved vector size
        int append_block(Block *block);
        uint32_t get_len(void);
        int get_blkadd_from_idx(uint32_t idx, uint32_t &blk_idx, uint32_t &blk_sample_idx);
        Block* block_advance(uint32_t &blk_idx, uint32_t &blk_sample_idx, uint32_t num_sample);
        void get_range_minmax(uint32_t &blk_idx, uint32_t &blk_sample_idx, uint32_t num_sample, uint8_t &min, uint8_t &max);
        uint32_t get_disp_data(uint32_t start_sample_idx, uint32_t sample_per_pixel, uint32_t num_pixel, uint8_t *min, uint8_t *max);

        std::vector<Block *> blocks;
        std::mutex mutex;
        uint32_t len;
};

#endif
