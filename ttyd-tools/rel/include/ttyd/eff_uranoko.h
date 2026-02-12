#pragma once

#include <cstdint>

namespace ttyd::effdrv {

struct EffEntry;

}

namespace ttyd::eff_uranoko {

extern "C" {

// effUranokoDisp
// effUranokoMain
effdrv::EffEntry* effUranokoEntry(
    double x, double y, double z, int32_t unk0, int32_t unk1);

}

}