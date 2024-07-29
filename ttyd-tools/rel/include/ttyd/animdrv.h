#pragma once

#include <cstdint>

namespace ttyd::animdrv {

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
// animPoseGetAnimDataPtr
// animPoseGetAnimPosePtr
// animGroupBaseAsync
// animSetPaperTexMtx
// animPoseDisp_MakeExtTexture
// animPaperPoseDispSub
// animPaperPoseDisp
// animPoseAutoRelease
// animPaperPoseRelease
void animPoseRelease(int32_t animpose_idx);
// animSetPaperTexObj
// animPoseDrawMtx
// _animPoseDrawMtx
// animPoseDraw
// dispProc
// renderProc
// materialProc
// animSetMaterial_Texture
// pushGXModelMtx_JointNode__
// pushGXModelMtx_TransformNode__
// animPoseMain
// animPoseGetMaterialEvtColor
// animPoseGetMaterialLightFlag
// animPoseGetMaterialFlag
// animPoseSetMaterialEvtColor
// animPoseSetMaterialLightFlagOff
// animPoseSetMaterialLightFlagOn
// animPoseSetMaterialFlagOff
// animPoseSetMaterialFlagOn
// animPoseGetPeraEnd
// animPoseGetLoopTimes
// animPoseGetHeight
// animPoseGetRadius
// animPoseSetGXFunc
// animPoseSetEffectAnim
// animPoseSetEffect
// animPoseSetPaperAnim
// animPoseSetPaperAnimGroup
// animPaperPoseGetId
// animPoseSetAnim
// animPoseSetStartTime
// animPoseSetLocalTime
// animPoseSetLocalTimeRate
// animPosePaperPeraOff
// animPosePaperPeraOn
// animPosePeraOff
// animPosePeraOn
// animEffectAsync
// animPaperPoseEntry
// animPoseEntry
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