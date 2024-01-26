#pragma once

#include <cstdint>

namespace ttyd::gx::GXAttr {
    
enum GXAttribute {
    GX_VA_PNMTXIDX = 0,
    GX_VA_TEX0MTXIDX,
    GX_VA_TEX1MTXIDX,
    GX_VA_TEX2MTXIDX,
    GX_VA_TEX3MTXIDX,
    GX_VA_TEX4MTXIDX,
    GX_VA_TEX5MTXIDX,
    GX_VA_TEX6MTXIDX,
    GX_VA_TEX7MTXIDX,
    GX_VA_POS,
    GX_VA_NRM,
    GX_VA_CLR0,
    GX_VA_CLR1,
    GX_VA_TEX0,
    GX_VA_TEX1,
    GX_VA_TEX2,
    GX_VA_TEX3,
    GX_VA_TEX4,
    GX_VA_TEX5,
    GX_VA_TEX6,
    GX_VA_TEX7,
};

enum GXAttributeType {
    GX_NONE = 0,
    GX_DIRECT,
    GX_INDEX8,
    GX_INDEX16,
};

enum GXVtxFmt {
    GX_VTXFMT0 = 0,
    GX_VTXFMT1,
    GX_VTXFMT2,
    GX_VTXFMT3,
    GX_VTXFMT4,
    GX_VTXFMT5,
    GX_VTXFMT6,
    GX_VTXFMT7,
    GX_MAX_VTXFMT,
};

enum GXComponentContents {
    GX_POS_XY = 0,
    GX_POS_XYZ,
    GX_NRM_XYZ = 0,
    GX_NRM_NBT,
    GX_NRM_NBT3,
    GX_CLR_RGB = 0,
    GX_CLR_RGBA,
    GX_TEX_S = 0,
    GX_TEX_ST,
};

enum GXComponentType {
    GX_U8 = 0,
    GX_S8,
    GX_U16,
    GX_S16,
    GX_F32,
    GX_RGB565 = 0,
    GX_RGB8,
    GX_RGBX8,
    GX_RGBA4,
    GX_RGBA6,
    GX_RGBA8,
};

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