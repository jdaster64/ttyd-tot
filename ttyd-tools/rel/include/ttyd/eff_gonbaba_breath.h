#pragma once

#include <gc/types.h>

#include <cstdint>

namespace ttyd::effdrv {

struct EffEntry;

}

namespace ttyd::eff_gonbaba_breath {
    
struct EffGonbabaBreathWork {
    uint32_t    unk_0x00;
    gc::vec3    position;
    gc::vec3    velocity;
    int32_t     timer;
    int32_t     unk_0x20;
    int32_t     color_1[3];
    int32_t     color_2[3];
    int32_t     alpha;
    float       scale;
    int32_t     unk_0x44;
    float       rotation;
    uint32_t    unk_0x4c;   // EffEntry*?
};

static_assert(sizeof(EffGonbabaBreathWork) == 0x50);

extern "C" {

// effGonbabaBreathDisp
// effGonbabaBreathMain
// effGonbabaBreathEntry
void init_breath(EffGonbabaBreathWork* work, int32_t unk0, int32_t type);

}

}