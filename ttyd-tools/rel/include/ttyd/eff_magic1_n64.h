#pragma once

#include <cstdint>

namespace ttyd::effdrv {

struct EffEntry;

}

namespace ttyd::eff_magic1_n64 {

extern "C" {

// effMagic1Disp
// effMagic1_main_dl
// effMagic1Main
effdrv::EffEntry* effMagic1N64Entry(
    double x, double y, double z, double unk0, double unk1, double unk2,
    int32_t unk3, uint32_t unk4);

}

}