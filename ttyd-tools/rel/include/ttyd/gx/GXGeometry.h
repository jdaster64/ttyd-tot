#pragma once

#include <ttyd/gx/GXCommon.h>

#include <cstdint>

namespace ttyd::gx::GXGeometry {

extern "C" {
    
// __GXSetDirtyState
void GXBegin(GXPrimitive type, GXVtxFmt vtxfmt, uint16_t num_verts);
// __GXSendFlushPrim
// GXSetLineWidth
// GXSetPointSize
// GXEnableTexOffsets
void GXSetCullMode(GXCullMode mode);
// GXGetCullMode
// GXSetCoPlanar
// __GXSetGenMode

}

#define FIFO_PUTU8(x)	*(volatile uint8_t*)GXFIFO   = (uint8_t)(x)
#define FIFO_PUTS8(x)	*(volatile int8_t*)GXFIFO    = (int8_t)(x)
#define FIFO_PUTU16(x)	*(volatile uint16_t*)GXFIFO  = (uint16_t)(x)
#define FIFO_PUTS16(x)	*(volatile int16_t*)GXFIFO   = (int16_t)(x)
#define FIFO_PUTU32(x)	*(volatile uint32_t*)GXFIFO  = (uint32_t)(x)
#define FIFO_PUTS32(x)	*(volatile int32_t*)GXFIFO   = (int32_t)(x)
#define FIFO_PUTF32(x)	*(volatile float*)GXFIFO     = (float)(x)

static inline void GX_Position3f32(float x, float y, float z) {
	FIFO_PUTF32(x);
	FIFO_PUTF32(y);
	FIFO_PUTF32(z);
}

static inline void GXEnd() {}

}