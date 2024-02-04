#pragma once

#include <cstdint>

namespace ttyd::effdrv {

struct EffEntry;

}

namespace ttyd::eff_stamp_n64 {

extern "C" {

// effStampDisp
// effStampMain
effdrv::EffEntry* effStampN64Entry(double x, double y, double z, int unkParam0);

}

}