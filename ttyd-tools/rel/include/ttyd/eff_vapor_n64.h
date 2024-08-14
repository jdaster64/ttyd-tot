#pragma once

#include <cstdint>

namespace ttyd::effdrv {

struct EffEntry;

}

namespace ttyd::eff_vapor_n64 {

extern "C" {

// effVaporDisp
// effVaporMain
effdrv::EffEntry* effVaporN64Entry(
    double x, double y, double z, double unk0, int32_t unk1, int32_t unk2);

}

}