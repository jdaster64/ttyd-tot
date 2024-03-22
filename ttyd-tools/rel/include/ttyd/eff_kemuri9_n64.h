#pragma once

#include <cstdint>

namespace ttyd::effdrv {

struct EffEntry;

}

namespace ttyd::eff_kemuri9_n64 {

extern "C" {

// effKemuri9Disp
// effKemuri9Main
effdrv::EffEntry* effKemuri9N64Entry(
    double x, double y, double z, double unk0, double unk1, int16_t unk2,
    int32_t unk3, int32_t unk4);

}

}