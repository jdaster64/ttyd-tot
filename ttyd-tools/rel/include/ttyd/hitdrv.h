#pragma once

#include <gc/types.h>

#include <cstdint>

namespace ttyd::mapdrv { 
struct MapFileJoint;
struct MapObject;
}

namespace ttyd::hitdrv {

struct HitReturnPoint {
    const char* hitobj_name;
    gc::vec3 position;
};

struct HitObj {
	uint16_t flags; //0x0
    uint8_t unk2[0x4 - 0x2]; //0x2
    int32_t attributes; //0x4
    ttyd::mapdrv::MapFileJoint* joint; //0x8
    gc::mtx34 unkC; //0xC
    gc::mtx34 unk3C; //0x3C
    gc::mtx34 unk6C; //0x6C
    gc::vec3 unk9C; //0x9C
    int16_t unkA8; //0xA8
    int16_t mapIndex; //0xAA
    void* unkAC; //0xAC  // HitVector*
    HitReturnPoint* damage; //0xB0
    uint8_t unkB4[0xC0 - 0xB4]; //0xB4
    gc::vec3 unkC0; //0xC0
    float unkCC; //0xCC
    ttyd::mapdrv::MapObject* mapObj; //0xD0
    struct HitObj* parent; //0xD4
    struct HitObj* child; //0xD8
    struct HitObj* next; //0xDC
    struct HitObj* nextActive; //0xE0
};

extern "C" {

// hitBindUpdate
// hitBindMapObj
// hitGetDamageReturnPos
// hitGrpDamageReturnSet
// hitDamageReturnSet
// hitGetAttr
// hitGetName
// hitObjGetNormal
// hitObjGetPos
// hitObjGetPosSub
// hitNameToPtr
// hitCheckSphereFilter
// hitCheckAttr
// chkFilterAttr
// hitCheckVecHitObjXZ
// checkTriVec_xz
// hitCheckFilter
// hitCheckVecFilter
// hitCalcVtxPosition
// hitReCalcMatrix2
// hitReCalcMatrix
// hitGrpAttrOff
// hitGrpAttrOn
// hitAtrOff
// hitAtrOn
// hitObjAttrOff
// hitObjAttrOn
// hitGrpFlagOff
// hitGrpFlagOn
// hitFlgOff
// hitFlgOn
// hitObjFlagOff
// hitObjFlagOn
// hitMain
// hitDelete
// hitEntryMOBJ
// hitEntry
// hitEntrySub
// _hitEnt
// hitReInit
// hitInit

}

}
