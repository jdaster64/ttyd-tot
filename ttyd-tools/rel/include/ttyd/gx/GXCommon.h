#pragma once

#include <cstdint>

namespace ttyd::gx {
    
enum GXDisplayListOp {
    GX_NOP = 0,
    GX_LOAD_BP_REG = 0x61,
    
    // Last three bits should be bitwise-OR'd with GXVtxFmt.
    GX_DRAW_QUADS = 0x80,
    GX_DRAW_TRIANGLES = 0x90,
    GX_DRAW_TRIANGLE_STRIP = 0x98,
    GX_DRAW_TRIANGLE_FAN = 0xa0,
    GX_DRAW_LINES = 0xa8,
    GX_DRAW_LINE_STRIP = 0xb0,
    GX_DRAW_POINTS = 0xb8,
};

enum GXPrimitive {
    GX_QUADS            = 0x80,
    GX_TRIANGLES        = 0x90,
    GX_TRIANGLESTRIP    = 0x98,
    GX_TRIANGLEFAN      = 0xa0,
    GX_LINES            = 0xa8,
    GX_LINESTRIP        = 0xb0,
    GX_POINTS           = 0xb8,
};
    
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
    
enum GXColorSrc {
    GX_SRC_REG = 0,
    GX_SRC_VTX,
};
    
enum GXLightId {
    GX_LIGHT0       = 0x001,
    GX_LIGHT1       = 0x002,
    GX_LIGHT2       = 0x004,
    GX_LIGHT3       = 0x008,
    GX_LIGHT4       = 0x010,
    GX_LIGHT5       = 0x020,
    GX_LIGHT6       = 0x040,
    GX_LIGHT7       = 0x080,
    GX_MAXLIGHT     = 0x100,
    GX_LIGHTNULL    = 0x000,
};
    
enum GXDiffuseFn {
    GX_DF_NONE = 0,
    GX_DF_SIGNED,
    GX_DF_CLAMP,
};
    
enum GXAttnFn {
    GX_AF_SPEC = 0,
    GX_AF_SPOT,
    GX_AF_NONE,
};

enum GXCullMode {
    GX_CULL_NONE = 0,
    GX_CULL_FRONT,
    GX_CULL_BACK,
    GX_CULL_ALL,
};
    
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

enum GXTevAlphaArg {
    GX_CA_APREV = 0,
    GX_CA_A0,
    GX_CA_A1,
    GX_CA_A2,
    GX_CA_TEXA,
    GX_CA_RAS,
    GX_CA_KONST,
    GX_CA_ZERO,
};

enum GXTevColorArg {
    GX_CC_CPREV = 0,
    GX_CC_APREV,
    GX_CC_C0,
    GX_CC_A0,
    GX_CC_C1,
    GX_CC_A1,
    GX_CC_C2,
    GX_CC_A2,
    GX_CC_TEXC,
    GX_CC_TEXA,
    GX_CC_RASC,
    GX_CC_RASA,
    GX_CC_ONE,
    GX_CC_HALF,
    GX_CC_KONST,
    GX_CC_ZERO,
};

// Address used as volatile buffer used for graphics operations.
constexpr const uint32_t GXFIFO = 0xcc008000U;

}  // namespace