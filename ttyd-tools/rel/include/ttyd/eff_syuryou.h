#pragma once

#include <cstdint>

namespace ttyd::effdrv {

struct EffEntry;

}

namespace ttyd::eff_syuryou {

extern "C" {

// effSyuryouDisp
// effSyuryouMain
effdrv::EffEntry* effSyuryouEntry(
    double x, double y, double z, int32_t unk1, int32_t unk2);

}

}