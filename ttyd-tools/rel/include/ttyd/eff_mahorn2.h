#pragma once

#include <cstdint>

namespace ttyd::effdrv {

struct EffEntry;

}

namespace ttyd::eff_mahorn2 {

extern "C" {

// effMahorn2Disp
// effMahorn2Main
effdrv::EffEntry* effMahorn2Entry(
    double x, double y, double z, double target_x, double target_y,
    double target_z, double size, double speed, int32_t unk);

}

}