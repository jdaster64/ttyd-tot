#pragma once

#include <gc/types.h>
#include <ttyd/dispdrv.h>

#include <cstdint>

namespace ttyd::camdrv {
    
struct Camera {
    uint16_t    flags;
    int16_t     mode;
    int8_t      unk_0x004[8];
    gc::vec3    position;
    gc::vec3    target;
    gc::vec3    up;
    float       near_z;
    float       far_z;
    float       fov_y;
    float       aspect;
    int8_t      unk_0x040[0xdc];
    gc::mtx34   view_mtx;
    float       bank_rotation;
    gc::vec3    post_translation;
    gc::mtx44   projection_mtx;
    int32_t     projection_type;
    int8_t      unk_0x1a0[0xc0];
} __attribute__((__packed__));

static_assert(sizeof(Camera) == 0x260);

extern "C" {

// camLetterBox
// camLetterBoxDraw
// getScreenPoint
// camSetTypeOrtho
// camSetTypePersp
// camCtrlOff
// camCtrlOn
// L_camDispOff
// L_camDispOn
// camSetMode
// camEffMain
// cam3dImgMain
// cam3dMain
// camSetCurNo
// camGetCurNo
// camGetCurPtr
Camera* camGetPtr(ttyd::dispdrv::CameraId cam_id);
// camUnLoadRoad
// camLoadRoad
// camDraw
// camEvalNearFar
// camMain
// camEntryPersp
// camInit

}

}