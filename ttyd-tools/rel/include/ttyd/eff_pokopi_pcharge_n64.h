#pragma once

#include <cstdint>

namespace ttyd::effdrv {

struct EffEntry;

}

namespace ttyd::eff_pokopi_pcharge_n64 {

extern "C" {

// effPokopiPchargeDisp
// effPokopiPchargeMain
effdrv::EffEntry* effPokopiPchargeN64Entry(
    double x, double y, double z, double unk0, int32_t unk1, int32_t unk2);

}

}