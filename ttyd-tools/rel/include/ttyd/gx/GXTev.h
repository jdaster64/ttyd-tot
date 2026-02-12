#pragma once

#include <ttyd/gx/GXCommon.h>

#include <cstdint>

namespace ttyd::gx::GXTev {

extern "C" {

void GXSetTevOp(GXTevStageID stage, GXTevMode mode);
void GXSetTevColorIn(
    GXTevStageID stage_id, GXTevColorArg cc1, GXTevColorArg cc2,
    GXTevColorArg cc3, GXTevColorArg cc4);
void GXSetTevAlphaIn(
    GXTevStageID stage_id, GXTevAlphaArg ca1, GXTevAlphaArg ca2,
    GXTevAlphaArg ca3, GXTevAlphaArg ca4);
void GXSetTevColorOp(
    int32_t unk0, int32_t unk1, int32_t unk2,
    int32_t unk3, int32_t unk4, int32_t unk5);
void GXSetTevAlphaOp(
    int32_t unk0, int32_t unk1, int32_t unk2,
    int32_t unk3, int32_t unk4, int32_t unk5);
// GXSetTevColor
// GXSetTevColorS10
// GXSetTevKColor
// GXSetTevKColorSel
// GXSetTevKAlphaSel
// GXSetTevSwapMode
// GXSetTevSwapModeTable
// GXSetAlphaCompare
// GXSetZTexture
void GXSetTevOrder(
    GXTevStageID stage, GXTexCoordID coord, GXTexMapID map, GXChannelID chan);
void GXSetNumTevStages(int32_t num);

}

}