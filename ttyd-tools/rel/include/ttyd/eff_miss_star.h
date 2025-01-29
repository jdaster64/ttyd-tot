#pragma once

#include <cstdint>

namespace ttyd::effdrv {

struct EffEntry;

}

namespace ttyd::eff_miss_star {

extern "C" {

// effMissStarDisp
// effMissStarMain
effdrv::EffEntry* effMissStarEntry(
    double x, double y, double z, int32_t unk0, int32_t unk1, int32_t unk2);

}

}