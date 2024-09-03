#pragma once

#include "seqdrv.h"

#include <cstdint>

namespace ttyd::seq_load {

struct SeqLoadWinDataEntry {
    float x;
    float y;
    float cursor_x_offset;
    float cursor_y_offset;
    uint32_t texture_idx_0;
    uint32_t texture_idx_1;
    int8_t neighbors[4];    // adjacent win data entries up, down, left, right.
};
static_assert(sizeof(SeqLoadWinDataEntry) == 0x1c);

extern "C" {

// continueGame
// loadDraw
// loadMain
// seq_loadMain
// seq_loadExit
// seq_loadInit
// unk_800f72e4

}

}