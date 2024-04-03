#pragma once

#include <gc/types.h>

#include <cstdint>

namespace ttyd::extdrv {

struct ExtEntryData {
    const char* model_name;
    const char* pose_name;
    float       f;              // Frame count / frame number?
} ;

static_assert(sizeof(ExtEntryData) == 0xc);

struct ExtPoseWork {
    uint32_t    flags;
    float       facing_dir;
    int8_t      unk_0x08[4];
    gc::mtx34*  mtx;
    int32_t     anim_frame;
    int8_t      unk_0x14[12];
} ;

static_assert(sizeof(ExtPoseWork) == 0x20);

extern "C" {

void extDrawShadow();
void extLoadShadowMtx(gc::mtx34* mtx);
void extLoadShadowRenderMode();
void extLoadShadowTev();
void extLoadShadowVertex();
void extLoadTev();
void extLoadTextureExit();
void extLoadTexture();
void extLoadVertex();
void extLoadRenderMode();
void extDraw();
void extPoseDraw(int32_t pose);
int32_t extGetPoseNum();
ExtPoseWork* extGetPosePtr();
// int32_t compare(void* lhs, void* rhs);
void extMain();
void extReset();
// extMakeTexture
void extEntry(int32_t count, ExtEntryData* data, 
              void* init_func, void* main_func, void* disp_func);
void extInit();

}

}