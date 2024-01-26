#pragma once

#include <cstdint>

namespace ttyd::gx::GXDisplayList {
    
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

extern "C" {
    
// GXBeginDisplayList
// GXEndDisplayList
void GXCallDisplayList(const void* data, int32_t size);

}

}