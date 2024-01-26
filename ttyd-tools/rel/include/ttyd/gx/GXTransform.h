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
// GXSetScissor
// GXGetScissor
// GXSetScissorBoxOffset
// GXSetClipMode
// __GXSetMatrixIndex

}

}