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

Track::Track(uint32_t num_block) {
    blocks.reserve(num_block);
    len = 0;
}

int Track::append_block(Block *block) {
    mutex.lock();
    len += block->len;
    blocks.push_back(block);
    mutex.unlock();
    
    return 0;
}

uint32_t Track::get_len(void) {
    //uint32_t num = 0;
    //for (uint32_t i = 0; i < blocks.size(); i++) {
    //    num += blocks[i]->len;
    //}
    //return num;
    mutex.lock();
    uint32_t ret = len;
    mutex.unlock();
    return ret;
}

int Track::get_blkadd_from_idx(uint32_t idx, uint32_t &blk_idx, uint32_t &blk_sample_idx) {
    if (idx >= get_len())
        return -1;
    blk_idx = 0;
    while (idx >= blocks[blk_idx]->len) {
        idx -= blocks[blk_idx]->len;
        blk_idx++;
    }
    blk_sample_idx = idx;
    return 0;
}

Block* Track::block_advance(uint32_t &blk_idx, uint32_t &blk_sample_idx, uint32_t num_sample) {
    Block *blk = blocks[blk_idx];
    while (num_sample > 0) {
        if (blk_sample_idx + num_sample < blk->len) {
            blk_sample_idx += num_sample;
            break;
        } else {
            num_sample -= blk->len - blk_sample_idx;
            blk_idx++;
            if (blk_idx >= blocks.size())
                return NULL;
            blk = blocks[blk_idx];
            blk_sample_idx = 0;
        }
    }
    return blk;
}

void Track::get_range_minmax(uint32_t &blk_idx, uint32_t &blk_sample_idx, uint32_t num_sample, uint8_t &min, uint8_t &max) {
    uint8_t _min = 255;
    uint8_t _max = 0;
    Block *blk = blocks[blk_idx];
    uint32_t _mask = blk->bin_size - 1;
    // ahead of bin boundary, iterate slowly
    uint32_t num_slow_sample = (blk->bin_size - blk_sample_idx) & _mask;
    num_slow_sample = std::min(num_slow_sample, num_sample);
    num_slow_sample = std::min(num_slow_sample, blk->len - blk_sample_idx); // this is less likely to happen? no!
    for (int i = blk_sample_idx; i < blk_sample_idx + num_slow_sample; i++) {
        _min = std::min(_min, blk->buff[i]);
        _max = std::max(_max, blk->buff[i]);
    }
    num_sample -= num_slow_sample;
    blk = block_advance(blk_idx, blk_sample_idx, num_slow_sample);
    // fast lookup
    while (blk && num_sample > blk->bin_size) {
        uint32_t num_bin = num_sample / blk->bin_size;
        uint32_t num_bin_blk = ceil((float)blk->len / blk->bin_size);
        uint32_t bin_idx = blk_sample_idx / blk->bin_size;
        uint32_t num_bin_lft = num_bin_blk -  bin_idx;
        num_bin = std::min(num_bin, num_bin_lft);
        for (int i = bin_idx; i < bin_idx + num_bin; i++) {
            _min = std::min(_min, blk->min[i]);
            _max = std::max(_max, blk->max[i]);
        }
        uint32_t num_fast_sample = num_bin * blk->bin_size;
        num_sample -= num_fast_sample;
        blk = block_advance(blk_idx, blk_sample_idx, num_fast_sample);
    }
    // behind bin boundary, iterate slowly
    while (blk && num_sample > 0) {
        uint32_t num_sample_blk = std::min(num_sample, blk->len - blk_sample_idx);
        for (int i = blk_sample_idx; i < blk_sample_idx + num_sample_blk; i++) {
            _min = std::min(_min, blk->buff[i]);
            _max = std::max(_max, blk->buff[i]);
        }
        num_sample -= num_sample_blk;
        blk = block_advance(blk_idx, blk_sample_idx, num_sample_blk);
    }
    min = _min;
    max = _max;
}

uint32_t Track::get_disp_data(uint32_t start_sample_idx, uint32_t sample_per_pixel, uint32_t num_pixel, uint8_t *min, uint8_t *max) {
    uint32_t blk_idx, blk_sample_idx;
    int ret = get_blkadd_from_idx(start_sample_idx, blk_idx, blk_sample_idx);
    if (ret == -1)
        return 0;
    int x;
    for (x = 0; x < num_pixel; x++) {
        if (blk_idx >= blocks.size())
            break;
        
        get_range_minmax(blk_idx, blk_sample_idx, sample_per_pixel, min[x], max[x]);
        
    }
    return x;
}

#endif
