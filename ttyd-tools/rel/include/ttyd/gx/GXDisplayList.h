#pragma once

#include <ttyd/gx/GXCommon.h>

#include <cstdint>

namespace ttyd::gx::GXDisplayList {

extern "C" {
    
// GXBeginDisplayList
// GXEndDisplayList
void GXCallDisplayList(const void* data, int32_t size);

}

}