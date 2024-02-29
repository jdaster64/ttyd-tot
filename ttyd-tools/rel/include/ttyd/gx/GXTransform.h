#pragma once

#include <gc/types.h>

#include <cstdint>

namespace ttyd::gx::GXTransform {

extern "C" {
    
// GXProject
// GXSetProjection
// GXSetProjectionv
// GXGetProjectionv
void GXLoadPosMtxImm(gc::mtx34* mtx, int32_t unk0);
// GXLoadNrmMtxImm
// GXSetCurrentMtx
// GXLoadTexMtxImm
// __GXSetViewport
// GXSetViewportJitter
// GXSetViewport
// GXGetViewportv
// GXSetZScaleOffset
void GXSetScissor(int32_t x, int32_t y, int32_t width, int32_t height);
void GXGetScissor(int32_t* x, int32_t* y, int32_t* width, int32_t* height);
// GXSetScissorBoxOffset
// GXSetClipMode
// __GXSetMatrixIndex

}

}