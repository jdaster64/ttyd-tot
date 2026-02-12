#pragma once

#include <cstdint>

namespace ttyd::effdrv {

struct EffEntry;

}

namespace ttyd::eff_fire {

extern "C" {

// effFireSmokeDisp
// effFireSmokeMain
// effFireDisp3
// effFireDisp2
// effFireDisp
// effFireMain

effdrv::EffEntry* effFireEntry(
    double x, double y, double z, double unk0, int32_t unk1, int32_t unk2);

}

}