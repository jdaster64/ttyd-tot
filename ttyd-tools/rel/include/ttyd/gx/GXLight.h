#pragma once

#include <cstdint>

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
// GXSetChanCtrl

}

}