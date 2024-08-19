#pragma once

#include <gc/types.h>

#include <cstdint>

namespace ttyd::hitdrv {

struct HitReturnPoint {
    const char* hitobj_name;
    gc::vec3 position;
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
