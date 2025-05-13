#pragma once

#include <cstdint>

namespace ttyd::effdrv {

struct EffEntry;

}

namespace ttyd::eff_stardust {

extern "C" {

effdrv::EffEntry* effStardustEntry(
    double x, double y, double z, double unk0, double unk1,
    int32_t unk2, int32_t unk3, int32_t unk4);

}

}