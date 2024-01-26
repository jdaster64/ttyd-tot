#include "mod_gfxtest.h"

#include <ttyd/gx/GXAttr.h>
#include <ttyd/gx/GXDisplayList.h>
#include <ttyd/gx/GXLight.h>
#include <ttyd/gx/GXPixel.h>
#include <ttyd/gx/GXTev.h>
#include <ttyd/gx/GXTransform.h>
#include <ttyd/camdrv.h>
#include <ttyd/dispdrv.h>

#include <cinttypes>

namespace mod::infinite_pit {

namespace {
    
// Include everything for convenience.
using namespace ::ttyd::gx::GXAttr;
using namespace ::ttyd::gx::GXDisplayList;
using namespace ::ttyd::gx::GXLight;
using namespace ::ttyd::gx::GXPixel;
using namespace ::ttyd::gx::GXTev;
using namespace ::ttyd::gx::GXTransform;

using ::ttyd::dispdrv::CameraId;
    
}

void GfxTestManager::Update() {}

void GfxTestManager::Draw() {
    // Test drawing a few squares.
    
    static const float kPositions[] = {
        0.0,    0.0,    0.0,
        10.0,   0.0,    0.0,
        10.0,   10.0,   0.0,
        0.0,    10.0,   0.0,
        
        20.0,   20.0,   0.0,
        20.0,   40.0,   0.0,
        40.0,   40.0,   0.0,
        40.0,   20.0,   0.0,
        
        -90.0,  -90.0,  0.0,
        -90.0,  -5.0,   0.0,
        -5.0,   -5.0,   0.0,
        -5.0,   -90.0,  0.0,
    };
    static const uint8_t kColors[] = {
        0xff, 0x00, 0x00, 0xff,
        0x00, 0xff, 0x00, 0x80,
        0x00, 0x00, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff,
    };
    
    static const uint8_t kDisplayList[] = {
        GX_DRAW_QUADS | GX_VTXFMT1, 0x00, 0x0c,  // quads, 12 vertices
        0, 0,   3, 0,   2, 0,   1, 0,
        4, 2,   7, 2,   6, 2,   5, 2,
        8, 0,   11, 1,  10, 2,  9, 3,
        // pad to length 32
        0,0,0,0,0,
    };
    static_assert(sizeof(kDisplayList) == 32);
    
    GXSetNumChans(0);  // = 1 causes weird blending, 0 disables alpha
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    
    GXSetBlendMode(0,1,0,0);
    GXSetZCompLoc(0);
    GXSetZMode(0,7,0);
    
    GXLoadPosMtxImm(&ttyd::camdrv::camGetPtr(CameraId::k2d)->view_mtx, 0);

    GXClearVtxDesc();
    GXSetVtxDesc( GX_VA_POS, GX_INDEX8 );
    GXSetVtxDesc( GX_VA_CLR0, GX_INDEX8 );
    GXSetVtxAttrFmt( GX_VTXFMT1, GX_VA_POS, GX_POS_XYZ, GX_F32, 0 );
    GXSetVtxAttrFmt( GX_VTXFMT1, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0 );
    GXSetArray( GX_VA_POS, &kPositions, sizeof(float)*3 );
    GXSetArray( GX_VA_CLR0, &kColors, sizeof(uint8_t)*4 );
    GXCallDisplayList( &kDisplayList, sizeof(kDisplayList) );
}

}