#pragma once

#include <cstdint>

namespace ttyd::gx::GXTev {
    
enum GXTevStageID {
    GX_TEVSTAGE0 = 0,
    GX_TEVSTAGE1,
    GX_TEVSTAGE2,
    GX_TEVSTAGE3,
    GX_TEVSTAGE4,
    GX_TEVSTAGE5,
    GX_TEVSTAGE6,
    GX_TEVSTAGE7,
    GX_TEVSTAGE8,
    GX_TEVSTAGE9,
    GX_TEVSTAGE10,
    GX_TEVSTAGE11,
    GX_TEVSTAGE12,
    GX_TEVSTAGE13,
    GX_TEVSTAGE14,
    GX_TEVSTAGE15,
    GX_MAX_TEVSTAGE,
};

enum GXTexCoordID {
    GX_TEXCOORD0 = 0,
    GX_TEXCOORD1,
    GX_TEXCOORD2,
    GX_TEXCOORD3,
    GX_TEXCOORD4,
    GX_TEXCOORD5,
    GX_TEXCOORD6,
    GX_TEXCOORD7,
    GX_MAX_TEXCOORD,
    GX_TEXCOORD_NULL = 255,
};

enum GXTexMapID {
    GX_TEXMAP0 = 0,
    GX_TEXMAP1,
    GX_TEXMAP2,
    GX_TEXMAP3,
    GX_TEXMAP4,
    GX_TEXMAP5,
    GX_TEXMAP6,
    GX_TEXMAP7,
    GX_MAX_TEXMAP,
    GX_TEXMAP_NULL = 255,
    GX_TEX_DISABLE,
};

enum GXChannelID {
    GX_COLOR0 = 0,
    GX_COLOR1,
    GX_ALPHA0,
    GX_ALPHA1,
    GX_COLOR0A0,
    GX_COLOR1A1,
    GX_COLOR_ZERO,
    GX_ALPHA_BUMP,
    GX_ALPHA_BUMPN,
    GX_COLOR_NULL = 255,
};

enum GXTevMode {
    GX_MODULATE = 0,
    GX_DECAL,
    GX_BLEND,
    GX_REPLACE,
    GX_PASSCLR,
};

extern "C" {

void GXSetTevOp(GXTevStageID stage, GXTevMode mode);
// GXSetTevColorIn
// GXSetTevAlphaIn
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