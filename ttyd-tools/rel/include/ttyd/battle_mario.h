#pragma once

#include "battle_database_common.h"

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::battle_mario {

extern "C" {

// .text
EVT_DECLARE_USER_FUNC(_paper_light_off, 2)
EVT_DECLARE_USER_FUNC(_battle_majinai_effect, 1)
EVT_DECLARE_USER_FUNC(_majinai_powerup_check, 1)
EVT_DECLARE_USER_FUNC(_fire_wave, 4)
EVT_DECLARE_USER_FUNC(_mario_fire_ball_controll, 2)
EVT_DECLARE_USER_FUNC(_kaiten_hammer_acrobat_rotate, 4)
EVT_DECLARE_USER_FUNC(_hammer_star_effect, 2)
EVT_DECLARE_USER_FUNC(mario_get_renzoku_count_max, 3)
EVT_DECLARE_USER_FUNC(_record_renzoku_count, 1)
EVT_DECLARE_USER_FUNC(_tatsumaki_effect, 1)
EVT_DECLARE_USER_FUNC(_wait_jyabara_hit_iron_frame, 1)
EVT_DECLARE_USER_FUNC(_bgset_iron_frame_check, 1)
EVT_DECLARE_USER_FUNC(_jump_star_effect, 2)
EVT_DECLARE_USER_FUNC(_whirlwind_effect, 2)
EVT_DECLARE_USER_FUNC(_get_mario_hammer_lv, 1)

// .data

// Weapons / badge weapons.
extern battle_database_common::BattleWeapon marioDefaultWeapon_Jump;
extern battle_database_common::BattleWeapon marioWeapon_KururinJump;
extern battle_database_common::BattleWeapon marioWeapon_KururinJump2;
extern battle_database_common::BattleWeapon marioWeapon_JyabaraJump;
extern battle_database_common::BattleWeapon marioWeapon_JyabaraJumpFailed;
extern battle_database_common::BattleWeapon marioDefaultWeapon_FirstAttackKaitenHammer;
extern battle_database_common::BattleWeapon marioDefaultWeapon_FirstAttackUltraHammer;
extern battle_database_common::BattleWeapon marioDefaultWeapon_Hammer;
extern battle_database_common::BattleWeapon marioWeapon_KaitenHammer;
extern battle_database_common::BattleWeapon marioWeapon_KaitenHammerInvolved;
extern battle_database_common::BattleWeapon marioWeapon_UltraHammer;
extern battle_database_common::BattleWeapon marioWeapon_UltraHammerInvolved;
extern battle_database_common::BattleWeapon marioWeapon_UltraHammerFinish;
extern battle_database_common::BattleWeapon badgeWeapon_TugitugiJump;
extern battle_database_common::BattleWeapon badgeWeapon_GatsunJump;
extern battle_database_common::BattleWeapon badgeWeapon_DokanJump;
extern battle_database_common::BattleWeapon badgeWeapon_RenzokuJump;
extern battle_database_common::BattleWeapon badgeWeapon_TatsumakiJump;
extern battle_database_common::BattleWeapon badgeWeapon_TatsumakiJumpInvolved;
extern battle_database_common::BattleWeapon badgeWeapon_MiniminiFumi;
extern battle_database_common::BattleWeapon badgeWeapon_NemuraseFumi;
extern battle_database_common::BattleWeapon badgeWeapon_FunyafunyaJump;
extern battle_database_common::BattleWeapon badgeWeapon_GatsunHammer;
extern battle_database_common::BattleWeapon badgeWeapon_DokanHammer;
extern battle_database_common::BattleWeapon badgeWeapon_JishinHammer;
extern battle_database_common::BattleWeapon badgeWeapon_UltraJishinHammer;
extern battle_database_common::BattleWeapon badgeWeapon_HammerNageru;
extern battle_database_common::BattleWeapon badgeWeapon_TsuranukiNaguri;
extern battle_database_common::BattleWeapon badgeWeapon_ConfuseHammer;
extern battle_database_common::BattleWeapon badgeWeapon_FireNaguri;
extern battle_database_common::BattleWeapon badgeWeapon_FireNaguriFailed;
extern battle_database_common::BattleWeapon badgeWeapon_IceNaguri;
extern battle_database_common::BattleWeapon badgeWeapon_Charge;
extern battle_database_common::BattleWeapon badgeWeapon_ChargeP;
extern battle_database_common::BattleWeapon badgeWeapon_SuperCharge;
extern battle_database_common::BattleWeapon badgeWeapon_SuperChargeP;
extern battle_database_common::BattleWeapon badgeWeapon_2KaiItem;
extern battle_database_common::BattleWeapon badgeWeapon_3KaiItem;
extern battle_database_common::BattleWeapon badgeWeapon_DefenceCommand;
extern battle_database_common::BattleWeapon badgeWeapon_DefenceCommandP;
extern battle_database_common::BattleWeapon marioWeapon_BakuGame;
extern battle_database_common::BattleWeapon marioWeapon_Scissor;
extern battle_database_common::BattleWeapon marioWeapon_ZubaStar;
extern battle_database_common::BattleWeapon marioWeapon_Genki0;
extern battle_database_common::BattleWeapon marioWeapon_Genki1;
extern battle_database_common::BattleWeapon marioWeapon_Deka;
extern battle_database_common::BattleWeapon marioWeapon_Muki;
extern battle_database_common::BattleWeapon marioWeapon_Suki;
extern battle_database_common::BattleWeapon* superActionTable[18];

// Attack event code.
extern int32_t marioAttackEvent_NormalJump[1];
extern int32_t marioAttackEvent_KururinJump[1];
extern int32_t marioAttackEvent_JyabaraJump[1];
extern int32_t marioAttackEvent_GatsuDokaJump[1];
extern int32_t marioAttackEvent_GatsunJump[1];
extern int32_t marioAttackEvent_DokanJump[1];
extern int32_t marioAttackEvent_TatsumakiJump[1];
extern int32_t marioAttackEvent_MiniminiFumi[1];
extern int32_t marioAttackEvent_NemuraseFumi[1];
extern int32_t marioAttackEvent_FunyafunyaJump[1];
extern int32_t marioAttackEvent_RenzokuJump[1];
extern int32_t marioAttackEvent_TugiTugiJump[1];
extern int32_t marioAttackEvent_NormalHammer[1];
extern int32_t marioAttackEvent_GatsunHammer[1];
extern int32_t marioAttackEvent_IceNaguri[1];
extern int32_t marioAttackEvent_TsuranukiHammer[1];
extern int32_t marioAttackEvent_NormalHammer_Core[1];
extern int32_t marioAttackEvent_KaitenHammer[1];
extern int32_t marioAttackEvent_FirstAttackKaitenHammer[1];
extern int32_t marioAttackEvent_JishinHammer[1];
extern int32_t marioAttackEvent_UltraJishinHammer[1];
extern int32_t marioAttackEvent_HammerRotate[1];
extern int32_t marioAttackEvent_HammerNageru[1];
extern int32_t marioAttackEvent_HammerNageru_object[1];
extern int32_t marioAttackEvent_FireNaguri[1];
extern int32_t marioAttackEvent_FireNaguri_object[1];
extern int32_t _NormalCharge_core[1];
extern int32_t marioAttackEvent_NormalCharge[1];
extern int32_t marioAttackEvent_NormalChargeP[1];
extern int32_t _SuperCharge_core[1];
extern int32_t marioAttackEvent_SuperCharge[1];
extern int32_t marioAttackEvent_SuperChargeP[1];
extern int32_t marioAttackEvent_MajinaiPowerUpCheck[1];
extern int32_t marioAttackEvent_MajinaiJump[1];
extern int32_t _MajinaiPoseReset[1];
extern int32_t marioAttackEvent_MajinaiDefenceUp[1];
extern int32_t marioAttackEvent_MajinaiExpUp[1];

}

}