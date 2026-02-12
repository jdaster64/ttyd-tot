#pragma once

#include <cstdint>

#include <ttyd/gx/GXCommon.h>

namespace ttyd::gx::GXLight {

extern "C" {

// GXInitLightAttn
// GXInitLightSpot
// GXInitLightDistAttn
// GXInitLightPos
// GXInitLightDir
// GXInitLightColor
// GXLoadLightObjImm
void GXSetChanAmbColor(int32_t channel, uint32_t* color);
void GXSetChanMatColor(int32_t channel, uint32_t* color);
void GXSetNumChans(int32_t unk0);
void GXSetChanCtrl(
    int32_t channel, int32_t enable, GXColorSrc amb_src, GXColorSrc mat_src, 
    GXLightId light_mask, GXDiffuseFn diff_fn, GXAttnFn attn_fn);

}

}