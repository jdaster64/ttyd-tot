#pragma once

#include <cstdint>

namespace ttyd::effdrv {

struct EffEntry;

}

namespace ttyd::eff_scanning {

extern "C" {

// effScanningDisp
// effScanningMain
effdrv::EffEntry* effScanningEntry(double x, double y, double z, int unk0);

}

}