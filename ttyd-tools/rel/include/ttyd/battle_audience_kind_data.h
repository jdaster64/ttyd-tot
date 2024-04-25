#pragma once

#include <cstdint>

namespace ttyd::battle_audience_kind_data {

struct AudienceKindData {
    uint8_t     large;      // takes up 2 audience slots
    uint8_t     pad_0x01[3];
    void*       anim_data;
    void*       held_item_data;
    float       item_offset_x;
    float       item_offset_y;
    float       attack_sp_multiplier;
    float       disp_scale_x;
    float       disp_scale_y;
};

static_assert(sizeof(AudienceKindData) == 0x20);

extern "C" {

// .text
// none

// .data
// ...
extern AudienceKindData audience_kind[13];

}

}