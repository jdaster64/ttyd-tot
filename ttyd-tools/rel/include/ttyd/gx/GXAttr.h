#pragma once

#include <ttyd/gx/GXCommon.h>

#include <cstdint>

namespace ttyd::gx::GXAttr {

extern "C" {
    
void GXSetVtxDesc(GXAttribute attr, GXAttributeType attr_type);
// GXSetVtxDescv
// __GXSetVCD
// __GXCalculateVLim
// GXGetVtxDesc
// GXGetVtxDescv
void GXClearVtxDesc();
void GXSetVtxAttrFmt(
    GXVtxFmt vtx_fmt, GXAttribute attr, GXComponentContents contents,
    GXComponentType type, uint8_t frac_bits);
// GXSetVtxAttrFmtv
// __GXSetVAT
// GXGetVtxAttrFmt
// GXGetVtxAttrFmtv
void GXSetArray(GXAttribute attr, const void* data, int32_t spread);
// GXInvalidateVtxCache
void GXSetTexCoordGen2(
    int32_t unk0, int32_t unk1, int32_t unk2,
    int32_t unk3, int32_t unk4, int32_t unk5);
void GXSetNumTexGens(int32_t num);

}

}