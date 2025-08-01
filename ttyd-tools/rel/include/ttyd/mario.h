#pragma once

#include <gc/types.h>

#include <cstdint>
#include <cstddef>

namespace ttyd::hitdrv { struct HitObj; }

namespace ttyd::mario {

struct Player
{
	uint32_t flags1;
	uint32_t flags2;
	uint32_t unk_8;
	uint32_t flags3;
	uint8_t gap_10[8];
	const char *animName;
	uint32_t unk_1c;
	uint32_t unk_20;
	uint32_t unk_24;
	uint16_t unk_28;
	uint16_t pad_2a;
	uint32_t unk_2c;
	uint16_t prevMotionId;
	uint16_t pad_32;
	uint32_t wMapTime;
	uint32_t wUnkCounter;
	int8_t characterId;
	int8_t colorId;
	uint8_t unk_3e;
	int8_t wWallTimer;
	uint8_t wMotionTimer;
	uint8_t wMotionCounter;
	uint16_t pad_42;
	uint32_t unk_44;
	uint32_t wYawnTimer;
	uint16_t wInvincibleTimer;
	uint16_t unk_4e;
	uint16_t wAirTimer;
	uint16_t pad_52;
	uint8_t gap_54[40];
	float wJumpVelocityY;
	float wJumpAccelerationY;
	float unk_84;
	float unk_88;
	float playerPosition[3];
	float wModelPosition[3];
	float wAnimPosition[3];
	uint8_t gap_b0[20];
	float wRotationY;
	float wSizeVector[3];
	float wLastGroundPosition[3];
	float unkPosition[3];
	float wCameraPosition[3];
	uint8_t gap_f8[24];
	float wPlayerPosition2[3];
	uint8_t gap_11c[16];
	float wJumpApopasis;
	uint32_t unk_130;
	uint32_t wRenderFlags;
	uint8_t gap_138[16];
	float wCamZoom;
	float wCamPan;
	uint8_t gap_150[48];
	float wPlayerTarget;
	float unk_184;
	float unk_188;
	float wPlayerEffectiveSpeed;
	float unk_190;
	float wControlStickSensitivity;
	float wControlStickAngle;
	float unk_19c;
	float unk_1a0;
	float wPlayerAngle;
	float unk_1a8;
	float wPlayerDirection;
	uint32_t unk_1b0;
	uint32_t unk_1b4;
	float wPlayerCollisionBox[3];
	float wPlayerCollisionRelated[3];
	uint8_t gap_1d0[20];
	ttyd::hitdrv::HitObj* wObjInteract;
	void *wObjStandOn;
	void *wObjJumpFrom;
	void *wUnkObj1;
	void *wUnkObj2;
	void *wUnkObj3;
	void *wUnkObj4;
	void *wUnkObj5;
	void *wObjHammer;
	void *wObjHazardRespawn;
	void *wUnkObj6;
	uint8_t gap_210[52];
	uint8_t unk_244;
	uint8_t wFollowerFlags[2];
	uint8_t prevFollowerId[2];
	uint8_t unk_249;
	uint16_t wPauseButtonBuffer;
	uint32_t unk_24c;
	uint16_t unk_250;
	uint8_t wStickDir1;
	uint8_t wStickDir2;
	uint8_t wSubStickDir1;
	uint8_t wSubStickDir2;
	uint8_t wPauseLeftTrigger;
	uint8_t wPauseRightTrigger;
	uint32_t unk_258;
	uint32_t unk_25c;
	uint8_t gap_260[88];
	float wMultiVal1;
	float wYoshiHoverHeight;
	float wCamVal1;
	float wHazardSpeed;
	uint8_t gap_2c8[28];
	float wResetPosition[3];
	uint32_t unk_2f0;
	uint32_t unk_2f4;
} ;

static_assert(sizeof(Player) == 0x2F8);

extern "C" {

// toFrontPose
// toRearPose
// marioDisp
// marioDispBlur
// marioDispBlurSub
// marioPreDisp
// marioRearAnime
// marioMakeDispDir
// marioGetScale
// marioChkInScreen
void marioGetScreenPos(gc::vec3* worldPos, float* x, float* y, float* z);
// marioPaperLightOff
// marioPaperOff
// marioPaperOn
// marioSetPaperAnimeLocalTime
// marioChgPaper
// marioChgPoseTime
// marioChgPose
// toDotMarioPose
// marioChgEvtPose
// marioAnimeId
// marioChkPushAnime
// marioMoveMain
// marioMove
// marioCtrlOff2Main
// marioMain
// marioReInit
// marioInit
// marioPoseInit
// marioSoundInit
// marioOfsRotReset
// marioReset
// marioResetHitObj
// marioChkSts
// marioBgmodeOff
// marioBgmodeOn
uint32_t marioBgmodeChk();
uint16_t marioSetMutekiTime(int32_t msec);
// marioKeyOn
// marioKeyOff
// marioCtrlOn2
// marioCtrlOff2
// marioCtrlOn
// marioCtrlOff
int32_t marioKeyOffChk();
// marioCtrlOffChk
// marioFBattlePost
// marioFBattlePrepare
// marioChkCtrl
// marioChkKey
// marioCheckMenuDisable
// marioCaseEventValidChk
// marioItemGetOk
// marioItemGetChk
// marioItemGetDisable
// marioEntry
// marioSetSpec
// marioSetFamicomMode
void marioSetCharMode(uint32_t mode);
int8_t marioGetColor();
Player *marioGetPtr();

}

}