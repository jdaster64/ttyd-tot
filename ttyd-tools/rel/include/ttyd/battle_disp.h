#pragma once

#include <cstdint>

namespace ttyd::battle_unit {
struct BattleWorkUnit;
struct BattleWorkUnitPart;
}

namespace ttyd::battle_disp {

extern "C" {

// _btlStockExpDisp
// BattleStockExpDisp
// _btlDispTex4
// btlDispTex4
// btlGetScreenPoint
// btlDispAnimeSpeed
// _pose_two_pattern
// _pose_one_pattern
// _pose_def
// statusPoseControl
// getFloatDispOffset
// floatOffsetControl
// getGravityDispOffset
// gravityOffsetControl
void btlDispPoseAnime(battle_unit::BattleWorkUnitPart* part);
// btlDispChangePeraAnime
// btlDispChangeAnime
void btlDispEntAnime(battle_unit::BattleWorkUnit* unit);
// btlUnitPartsBlurDisp
// _partsBlurControl
// btlUnitPartsBlurControl
// btlUnitStolenItemDisp
// btlUnitItemDisp2
// btlUnitItemDisp
// btlUnitPartsDisp
// btlDispMain
// _GetStatusPoseType
// btlDispInit
// btlDispGetTexSize
// btlDispTexPlane3
// btlDispTexPlane2
// btlDispTexPlane
// btlDispTexPlainGX
// btlDispTexPlaneInit
// btlDispGXQuads2D
// btlDispGXPoint2D
// btlDispGXQuads2DRasta
// btlDispGXPoint2DRasta
// btlDispGXInit2D
// btlDispGXInit2DRasta
// btlDispGXInit2DSub
// searchPoseTbl

}

}