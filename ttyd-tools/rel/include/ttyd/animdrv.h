#pragma once

#include <gc/types.h>

#include <cstdint>

namespace ttyd::animdrv {

struct AnimPoseLoopData {
    int32_t is_looping;
    float   start;
    float   end;
};
static_assert(sizeof(AnimPoseLoopData) == 0xc);

struct AnimPoseFrameData {
    float   time;
    int32_t upd_buf_pos_start;
    int32_t upd_buf_pos_num;
    int32_t upd_buf_nrm_start;
    int32_t upd_buf_nrm_num;
    int32_t upd_buf_tex_anim_start;
    int32_t upd_buf_tex_anim_num;
    int32_t upd_buf_grp_visibility_start;
    int32_t upd_buf_grp_visibility_num;
    int32_t upd_buf_node_start;
    int32_t upd_buf_node_num;
};
static_assert(sizeof(AnimPoseFrameData) == 0x2c);

struct AnimPoseData {
    int32_t size;
    int32_t loop_count;
    int32_t frame_count;
    int32_t upd_buf_pos_num;
    int32_t upd_buf_nrm_num;
    int32_t upd_buf_tex_anim_num;
    int32_t upd_buf_grp_visibility_num;
    int32_t upd_buf_node_num;
    int32_t unk_0x20;
    AnimPoseLoopData* loop_data;
    AnimPoseFrameData* frame_data;
    void* upd_buf_pos_data;
    void* upd_buf_nrm_data;
    void* upd_buf_tex_anim_data;
    void* upd_buf_grp_visibility_data;
    void* upd_buf_node_data;
    float* unk_0x40;
};
static_assert(sizeof(AnimPoseData) == 0x44);

extern "C" {

// animPoseGetGroupName
// animPoseGetGroupIdx
// animPoseGetShapeIdx
// animPoseDrawShape
// animPoseSetDispCallBack
// animPoseGetVivianType
// animPoseVivianMain
// animPoseWorldMatrixEvalOn
// L_animPoseWorldPositionEvalOff
// animPoseWorldPositionEvalOn
// evalProc
// animPoseTestXLU
// animPoseGetCurrentAnim
// animPoseGetAnimBaseDataPtr
AnimPoseData* animPoseGetAnimDataPtr(int32_t pose_idx);
// animPoseGetAnimPosePtr
// animGroupBaseAsync
// animSetPaperTexMtx
// animPoseDisp_MakeExtTexture
// animPaperPoseDispSub
// animPaperPoseDisp
// animPoseAutoRelease
// animPaperPoseRelease
void animPoseRelease(int32_t pose_idx);
// animSetPaperTexObj
void animPoseDrawMtx(
    int pose_idx, gc::mtx34* mtx, int disp_mode, double rot, double scale);
// _animPoseDrawMtx
// animPoseDraw
// dispProc
// renderProc
// materialProc
// animSetMaterial_Texture
// pushGXModelMtx_JointNode__
// pushGXModelMtx_TransformNode__
void animPoseMain(int32_t pose_idx);
// animPoseGetMaterialEvtColor
// animPoseGetMaterialLightFlag
// animPoseGetMaterialFlag
// animPoseSetMaterialEvtColor
// animPoseSetMaterialLightFlagOff
// animPoseSetMaterialLightFlagOn
void animPoseSetMaterialFlagOff(int32_t pose_idx, uint32_t flags);
void animPoseSetMaterialFlagOn(int32_t pose_idx, uint32_t flags);
// animPoseGetPeraEnd
double animPoseGetLoopTimes(int32_t pose_idx);
// animPoseGetHeight
// animPoseGetRadius
// animPoseSetGXFunc
// animPoseSetEffectAnim
// animPoseSetEffect
// animPoseSetPaperAnim
// animPoseSetPaperAnimGroup
// animPaperPoseGetId
void animPoseSetAnim(int32_t pose_idx, const char* pose_name, int32_t unk0);
// animPoseSetStartTime
void animPoseSetLocalTime(double frame_count, int32_t pose_idx);
// animPoseSetLocalTimeRate
// animPosePaperPeraOff
// animPosePaperPeraOn
// animPosePeraOff
// animPosePeraOn
// animEffectAsync
// animPaperPoseEntry
int32_t animPoseEntry(const char* model, uint32_t unk0);
// animPoseBattleInit
// animPoseRefresh
// animPose_AllocBuffer
// animMain
// animInit
// testAlloc
// initTestHeap
// animTimeGetTime
// animGetPtr

}

}