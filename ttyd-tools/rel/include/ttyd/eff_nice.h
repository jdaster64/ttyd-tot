#pragma once

#include <cstdint>

namespace ttyd::effdrv {

struct EffEntry;

}

namespace ttyd::eff_nice {

extern "C" {

// effAcrobatDisp
// acrobatMain
// effNiceDisp
// effNiceMain
effdrv::EffEntry* effNiceEntry(double x, double y, double z, int32_t type);
// rendermodeFunc
// effNiceAsync

}

}