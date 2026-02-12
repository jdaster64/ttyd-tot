#pragma once

#include <cstdint>

namespace ttyd::effdrv {

struct EffEntry;

}

namespace ttyd::eff_kemutest {

extern "C" {

// effKemuTestSetRxRz
// effKemuTestDrawCam
// effKemuTestDisp
// effKemuTestMain
effdrv::EffEntry* effKemuTestEntry(
    double x, double y, double z, double size, int32_t unk1);

}

}
