#pragma once

#include <cstdint>

namespace ttyd::gx::GXTexture {

extern "C" {
    
// __GXGetTexTileShift
// GXGetTexBufferSize
// __GetImageTileCount
// GXInitTexObj
// GXInitTexObjCI
// GXInitTexObjLOD
// GXInitTexObjData
// GXGetTexObjData
// GXGetTexObjWidth
// GXGetTexObjHeight
// GXGetTexObjFmt
// GXGetTexObjMipMap
// GXLoadTexObjPreLoaded
void GXLoadTexObj(void* tex_obj, int32_t unk0);
// GXInitTlutObj
// GXLoadTlut
// GXInitTexCacheRegion
// GXInitTexPreLoadRegion
// GXInitTlutRegion
// GXInvalidateTexAll
// GXSetTexRegionCallback
// GXSetTlutRegionCallback
// GXPreLoadEntireTexture
// __SetSURegs
// __GXSetSUTexRegs
// __GXSetTmemConfig

}

}