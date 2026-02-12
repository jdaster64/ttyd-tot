#pragma once

#include <cstdint>

namespace ttyd::effdrv {

struct EffEntry;

}

namespace ttyd::eff_kumokumo_n64 {

extern "C" {

// effKumokumoDisp
// effKumokumoMain
effdrv::EffEntry* effKumokumoN64Entry(
    double x, double y, double z, double unk0, double unk1, double unk2,
    int32_t unk3, int32_t unk4);

}

}