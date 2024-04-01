#pragma once

#include <cstdint>

namespace ttyd::eff_updown {

extern "C" {

// .text
// effUpdownDisp
void polygon(
    double unk0, double unk1, double unk2, double unk3, double unk4,
    double unk5, int32_t unk6, uint32_t unk7);
// effUpdownMain
void* effUpdownEntry(
    float x, float y, float z, int32_t type, int32_t strength, int32_t unk0);

// .data
extern uint16_t icon_id[10];

}

}