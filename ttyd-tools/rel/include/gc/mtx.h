#pragma once

#include "types.h"

#include <cstdint>

namespace gc::mtx {
    
extern "C" {

void PSMTXIdentity(mtx34* out);
void PSMTXCopy(mtx34* src, mtx34* dst);
void PSMTXConcat(mtx34* a, mtx34* b, mtx34* out);
// PSMTXInverse
// PSMTXInvXpose
// PSMTXRotRad
// PSMTXRotTrig
// __PSMTXRotAxisRadInternal
// PSMTXRotAxisRad
void PSMTXTrans(mtx34* out, double x, double y, double z);
// PSMTXTransApply
void PSMTXScale(mtx34* out, double x, double y, double z);
// PSMTXScaleApply
// C_MTXLookAt
// C_MTXLightFrustum
// C_MTXLightPerspective
// C_MTXLightOrtho
// PSMTXMultVec
// PSMTXMultVecSR
// C_MTXFrustum
// C_MTXPerspective
// C_MTXOrtho
// PSMTX44Copy
// PSMTX44Concat
// PSMTX44Trans
// PSMTX44Scale
// PSMTX44MultVec
// PSVECAdd
// PSVECSubtract
// PSVECScale
// PSVECNormalize
// PSVECSquareMag
// PSVECMag
// PSVECDotProduct
// PSVECCrossProduct
// C_VECHalfAngle
// C_VECReflect
// PSVECSquareDistance
// PSVECDistance

}

}