#pragma once

#include <cstdint>

namespace ttyd::gx::GXPixel {

extern "C" {

void GXSetFog(
    uint32_t type, float unk1, float unk2, float unk3, float unk4,
    const uint32_t* color);
// GXSetFogRangeAdj
void GXSetBlendMode(int32_t unk0, int32_t unk1, int32_t unk2, int32_t unk3);
void GXSetColorUpdate(int32_t unk0);
void GXSetAlphaUpdate(int32_t unk0);
void GXSetZMode(int32_t unk0, int32_t unk1, int32_t unk2);
void GXSetZCompLoc(int32_t unk0);
// GXSetPixelFmt
// GXSetDither
// GXSetDstAlpha
// GXSetFieldMask
// GXSetFieldMode

}

}