#include "tot_party_mario.h"

#include "evt_cmd.h"
#include "tot_gsw.h"
#include "tot_manager_achievements.h"
#include "tot_manager_move.h"
#include "tot_party_bobbery.h"
#include "tot_party_koops.h"

#include <ttyd/battle.h>
#include <ttyd/battle_camera.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_event_default.h>
#include <ttyd/battle_item_data.h>
#include <ttyd/battle_mario.h>
#include <ttyd/battle_seq_command.h>
#include <ttyd/battle_unit.h>
#include <ttyd/battle_unit_event.h>
#include <ttyd/battle_weapon_power.h>
#include <ttyd/evt_audience.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/icondrv.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/msgdrv.h>

namespace mod::tot::party_mario {

namespace {
    
// Including entire namespaces for convenience.
using namespace ::ttyd::battle_camera;
using namespace ::ttyd::battle_database_common;
using namespace ::ttyd::battle_event_cmd;
using namespace ::ttyd::battle_event_default;
using namespace ::ttyd::battle_mario;
using namespace ::ttyd::battle_weapon_power;
using namespace ::ttyd::evt_audience;
using namespace ::ttyd::evt_eff;
using namespace ::ttyd::evt_snd;
using namespace ::ttyd::evt_sub;

using ::ttyd::battle::BattleWork;
using ::ttyd::battle::BattleWorkCommand;
using ::ttyd::battle::BattleWorkCommandCursor;
using ::ttyd::battle_seq_command::_btlcmd_GetCursorPtr;
using ::ttyd::battle_unit::BattleWorkUnit;
using ::ttyd::evtmgr_cmd::evtGetFloat;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::item_data::itemDataTable;
using ::ttyd::mario_pouch::PouchData;
using ::ttyd::msgdrv::msgSearch;

namespace BattleUnitToken_Flags = ::ttyd::battle_unit::BattleUnitToken_Flags;
namespace IconType = ::ttyd::icondrv::IconType;
namespace ItemType = ::ttyd::item_data::ItemType;

}  // namespace

// Declaration of weapon structs.

// Base jump + hammer moveset.
extern BattleWeapon customWeapon_Jump;
extern BattleWeapon customWeapon_SpinJump;
extern BattleWeapon customWeapon_SpinJump2;
extern BattleWeapon customWeapon_SpringJump;
extern BattleWeapon customWeapon_SpringJump2;
extern BattleWeapon customWeapon_SpringJumpFailed;
extern BattleWeapon customWeapon_Hammer;
extern BattleWeapon customWeapon_SuperHammer;
extern BattleWeapon customWeapon_SuperHammerRecoil;
extern BattleWeapon customWeapon_UltraHammer;
extern BattleWeapon customWeapon_UltraHammerRecoil;
extern BattleWeapon customWeapon_UltraHammerFinisher;
extern BattleWeapon customWeapon_FSSuperHammer;
extern BattleWeapon customWeapon_FSUltraHammer;

// Selection of "bonus" jump moves.
extern BattleWeapon customWeapon_PowerSoftStomp;
extern BattleWeapon customWeapon_Multibounce;
extern BattleWeapon customWeapon_PowerBounce;
extern BattleWeapon customWeapon_SleepyStomp;
extern BattleWeapon customWeapon_TornadoJump;
extern BattleWeapon customWeapon_TornadoJumpRecoil;

// Selection of "bonus" hammer moves.
extern BattleWeapon customWeapon_PowerPiercingSmash;
extern BattleWeapon customWeapon_ShrinkSmash;
extern BattleWeapon customWeapon_IceSmash;
extern BattleWeapon customWeapon_QuakeHammer;
extern BattleWeapon customWeapon_FireDrive;
extern BattleWeapon customWeapon_FireDriveFailed;

// Thrown variants of single-target hammer moves, for if Hammerman is equipped.
extern BattleWeapon customWeapon_HammerThrow;
extern BattleWeapon customWeapon_PowerPiercingSmashThrow;
extern BattleWeapon customWeapon_ShrinkSmashThrow;
extern BattleWeapon customWeapon_IceSmashThrow;

BattleWeapon* g_CustomJumpWeapons[] = {
    &customWeapon_Jump, &customWeapon_SpinJump, 
    &customWeapon_SpringJump, &customWeapon_PowerSoftStomp, 
    &customWeapon_Multibounce, &customWeapon_PowerBounce,
    &customWeapon_SleepyStomp, &customWeapon_TornadoJump
};
BattleWeapon* g_CustomHammerWeapons[] = {
    &customWeapon_Hammer, &customWeapon_SuperHammer,
    &customWeapon_UltraHammer, &customWeapon_PowerPiercingSmash,
    &customWeapon_ShrinkSmash, &customWeapon_IceSmash,
    &customWeapon_QuakeHammer, &customWeapon_FireDrive
};
BattleWeapon* g_CustomHammerThrowWeapons[] = {
    &customWeapon_HammerThrow, &customWeapon_SuperHammer,
    &customWeapon_UltraHammer, &customWeapon_PowerPiercingSmashThrow,
    &customWeapon_ShrinkSmashThrow, &customWeapon_IceSmashThrow,
    &customWeapon_QuakeHammer, &customWeapon_FireDrive
};

// Custom function to populate the weapon selection dialog.
int32_t MakeSelectWeaponTable(BattleWork* battleWork, int32_t table_type) {
    BattleWorkUnit* unit = BattleGetUnitPtr(battleWork, battleWork->active_unit_idx);
    BattleWorkCommand& command_work = battleWork->command_work;
    const PouchData& pouch = *ttyd::mario_pouch::pouchGetPtr();
    
    BattleWorkCommandCursor* cursor;
    _btlcmd_GetCursorPtr(&command_work, table_type, &cursor);
    int32_t num_options = 0;
    
    switch (table_type) {
        case 0: {  // Jump
            for (int32_t i = 0; i < 8; ++i) {
                auto& weapon_entry = command_work.weapon_table[num_options];
                BattleWeapon* weapon = g_CustomJumpWeapons[i];
                
                if (MoveManager::GetUnlockedLevel(MoveType::JUMP_BASE + i)) {
                    weapon_entry.index = MoveType::JUMP_BASE + i;
                    weapon_entry.item_id = 0;
                    weapon_entry.unk_04 = 0;
                    weapon_entry.unk_18 = 0;
                    weapon_entry.weapon = weapon;
                    weapon_entry.icon = weapon->icon;
                    weapon_entry.name = msgSearch(weapon->name);
                    
                    ++num_options;
                }
            }
            break;
        }
        case 1: {  // Hammer
            for (int32_t i = 0; i < 8; ++i) {
                auto& weapon_entry = command_work.weapon_table[num_options];
                BattleWeapon* weapon;
                if (unit->badges_equipped.hammerman == 0 ||
                    (unit->token_flags & BattleUnitToken_Flags::CONFUSE_PROC)) {
                    weapon = g_CustomHammerWeapons[i];
                } else {
                    weapon = g_CustomHammerThrowWeapons[i];
                }
                
                if (MoveManager::GetUnlockedLevel(MoveType::HAMMER_BASE + i)) {
                    weapon_entry.index = MoveType::HAMMER_BASE + i;
                    weapon_entry.item_id = 0;
                    weapon_entry.unk_04 = 0;
                    weapon_entry.unk_18 = 0;
                    weapon_entry.weapon = weapon;
                    weapon_entry.icon = weapon->icon;
                    weapon_entry.name = msgSearch(weapon->name);
                    
                    ++num_options;
                }
            }
            break;
        }
        case 2: {  // Items
            for (int32_t i = 0; i < 20; ++i) {
                int32_t item = pouch.items[i];
                if (item == 0 || (
                        itemDataTable[item].usable_locations & 
                        ttyd::item_data::ItemUseLocation_Flags::kBattle
                    ) == 0) {
                    continue;
                }
                
                auto& weapon_entry = command_work.weapon_table[num_options];
                BattleWeapon* weapon = itemDataTable[item].weapon_params;
                
                weapon_entry.index = i;
                weapon_entry.item_id = item;
                weapon_entry.unk_04 = 0;
                weapon_entry.unk_18 = 0;
                weapon_entry.icon = itemDataTable[item].icon_id;
                
                if (weapon) {
                    weapon_entry.weapon = weapon;
                    if (weapon->name) {
                         weapon_entry.name = msgSearch(weapon->name);
                    } else {
                         weapon_entry.name = 
                            msgSearch(itemDataTable[item].name);
                    }
                } else {
                    weapon_entry.weapon = 
                        &ttyd::battle_item_data::ItemWeaponData_CookingItem;
                    weapon_entry.name = msgSearch(itemDataTable[item].name);
                }
                
                ++num_options;
            }
            // If no items, include dummy 'No Item' selection.
            if (num_options == 0) {
                auto& weapon_entry = command_work.weapon_table[0];
                weapon_entry.index = 0;
                weapon_entry.item_id = 0;
                weapon_entry.weapon =
                    &ttyd::battle_seq_command::defaultWeapon_Dummy_NoItem;
                weapon_entry.icon = IconType::DO_NOTHING;
                weapon_entry.unk_04 = 0;
                weapon_entry.unk_18 = 0;
                weapon_entry.name = msgSearch(weapon_entry.weapon->name);
                
                ++num_options;
            }
            break;
        }
        case 4: {  // Special moves
            // Sort in ascending order of SP cost.
            static constexpr const int32_t kMoveOrder[] = {
                MoveType::SP_SWEET_TREAT,   MoveType::SP_EARTH_TREMOR,
                MoveType::SP_CLOCK_OUT,     MoveType::SP_POWER_LIFT,
                MoveType::SP_SWEET_FEAST,   MoveType::SP_SHOWSTOPPER,
                MoveType::SP_ART_ATTACK,    MoveType::SP_SUPERNOVA,
            };
        
            for (int32_t i = 0; i < 8; ++i) {
                int32_t move = kMoveOrder[i];
                int32_t index = move - MoveType::SP_SWEET_TREAT;
                
                if ((pouch.star_powers_obtained & (1 << index)) == 0) continue;
                
                auto& weapon_entry = command_work.weapon_table[num_options];
                BattleWeapon* weapon = superActionTable[index];
                
                if (MoveManager::GetUnlockedLevel(move)) {
                    weapon_entry.index = move;
                    weapon_entry.item_id = 0;
                    weapon_entry.unk_04 = 0;
                    weapon_entry.unk_18 = 0;
                    weapon_entry.weapon = weapon;
                    weapon_entry.icon = weapon->icon;
                    
                    if (weapon->name) {
                         weapon_entry.name = msgSearch(weapon->name);
                    } else {
                         weapon_entry.name = 
                            msgSearch(itemDataTable[weapon->item_id].name);
                    }
                    
                    ++num_options;
                }
            }
            break;
        }
        case 6: {  // Partner moves
            // Delegates to partner's weapon select population function.
            auto partner_move_func = (void (*)(BattleWorkCommand*, int32_t*))
                ttyd::battle_unit::BtlUnit_GetData(unit, 1);
            partner_move_func(&command_work, &num_options);
            break;
        }
        default:
            // Should never occur.
            break;
    };
    
    // Update cursor bounds / position.
    if (num_options < 1) {
        num_options = 0;
    }
    else {
        cursor->num_options = num_options;
        if (cursor->num_options <= cursor->abs_position) {
            cursor->abs_position = cursor->num_options - 1;
        }
        while (cursor->num_options < cursor->rel_position + 6 &&
               0 < cursor->rel_position) {
            --cursor->rel_position;
        }
        ttyd::battle_seq_command::_btlcmd_UpdateSelectWeaponTable(
            battleWork, table_type);
    }
    
    return num_options;
}

// Custom function to get First Strike attacks.
BattleWeapon* GetFirstAttackWeapon(int32_t attack_type) {
    switch (attack_type) {
        case 1: return &customWeapon_Jump;
        case 2: return &customWeapon_SpinJump;
        case 3: return &customWeapon_Jump;  // Per original game
        case 4: return &customWeapon_Hammer;
        case 5: return &customWeapon_FSSuperHammer;
        case 6: return &customWeapon_FSUltraHammer;
        case 7: return party_koops::GetFirstAttackWeapon();
        case 8: return party_bobbery::GetFirstAttackWeapon();
    }
    return nullptr;
}

EVT_DECLARE_USER_FUNC(evtTot_CheckBombSquadHit, 2)
EVT_DEFINE_USER_FUNC(evtTot_CheckBombSquadHit) {
    float last_position = evtGetFloat(evt, evt->evtArguments[0]);
    float cur_position = evtGetFloat(evt, evt->evtArguments[1]);
    if (cur_position < last_position) {
        float temp = cur_position;
        cur_position = last_position;
        last_position = temp;
    }

    auto* battleWork = ttyd::battle::g_BattleWork;
    for (int32_t i = 0; i < 64; ++i) {
        auto* unit = battleWork->battle_units[i];
        if (unit && unit->current_kind == BattleUnitType::BOMB_SQUAD_BOMB &&
            unit->unit_work[2] < 2 &&   // Not Megaton bomb.
            unit->position.x >= last_position &&
            unit->position.x < cur_position) {
            // Run explosion event.
            ttyd::battle_unit_event::BattleRunHitEvent(unit, 0x717);
            // Mark off achievement.
            AchievementsManager::MarkCompleted(AchievementId::MISC_BOMB_SQUAD_FIRE);
        }
    }

    return 2;
}

EVT_BEGIN(marioAttackEvent_NormalJump)
    SET(LW(11), 0)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.70))
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetFirstAttackTarget, LW(3), LW(4))
        IF_EQUAL(LW(3), -1)
            GOTO(99)
        END_IF()
        SET(LW(12), PTR(&customWeapon_Jump))
        USER_FUNC(btlevtcmd_ACHelpSet, PTR(&customWeapon_Jump))
        USER_FUNC(evt_btl_camera_set_mode, 0, 10)
        USER_FUNC(evt_btl_camera_set_homing_unitparts, 0, LW(3), LW(4), -2, 1)
        USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
        USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    ELSE()
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
        ELSE()
            USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
            USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(12))
            USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(12), LW(3), LW(4))
        END_IF()
        IF_EQUAL(LW(3), -1)
            GOTO(99)
        END_IF()
        USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
        USER_FUNC(btlevtcmd_WaitGuardMove)
        USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(12))
        USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
        RUN_CHILD_EVT(PTR(&marioAttackEvent_MajinaiPowerUpCheck))
        USER_FUNC(evt_btl_camera_set_mode, 0, 11)
        USER_FUNC(evt_btl_camera_set_homing_unitparts, 0, -2, 1, LW(3), LW(4))
        USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
        USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    END_IF()
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_EQUAL(LW(0), 0)
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_GetWeaponActionLv, LW(12), LW(0))
            USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
            USER_FUNC(btlevtcmd_SetupAC, -2, 1, 1, 0)
        END_IF()
    END_IF()
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 250)
        USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
        WAIT_MSEC(1000)
        USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(5.0))
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
        USER_FUNC(btlevtcmd_GetTakeoffPosition, -2, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(6))
        USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(6))
        USER_FUNC(btlevtcmd_GetMoveFrame, -2, LW(0), LW(1), LW(2), 0, LW(7))
        IF_LARGE(LW(7), 22)
            SUB(LW(7), 22)
        ELSE()
            SUB(LW(7), 22)
            MUL(LW(7), -1)
        END_IF()
        BROTHER_EVT()
            WAIT_FRM(LW(7))
            USER_FUNC(btlevtcmd_GetWeaponActionLv, LW(12), LW(5))
            USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(5))
            USER_FUNC(btlevtcmd_SetupAC, -2, 1, 1, 0)
        END_BROTHER()
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    ELSE()
        USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
        USER_FUNC(btlevtcmd_GetTakeoffPosition, -2, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(6))
        USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(6))
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
        BROTHER_EVT()
            IF_EQUAL(LW(12), PTR(&customWeapon_SleepyStomp))
                SET(LW(13), 0)
                USER_FUNC(_jump_star_effect, -2, LW(13))
                USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_MARIO_HAMMER_SHINE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            END_IF()
        END_BROTHER()
    END_IF()
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_StartAC, 1)
    END_IF()
LBL(10)
    IF_LARGE_EQUAL(LW(11), 1)
        USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(12), 1048832, LW(5))
    ELSE()
        USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
    END_IF()
    SWITCH(LW(5))
        CASE_OR(2)
        CASE_OR(3)
        CASE_OR(4)
        CASE_OR(6)
            CASE_END()
        CASE_EQUAL(1)
        CASE_ETC()
            WAIT_MSEC(1000)
    END_SWITCH()
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    ADD(LW(2), 5)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
    IF_LARGE_EQUAL(LW(11), 1)
        SET(LW(6), 36)
        USER_FUNC(btlevtcmd_MarioJumpParam, -2, LW(0), LW(1), LW(2), LW(6))
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            BROTHER_EVT()
                WAIT_FRM(3)
                SET(LW(0), LW(6))
                DIV(LW(0), 2)
                SUB(LW(0), 3)
                SET(LW(1), LW(0))
                SET(LW(2), LW(0))
                SUB(LW(1), 7)
                ADD(LW(2), 5)
                SET(LW(0), LW(6))
                SUB(LW(0), 4)
                USER_FUNC(btlevtcmd_ACRStart, -2, LW(1), LW(2), LW(0), 0)
                USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
                SWITCH(LW(6))
                    CASE_LARGE_EQUAL(2)
                        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_A_5"))
                        USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, -60, -25, 0)
                        USER_FUNC(btlevtcmd_snd_voice, -2, 1)
                        // First part of Stylish.
                        USER_FUNC(evtTot_LogActiveMoveStylish, 1)
                    CASE_ETC()
                        USER_FUNC(evt_audience_acrobat_notry)
                END_SWITCH()
            END_BROTHER()
        END_IF()
        USER_FUNC(btlevtcmd_MarioJumpPosition, -2, LW(0), LW(1), LW(2), LW(6), 3, -1)
    ELSE()
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_BATTLE_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        SET(LW(6), 36)
        USER_FUNC(btlevtcmd_MarioJumpParam, -2, LW(0), LW(1), LW(2), LW(6))
        USER_FUNC(btlevtcmd_MarioJumpPosition, -2, LW(0), LW(1), LW(2), LW(6), 0, -1)
    END_IF()
    IF_NOT_EQUAL(LW(5), 1)
        IF_EQUAL(LW(5), 3)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
        END_IF()
        IF_EQUAL(LW(5), 6)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
        END_IF()
        IF_EQUAL(LW(5), 2)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
        END_IF()
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_StopAC)
        END_IF()
        USER_FUNC(btlevtcmd_JumpContinue, -2)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_LAND_DAMAGE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_D_2"))
        WAIT_FRM(40)
        GOTO(95)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1D"))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 1, 1)
        USER_FUNC(btlevtcmd_ac_timing_get_success_frame, LW(0))
        DIV(LW(0), 2)
        WAIT_FRM(LW(0))
        USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 0, 1)
        USER_FUNC(btlevtcmd_ResultAC)
        USER_FUNC(btlevtcmd_GetResultAC, LW(6))
    ELSE()
        WAIT_FRM(4)
        SET(LW(6), 0x2)
    END_IF()
    
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(12))

    IF_NOT_FLAG(LW(6), 0x2)
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
        IF_EQUAL(LW(11), 0)
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
        END_IF()
        USER_FUNC(evt_btl_camera_set_mode, 0, 0)
        USER_FUNC(btlevtcmd_StopAC)
        USER_FUNC(evt_audience_ap_recovery)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
        USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetHomePos, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 27)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.60))
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 8)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 8, -1)
        GOTO(95)
    END_IF()
    USER_FUNC(btlevtcmd_snd_voice, -2, 2)
    ADD(LW(11), 1)
    IF_SMALL_EQUAL(LW(11), 1)
        INLINE_EVT()
            USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 0)
            USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
            WAIT_FRM(10)
            USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
            USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
        END_INLINE()
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 131072, LW(5))
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 0, LW(6))
            USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(6))
        END_IF()
        WAIT_FRM(2)
        GOTO(10)
    ELSE()
        USER_FUNC(evt_btl_camera_set_mode, 0, 0)
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 131328, LW(5))
        WAIT_FRM(2)
    END_IF()
LBL(90)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_StopAC)
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHomePos, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 60)
    SET(LW(6), 32)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        BROTHER_EVT()
            WAIT_FRM(3)
            SET(LW(0), LW(6))
            DIV(LW(0), 2)
            SUB(LW(0), 3)
            SET(LW(1), LW(0))
            SET(LW(2), LW(0))
            SUB(LW(1), 7)
            ADD(LW(2), 5)
            SET(LW(0), LW(6))
            SUB(LW(0), 4)
            USER_FUNC(btlevtcmd_ACRStart, -2, LW(1), LW(2), LW(0), 0)
            USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
            SWITCH(LW(6))
                CASE_LARGE_EQUAL(2)
                    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_A_5"))
                    USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, -60, -25, 0)
                    USER_FUNC(btlevtcmd_snd_voice, -2, 1)
                    // Second part of Stylish.
                    USER_FUNC(evtTot_LogActiveMoveStylish, 2)
                CASE_ETC()
                    USER_FUNC(evt_audience_acrobat_notry)
            END_SWITCH()
        END_BROTHER()
    END_IF()
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), LW(6), -1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
    END_IF()
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.60))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 8)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 8, -1)
LBL(95)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_W_1"))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(marioAttackEvent_NemuraseFumi)
    RUN_CHILD_EVT(PTR(&marioAttackEvent_NormalJump))
    RETURN()
EVT_END()

EVT_BEGIN(marioAttackEvent_MiniminiFumi)
    RUN_CHILD_EVT(PTR(&marioAttackEvent_NormalJump))
    RETURN()
EVT_END()

EVT_BEGIN(marioAttackEvent_FunyafunyaJump)
    RUN_CHILD_EVT(PTR(&marioAttackEvent_NormalJump))
    RETURN()
EVT_END()

EVT_BEGIN(marioAttackEvent_KururinJump)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.40))
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        SET(LW(12), PTR(&customWeapon_SpinJump))
        USER_FUNC(btlevtcmd_ACHelpSet, PTR(&customWeapon_SpinJump))
        USER_FUNC(btlevtcmd_GetFirstAttackTarget, LW(3), LW(4))
        IF_EQUAL(LW(3), -1)
            GOTO(99)
        END_IF()
        USER_FUNC(evt_btl_camera_set_mode, 0, 10)
        USER_FUNC(evt_btl_camera_set_homing_unitparts, 0, LW(3), LW(4), -2, 1)
        USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
        USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
        USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 250)
        USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
        WAIT_FRM(30)
        USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(5.0))
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
        USER_FUNC(btlevtcmd_GetTakeoffPosition, -2, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(6))
        USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(6))
        USER_FUNC(btlevtcmd_GetMoveFrame, -2, LW(0), LW(1), LW(2), 0, LW(7))
        IF_LARGE(LW(7), 22)
            SUB(LW(7), 22)
        ELSE()
            SUB(LW(7), 22)
            MUL(LW(7), -1)
        END_IF()
        BROTHER_EVT()
            WAIT_FRM(LW(7))
            USER_FUNC(btlevtcmd_GetWeaponActionLv, LW(12), LW(5))
            USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(5))
            USER_FUNC(btlevtcmd_SetupAC, -2, 1, 1, 0)
        END_BROTHER()
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    ELSE()
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
        ELSE()
            USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
            USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(12))
            USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(12), LW(3), LW(4))
        END_IF()
        IF_EQUAL(LW(3), -1)
            GOTO(99)
        END_IF()
        USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
        USER_FUNC(btlevtcmd_WaitGuardMove)
        USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(12))
        USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
        RUN_CHILD_EVT(PTR(&marioAttackEvent_MajinaiPowerUpCheck))
        USER_FUNC(evt_btl_camera_set_mode, 0, 11)
        USER_FUNC(evt_btl_camera_set_homing_unitparts, 0, -2, 1, LW(3), LW(4))
        USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
        USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
        USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_GetWeaponActionLv, LW(12), LW(0))
            USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
            USER_FUNC(btlevtcmd_SetupAC, -2, 1, 1, 0)
        END_IF()
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
        USER_FUNC(btlevtcmd_GetTakeoffPosition, -2, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(6))
        USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(6))
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    END_IF()
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_StartAC, 1)
    END_IF()
    SET(LW(10), 0)
LBL(10)
    IF_LARGE_EQUAL(LW(10), 1)
        USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(12), 1048832, LW(5))
    ELSE()
        USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
    END_IF()
    SWITCH(LW(5))
        CASE_OR(2)
        CASE_OR(3)
        CASE_OR(4)
        CASE_OR(6)
            CASE_END()
        CASE_EQUAL(1)
        CASE_ETC()
            WAIT_MSEC(1000)
    END_SWITCH()
    IF_SMALL_EQUAL(LW(10), 0)
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        ADD(LW(2), 5)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
        SET(LW(6), 36)
        USER_FUNC(btlevtcmd_MarioJumpParam, -2, LW(0), LW(1), LW(2), LW(6))
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            BROTHER_EVT()
                SET(LW(0), LW(6))
                SET(LW(1), LW(6))
                DIV(LW(0), 2)
                SUB(LW(1), 1)
                USER_FUNC(btlevtcmd_ACRStart, -2, 5, LW(0), LW(1), 0)
                USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
                SWITCH(LW(6))
                    CASE_LARGE_EQUAL(2)
                        USER_FUNC(evtTot_LogActiveMoveStylish, 0)
                        USER_FUNC(btlevtcmd_snd_voice, -2, 1)
                        USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
                        USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 14, 0)
                        DO(15)
                            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, -24)
                            WAIT_FRM(1)
                        WHILE()
                        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
                    CASE_ETC()
                        USER_FUNC(evt_audience_acrobat_notry)
                END_SWITCH()
            END_BROTHER()
        END_IF()
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_BATTLE_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_MarioJumpPosition, -2, LW(0), LW(1), LW(2), LW(6), 0, -1)
        IF_NOT_EQUAL(LW(5), 1)
            IF_EQUAL(LW(5), 3)
                USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
            END_IF()
            IF_EQUAL(LW(5), 6)
                USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
            END_IF()
            IF_EQUAL(LW(5), 2)
                USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
            END_IF()
            IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
                USER_FUNC(btlevtcmd_StopAC)
            END_IF()
            USER_FUNC(btlevtcmd_JumpContinue, -2)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_LAND_DAMAGE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_D_2"))
            WAIT_FRM(40)
            GOTO(95)
        END_IF()
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1D"))
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 1, 1)
            USER_FUNC(btlevtcmd_ac_timing_get_success_frame, LW(0))
            DIV(LW(0), 2)
            WAIT_FRM(LW(0))
            USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 0, 1)
            USER_FUNC(btlevtcmd_ResultAC)
            USER_FUNC(btlevtcmd_GetResultAC, LW(6))
        ELSE()
            WAIT_FRM(4)
            SET(LW(6), 0x2)
        END_IF()
        
        USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(12))

        IF_NOT_FLAG(LW(6), 0x2)
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
            USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
            USER_FUNC(evt_btl_camera_set_mode, 0, 0)
            USER_FUNC(btlevtcmd_StopAC)
            USER_FUNC(evt_audience_ap_recovery)
            USER_FUNC(btlevtcmd_InviteApInfoReport)
            USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
            USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_GetHomePos, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
            USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 27)
            USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
            USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.60))
            USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
            USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
            USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 8)
            USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 8, -1)
            GOTO(95)
        END_IF()
        INLINE_EVT()
            USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 0)
            USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
            WAIT_FRM(10)
            USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
            USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
        END_INLINE()
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 131072, LW(5))
        ADD(LW(10), 1)
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 0, LW(6))
            USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
        END_IF()
    END_IF()
    BROTHER_EVT_ID(LW(14))
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.5))
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        ADD(LW(2), 5)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_BATTLE_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        SET(LW(6), 36)
        USER_FUNC(btlevtcmd_MarioJumpParam, -2, LW(0), LW(1), LW(2), LW(6))
        USER_FUNC(btlevtcmd_MarioJumpPosition, -2, LW(0), LW(1), LW(2), LW(6), 0, -1)
    END_BROTHER()
    WAIT_FRM(15)
    DELETE_EVT(LW(14))
    GOTO(55)
LBL(52)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_N_7"))
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.5))
    USER_FUNC(btlevtcmd_FallPosition, -2, LW(0), LW(1), LW(2), 14)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_LAND_DAMAGE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_D_2"))
    WAIT_FRM(28)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_U_3"))
    WAIT_FRM(30)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_StopAC)
        USER_FUNC(evt_audience_ap_recovery)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
    END_IF()
    GOTO(95)
LBL(55)
    USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(15))
    MUL(LW(15), -30)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_A_1A"))
    USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 12, 0)
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    WAIT_FRM(15)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_JUMP_KURURIN1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_POWER1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    DO(12)
        USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, LW(15))
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 0, 0)
    WAIT_FRM(15)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 4, 0, 0, 0, -1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_A_1B"))
    WAIT_FRM(2)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 1, 1)
        USER_FUNC(btlevtcmd_ac_timing_get_success_frame, LW(0))
        DIV(LW(0), 2)
        ADD(LW(0), 1)
        WAIT_FRM(LW(0))
        USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 0, 1)
        USER_FUNC(evt_btl_camera_set_mode, 0, 0)
        USER_FUNC(btlevtcmd_ResultAC)
        USER_FUNC(btlevtcmd_GetResultAC, LW(6))
    ELSE()
        WAIT_FRM(4)
        SET(LW(6), 0x2)
    END_IF()

    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(12))

    IF_FLAG(LW(6), 0x2)
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&customWeapon_SpinJump2), 131328, LW(5))
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 1, LW(6))
        USER_FUNC(evt_btl_camera_set_mode, 0, 0)
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, PTR(&customWeapon_SpinJump2), LW(6))
            USER_FUNC(btlevtcmd_StopAC)
            USER_FUNC(evt_audience_ap_recovery)
            USER_FUNC(btlevtcmd_InviteApInfoReport)
        END_IF()
        USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetHomePos, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 60)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 30, -1)
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.60))
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 8)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 8, -1)
    ELSE()
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&customWeapon_SpinJump2), 256, LW(5))
        USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 0, LW(6))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, PTR(&customWeapon_SpinJump2), LW(6))
        USER_FUNC(evt_btl_camera_set_mode, 0, 0)
        USER_FUNC(btlevtcmd_StopAC)
        USER_FUNC(evt_audience_ap_recovery)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
        USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetHomePos, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 27)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.60))
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 8)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 8, -1)
    END_IF()
    GOTO(95)
LBL(90)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_StopAC)
        USER_FUNC(evt_audience_ap_recovery)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHomePos, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 60)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 30, -1)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.60))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 8)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 8, -1)
LBL(95)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_W_1"))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(marioAttackEvent_JyabaraJump)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.70))
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    ELSE()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(12))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(12), LW(3), LW(4))
    END_IF()
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(12))
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    RUN_CHILD_EVT(PTR(&marioAttackEvent_MajinaiPowerUpCheck))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetWeaponActionLv, LW(12), LW(0))
        USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
        USER_FUNC(btlevtcmd_SetupAC, -2, 1, 1, 0)
        WAIT_FRM(21)
        USER_FUNC(btlevtcmd_StartAC, 1)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.5))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_S_1"))
    WAIT_FRM(1)
    USER_FUNC(btlevtcmd_AnimeChangePaperGroup, -2, 1, PTR("p_jyabara"), 1)
    USER_FUNC(btlevtcmd_AnimeChangePaperAnime, -2, 1, PTR("PM_J_1E"))
    WAIT_FRM(16)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_POWER2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_JUMP_JABARA1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePaperGroup, -2, 1, PTR("p_jyabara"), 0)
    USER_FUNC(btlevtcmd_AnimeChangePaperAnime, -2, 1, PTR("PM_J_1A"))
    WAIT_FRM(8)
    USER_FUNC(btlevtcmd_AnimeChangePaperAnime, -2, 1, PTR("PM_J_1B"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_JUMP_JABARA2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePaperAnime, -2, 1, PTR("PM_J_2A"))
    WAIT_FRM(8)
    USER_FUNC(btlevtcmd_AnimeChangePaperAnime, -2, 1, PTR("PM_J_2B"))
    WAIT_FRM(10)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_JUMP_JABARA3"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePaperAnime, -2, 1, PTR("PM_J_3A"))
    WAIT_FRM(10)
    USER_FUNC(btlevtcmd_AnimeChangePaperAnime, -2, 1, PTR("PM_J_3B"))
    WAIT_FRM(20)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_JUMP_JABARA4"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_AC3_5"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(_bgset_iron_frame_check, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
        USER_FUNC(btlevtcmd_AnimeChangePaperAnime, -2, 1, PTR("PM_J_1C"))
        INLINE_EVT_ID(LW(14))
            DO(50)
                USER_FUNC(btlevtcmd_AddRotate, -2, 0, 48, 0)
                WAIT_FRM(1)
            WHILE()
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        END_INLINE()
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        ADD(LW(2), 5)
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.30))
        USER_FUNC(btlevtcmd_SetMoveSoundDataWork, -2, PTR("SFX_BTL_JUMP_CONTINUE1"), 0, 0, -1, -1, 0, 0)
        INLINE_EVT_ID(LW(15))
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_BATTLE_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 90, -2)
        END_INLINE()
        USER_FUNC(_wait_jyabara_hit_iron_frame, -2)
        DELETE_EVT(LW(14))
        DELETE_EVT(LW(15))
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&customWeapon_SpringJumpFailed))
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_StopAC)
        END_IF()
        USER_FUNC(evt_btl_camera_shake_h, 0, 2, 0, 10, 13)
        BROTHER_EVT()
            SETF(LW(0), FLOAT(1.0))
            DO(5)
                WAIT_FRM(1)
                SUBF(LW(0), FLOAT(0.20))
                USER_FUNC(btlevtcmd_SetScale, -2, FLOAT(1.0), LW(0), FLOAT(1.0))
            WHILE()
        END_BROTHER()
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), 130, LW(2), 5, 0, 0, 0, -1)
        WAIT_FRM(16)
        USER_FUNC(btlevtcmd_SetScale, -2, FLOAT(1.0), FLOAT(1.0), FLOAT(1.0))
        USER_FUNC(btlevtcmd_AnimeChangePaperAnime, -2, 1, PTR("PM_J_1H"))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(5), LW(6), LW(7))
        USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(5), LW(6), LW(7))
        BROTHER_EVT()
            SET(LW(5), LW(6))
            USER_FUNC(btlevtcmd_GetPos, -2, EVT_NULLPTR, LW(9), EVT_NULLPTR)
            SUB(LW(9), LW(5))
            DIV(LW(9), 4)
            USER_FUNC(btlevtcmd_GetPos, -2, EVT_NULLPTR, LW(6), EVT_NULLPTR)
            SUB(LW(6), LW(9))
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(7), LW(1))
            USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(7), LW(1))
            ADD(LW(7), LW(9))
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(8), LW(1))
            USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(8), LW(1))
            USER_FUNC(btlevtcmd_GetPos, -2, EVT_NULLPTR, LW(0), EVT_NULLPTR)
            USER_FUNC(evt_sub_intpl_init, 0, LW(0), LW(6), 32)
            DO(32)
                USER_FUNC(evt_sub_intpl_get_value)
                USER_FUNC(btlevtcmd_SetPos, -2, EVT_NULLPTR, LW(0), EVT_NULLPTR)
                WAIT_FRM(1)
            WHILE()
            USER_FUNC(btlevtcmd_GetPos, -2, EVT_NULLPTR, LW(0), EVT_NULLPTR)
            USER_FUNC(evt_sub_intpl_init, 0, LW(0), LW(7), 64)
            DO(64)
                USER_FUNC(evt_sub_intpl_get_value)
                USER_FUNC(btlevtcmd_SetPos, -2, EVT_NULLPTR, LW(0), EVT_NULLPTR)
                WAIT_FRM(1)
            WHILE()
            USER_FUNC(btlevtcmd_GetPos, -2, EVT_NULLPTR, LW(0), EVT_NULLPTR)
            USER_FUNC(evt_sub_intpl_init, 0, LW(0), LW(8), 32)
            DO(32)
                USER_FUNC(evt_sub_intpl_get_value)
                USER_FUNC(btlevtcmd_SetPos, -2, EVT_NULLPTR, LW(0), EVT_NULLPTR)
                WAIT_FRM(1)
            WHILE()
        END_BROTHER()
        BROTHER_EVT()
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), EVT_NULLPTR, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(7))
            MUL(LW(7), -30)
            SET(LW(1), LW(0))
            ADD(LW(1), LW(7))
            USER_FUNC(evt_sub_intpl_init, 11, LW(0), LW(1), 26)
            DO(26)
                USER_FUNC(evt_sub_intpl_get_value)
                USER_FUNC(btlevtcmd_SetPos, -2, LW(0), EVT_NULLPTR, EVT_NULLPTR)
                WAIT_FRM(1)
            WHILE()
            WAIT_FRM(6)
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), EVT_NULLPTR, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(7))
            MUL(LW(7), 60)
            SET(LW(1), LW(0))
            ADD(LW(1), LW(7))
            USER_FUNC(evt_sub_intpl_init, 11, LW(0), LW(1), 58)
            DO(58)
                USER_FUNC(evt_sub_intpl_get_value)
                USER_FUNC(btlevtcmd_SetPos, -2, LW(0), EVT_NULLPTR, EVT_NULLPTR)
                WAIT_FRM(1)
            WHILE()
            WAIT_FRM(6)
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), EVT_NULLPTR, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(7))
            MUL(LW(7), -30)
            SET(LW(1), LW(0))
            ADD(LW(1), LW(7))
            USER_FUNC(btlevtcmd_GetHomePos, -2, LW(1), EVT_NULLPTR, EVT_NULLPTR)
            USER_FUNC(evt_sub_intpl_init, 11, LW(0), LW(1), 32)
            DO(32)
                USER_FUNC(evt_sub_intpl_get_value)
                USER_FUNC(btlevtcmd_SetPos, -2, LW(0), EVT_NULLPTR, EVT_NULLPTR)
                WAIT_FRM(1)
            WHILE()
        END_BROTHER()
        WAIT_FRM(130)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_I_S"))
        USER_FUNC(btlevtcmd_AnimeChangePaperAnime, -2, 1, PTR("PM_J_1E"))
        BROTHER_EVT_ID(LW(15))
            USER_FUNC(evt_sub_intpl_init_float, 5, FLOAT(0.0), FLOAT(1.20), 10)
            DO(10)
                USER_FUNC(evt_sub_intpl_get_float, LW(5), EVT_NULLPTR)
                USER_FUNC(btlevtcmd_GetScale, -2, LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_SetScale, -2, LW(0), LW(5), LW(2))
                WAIT_FRM(1)
            WHILE()
            USER_FUNC(evt_sub_intpl_init_float, 7, FLOAT(1.20), FLOAT(1.0), 30)
            DO(30)
                USER_FUNC(evt_sub_intpl_get_float, LW(5), EVT_NULLPTR)
                USER_FUNC(btlevtcmd_GetScale, -2, LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_SetScale, -2, LW(0), LW(5), LW(2))
                WAIT_FRM(1)
            WHILE()
        END_BROTHER()
        USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
        WAIT_FRM(20)
        // USER_FUNC(btlevtcmd_InviteApInfoReport)
        GOTO(95)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
    USER_FUNC(btlevtcmd_AnimeChangePaperAnime, -2, 1, PTR("PM_J_1C"))
    BROTHER_EVT()
        DO(50)
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 48, 0)
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_A_1A"))
        USER_FUNC(btlevtcmd_SetPartsBlur, -2, 1, 255, 255, 255, 80, 255, 255, 255, 0, 0)
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 1, 67108864)
    END_BROTHER()
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    ADD(LW(2), 5)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.30))
    USER_FUNC(btlevtcmd_SetMoveSoundDataWork, -2, PTR("SFX_BTL_JUMP_CONTINUE1"), 0, 0, -1, -1, 0, 0)
    USER_FUNC(evt_sub_random, 20, LW(5))
    SUB(LW(5), 10)
    // Shortened travel time overall; 90 - 150 frames instead of 120 - 240.
    MUL(LW(5), 3)
    ADD(LW(5), 120)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.15))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    ADD(LW(2), 5)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), LW(5), -2)
    SET(LW(12), PTR(&customWeapon_SpringJump2))
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
    IF_NOT_EQUAL(LW(5), 1)
        IF_EQUAL(LW(5), 3)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
        END_IF()
        IF_EQUAL(LW(5), 6)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
        END_IF()
        IF_EQUAL(LW(5), 2)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
        END_IF()
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_StopAC)
        END_IF()
        USER_FUNC(btlevtcmd_JumpContinue, -2)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_B_2"))
        USER_FUNC(btlevtcmd_AnimeChangePaperGroup, -2, 1, 0, 0)
        USER_FUNC(btlevtcmd_SetPartsBlur, -2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0)
        INLINE_EVT()
            WAIT_FRM(10)
            USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 67108864)
        END_INLINE()
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 35)
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.5))
        USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 12, 0)
        BROTHER_EVT()
            USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(0))
            MUL(LW(0), -24)
            DO(20)
                USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, LW(0))
                WAIT_FRM(1)
            WHILE()
        END_BROTHER()
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 20)
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.5))
        BROTHER_EVT()
            USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(0))
            MUL(LW(0), -20)
            DO(12)
                USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, LW(0))
                WAIT_FRM(1)
            WHILE()
        END_BROTHER()
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_Z_11"))
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 10)
        USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 20, 0, 4, 0, -1)
        WAIT_FRM(30)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_U_3"))
        WAIT_FRM(12)
        GOTO(95)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1D"))
    WAIT_FRM(2)
    USER_FUNC(btlevtcmd_AnimeChangePaperGroup, -2, 1, 0, 0)
    USER_FUNC(btlevtcmd_SetPartsBlur, -2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0)
    INLINE_EVT()
        WAIT_FRM(10)
        USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 67108864)
    END_INLINE()
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 1, 1)
        USER_FUNC(btlevtcmd_ac_timing_get_success_frame, LW(0))
        DIV(LW(0), 2)
        ADD(LW(0), 4)
        WAIT_FRM(LW(0))
        USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 0, 1)
        USER_FUNC(btlevtcmd_ResultAC)
        USER_FUNC(btlevtcmd_GetResultAC, LW(6))
    ELSE()
        WAIT_FRM(8)
        SET(LW(6), 0x2)
    END_IF()
    
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(12))

    IF_NOT_FLAG(LW(6), 0x2)
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
        USER_FUNC(btlevtcmd_GetDamage, LW(3), LW(0))
        IF_LARGE_EQUAL(LW(0), 10)
            SET(LW(0), 9)
        END_IF()
        USER_FUNC(evt_btl_camera_shake_h, 0, LW(0), 0, 10, 13)
        USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
        USER_FUNC(btlevtcmd_StopAC)
        // USER_FUNC(btlevtcmd_InviteApInfoReport)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetHomePos, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 27)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.60))
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 8)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 8, -1)
        GOTO(95)
    ELSE()
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 131072, LW(5))
    END_IF()
        
    // New second regular jump w/Stylish between landing and spin jump:
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 0, LW(6))
        USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
    END_IF()
    ADD(LW(2), 5)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
    USER_FUNC(btlevtcmd_snd_voice, -2, 2)
    SET(LW(6), 36)
    USER_FUNC(btlevtcmd_MarioJumpParam, -2, LW(0), LW(1), LW(2), LW(6))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        BROTHER_EVT()
            WAIT_FRM(3)
            SET(LW(0), LW(6))
            DIV(LW(0), 2)
            SUB(LW(0), 3)
            SET(LW(1), LW(0))
            SET(LW(2), LW(0))
            SUB(LW(1), 7)
            ADD(LW(2), 5)
            SET(LW(0), LW(6))
            SUB(LW(0), 4)
            USER_FUNC(btlevtcmd_ACRStart, -2, LW(1), LW(2), LW(0), 0)
            USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
            SWITCH(LW(6))
                CASE_LARGE_EQUAL(2)
                    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_A_5"))
                    USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, -60, -25, 0)
                    USER_FUNC(btlevtcmd_snd_voice, -2, 1)
                    // First part of Stylish.
                    USER_FUNC(evtTot_LogActiveMoveStylish, 1)
                CASE_ETC()
                    USER_FUNC(evt_audience_acrobat_notry)
            END_SWITCH()
        END_BROTHER()
    END_IF()
    USER_FUNC(btlevtcmd_MarioJumpPosition, -2, LW(0), LW(1), LW(2), LW(6), 3, -1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1D"))
    SET(LW(12), PTR(&customWeapon_SpringJump))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 1, 1)
        USER_FUNC(btlevtcmd_ac_timing_get_success_frame, LW(0))
        DIV(LW(0), 2)
        ADD(LW(0), 1)
        WAIT_FRM(LW(0))
        USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 0, 1)
        USER_FUNC(btlevtcmd_ResultAC)
        USER_FUNC(btlevtcmd_GetResultAC, LW(6))
    ELSE()
        WAIT_FRM(4)
        SET(LW(6), 0x2)
    END_IF()

    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(12))

    IF_NOT_FLAG(LW(6), 0x2)
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
        USER_FUNC(evt_btl_camera_set_mode, 0, 0)
        USER_FUNC(btlevtcmd_StopAC)
        USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 0, LW(6))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(6))
        USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetHomePos, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 27)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.60))
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 8)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 8, -1)
        GOTO(95)
    ELSE()
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 131072, LW(5))
    END_IF()
    
    // Original (now third) spin jump code...
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        // Upgraded to Good.
        USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 1, LW(6))
        USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
    END_IF()
    BROTHER_EVT_ID(LW(14))
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.5))
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        ADD(LW(2), 5)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_BATTLE_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        SET(LW(6), 36)
        USER_FUNC(btlevtcmd_MarioJumpParam, -2, LW(0), LW(1), LW(2), LW(6))
        USER_FUNC(btlevtcmd_MarioJumpPosition, -2, LW(0), LW(1), LW(2), LW(6), 0, -1)
    END_BROTHER()
    WAIT_FRM(15)
    DELETE_EVT(LW(14))
    USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(15))
    MUL(LW(15), -30)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_A_1A"))
    USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 12, 0)
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    WAIT_FRM(15)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_POWER1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_JUMP_JABARA5"), EVT_NULLPTR, 0, EVT_NULLPTR)
    DO(12)
        USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, LW(15))
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 0, 0)
    WAIT_FRM(15)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 4, 0, 0, 0, -1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_A_1B"))
    WAIT_FRM(2)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 1, 1)
        USER_FUNC(btlevtcmd_ac_timing_get_success_frame, LW(0))
        DIV(LW(0), 2)
        ADD(LW(0), 1)
        WAIT_FRM(LW(0))
        USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 0, 1)
        USER_FUNC(btlevtcmd_ResultAC)
        USER_FUNC(btlevtcmd_GetResultAC, LW(6))
    ELSE()
        WAIT_FRM(4)
        SET(LW(6), 0x2)
    END_IF()
    SET(LW(12), PTR(&customWeapon_SpringJump2))
    
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(12))

    IF_FLAG(LW(6), 0x2)
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 131328, LW(5))
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
            // Upgraded to Great.
            USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 2, LW(6))
            USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(6))
        END_IF()
    ELSE()
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
        USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 1, LW(6))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(6))
        USER_FUNC(btlevtcmd_GetDamage, LW(3), LW(0))
        IF_LARGE_EQUAL(LW(0), 10)
            SET(LW(0), 9)
        END_IF()
        USER_FUNC(evt_btl_camera_shake_h, 0, LW(0), 0, 10, 13)
        USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
        USER_FUNC(btlevtcmd_StopAC)
        // USER_FUNC(btlevtcmd_InviteApInfoReport)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetHomePos, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 27)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.60))
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 8)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 8, -1)
        GOTO(95)
    END_IF()
LBL(90)
    USER_FUNC(btlevtcmd_GetDamage, LW(3), LW(0))
    IF_LARGE_EQUAL(LW(0), 10)
        SET(LW(0), 9)
    END_IF()
    USER_FUNC(evt_btl_camera_shake_h, 0, LW(0), 0, 10, 13)
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_StopAC)
        // USER_FUNC(btlevtcmd_InviteApInfoReport)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHomePos, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 60)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        BROTHER_EVT()
            WAIT_FRM(1)
            USER_FUNC(btlevtcmd_ACRStart, -2, 8, 17, 29, 0)
            USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
            SWITCH(LW(6))
                CASE_LARGE_EQUAL(2)
                    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_A_5"))
                    USER_FUNC(btlevtcmd_snd_voice, -2, 1)
                    USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
                    // Second part of Stylish.
                    USER_FUNC(evtTot_LogActiveMoveStylish, 2)
                CASE_ETC()
                    USER_FUNC(evt_audience_acrobat_notry)
            END_SWITCH()
        END_BROTHER()
    END_IF()
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 30, -1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.60))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 8)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 8, -1)
LBL(95)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_W_1"))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
LBL(99)
    // Move Star Power generation to the end so Stylishes affect it.
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
    END_IF()
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(marioAttackEvent_GatsuDokaJump)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.70))
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    ELSE()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(12))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(12), LW(3), LW(4))
    END_IF()
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    RUN_CHILD_EVT(PTR(&marioAttackEvent_MajinaiPowerUpCheck))
    USER_FUNC(evt_btl_camera_set_mode, 0, 11)
    USER_FUNC(evt_btl_camera_set_homing_unitparts, 0, -2, 1, LW(3), LW(4))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
        USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
        USER_FUNC(btlevtcmd_SetupAC, -2, 1, 1, 0)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
    USER_FUNC(btlevtcmd_GetTakeoffPosition, -2, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_StartAC, 1)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_I_Y"))
    WAIT_FRM(10)
    
    // Adding star effect to signify Soft status.
    BROTHER_EVT()
        SET(LW(13), 3)
        USER_FUNC(_jump_star_effect, -2, LW(13))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_MARIO_HAMMER_SHINE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    END_BROTHER()
    
    USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(3), LW(4), 256, LW(5))
    SWITCH(LW(5))
        CASE_OR(2)
        CASE_OR(3)
        CASE_OR(6)
        CASE_OR(4)
            CASE_END()
        CASE_EQUAL(1)
        CASE_ETC()
            WAIT_MSEC(1000)
    END_SWITCH()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    ADD(LW(2), 5)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_BATTLE_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    SET(LW(6), 36)
    USER_FUNC(btlevtcmd_MarioJumpParam, -2, LW(0), LW(1), LW(2), LW(6))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        BROTHER_EVT()
            SET(LW(0), LW(6))
            SET(LW(1), LW(6))
            DIV(LW(0), 2)
            SUB(LW(1), 1)
            USER_FUNC(btlevtcmd_ACRStart, -2, 5, LW(0), LW(1), 0)
            USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
            SWITCH(LW(6))
                CASE_LARGE_EQUAL(2)
                    USER_FUNC(evtTot_LogActiveMoveStylish, 0)
                    USER_FUNC(btlevtcmd_snd_voice, -2, 1)
                    USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
                    USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 14, 0)
                    DO(15)
                        USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, -24)
                        WAIT_FRM(1)
                    WHILE()
                    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
                CASE_ETC()
                    USER_FUNC(evt_audience_acrobat_notry)
            END_SWITCH()
        END_BROTHER()
    END_IF()
    USER_FUNC(btlevtcmd_MarioJumpPosition, -2, LW(0), LW(1), LW(2), LW(6), 0, -1)
    IF_NOT_EQUAL(LW(5), 1)
        IF_EQUAL(LW(5), 3)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
        END_IF()
        IF_EQUAL(LW(5), 6)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
        END_IF()
        IF_EQUAL(LW(5), 2)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
        END_IF()
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_StopAC)
        END_IF()
        USER_FUNC(btlevtcmd_JumpContinue, -2)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_LAND_DAMAGE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_D_2"))
        WAIT_FRM(40)
        GOTO(95)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1D"))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 1, 1)
        USER_FUNC(btlevtcmd_ac_timing_get_success_frame, LW(0))
        DIV(LW(0), 2)
        WAIT_FRM(LW(0))
        USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 0, 1)
        USER_FUNC(btlevtcmd_ResultAC)
        USER_FUNC(btlevtcmd_GetResultAC, LW(6))
    ELSE()
        WAIT_FRM(4)
        SET(LW(6), 0x2)
    END_IF()

    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(12))

    IF_NOT_FLAG(LW(6), 0x2)
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 256, LW(5))
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), -1)
        USER_FUNC(btlevtcmd_GetDamage, LW(3), LW(0))
        IF_LARGE_EQUAL(LW(0), 10)
            SET(LW(0), 9)
        END_IF()
        USER_FUNC(evt_btl_camera_shake_h, 0, LW(0), 0, 10, 13)
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        USER_FUNC(btlevtcmd_WeaponAftereffect, LW(0))
        USER_FUNC(evt_btl_camera_set_mode, 0, 0)
        USER_FUNC(btlevtcmd_StopAC)
        USER_FUNC(evt_audience_ap_recovery)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetHomePos, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 27)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.60))
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 8)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 8, -1)
        GOTO(95)
    END_IF()
    USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131328, LW(5))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 0, LW(6))
        USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), LW(6))
    END_IF()
    WAIT_FRM(2)
LBL(90)
    USER_FUNC(btlevtcmd_GetDamage, LW(3), LW(0))
    IF_LARGE_EQUAL(LW(0), 10)
        SET(LW(0), 9)
    END_IF()
    USER_FUNC(evt_btl_camera_shake_h, 0, LW(0), 0, 10, 13)
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(0))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_StopAC)
        USER_FUNC(evt_audience_ap_recovery)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHomePos, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 60)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 30, -1)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.60))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 8)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 8, -1)
LBL(95)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_W_1"))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(marioAttackEvent_GatsunJump)
    SET(LW(12), PTR(&customWeapon_PowerSoftStomp))
    RUN_CHILD_EVT(PTR(&marioAttackEvent_GatsuDokaJump))
    RETURN()
EVT_END()

EVT_BEGIN(marioAttackEvent_TugiTugiJump)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.70))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    ELSE()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(12))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(12), LW(3), LW(4))
    END_IF()
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    RUN_CHILD_EVT(PTR(&marioAttackEvent_MajinaiPowerUpCheck))
    USER_FUNC(evt_btl_camera_set_mode, 0, 11)
    USER_FUNC(evt_btl_camera_set_homing_unitparts, 0, -2, 1, LW(3), LW(4))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
        USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
        USER_FUNC(btlevtcmd_SetupAC, -2, 1, 1, 0)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
    USER_FUNC(btlevtcmd_GetTakeoffPosition, -2, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_StartAC, 1)
    END_IF()
    SET(LW(10), 0)
LBL(10)
    USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(3), LW(4), 256, LW(5))
    SWITCH(LW(5))
        CASE_OR(2)
        CASE_OR(3)
        CASE_OR(6)
        CASE_OR(4)
            CASE_END()
        CASE_EQUAL(1)
        CASE_ETC()
            WAIT_MSEC(1000)
    END_SWITCH()
    USER_FUNC(btlevtcmd_SetMoveSoundDataWork, -2, PTR("SFX_BTL_JUMP_TUGITUGI1"), 0, 0, -1, -1, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    ADD(LW(2), 5)
    SWITCH(LW(10))
        CASE_EQUAL(0)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_BATTLE_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            SET(LW(8), 36)
            USER_FUNC(btlevtcmd_MarioJumpParam, -2, LW(0), LW(1), LW(2), LW(8))
            USER_FUNC(btlevtcmd_MarioJumpPosition, -2, LW(0), LW(1), LW(2), LW(8), 0, -2)
        CASE_EQUAL(1)
            SET(LW(8), 36)
            USER_FUNC(btlevtcmd_MarioJumpParam, -2, LW(0), LW(1), LW(2), LW(8))
            IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
                BROTHER_EVT()
                    WAIT_FRM(3)
                    USER_FUNC(btlevtcmd_ACRStart, -2, 10, 19, 30, 0)
                    USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
                    SWITCH(LW(6))
                        CASE_LARGE_EQUAL(2)
                            // First part of Stylish.
                            USER_FUNC(evtTot_LogActiveMoveStylish, 1)
                            USER_FUNC(evt_sub_random, 2, LW(0))
                            SWITCH(LW(0))
                                CASE_EQUAL(0)
                                    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_A_4"))
                                CASE_EQUAL(1)
                                    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_A_5"))
                                CASE_ETC()
                                    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_A_6"))
                            END_SWITCH()
                            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
                            USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, -60, -30, 0)
                        CASE_ETC()
                            USER_FUNC(evt_audience_acrobat_notry)
                    END_SWITCH()
                END_BROTHER()
            END_IF()
            USER_FUNC(btlevtcmd_MarioJumpPosition, -2, LW(0), LW(1), LW(2), LW(8), 3, -2)
        CASE_ETC()
            SET(LW(8), 36)
            USER_FUNC(btlevtcmd_MarioJumpParam, -2, LW(0), LW(1), LW(2), LW(8))
            IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
                BROTHER_EVT()
                    WAIT_FRM(3)
                    USER_FUNC(btlevtcmd_ACRStart, -2, 10, 19, 30, 0)
                    USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
                    SWITCH(LW(6))
                        CASE_LARGE_EQUAL(2)
                            // Second part of Stylish.
                            USER_FUNC(evtTot_LogActiveMoveStylish, 2)
                            USER_FUNC(evt_sub_random, 2, LW(0))
                            SWITCH(LW(0))
                                CASE_EQUAL(0)
                                    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_A_4"))
                                CASE_EQUAL(1)
                                    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_A_5"))
                                CASE_ETC()
                                    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_A_6"))
                            END_SWITCH()
                            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
                            USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, -60, -30, 0)
                        CASE_ETC()
                            USER_FUNC(evt_audience_acrobat_notry)
                    END_SWITCH()
                END_BROTHER()
            END_IF()
            USER_FUNC(btlevtcmd_MarioJumpPosition, -2, LW(0), LW(1), LW(2), LW(8), 4, -2)
    END_SWITCH()
    IF_NOT_EQUAL(LW(5), 1)
        IF_EQUAL(LW(5), 3)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
        END_IF()
        IF_EQUAL(LW(5), 6)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
        END_IF()
        IF_EQUAL(LW(5), 2)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
        END_IF()
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_StopAC)
        END_IF()
        USER_FUNC(btlevtcmd_JumpContinue, -2)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_LAND_DAMAGE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_D_2"))
        WAIT_FRM(40)
        GOTO(95)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1D"))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 1, 1)
        USER_FUNC(btlevtcmd_ac_timing_get_success_frame, LW(0))
        DIV(LW(0), 2)
        WAIT_FRM(LW(0))
        USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 0, 1)
        USER_FUNC(btlevtcmd_ResultAC)
        USER_FUNC(btlevtcmd_GetResultAC, LW(6))
    ELSE()
        WAIT_FRM(4)
        SET(LW(6), 0x2)
    END_IF()
    
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(12))

    IF_NOT_FLAG(LW(6), 0x2)
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 256, LW(5))
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        IF_EQUAL(LW(10), 0)
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), -1)
        ELSE()
            SET(LW(6), LW(10))
            SUB(LW(6), 1)
            USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), LW(6), LW(6))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), LW(6))
        END_IF()
        USER_FUNC(evt_btl_camera_set_mode, 0, 0)
        USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
        USER_FUNC(btlevtcmd_StopAC)
        USER_FUNC(evt_audience_ap_recovery)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        USER_FUNC(btlevtcmd_WeaponAftereffect, LW(0))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetHomePos, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 27)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.60))
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 8)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 8, -1)
        GOTO(95)
    END_IF()
    USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131328, LW(5))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetResultAC, LW(0))
        IF_FLAG(LW(0), 0x2)
            USER_FUNC(btlevtcmd_snd_voice, -2, 2)
            USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), LW(10), LW(6))
            USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
        END_IF()
    ELSE()
        USER_FUNC(btlevtcmd_snd_voice, -2, 2)
    END_IF()
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        INLINE_EVT()
            USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
            USER_FUNC(evt_btl_camera_set_zoomSpeedLv, 0, 0)
            USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
            WAIT_FRM(10)
            USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
            USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
        END_INLINE()
        WAIT_FRM(2)
        ADD(LW(10), 1)
        INLINE_EVT()
            WAIT_FRM(10)
            USER_FUNC(evt_btl_camera_set_homing_unitparts, 0, -2, 1, LW(3), LW(4))
            USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
        END_INLINE()
        GOTO(10)
    END_IF()
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), LW(10), LW(6))
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), LW(6))
    END_IF()
LBL(90)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_StopAC)
        USER_FUNC(evt_audience_ap_recovery)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
    END_IF()
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(0))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHomePos, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 60)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 30, -1)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.60))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 8)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 8, -1)
LBL(95)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_W_1"))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(marioAttackEvent_RenzokuJump)
    SET(LW(11), 0)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.70))
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    ELSE()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(12))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(12), LW(3), LW(4))
    END_IF()
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    RUN_CHILD_EVT(PTR(&marioAttackEvent_MajinaiPowerUpCheck))
    USER_FUNC(evt_btl_camera_set_mode, 0, 11)
    USER_FUNC(evt_btl_camera_set_homing_unitparts, 0, -2, 1, LW(3), LW(4))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
        USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
        USER_FUNC(btlevtcmd_SetupAC, -2, 1, 1, 0)
        USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 1, 2)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
    USER_FUNC(btlevtcmd_GetTakeoffPosition, -2, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_StartAC, 1)
    END_IF()
LBL(10)
    IF_LARGE_EQUAL(LW(11), 1)
        USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(3), LW(4), 1048832, LW(5))
    ELSE()
        USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(3), LW(4), 256, LW(5))
    END_IF()
    SWITCH(LW(5))
        CASE_OR(2)
        CASE_OR(3)
        CASE_OR(6)
        CASE_OR(4)
            CASE_END()
        CASE_EQUAL(1)
        CASE_ETC()
            WAIT_MSEC(1000)
    END_SWITCH()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    ADD(LW(2), 5)
    IF_LARGE_EQUAL(LW(11), 1)
        SET(LW(8), 36)
        USER_FUNC(btlevtcmd_MarioJumpParam, -2, LW(0), LW(1), LW(2), LW(8))
        SWITCH(LW(11))
            CASE_EQUAL(1)
                SET(LW(6), PTR("SFX_BTL_JUMP_CONTINUE2"))
            CASE_EQUAL(2)
                SET(LW(6), PTR("SFX_BTL_JUMP_CONTINUE3"))
            CASE_EQUAL(3)
                SET(LW(6), PTR("SFX_BTL_JUMP_CONTINUE4"))
            CASE_EQUAL(4)
                SET(LW(6), PTR("SFX_BTL_JUMP_CONTINUE5"))
            CASE_EQUAL(5)
                SET(LW(6), PTR("SFX_BTL_JUMP_CONTINUE6"))
            CASE_EQUAL(6)
                SET(LW(6), PTR("SFX_BTL_JUMP_CONTINUE7"))
            CASE_EQUAL(7)
                SET(LW(6), PTR("SFX_BTL_JUMP_CONTINUE8"))
            CASE_EQUAL(8)
                SET(LW(6), PTR("SFX_BTL_JUMP_CONTINUE9"))
            CASE_ETC()
                SET(LW(6), PTR("SFX_BTL_JUMP_CONTINUE10"))
        END_SWITCH()
        USER_FUNC(btlevtcmd_SetMoveSoundDataWork, -2, LW(6), 0, 0, -1, -1, 0, 0)
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            BROTHER_EVT()
                WAIT_FRM(3)
                USER_FUNC(btlevtcmd_ACRStart, -2, 10, 17, 30, 0)
                USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
                SWITCH(LW(6))
                    CASE_LARGE_EQUAL(2)
                        USER_FUNC(evtTot_LogActiveMoveStylish, 0)
                        USER_FUNC(evt_sub_random, 2, LW(0))
                        SWITCH(LW(0))
                            CASE_EQUAL(0)
                                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_A_4"))
                            CASE_EQUAL(1)
                                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_A_5"))
                            CASE_ETC()
                                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_A_6"))
                        END_SWITCH()
                        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
                        USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, -60, -30, 0)
                        USER_FUNC(btlevtcmd_snd_voice, -2, 1)
                    CASE_ETC()
                        USER_FUNC(evt_audience_acrobat_notry)
                END_SWITCH()
            END_BROTHER()
        END_IF()
        USER_FUNC(btlevtcmd_MarioJumpPosition, -2, LW(0), LW(1), LW(2), LW(8), 3, -2)
    ELSE()
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_BATTLE_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_SetMoveSoundDataWork, -2, PTR("SFX_BTL_JUMP_CONTINUE1"), 0, 0, -1, -1, 0, 0)
        SET(LW(8), 36)
        USER_FUNC(btlevtcmd_MarioJumpParam, -2, LW(0), LW(1), LW(2), LW(8))
        USER_FUNC(btlevtcmd_MarioJumpPosition, -2, LW(0), LW(1), LW(2), LW(8), 0, -2)
    END_IF()
    IF_NOT_EQUAL(LW(5), 1)
        IF_EQUAL(LW(5), 3)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
        END_IF()
        IF_EQUAL(LW(5), 6)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
        END_IF()
        IF_EQUAL(LW(5), 2)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
        END_IF()
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_StopAC)
        END_IF()
        USER_FUNC(btlevtcmd_JumpContinue, -2)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_LAND_DAMAGE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_D_2"))
        WAIT_FRM(40)
        GOTO(95)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1D"))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 1, 1)
        USER_FUNC(btlevtcmd_ac_timing_get_success_frame, LW(0))
        DIV(LW(0), 2)
        WAIT_FRM(LW(0))
        USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 0, 1)
        USER_FUNC(btlevtcmd_ResultAC)
        USER_FUNC(btlevtcmd_GetResultAC, LW(6))
    ELSE()
        SET(LW(0), 9)
        SUB(LW(0), LW(11))
        DIV(LW(0), 2)
        IF_LARGE(LW(0), 0)
            WAIT_FRM(LW(0))
        END_IF()
        SET(LW(6), 0x2)
    END_IF()

    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(12))

    IF_NOT_FLAG(LW(6), 0x2)
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 256, LW(5))
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        IF_EQUAL(LW(11), 0)
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), -1)
        ELSE()
            SET(LW(6), LW(11))
            SUB(LW(6), 1)
            USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), LW(6), LW(6))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), LW(6))
        END_IF()
        USER_FUNC(btlevtcmd_StopAC)
        USER_FUNC(evt_btl_camera_set_mode, 0, 0)
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        USER_FUNC(btlevtcmd_WeaponAftereffect, LW(0))
        USER_FUNC(evt_audience_ap_recovery)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetHomePos, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 27)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.60))
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 8)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 8, -1)
        GOTO(95)
    END_IF()
    INLINE_EVT()
        USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 0)
        USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
        WAIT_FRM(10)
        USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
        USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    END_INLINE()
    USER_FUNC(btlevtcmd_snd_voice, -2, 2)

    // Add to current Power Bounce count + check for cap.
    ADD(LW(11), 1)
    USER_FUNC(_record_renzoku_count, LW(11))
    USER_FUNC(mario_get_renzoku_count_max, LW(3), LW(11), LW(7))
    IF_SMALL(LW(11), LW(7))
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131072, LW(5))
    ELSE()
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131328, LW(5))
    END_IF()

    // Save Power Bounce record to GSW.
    SET(LW(1), GSW(32))
    SET(LW(2), GSW(35))
    MUL(LW(2), 256)
    ADD(LW(1), LW(2))
    IF_LARGE(LW(11), LW(1))
        SET(LW(0), LW(11))
        IF_LARGE(LW(0), 9999)
            SET(LW(0), 9999)
        END_IF()
        SET(LW(1), LW(0))
        MOD(LW(1), 256)
        SET(LW(2), LW(0))
        DIV(LW(2), 256)
        SET(GSW(32), LW(1))
        SET(GSW(35), LW(2))
    END_IF()

    // Handle Action Command results + starting next bounce.
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        SET(LW(0), LW(11))
        SUB(LW(0), 1)
        USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), LW(0), LW(6))
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), LW(6))
        IF_SMALL(LW(11), LW(7))
            WAIT_FRM(2)
            USER_FUNC(btlevtcmd_StartAC, 0)
            GOTO(10)
        END_IF()
    ELSE()
        IF_SMALL(LW(11), LW(7))
            WAIT_FRM(2)
            GOTO(10)
        END_IF()
    END_IF()

LBL(90)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(0))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_StopAC)
        USER_FUNC(evt_audience_ap_recovery)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHomePos, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 60)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 30, -1)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.60))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 8)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 8, -1)
LBL(95)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_W_1"))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(marioAttackEvent_TatsumakiJump)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.70))
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    ELSE()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(12))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(12), LW(3), LW(4))
    END_IF()
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    RUN_CHILD_EVT(PTR(&marioAttackEvent_MajinaiPowerUpCheck))
    USER_FUNC(evt_btl_camera_set_mode, 0, 11)
    USER_FUNC(evt_btl_camera_set_homing_unitparts, 0, -2, 1, LW(3), LW(4))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
        USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
        USER_FUNC(btlevtcmd_SetupAC, -2, 1, 1, 0)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
    USER_FUNC(btlevtcmd_GetTakeoffPosition, -2, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetMoveFrame, -2, LW(0), LW(1), LW(2), 0, LW(7))
    SUB(LW(7), 21)
    IF_SMALL(LW(7), 0)
        MUL(LW(7), -1)
        WAIT_FRM(LW(7))
    END_IF()
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_StartAC, 1)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_I_Y"))
    WAIT_FRM(10)
    USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(3), LW(4), 256, LW(5))
    SWITCH(LW(5))
        CASE_OR(2)
        CASE_OR(3)
        CASE_OR(6)
        CASE_OR(4)
            CASE_END()
        CASE_EQUAL(1)
        CASE_ETC()
            WAIT_MSEC(1000)
    END_SWITCH()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    ADD(LW(2), 5)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_BATTLE_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    SET(LW(6), 36)
    USER_FUNC(btlevtcmd_MarioJumpParam, -2, LW(0), LW(1), LW(2), LW(6))
    USER_FUNC(btlevtcmd_MarioJumpPosition, -2, LW(0), LW(1), LW(2), LW(6), 0, -1)
    IF_NOT_EQUAL(LW(5), 1)
        IF_EQUAL(LW(5), 3)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
        END_IF()
        IF_EQUAL(LW(5), 6)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
        END_IF()
        IF_EQUAL(LW(5), 2)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
        END_IF()
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_StopAC)
        END_IF()
        USER_FUNC(btlevtcmd_JumpContinue, -2)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_LAND_DAMAGE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_D_2"))
        WAIT_FRM(40)
        GOTO(95)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1D"))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 1, 1)
        USER_FUNC(btlevtcmd_ac_timing_get_success_frame, LW(0))
        DIV(LW(0), 2)
        WAIT_FRM(LW(0))
        USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 0, 1)
        USER_FUNC(btlevtcmd_ResultAC)
        USER_FUNC(btlevtcmd_GetResultAC, LW(6))
    ELSE()
        WAIT_FRM(4)
        SET(LW(6), 0x2)
    END_IF()

    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(12))

    IF_NOT_FLAG(LW(6), 0x2)
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 256, LW(5))
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), -1)
        GOTO(90)
    END_IF()
    INLINE_EVT()
        USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 0)
        USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
        WAIT_FRM(10)
        USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
        USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    END_INLINE()
    USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131328, LW(5))
    USER_FUNC(btlevtcmd_GetFriendBelong, LW(3), LW(15))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetResultAC, LW(0))
        IF_FLAG(LW(0), 0x2)
            USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 0, LW(6))
            USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
        END_IF()
        WAIT_FRM(2)
        USER_FUNC(btlevtcmd_StopAC)
    ELSE()
        WAIT_FRM(2)
    END_IF()
    INLINE_EVT_ID(LW(13))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.15))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_MarioJumpPosition, -2, LW(0), LW(1), LW(2), 80, 3, -1)
    END_INLINE()
    BROTHER_EVT()
        DO(24)
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 15, 0)
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    WAIT_FRM(27)
    DELETE_EVT(LW(13))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
        USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
        USER_FUNC(btlevtcmd_AcGetDifficulty, LW(0))
        SUB(LW(0), 3)
        SWITCH(LW(0))
            CASE_SMALL_EQUAL(-3)
                USER_FUNC(btlevtcmd_ftof, 600, LW(0))
            CASE_EQUAL(-2)
                USER_FUNC(btlevtcmd_ftof, 480, LW(0))
            CASE_EQUAL(-1)
                USER_FUNC(btlevtcmd_ftof, 420, LW(0))
            CASE_EQUAL(0)
                USER_FUNC(btlevtcmd_ftof, 360, LW(0))
            CASE_EQUAL(1)
                USER_FUNC(btlevtcmd_ftof, 300, LW(0))
            CASE_EQUAL(2)
                USER_FUNC(btlevtcmd_ftof, 200, LW(0))
            CASE_ETC()
                USER_FUNC(btlevtcmd_ftof, 100, LW(0))
        END_SWITCH()
        USER_FUNC(btlevtcmd_AcSetParamAll, LW(0), 1, 3, -4, 3, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AcSetFlag, 0)
        USER_FUNC(btlevtcmd_SetupAC, -2, 10, 1, 0)
        WAIT_FRM(1)
        USER_FUNC(btlevtcmd_StartAC, 1)
    END_IF()
    USER_FUNC(btlevtcmd_SetUnitWork, -3, 5, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -3, 6, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -3, 7, 0)
    INLINE_EVT_ID(LW(13))
        USER_FUNC(btlevtcmd_SetUnitWork, -2, 2, 1)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, 3, 30)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, 4, 0)
        SET(LW(0), 15)
LBL(60)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, 2, LW(1))
        IF_EQUAL(LW(1), 0)
            GOTO(61)
        END_IF()
        USER_FUNC(btlevtcmd_GetUnitWork, -2, 3, LW(10))
        ADD(LW(0), 1)
        IF_LARGE(LW(0), LW(10))
            SET(LW(0), LW(10))
        END_IF()
        USER_FUNC(btlevtcmd_SetUnitWork, -2, 4, LW(0))
        USER_FUNC(btlevtcmd_AddRotate, -2, 0, LW(0), 0)
        WAIT_FRM(1)
        GOTO(60)
LBL(61)
    END_INLINE()
    BROTHER_EVT_ID(LW(14))
        SET(LW(0), 0)
        DO(1000)
            WAIT_FRM(1)
            IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
                USER_FUNC(btlevtcmd_AcGetOutputParam, 0, LW(1))
            ELSE()
                SET(LW(1), 3)
            END_IF()
            IF_NOT_EQUAL(LW(1), LW(0))
                SET(LW(0), LW(1))
                SWITCH(LW(0))
                    CASE_EQUAL(0)
                    CASE_EQUAL(1)
                        USER_FUNC(btlevtcmd_SetUnitWork, -3, 5, 1023)
                        USER_FUNC(btlevtcmd_GetUnitWork, -3, 6, LW(10))
                        SUB(LW(10), 1023)
                        MUL(LW(10), -1)
                        DIV(LW(10), 10)
                        USER_FUNC(btlevtcmd_SetUnitWork, -3, 7, LW(10))
                    CASE_EQUAL(2)
                        USER_FUNC(btlevtcmd_SetUnitWork, -2, 3, 45)
                        USER_FUNC(btlevtcmd_SetUnitWork, -3, 5, 2046)
                        USER_FUNC(btlevtcmd_GetUnitWork, -3, 6, LW(10))
                        SUB(LW(10), 2046)
                        MUL(LW(10), -1)
                        DIV(LW(10), 10)
                        USER_FUNC(btlevtcmd_SetUnitWork, -3, 7, LW(10))
                    CASE_EQUAL(3)
                        USER_FUNC(btlevtcmd_SetUnitWork, -3, 5, 3069)
                        USER_FUNC(btlevtcmd_GetUnitWork, -3, 6, LW(10))
                        SUB(LW(10), 3069)
                        MUL(LW(10), -1)
                        DIV(LW(10), 10)
                        USER_FUNC(btlevtcmd_SetUnitWork, -3, 7, LW(10))
                    CASE_ETC()
                        USER_FUNC(btlevtcmd_SetUnitWork, -2, 3, 60)
                END_SWITCH()
            END_IF()
        WHILE()
    END_BROTHER()
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_JUMP_TATUMAKI1"), LW(11))
    BROTHER_EVT_ID(LW(12))
        SET(LW(5), 5)
        DO(LW(5))
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            USER_FUNC(evt_snd_sfx_pos, LW(11), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_GetUnitWork, -3, 5, LW(1))
            USER_FUNC(btlevtcmd_GetUnitWork, -3, 6, LW(0))
            IF_NOT_EQUAL(LW(0), LW(1))
                USER_FUNC(btlevtcmd_GetUnitWork, -3, 7, LW(2))
                ADD(LW(0), LW(2))
                IF_LARGE_EQUAL(LW(2), 0)
                    IF_LARGE_EQUAL(LW(0), LW(1))
                        SET(LW(0), LW(1))
                    END_IF()
                ELSE()
                    IF_SMALL_EQUAL(LW(0), LW(1))
                        SET(LW(0), LW(1))
                    END_IF()
                END_IF()
                USER_FUNC(btlevtcmd_SetUnitWork, -3, 6, LW(0))
                USER_FUNC(evt_snd_sfx_pit, LW(11), LW(0))
            END_IF()
            WAIT_FRM(1)
            SET(LW(5), 5)
        WHILE()
    END_BROTHER()
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_ResultAC)
        WAIT_FRM(1)
        USER_FUNC(btlevtcmd_GetResultAC, LW(6))
    ELSE()
        SET(LW(6), 0x2)
    END_IF()
    IF_NOT_FLAG(LW(6), 0x2)
        DELETE_EVT(LW(14))
        BROTHER_EVT()
            USER_FUNC(btlevtcmd_SetUnitWork, -3, 5, -2728)
            USER_FUNC(btlevtcmd_GetUnitWork, -3, 6, LW(10))
            SUB(LW(10), -2728)
            MUL(LW(10), -1)
            DIV(LW(10), 40)
            USER_FUNC(btlevtcmd_SetUnitWork, -3, 7, LW(10))
        END_BROTHER()
        BROTHER_EVT()
            USER_FUNC(btlevtcmd_GetRotate, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
            DO(40)
                USER_FUNC(btlevtcmd_GetUnitWork, -2, 3, LW(0))
                IF_LARGE(LW(0), 10)
                    SUB(LW(0), 1)
                    USER_FUNC(btlevtcmd_SetUnitWork, -2, 3, LW(0))
                END_IF()
                USER_FUNC(btlevtcmd_GetRotate, -2, EVT_NULLPTR, LW(0), EVT_NULLPTR)
                IF_SMALL(LW(1), 90)
                    IF_LARGE_EQUAL(LW(0), 90)
                        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_N_7"))
                        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_MARIO_FALL_CRUSH1"), EVT_NULLPTR, 0, EVT_NULLPTR)
                    END_IF()
                END_IF()
                SET(LW(1), LW(0))
                WAIT_FRM(1)
            WHILE()
        END_BROTHER()
        WAIT_FRM(40)
        DELETE_EVT(LW(13))
        DELETE_EVT(LW(12))
        USER_FUNC(evt_snd_sfxoff, LW(11))
        BROTHER_EVT_ID(LW(13))
            DO(60)
                USER_FUNC(btlevtcmd_GetRotate, -2, EVT_NULLPTR, LW(0), EVT_NULLPTR)
                USER_FUNC(btlevtcmd_GetUnitWork, -2, 3, LW(1))
                ADD(LW(0), LW(1))
                IF_LARGE_EQUAL(LW(0), 360)
                    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
                    DO_BREAK()
                END_IF()
                USER_FUNC(btlevtcmd_SetRotate, -2, 0, LW(0), 0)
                WAIT_FRM(1)
            WHILE()
        END_BROTHER()
        USER_FUNC(btlevtcmd_WaitEventEnd, LW(13))
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        WAIT_FRM(60)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_JumpSetting, -2, 40, FLOAT(0.0), FLOAT(0.10))
        USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
        BROTHER_EVT()
            WAIT_FRM(12)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_LAND_DAMAGE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_D_2"))
        END_BROTHER()
        USER_FUNC(btlevtcmd_FallPosition, -2, LW(0), LW(1), LW(2), 16)
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), -1)
        WAIT_FRM(32)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_U_3"))
        WAIT_FRM(12)
        USER_FUNC(evt_audience_ap_recovery)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
        GOTO(95)
    ELSE()
        DELETE_EVT(LW(14))
        USER_FUNC(btlevtcmd_SetUnitWork, -2, 3, 45)
LBL(62)
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_GetUnitWork, -2, 4, LW(0))
            IF_SMALL(LW(0), 44)
                WAIT_FRM(1)
                GOTO(62)
            END_IF()
        ELSE()
            WAIT_FRM(20)
        END_IF()
        // Clear fog before tornadoes hit.
        BROTHER_EVT()
            WAIT_FRM(30)
            USER_FUNC(btlevtcmd_StageDispellFog)
        END_BROTHER()
        BROTHER_EVT()
            SET(LW(6), LW(3))
            USER_FUNC(_tatsumaki_effect, -2)
            SET(LW(0), LW(15))
            USER_FUNC(btlevtcmd_SamplingEnemy, -5, LW(0), PTR(&customWeapon_TornadoJumpRecoil))
            USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
LBL(50)
            IF_EQUAL(LW(3), -1)
                GOTO(59)
            END_IF()
            IF_EQUAL(LW(3), LW(6))
                GOTO(58)
            END_IF()
            USER_FUNC(btlevtcmd_PreCheckDamage, -5, LW(3), LW(4), PTR(&customWeapon_TornadoJumpRecoil), 256, LW(5))
            SWITCH(LW(5))
                CASE_EQUAL(2)
                    USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
                CASE_EQUAL(3)
                    USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
                CASE_EQUAL(6)
                    USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
                CASE_EQUAL(4)
                CASE_EQUAL(1)
                    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
                        USER_FUNC(btlevtcmd_GetResultAC, LW(0))
                        IF_FLAG(LW(0), 0x2)
                            USER_FUNC(btlevtcmd_CheckDamage, -5, LW(3), LW(4), PTR(&customWeapon_TornadoJumpRecoil), 131328, LW(5))
                            USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
                            USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 1, LW(7))
                            USER_FUNC(btlevtcmd_ACSuccessEffect, LW(7), LW(0), LW(1), LW(2))
                        ELSE()
                            USER_FUNC(btlevtcmd_CheckDamage, -5, LW(3), LW(4), PTR(&customWeapon_TornadoJumpRecoil), 256, LW(5))
                        END_IF()
                    ELSE()
                        USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&customWeapon_TornadoJumpRecoil))
                        USER_FUNC(btlevtcmd_CheckDamage, -5, LW(3), LW(4), PTR(&customWeapon_TornadoJumpRecoil), 131328, LW(5))
                    END_IF()
                CASE_ETC()
            END_SWITCH()
LBL(58)
            USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
            GOTO(50)
LBL(59)
            IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
                USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
                USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 1, LW(6))
                USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), LW(6))
            END_IF()
        END_BROTHER()
        WAIT_FRM(60)
        INLINE_EVT_ID(LW(13))
            DO(30)
                USER_FUNC(btlevtcmd_GetUnitWork, -2, 3, LW(0))
                SUB(LW(0), 1)
                USER_FUNC(btlevtcmd_SetUnitWork, -2, 3, LW(0))
                WAIT_FRM(1)
            END_IF()
            USER_FUNC(btlevtcmd_SetUnitWork, -2, 2, 0)
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
            DO(6)
                WAIT_FRM(1)
                USER_FUNC(btlevtcmd_AddRotate, -2, 0, 45, 0)
            WHILE()
            DO(6)
                WAIT_FRM(1)
                USER_FUNC(btlevtcmd_AddRotate, -2, 0, 15, 0)
            WHILE()
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        END_INLINE()
        USER_FUNC(btlevtcmd_WaitEventEnd, LW(13))
        DELETE_EVT(LW(12))
        USER_FUNC(evt_snd_sfxoff, LW(11))
        WAIT_FRM(30)
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            BROTHER_EVT()
                USER_FUNC(btlevtcmd_ACRStart, -2, 7, 21, 21, 0)
                USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
                IF_LARGE_EQUAL(LW(6), 2)
                    // First part of Stylish.
                    USER_FUNC(evtTot_LogActiveMoveStylish, 1)
                    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_B_1"))
                END_IF()
            END_BROTHER()
        END_IF()
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.15))
        USER_FUNC(btlevtcmd_FallPosition, -2, LW(0), LW(1), LW(2), 16)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1D"))
        WAIT_FRM(6)
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
            IF_LARGE_EQUAL(LW(6), 2)
                GOTO(80)
            END_IF()
        END_IF()
    END_IF()
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(evt_audience_ap_recovery)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
    END_IF()
    GOTO(95)
LBL(80)
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
    USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, 0, 0, 0)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetMoveFrame, -2, LW(0), LW(1), LW(2), 0, LW(7))
    SET(LW(8), LW(7))
    ADD(LW(8), 12)
    BROTHER_EVT()
        WAIT_FRM(1)
        USER_FUNC(btlevtcmd_ACRStart, -2, LW(7), LW(8), LW(8), 0)
    END_BROTHER()
    ADD(LW(8), 1)
    USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(15))
    MUL(LW(15), 30)
    USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 14, 0)
    BROTHER_EVT()
        SET(LW(10), 0)
        DO(LW(8))
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, LW(15))
            IF_EQUAL(LW(10), 0)
                USER_FUNC(btlevtcmd_snd_voice, -2, 1)
            END_IF()
            ADD(LW(10), 1)
            IF_LARGE_EQUAL(LW(10), 12)
                SET(LW(10), 0)
            END_IF()
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), LW(8), -1, 0)
    USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(5))
    MUL(LW(5), 36)
    INLINE_EVT_ID(LW(14))
        USER_FUNC(btlevtcmd_GetRotate, -2, LW(0), LW(1), LW(2))
        MOD(LW(2), 360)
        DIV(LW(2), 10)
        ADD(LW(2), LW(5))
        DO(10)
        WHILE()
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    END_INLINE()
    USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
    SWITCH(LW(6))
        CASE_LARGE_EQUAL(2)
            // Second part of Stylish.
            USER_FUNC(evtTot_LogActiveMoveStylish, 2)
            USER_FUNC(btlevtcmd_WaitEventEnd, LW(14))
            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
            USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, 0, 0, 0)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_A_3B"))
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            ADD(LW(1), 50)
            USER_FUNC(evt_eff, PTR("_BTLEF"), PTR("confetti"), 3, LW(0), LW(1), LW(2), 90, 0, 0, 0, 0, 0, 0, 0)
            USER_FUNC(btlevtcmd_snd_voice, -2, 1)
            USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_ACROBAT_FINISH1"), 0)
            WAIT_FRM(90)
        CASE_SMALL_EQUAL(-1)
            DELETE_EVT(LW(14))
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
            USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 0, 0, 0, 0)
            USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_ACROBAT_MISS1"), 0)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_I_S"))
            WAIT_MSEC(1500)
        CASE_ETC()
            USER_FUNC(btlevtcmd_WaitEventEnd, LW(14))
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
            USER_FUNC(evt_audience_acrobat_notry)
    END_SWITCH()
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    GOTO(99)
LBL(90)
    USER_FUNC(btlevtcmd_StopAC)
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(0))
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_J_1B"))
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHomePos, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 27)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.60))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 8)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 8, -1)
LBL(95)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_W_1"))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(marioAttackEvent_NormalHammer)
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        SET(LW(12), PTR(&customWeapon_Hammer))
        USER_FUNC(btlevtcmd_GetFirstAttackTarget, LW(3), LW(4))
        IF_EQUAL(LW(3), -1)
            GOTO(99)
        END_IF()
        USER_FUNC(evt_btl_camera_set_mode, 0, 7)
        USER_FUNC(evt_btl_camera_set_homing_unit, 0, LW(3), -1)
        USER_FUNC(evt_btl_camera_set_zoom, 0, 300)
        USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
        USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 250)
        USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
        WAIT_FRM(30)
    ELSE()
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
        ELSE()
            USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
            USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(12))
            USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(12), LW(3), LW(4))
        END_IF()
        IF_EQUAL(LW(3), -1)
            GOTO(99)
        END_IF()
        USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
        USER_FUNC(btlevtcmd_WaitGuardMove)
        USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(12))
        USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
        RUN_CHILD_EVT(PTR(&marioAttackEvent_MajinaiPowerUpCheck))
        USER_FUNC(evt_btl_camera_set_mode, 0, 11)
        USER_FUNC(evt_btl_camera_set_homing_unitparts, 0, -2, 1, LW(3), LW(4))
        USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
        USER_FUNC(evt_btl_camera_set_zoom, 0, 300)
    END_IF()
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_EQUAL(LW(0), 0)
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_GetWeaponActionLv, LW(12), LW(0))
            USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
            SWITCH(LW(12))
                CASE_EQUAL(PTR(&customWeapon_PowerPiercingSmash))
                    USER_FUNC(btlevtcmd_AcSetParamAll, 4, 40, 0, 0, 0, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
                CASE_ETC()
                    USER_FUNC(btlevtcmd_AcSetParamAll, 4, 25, 0, 0, 0, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
            END_SWITCH()
            USER_FUNC(btlevtcmd_AcSetFlag, 0)
            USER_FUNC(btlevtcmd_SetupAC, -2, 3, 1, 0)
        END_IF()
    END_IF()
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 250)
        USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
        WAIT_MSEC(1000)
        USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(5.0))
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetPos, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 35)
        USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(5.0))
        ADD(LW(2), 5)
        USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
        USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
        USER_FUNC(btlevtcmd_GetMoveFrame, -2, LW(0), LW(1), LW(2), 0, LW(7))
        IF_LARGE(LW(7), 22)
            SUB(LW(7), 22)
        ELSE()
            SUB(LW(7), 22)
            MUL(LW(7), -1)
        END_IF()
        BROTHER_EVT()
            WAIT_FRM(LW(7))
            USER_FUNC(btlevtcmd_GetWeaponActionLv, LW(12), LW(0))
            USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
            SWITCH(LW(12))
                CASE_EQUAL(PTR(&customWeapon_PowerPiercingSmash))
                    USER_FUNC(btlevtcmd_AcSetParamAll, 4, 40, 0, 0, 0, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
                CASE_ETC()
                    USER_FUNC(btlevtcmd_AcSetParamAll, 4, 25, 0, 0, 0, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
            END_SWITCH()
            USER_FUNC(btlevtcmd_AcSetFlag, 0)
            USER_FUNC(btlevtcmd_SetupAC, -2, 3, 1, 0)
        END_BROTHER()
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    ELSE()
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetPos, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 35)
        USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
        ADD(LW(2), 5)
        USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
        USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    END_IF()
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(6))
    IF_EQUAL(LW(6), 5)
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_StopAC)
        END_IF()
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
        RETURN()
    END_IF()
    IF_EQUAL(LW(12), PTR(&customWeapon_PowerPiercingSmash))
        USER_FUNC(_get_mario_hammer_lv, LW(0))
        SWITCH(LW(0))
            CASE_EQUAL(2)
                SET(LW(0), PTR("M_H_5A"))
                SET(LW(1), PTR("M_H_5B"))
                SET(LW(2), PTR("M_H_14"))
            CASE_EQUAL(3)
                SET(LW(0), PTR("M_H_9A"))
                SET(LW(1), PTR("M_H_9B"))
                SET(LW(2), PTR("M_H_15"))
            CASE_ETC()
                SET(LW(0), PTR("M_H_1A"))
                SET(LW(1), PTR("M_H_1B"))
                SET(LW(2), PTR("M_H_13"))
        END_SWITCH()
    ELSE()
        USER_FUNC(_get_mario_hammer_lv, LW(0))
        SWITCH(LW(0))
            CASE_EQUAL(2)
                SET(LW(0), PTR("M_H_5A"))
                SET(LW(1), PTR("M_H_5B"))
                SET(LW(2), PTR("M_H_5C"))
            CASE_EQUAL(3)
                SET(LW(0), PTR("M_H_9A"))
                SET(LW(1), PTR("M_H_9B"))
                SET(LW(2), PTR("M_H_9C"))
            CASE_ETC()
                SET(LW(0), PTR("M_H_1A"))
                SET(LW(1), PTR("M_H_1B"))
                SET(LW(2), PTR("M_H_1C"))
        END_SWITCH()
    END_IF()
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 350)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_POWER2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, LW(0))
    USER_FUNC(_get_mario_hammer_lv, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL(2)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_MARIO_HAMMER_HOLD2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        CASE_EQUAL(1)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_MARIO_HAMMER_HOLD1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        CASE_ETC()
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_MARIO_HAMMER_HOLD3"), EVT_NULLPTR, 0, EVT_NULLPTR)
    END_SWITCH()
    BROTHER_EVT()
        SET(LW(13), -1)
        SWITCH(LW(12))
            CASE_EQUAL(PTR(&customWeapon_ShrinkSmash))
                SET(LW(13), 4)
            CASE_EQUAL(PTR(&customWeapon_IceSmash))
                SET(LW(13), 1)
            CASE_EQUAL(PTR(&customWeapon_PowerPiercingSmash))
                SET(LW(13), 2)
        END_SWITCH()
        IF_NOT_EQUAL(LW(13), -1)
            USER_FUNC(btlevtcmd_ftomsec, 9, LW(0))
            WAIT_MSEC(LW(0))
            USER_FUNC(_hammer_star_effect, -2, LW(13))
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_MARIO_HAMMER_SHINE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        END_IF()
    END_BROTHER()
    USER_FUNC(btlevtcmd_ftomsec, 30, LW(0))
    WAIT_MSEC(LW(0))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, LW(1))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_StartAC, 0)
        USER_FUNC(btlevtcmd_ResultAC)
    ELSE()
        WAIT_FRM(60)
    END_IF()
    USER_FUNC(_get_mario_hammer_lv, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL(2)
            USER_FUNC(evt_snd_sfxon, PTR("SFX_MARIO_HAMMER_SWING2"), 0)
        CASE_EQUAL(1)
            USER_FUNC(evt_snd_sfxon, PTR("SFX_MARIO_HAMMER_SWING1"), 0)
        CASE_ETC()
            USER_FUNC(evt_snd_sfxon, PTR("SFX_MARIO_HAMMER_SWING3"), 0)
    END_SWITCH()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, LW(2))
    WAIT_FRM(7)
    IF_EQUAL(LW(12), PTR(&customWeapon_PowerPiercingSmash))
        USER_FUNC(evt_snd_sfxon, PTR("SFX_MARIO_HAMMER_TURANUKI1"), 0)
    ELSE()
        USER_FUNC(_get_mario_hammer_lv, LW(0))
        SWITCH(LW(0))
            CASE_EQUAL(2)
                USER_FUNC(evt_snd_sfxon, PTR("SFX_MARIO_HAMMER_HIT_GROUND3"), 0)
            CASE_EQUAL(1)
                USER_FUNC(evt_snd_sfxon, PTR("SFX_MARIO_HAMMER_HIT_GROUND2"), 0)
            CASE_ETC()
                USER_FUNC(evt_snd_sfxon, PTR("SFX_MARIO_HAMMER_HIT_GROUND1"), 0)
        END_SWITCH()
    END_IF()
    IF_NOT_EQUAL(LW(6), 1)
        IF_EQUAL(LW(6), 3)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
        END_IF()
        IF_EQUAL(LW(6), 6)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
        END_IF()
        IF_EQUAL(LW(6), 2)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
        END_IF()
        GOTO(90)
    END_IF()

    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(12))

    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetResultAC, LW(6))
        IF_NOT_FLAG(LW(6), 0x2)
            USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
            GOTO(90)
        END_IF()
    END_IF()
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 131328, LW(5))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 0)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 0, LW(6))
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(6))
    END_IF()
LBL(90)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_StopAC)
    END_IF()
    USER_FUNC(btlevtcmd_GetDamage, LW(3), LW(0))
    IF_LARGE_EQUAL(LW(0), 10)
        SET(LW(0), 9)
    END_IF()
    USER_FUNC(evt_btl_camera_shake_h, 0, LW(0), 0, 10, 13)
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    
    // Determine whether to use four Stylish commands.
    IF_EQUAL(LW(12), PTR(&customWeapon_Hammer))
        SET(LW(14), 0)
    ELSE()
        SET(LW(14), 1)
    END_IF()
    
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_ACRStart, -2, 0, 12, 60, 0)
        USER_FUNC(btlevtcmd_ACRGetResult, LW(15), LW(13))
        IF_EQUAL(LW(15), 1)
            GOTO(91)
        END_IF()
        IF_SMALL_EQUAL(LW(15), -1)
            GOTO(91)
        END_IF()
        GOTO(92)
    END_IF()
LBL(91)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(evt_audience_acrobat_notry)
    END_IF()
    SET(LW(0), 60)
    SUB(LW(0), LW(13))
    IF_LARGE_EQUAL(LW(0), 1)
        WAIT_FRM(LW(0))
    END_IF()
    IF_EQUAL(LW(12), PTR(&customWeapon_Hammer))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_S_1"))
    ELSE()
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_H_16"))
    END_IF()
    WAIT_FRM(10)
    GOTO(95)
LBL(92)
    IF_NOT_EQUAL(LW(15), 0)
        USER_FUNC(evt_btl_camera_set_mode, 0, 0)
        USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
        USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
        BROTHER_EVT()
            USER_FUNC(btlevtcmd_snd_voice, -2, 1)
            USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 20, 0)
            DO(30)
                USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, 12)
                WAIT_FRM(1)
            WHILE()
        END_BROTHER()
        BROTHER_EVT()
            WAIT_FRM(8)
            USER_FUNC(btlevtcmd_ACRStart, -2, 12, 25, 25, 0)
        END_BROTHER()
        BROTHER_EVT()
            WAIT_FRM(4)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_Z_1"))
        END_BROTHER()
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_A_3A"))
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.30))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 35)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 30, -1)
        WAIT_FRM(4)
        USER_FUNC(btlevtcmd_ACRGetResult, LW(0), LW(1))
        IF_EQUAL(LW(14), 0)
            IF_EQUAL(LW(0), 2)
                USER_FUNC(evtTot_LogActiveMoveStylish, 0)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_A_3B"))
                USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
                ADD(LW(1), 50)
                USER_FUNC(evt_eff, PTR("_BTLEF"), PTR("confetti"), 3, LW(0), LW(1), LW(2), 90, 0, 0, 0, 0, 0, 0, 0)
                USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
                WAIT_FRM(90)
                GOTO(95)
            ELSE()
                USER_FUNC(evt_audience_acrobat_notry)
                GOTO(95)
            END_IF()
        ELSE()
            SWITCH(LW(0))
                CASE_EQUAL(2)
                    USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
                CASE_EQUAL(1)
                    USER_FUNC(evt_audience_acrobat_notry)
                    GOTO(95)
            END_SWITCH()
        END_IF()
        BROTHER_EVT()
            USER_FUNC(btlevtcmd_snd_voice, -2, 1)
            USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 20, 0)
            DO(40)
                USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, 18)
                WAIT_FRM(1)
            WHILE()
        END_BROTHER()
        BROTHER_EVT()
            WAIT_FRM(18)
            USER_FUNC(btlevtcmd_ACRStart, -2, 12, 25, 25, 0)
        END_BROTHER()
        BROTHER_EVT()
            WAIT_FRM(18)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_Z_1"))
        END_BROTHER()
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_A_3A"))
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.30))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 40)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 40, -1)
        WAIT_FRM(4)
        USER_FUNC(btlevtcmd_ACRGetResult, LW(0), LW(1))
        IF_NOT_EQUAL(LW(0), 2)
            GOTO(95)
        END_IF()
        USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
        USER_FUNC(btlevtcmd_snd_voice, -2, 1)
        USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 20, 0)
        BROTHER_EVT()
            DO(60)
                USER_FUNC(btlevtcmd_AddRotate, -2, 0, 12, 12)
                WAIT_FRM(1)
            WHILE()
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        END_BROTHER()
        BROTHER_EVT()
            WAIT_FRM(35)
            USER_FUNC(btlevtcmd_ACRStart, -2, 15, 28, 28, 0)
        END_BROTHER()
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_C_3"))
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.20))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 50)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 60, -1)
        WAIT_FRM(4)
        USER_FUNC(btlevtcmd_ACRGetResult, LW(0), LW(1))
        IF_NOT_EQUAL(LW(0), 2)
            GOTO(95)
        END_IF()
        USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_A_3B"))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        ADD(LW(1), 50)
        USER_FUNC(evtTot_LogActiveMoveStylish, 0)
        USER_FUNC(evt_eff, PTR("_BTLEF"), PTR("confetti"), 3, LW(0), LW(1), LW(2), 90, 0, 0, 0, 0, 0, 0, 0)
        USER_FUNC(btlevtcmd_snd_voice, -2, 1)
        USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_ACROBAT_FINISH1"), 0)
        WAIT_FRM(90)
    END_IF()
LBL(95)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(evt_audience_ap_recovery)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
    END_IF()
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_W_1"))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(marioAttackEvent_KaitenHammer)
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetFirstAttackTarget, LW(3), LW(4))
        IF_EQUAL(LW(3), -1)
            GOTO(99)
        END_IF()
        USER_FUNC(evt_btl_camera_set_mode, 0, 7)
        USER_FUNC(evt_btl_camera_set_homing_unit, 0, LW(3), -1)
        USER_FUNC(evt_btl_camera_set_zoom, 0, 300)
        USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
        USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 250)
        USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
        WAIT_FRM(30)
    ELSE()
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
        ELSE()
            USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
            USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(12))
            USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(12), LW(3), LW(4))
        END_IF()
        IF_EQUAL(LW(3), -1)
            GOTO(99)
        END_IF()
        USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
        USER_FUNC(btlevtcmd_WaitGuardMove)
        USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(12))
        USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
        RUN_CHILD_EVT(PTR(&marioAttackEvent_MajinaiPowerUpCheck))
        USER_FUNC(evt_btl_camera_set_mode, 0, 11)
        USER_FUNC(evt_btl_camera_set_homing_unitparts, 0, -2, 1, LW(3), LW(4))
        USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
        USER_FUNC(evt_btl_camera_set_zoom, 0, 300)
    END_IF()
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetWeaponActionLv, LW(12), LW(0))
        USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
        SWITCH(LW(12))
            CASE_OR(PTR(&customWeapon_UltraHammer))
            CASE_OR(PTR(&customWeapon_FSUltraHammer))
                USER_FUNC(btlevtcmd_AcSetParamAll, 10, 8, 0, 0, 0, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
                CASE_END()
            CASE_ETC()
                USER_FUNC(btlevtcmd_AcSetParamAll, 7, 15, 0, 0, 0, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
        END_SWITCH()
        USER_FUNC(btlevtcmd_AcSetFlag, 1)
        USER_FUNC(btlevtcmd_SetupAC, -2, 3, 1, 0)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetPos, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 30)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
    ADD(LW(2), 5)
    USER_FUNC(btlevtcmd_GetMoveFrame, -2, LW(0), LW(1), LW(2), 0, LW(7))
    SUB(LW(7), 21)
    IF_SMALL(LW(7), 0)
        MUL(LW(7), -1)
        WAIT_FRM(LW(7))
    END_IF()
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(evt_btl_camera_set_zoom, 0, 350)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(7))
    IF_EQUAL(LW(7), 5)
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_StopAC)
        END_IF()
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
        RETURN()
    END_IF()
    USER_FUNC(_get_mario_hammer_lv, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL(2)
            SET(LW(0), PTR("M_H_6A"))
            SET(LW(1), PTR("M_H_6B"))
        CASE_EQUAL(3)
            SET(LW(0), PTR("M_H_10A"))
            SET(LW(1), PTR("M_H_10B"))
        CASE_ETC()
            SET(LW(0), PTR("M_H_2A"))
            SET(LW(1), PTR("M_H_2B"))
    END_SWITCH()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_POWER2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePaperGroup, -2, 1, PTR("p_kaiten_h"), 0)
    USER_FUNC(btlevtcmd_AnimeChangePaperAnime, -2, 1, PTR("P_H_1A"))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, LW(0))
    USER_FUNC(_paper_light_off, -2, 1)
    SWITCH(LW(12))
        CASE_OR(PTR(&customWeapon_SuperHammer))
        CASE_OR(PTR(&customWeapon_FSSuperHammer))
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_MARIO_HAMMER_KAITEN1"), EVT_NULLPTR, 0, LW(15))
            CASE_END()
        CASE_OR(PTR(&customWeapon_UltraHammer))
        CASE_OR(PTR(&customWeapon_FSUltraHammer))
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_MARIO_HAMMER_ULTRA1"), EVT_NULLPTR, 0, LW(15))
            CASE_END()
        CASE_ETC()
    END_SWITCH()
    WAIT_FRM(28)
    USER_FUNC(btlevtcmd_AnimeChangePaperAnime, -2, 1, PTR("P_H_1B"))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_StartAC, 1)
        USER_FUNC(btlevtcmd_ResultAC)
    ELSE()
        WAIT_FRM(60)
    END_IF()
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    SWITCH(LW(12))
        CASE_OR(PTR(&customWeapon_SuperHammer))
        CASE_OR(PTR(&customWeapon_FSSuperHammer))
            USER_FUNC(evt_snd_sfxoff, LW(15))
        CASE_OR(PTR(&customWeapon_UltraHammer))
        CASE_OR(PTR(&customWeapon_FSUltraHammer))
            USER_FUNC(evt_snd_sfxoff, LW(15))
            CASE_END()
            CASE_END()
    END_SWITCH()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, LW(1))
    USER_FUNC(btlevtcmd_AnimeChangePaperAnime, -2, 1, PTR("P_H_1C"))
    WAIT_FRM(10)
    USER_FUNC(btlevtcmd_AnimeChangePaperGroup, -2, 1, 0, 0)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    IF_NOT_EQUAL(LW(7), 1)
        IF_EQUAL(LW(7), 3)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
        END_IF()
        IF_EQUAL(LW(7), 6)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
        END_IF()
        IF_EQUAL(LW(7), 2)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
        END_IF()
        GOTO(90)
    END_IF()
    
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(12))

    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetResultAC, LW(6))
        IF_NOT_FLAG(LW(6), 0x2)
            USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
            GOTO(90)
        END_IF()
    END_IF()
    SWITCH(LW(12))
        CASE_OR(PTR(&customWeapon_UltraHammer))
        CASE_OR(PTR(&customWeapon_FSUltraHammer))
            USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 131328, LW(5))
            CASE_END()
        CASE_ETC()
            USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 131328, LW(5))
    END_SWITCH()
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 0, LW(6))
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(6))
    END_IF()
LBL(90)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_StopAC)
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    SWITCH(LW(12))
        CASE_OR(PTR(&customWeapon_UltraHammer))
        CASE_OR(PTR(&customWeapon_FSUltraHammer))
            SET(LW(0), 1)
            CASE_END()
        CASE_ETC()
            SET(LW(0), 0)
    END_SWITCH()
    IF_EQUAL(LW(0), 1)
        INLINE_EVT()
            USER_FUNC(_whirlwind_effect, -2, 60)
        END_INLINE()
        USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(0))
        MUL(LW(0), 36)
        BROTHER_EVT_ID(LW(9))
            DO(6)
                SWITCH(LW(12))
                    CASE_OR(PTR(&customWeapon_UltraHammer))
                    CASE_OR(PTR(&customWeapon_FSUltraHammer))
                        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_MARIO_HAMMER_ULTRA2"), EVT_NULLPTR, 0, EVT_NULLPTR)
                        CASE_END()
                END_SWITCH()
                DO(10)
                    USER_FUNC(btlevtcmd_AddRotate, -2, 0, LW(0), 0)
                    WAIT_FRM(1)
                WHILE()
            WHILE()
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        END_BROTHER()
        IF_NOT_EQUAL(LW(7), 1)
            WAIT_FRM(50)
            DELETE_EVT(LW(9))
            BROTHER_EVT()
                DIV(LW(0), 3)
                DO(30)
                    USER_FUNC(btlevtcmd_AddRotate, -2, 0, LW(0), 0)
                    WAIT_FRM(1)
                WHILE()
                USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
            END_BROTHER()
            WAIT_FRM(8)
            USER_FUNC(btlevtcmd_AnimeChangePaperGroup, -2, 1, 0, 0)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_I_S"))
            WAIT_FRM(24)
            WAIT_MSEC(1500)
        ELSE()
            USER_FUNC(btlevtcmd_SetPartsBlur, -2, 1, 255, 255, 255, 80, 255, 255, 255, 0, 0)
            USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 1, 67108864)
            INLINE_EVT()
                WAIT_FRM(50)
                USER_FUNC(btlevtcmd_SetPartsBlur, -2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0)
                WAIT_FRM(10)
                USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 67108864)
            END_INLINE()
            IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
                USER_FUNC(btlevtcmd_ACRStart, -2, 40, 63, 63, 0)
                USER_FUNC(btlevtcmd_ACRGetResult, LW(6), EVT_NULLPTR)
            ELSE()
                SET(LW(6), 0)
            END_IF()
            SWITCH(LW(6))
                CASE_LARGE_EQUAL(2)
                    DELETE_EVT(LW(9))
                    USER_FUNC(evtTot_LogActiveMoveStylish, 0)
                    USER_FUNC(btlevtcmd_AnimeChangePaperGroup, -2, 1, 0, 0)
                    USER_FUNC(_kaiten_hammer_acrobat_rotate, -2, 1, 30, PTR("M_V_2"))
                    USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
                    WAIT_FRM(30)
                CASE_ETC()
                    USER_FUNC(btlevtcmd_WaitEventEnd, LW(9))
                    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
                    USER_FUNC(btlevtcmd_AnimeChangePaperGroup, -2, 1, 0, 0)
                    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
                        USER_FUNC(evt_audience_acrobat_notry)
                    END_IF()
            END_SWITCH()
        END_IF()
    ELSE()
        IF_NOT_EQUAL(LW(7), 1)
            USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(0))
            MUL(LW(0), 36)
            BROTHER_EVT_ID(LW(9))
                DO(6)
                    SWITCH(LW(12))
                        CASE_OR(PTR(&customWeapon_SuperHammer))
                        CASE_OR(PTR(&customWeapon_FSSuperHammer))
                            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_MARIO_HAMMER_KAITEN2"), EVT_NULLPTR, 0, EVT_NULLPTR)
                            CASE_END()
                    END_SWITCH()
                    DO(10)
                        USER_FUNC(btlevtcmd_AddRotate, -2, 0, LW(0), 0)
                        WAIT_FRM(1)
                    WHILE()
                WHILE()
                USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
            END_BROTHER()
            WAIT_FRM(50)
            DELETE_EVT(LW(9))
            BROTHER_EVT()
                DIV(LW(0), 3)
                DO(30)
                    USER_FUNC(btlevtcmd_AddRotate, -2, 0, LW(0), 0)
                    WAIT_FRM(1)
                WHILE()
                USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
            END_BROTHER()
            WAIT_FRM(8)
            USER_FUNC(btlevtcmd_AnimeChangePaperGroup, -2, 1, 0, 0)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_I_S"))
            WAIT_FRM(24)
            WAIT_MSEC(1500)
        ELSE()
            INLINE_EVT()
                USER_FUNC(_whirlwind_effect, -2, 60)
            END_INLINE()
            BROTHER_EVT_ID(LW(9))
                USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(0))
                MUL(LW(0), 36)
                DO(6)
                    SWITCH(LW(12))
                        CASE_OR(PTR(&customWeapon_SuperHammer))
                        CASE_OR(PTR(&customWeapon_FSSuperHammer))
                            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_MARIO_HAMMER_KAITEN2"), EVT_NULLPTR, 0, EVT_NULLPTR)
                            CASE_END()
                    END_SWITCH()
                    DO(10)
                        USER_FUNC(btlevtcmd_AddRotate, -2, 0, LW(0), 0)
                        WAIT_FRM(1)
                    WHILE()
                WHILE()
                USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
            END_BROTHER()
            IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
                USER_FUNC(btlevtcmd_ACRStart, -2, 40, 63, 63, 0)
                USER_FUNC(btlevtcmd_ACRGetResult, LW(6), EVT_NULLPTR)
            ELSE()
                SET(LW(6), 0)
            END_IF()
            SWITCH(LW(6))
                CASE_LARGE_EQUAL(2)
                    DELETE_EVT(LW(9))
                    USER_FUNC(evtTot_LogActiveMoveStylish, 0)
                    USER_FUNC(btlevtcmd_AnimeChangePaperGroup, -2, 1, 0, 0)
                    USER_FUNC(_kaiten_hammer_acrobat_rotate, -2, 1, 30, PTR("M_V_2"))
                    USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
                    WAIT_FRM(30)
                CASE_ETC()
                    USER_FUNC(btlevtcmd_WaitEventEnd, LW(9))
                    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
                    USER_FUNC(btlevtcmd_AnimeChangePaperGroup, -2, 1, 0, 0)
                    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
                        USER_FUNC(evt_audience_acrobat_notry)
                    END_IF()
            END_SWITCH()
        END_IF()
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    SWITCH(LW(12))
        CASE_OR(PTR(&customWeapon_UltraHammer))
        CASE_OR(PTR(&customWeapon_FSUltraHammer))
            CASE_END()
        CASE_ETC()
            GOTO(99)
    END_SWITCH()
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetResultAC, LW(6))
        IF_NOT_FLAG(LW(6), 0x2)
            GOTO(99)
        END_IF()
    END_IF()
    IF_NOT_EQUAL(LW(7), 1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CheckToken, LW(3), 32, LW(0))
    IF_EQUAL(LW(0), 0)
        GOTO(99)
    END_IF()
LBL(96)
    USER_FUNC(btlevtcmd_CheckToken, LW(3), 64, LW(0))
    IF_EQUAL(LW(0), 0)
        WAIT_FRM(1)
        GOTO(96)
    END_IF()
    USER_FUNC(btlevtcmd_OffToken, LW(3), 64)
LBL(98)
    USER_FUNC(btlevtcmd_CheckToken, LW(3), 128, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_CheckToken, -2, 256, LW(0))
        IF_EQUAL(LW(0), 0)
        END_IF()
        WAIT_FRM(1)
        GOTO(98)
    END_IF()
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&customWeapon_UltraHammerFinisher))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&customWeapon_UltraHammerFinisher), 131328, LW(5))
LBL(99)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(evt_audience_ap_recovery)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
    END_IF()
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(marioAttackEvent_FirstAttackKaitenHammer)
    USER_FUNC(_get_mario_hammer_lv, LW(0))
    IF_NOT_EQUAL(LW(0), 3)
        SET(LW(12), PTR(&customWeapon_FSSuperHammer))
    ELSE()
        SET(LW(12), PTR(&customWeapon_FSUltraHammer))
    END_IF()
    RUN_CHILD_EVT(PTR(&marioAttackEvent_KaitenHammer))
    RETURN()
    RETURN()
EVT_END()

EVT_BEGIN(marioAttackEvent_HammerNageru_object)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(1), 25)
    USER_FUNC(btlevtcmd_SetPartsPos, -2, 3, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 3, 16777216)
    SET(LW(0), -25)
    USER_FUNC(btlevtcmd_SetPartsRotate, -2, 3, 0, 0, 0)
    RUN_EVT_ID(PTR(&marioAttackEvent_HammerRotate), LW(13))
    USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 3, 0, FLOAT(6.0), FLOAT(0.35))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 3, LW(0), LW(1), LW(2), 0, -1)
    IF_NOT_EQUAL(LW(6), 1)
        IF_EQUAL(LW(6), 3)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
        END_IF()
        IF_EQUAL(LW(6), 6)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
        END_IF()
        IF_EQUAL(LW(6), 2)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
        END_IF()
        USER_FUNC(btlevtcmd_JumpPartsContinue, -2, 3)
        USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
        USER_FUNC(btlevtcmd_GetPartsPos, -2, 3, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), -30)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 3, LW(0), LW(1), LW(2), 20, -1)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), -15)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 3, LW(0), LW(1), LW(2), 12, -1)
        DELETE_EVT(LW(13))
        WAIT_FRM(30)
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 3, 16777216)
        GOTO(90)
    END_IF()
    INLINE_EVT()
        USER_FUNC(evt_btl_camera_set_mode, 0, 1)
        USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 4)
        USER_FUNC(evt_btl_camera_set_zoom, 0, 0)
        USER_FUNC(evt_btl_camera_off_posoffset_manual, 0)
    END_INLINE()
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))

    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(12))

    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetResultAC, LW(0))
        IF_FLAG(LW(0), 0x2)
            USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131328, LW(5))
            USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 0, LW(6))
            USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), LW(6))
        ELSE()
            USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 256, LW(5))
            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), -1)
        END_IF()
    ELSE()
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131328, LW(5))
    END_IF()
    USER_FUNC(btlevtcmd_GetDamage, LW(3), LW(0))
    IF_LARGE_EQUAL(LW(0), 10)
        SET(LW(0), 9)
    END_IF()
    USER_FUNC(evt_btl_camera_shake_h, 0, LW(0), 0, 10, 13)
    DELETE_EVT(LW(13))
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 3, 16777216)
    WAIT_FRM(20)
LBL(90)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_StopAC)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(marioAttackEvent_HammerNageru)
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    ELSE()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(12))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(12), LW(3), LW(4))
    END_IF()
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    RUN_CHILD_EVT(PTR(&marioAttackEvent_MajinaiPowerUpCheck))

    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
        USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
        
        IF_EQUAL(LW(12), PTR(&customWeapon_PowerPiercingSmashThrow))
            USER_FUNC(btlevtcmd_AcSetParamAll, 4, 40, 0, 0, 0, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
        ELSE()
            USER_FUNC(btlevtcmd_AcSetParamAll, 4, 30, 0, 0, 0, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
        END_IF()

        USER_FUNC(btlevtcmd_AcSetFlag, 0)
        USER_FUNC(btlevtcmd_SetupAC, -2, 3, 1, 0)

        USER_FUNC(evt_btl_camera_set_mode, 0, 7)
        USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -2)
        USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
        USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    END_IF()
    
    USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(3), LW(4), 256, LW(6))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))

    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 4)
        USER_FUNC(evt_btl_camera_set_zoom, 0, 450)
    END_IF()

    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_POWER2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(_get_mario_hammer_lv, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL(2)
            SET(LW(0), PTR("M_H_5A"))
            SET(LW(1), PTR("M_H_5B"))
            SET(LW(2), PTR("M_H_7"))
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 3, PTR("M_H_8"))
        CASE_EQUAL(3)
            SET(LW(0), PTR("M_H_9A"))
            SET(LW(1), PTR("M_H_9B"))
            SET(LW(2), PTR("M_H_11"))
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 3, PTR("M_H_12"))
        CASE_ETC()
            SET(LW(0), PTR("M_H_1A"))
            SET(LW(1), PTR("M_H_1B"))
            SET(LW(2), PTR("M_H_3"))
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 3, PTR("M_H_4"))
    END_SWITCH()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, LW(0))
    USER_FUNC(_get_mario_hammer_lv, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL(2)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_MARIO_HAMMER_HOLD2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        CASE_EQUAL(1)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_MARIO_HAMMER_HOLD1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        CASE_ETC()
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_MARIO_HAMMER_HOLD3"), EVT_NULLPTR, 0, EVT_NULLPTR)
    END_SWITCH()
    
    // Add star effect for variant moves.
    BROTHER_EVT()
        // Delay slightly to get a better arc.
        WAIT_FRM(5)
        SET(LW(13), -1)
        SWITCH(LW(12))
            CASE_EQUAL(PTR(&customWeapon_ShrinkSmashThrow))
                SET(LW(13), 4)
            CASE_EQUAL(PTR(&customWeapon_IceSmashThrow))
                SET(LW(13), 1)
            CASE_EQUAL(PTR(&customWeapon_PowerPiercingSmashThrow))
                SET(LW(13), 2)
        END_SWITCH()
        IF_NOT_EQUAL(LW(13), -1)
            USER_FUNC(btlevtcmd_ftomsec, 9, LW(0))
            WAIT_MSEC(LW(0))
            USER_FUNC(_hammer_star_effect, -2, LW(13))
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_MARIO_HAMMER_SHINE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        END_IF()
    END_BROTHER()

    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, LW(1))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_StartAC, 1)
        USER_FUNC(btlevtcmd_ResultAC)
    ELSE()
        WAIT_FRM(60)
    END_IF()
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_off_posoffset_manual, 0)
    USER_FUNC(evt_snd_sfxon, PTR("SFX_MARIO_HAMMER_NAGERU1"), 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, LW(2))
    WAIT_FRM(3)
    RUN_EVT_ID(PTR(&marioAttackEvent_HammerNageru_object), LW(15))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 1)
        WAIT_MSEC(1000)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_S_1"))
        WAIT_MSEC(667)
        GOTO(94)
    END_IF()
    USER_FUNC(btlevtcmd_ACRStart, -2, 0, 15, 15, 0)
    USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
    SWITCH(LW(6))
        CASE_LARGE_EQUAL(2)
        CASE_ETC()
            USER_FUNC(evt_audience_acrobat_notry)
            WAIT_MSEC(1000)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_S_1"))
            WAIT_MSEC(667)
            GOTO(94)
    END_SWITCH()
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_snd_voice, -2, 1)
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
        USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, -40, 30, 0)
        USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 20, 0)
        DO(30)
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, -24)
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    BROTHER_EVT()
        WAIT_FRM(8)
        USER_FUNC(btlevtcmd_ACRStart, -2, 12, 25, 25, 0)
    END_BROTHER()
    BROTHER_EVT()
        WAIT_FRM(4)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_Z_1"))
    END_BROTHER()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_A_3A"))
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.20))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 30, -1)
    WAIT_FRM(4)
    USER_FUNC(btlevtcmd_ACRGetResult, LW(0), LW(1))
    IF_EQUAL(LW(0), 2)
        USER_FUNC(evtTot_LogActiveMoveStylish, 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_A_3B"))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        ADD(LW(1), 50)
        USER_FUNC(evt_eff, PTR("_BTLEF"), PTR("confetti"), 3, LW(0), LW(1), LW(2), 90, 0, 0, 0, 0, 0, 0, 0)
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
        USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
        WAIT_MSEC(1500)
    END_IF()
LBL(94)
    USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(evt_audience_ap_recovery)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
    END_IF()
LBL(95)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 5)
    USER_FUNC(evt_btl_camera_off_posoffset_manual, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_W_1"))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(marioAttackEvent_JishinHammer)
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    RUN_CHILD_EVT(PTR(&marioAttackEvent_MajinaiPowerUpCheck))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
        USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
        USER_FUNC(btlevtcmd_AcSetParamAll, 4, 25, 0, 0, 0, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AcSetFlag, 0)
        USER_FUNC(btlevtcmd_SetupAC, -2, 3, 1, 0)
    END_IF()
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -2)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        SET(LW(0), -60)
    ELSE()
        SET(LW(0), 60)
    END_IF()
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 4)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 450)
    USER_FUNC(_get_mario_hammer_lv, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL(2)
            SET(LW(0), PTR("M_H_5A"))
            SET(LW(1), PTR("M_H_5B"))
            SET(LW(2), PTR("M_H_14"))
        CASE_EQUAL(3)
            SET(LW(0), PTR("M_H_9A"))
            SET(LW(1), PTR("M_H_9B"))
            SET(LW(2), PTR("M_H_15"))
        CASE_ETC()
            SET(LW(0), PTR("M_H_1A"))
            SET(LW(1), PTR("M_H_1B"))
            SET(LW(2), PTR("M_H_13"))
    END_SWITCH()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_POWER2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, LW(0))
    USER_FUNC(_get_mario_hammer_lv, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL(2)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_MARIO_HAMMER_HOLD2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        CASE_EQUAL(1)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_MARIO_HAMMER_HOLD1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        CASE_ETC()
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_MARIO_HAMMER_HOLD3"), EVT_NULLPTR, 0, EVT_NULLPTR)
    END_SWITCH()
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, LW(1))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_StartAC, 1)
    END_IF()
    WAIT_FRM(75)
    SET((int32_t)GSW_Battle_Multihit_GuardCount, 0)

    BROTHER_EVT_ID(LW(13))
        SET(LW(5), 5)
        DO(LW(5))
            IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
                USER_FUNC(btlevtcmd_AcGetOutputParam, 1, LW(6))
            ELSE()
                SET(LW(6), 3)
            END_IF()
            IF_LARGE_EQUAL(LW(6), 3)
                DO_BREAK()
            END_IF()
            WAIT_FRM(1)
            SET(LW(5), 5)
        WHILE()
        WAIT_FRM(25)
    END_BROTHER()

    BROTHER_EVT_ID(LW(14))
        SET(LW(5), 5)
        DO(LW(5))
            IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
                USER_FUNC(btlevtcmd_AcGetOutputParam, 1, LW(6))
            ELSE()
                SET(LW(6), 3)
            END_IF()
            IF_LARGE_EQUAL(LW(6), 3)
                DO_BREAK()
            END_IF()
            WAIT_FRM(1)
            SET(LW(5), 5)
        WHILE()
        WAIT_FRM(5)
        USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(5))
        MUL(LW(5), 45)
        USER_FUNC(evt_sub_intpl_init, 11, 0, LW(5), 10)
        DO(10)
            USER_FUNC(evt_sub_intpl_get_value)
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, LW(0))
            WAIT_FRM(1)
        WHILE()
        WAIT_FRM(8)
        USER_FUNC(evt_sub_intpl_init, 12, LW(5), 0, 15)
        DO(15)
            USER_FUNC(evt_sub_intpl_get_value)
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, LW(0))
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()

    BROTHER_EVT_ID(LW(15))
        SET(LW(5), 5)
        DO(LW(5))
            IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
                USER_FUNC(btlevtcmd_AcGetOutputParam, 1, LW(6))
            ELSE()
                SET(LW(6), 3)
            END_IF()
            IF_LARGE_EQUAL(LW(6), 3)
                DO_BREAK()
            END_IF()
            WAIT_FRM(1)
            SET(LW(5), 5)
        WHILE()
        USER_FUNC(btlevtcmd_GetPos, -2, LW(6), LW(7), LW(8))
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.30))
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(6), LW(7), LW(8), 40, -1)
    END_BROTHER()

    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_ResultAC)
        USER_FUNC(btlevtcmd_GetResultAC, LW(8))
        USER_FUNC(btlevtcmd_AcGetOutputParam, 1, LW(0))
    ELSE()
        SET(LW(0), 3)
        SET(LW(8), 0x2)
    END_IF()
    IF_LARGE_EQUAL(LW(0), 3)
        GOTO(10)
    END_IF()

    DELETE_EVT(LW(13))
    DELETE_EVT(LW(14))
    DELETE_EVT(LW(15))

    USER_FUNC(_get_mario_hammer_lv, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL(2)
            USER_FUNC(evt_snd_sfxon, PTR("SFX_MARIO_HAMMER_SWING2"), 0)
            SET(LW(2), PTR("M_H_14"))
        CASE_EQUAL(1)
            USER_FUNC(evt_snd_sfxon, PTR("SFX_MARIO_HAMMER_SWING1"), 0)
            SET(LW(2), PTR("M_H_13"))
        CASE_ETC()
            USER_FUNC(evt_snd_sfxon, PTR("SFX_MARIO_HAMMER_SWING3"), 0)
            SET(LW(2), PTR("M_H_15"))
    END_SWITCH()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, LW(2))
    USER_FUNC(evt_snd_sfxon, PTR("SFX_MARIO_ATTACK_ULTRAQUAKE1"), 0)
    WAIT_FRM(10)
    GOTO(40)
LBL(10)
    USER_FUNC(btlevtcmd_WaitEventEnd, LW(13))
    USER_FUNC(_get_mario_hammer_lv, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL(2)
            USER_FUNC(evt_snd_sfxon, PTR("SFX_MARIO_HAMMER_SWING2"), 0)
        CASE_EQUAL(1)
            USER_FUNC(evt_snd_sfxon, PTR("SFX_MARIO_HAMMER_SWING1"), 0)
        CASE_ETC()
            USER_FUNC(evt_snd_sfxon, PTR("SFX_MARIO_HAMMER_SWING3"), 0)
    END_SWITCH()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, LW(2))
    USER_FUNC(evt_snd_sfxon, PTR("SFX_MARIO_ATTACK_ULTRAQUAKE1"), 0)
    USER_FUNC(btlevtcmd_WaitEventEnd, LW(14))
    USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
LBL(40)
    USER_FUNC(evt_btl_camera_set_mode, 0, 1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 0)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 0)
    USER_FUNC(evt_btl_camera_off_posoffset_manual, 0)
    USER_FUNC(evt_snd_sfxon, PTR("SFX_MARIO_HAMMER_ULTRAQUAKE1"), 0)
    USER_FUNC(evt_btl_camera_shake_h, 0, 8, 0, 42, 13)
    USER_FUNC(evt_audience_reflesh_natural_all, 100)

    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetResultAC, LW(7))
        IF_FLAG(LW(7), 0x2)
            USER_FUNC(btlevtcmd_GetResultPrizeLv, -5, 0, LW(6))
            USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), 0, 50, 0)
            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), LW(6))
        ELSE()
            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), -1)
        END_IF()
    ELSE()
        SET(LW(7), 0x2)
    END_IF()

    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    ELSE()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(12))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(12), LW(3), LW(4))
    END_IF()
    IF_EQUAL(LW(3), -1)
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_StopAC)
        END_IF()
        GOTO(90)
    END_IF()
    GOTO(51)
LBL(50)
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_StopAC)
        END_IF()
        GOTO(90)
    END_IF()
LBL(51)
    USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(3), LW(4), 256, LW(6))
    IF_EQUAL(LW(6), 1)
        USER_FUNC(btlevtcmd_DamageDirect, LW(3), LW(4), 0, 0, 2, 0)
        INLINE_EVT()
            WAIT_FRM(42)
            
            IF_SMALL((int32_t)GSW_Battle_Multihit_GuardCount, 1)
                USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(5))
                USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(5))
                SET((int32_t)GSW_Battle_Multihit_GuardCount, 1)
            END_IF()

            IF_FLAG(LW(7), 0x2)
                USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131328, LW(5))
            ELSE()
                USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 256, LW(5))
            END_IF()
        END_INLINE()
    ELSE()
        IF_EQUAL(LW(6), 3)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
        END_IF()
        IF_EQUAL(LW(6), 6)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
        END_IF()
        IF_EQUAL(LW(6), 2)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
        END_IF()
    END_IF()
    GOTO(50)
LBL(90)
    WAIT_FRM(12)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(0))
    WAIT_FRM(60)
    BROTHER_EVT_ID(LW(15))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_H_16"))
        WAIT_FRM(18)
    END_BROTHER()

    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_ACRStart, -2, 0, 17, 17, 0)
        USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
        SWITCH(LW(6))
            CASE_LARGE_EQUAL(2)
            CASE_ETC()
                USER_FUNC(evt_audience_acrobat_notry)
                USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
                USER_FUNC(evt_audience_ap_recovery)
                USER_FUNC(btlevtcmd_InviteApInfoReport)
                GOTO(95)
        END_SWITCH()
    ELSE()
        GOTO(95)
    END_IF()

    USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_snd_voice, -2, 1)
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
        USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, -30, 20, 0)
        USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 20, 0)
        DO(60)
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, 12)
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    BROTHER_EVT()
        WAIT_FRM(8)
        USER_FUNC(btlevtcmd_ACRStart, -2, 42, 55, 55, 0)
    END_BROTHER()
    BROTHER_EVT()
        WAIT_FRM(4)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_Z_1"))
    END_BROTHER()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_A_3A"))
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.20))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 60, -1)
    WAIT_FRM(4)
    USER_FUNC(btlevtcmd_ACRGetResult, LW(0), LW(1))
    IF_EQUAL(LW(0), 2)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_A_3B"))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        ADD(LW(1), 50)
        USER_FUNC(evtTot_LogActiveMoveStylish, 0)
        USER_FUNC(evt_eff, PTR("_BTLEF"), PTR("confetti"), 3, LW(0), LW(1), LW(2), 90, 0, 0, 0, 0, 0, 0, 0)
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
        USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
        WAIT_MSEC(1500)
    END_IF()
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
LBL(95)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_W_1"))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(marioAttackEvent_FireNaguri_object)
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(15))
    SET((int32_t)GSW_Battle_Multihit_GuardCount, 0)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    ELSE()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(15))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(15), LW(3), LW(4))
    END_IF()
    IF_EQUAL(LW(3), -1)
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_StopAC)
        END_IF()
        USER_FUNC(btlevtcmd_GetPartsPos, -2, 3, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetStageSize, LW(5), EVT_NULLPTR, EVT_NULLPTR)
        MUL(LW(5), 2)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), LW(5))
        USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 3, 0, FLOAT(6.0), FLOAT(0.30))
        USER_FUNC(btlevtcmd_FallPartsPosition, -2, 3, LW(0), LW(1), LW(2), 0)
        USER_FUNC(btlevtcmd_SetPartsWork, -2, 3, 0, 0)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 20)
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 3, 0, FLOAT(4.0), FLOAT(0.5))
    USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 3, LW(0), LW(1), LW(2), 0, -1)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 3, 0, 0)
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(0))

    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetResultAC, LW(0))
        SET(LW(14), 0)
        IF_FLAG(LW(0), 0x2)
            USER_FUNC(btlevtcmd_GetResultPrizeLv, -5, 0, LW(6))
            USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), 0, 50, 0)
            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), LW(6))
            SET(LW(14), 1)
        ELSE()
            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), -1)
        END_IF()
    ELSE()
        SET(LW(14), 1)
    END_IF()

    USER_FUNC(btlevtcmd_GetPartsPos, -2, 3, LW(7), LW(8), LW(9))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, -1, -1, 2, LW(12))
    MUL(LW(12), 15)
    SET(LW(13), 5)

    // Run concurrent evt that handles igniting Bomb Squad bombs.
    IF_EQUAL(LW(14), 1)
        BROTHER_EVT()
            SETF(LW(10), LW(7))
            SETF(LW(11), LW(7))
            DIVF(LW(12), 6)
            ADDF(LW(11), LW(12))
            DO(0)
                USER_FUNC(evtTot_CheckBombSquadHit, LW(10), LW(11))
                SETF(LW(10), LW(11))
                ADDF(LW(11), LW(12))
                WAIT_FRM(1)
            WHILE()
        END_BROTHER()
    END_IF()

    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    ELSE()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(15))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(15), LW(3), LW(4))
    END_IF()
    
    IF_EQUAL(LW(3), -1)
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_StopAC)
        END_IF()
        GOTO(90)
    END_IF()
    GOTO(51)
LBL(50)
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_StopAC)
        END_IF()
        GOTO(90)
    END_IF()
LBL(51)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(11), EVT_NULLPTR, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, -1, -1, 2, LW(14))
    IF_LARGE_EQUAL(LW(14), 0)
        IF_SMALL_EQUAL(LW(7), LW(11))
            WAIT_FRM(6)
            ADD(LW(7), LW(12))
            INLINE_EVT()
                USER_FUNC(_fire_wave, LW(7), LW(8), LW(9), LW(13))
            END_INLINE()
            GOTO(51)
        END_IF()
    ELSE()
        IF_LARGE_EQUAL(LW(7), LW(11))
            WAIT_FRM(6)
            ADD(LW(7), LW(12))
            INLINE_EVT()
                USER_FUNC(_fire_wave, LW(7), LW(8), LW(9), LW(13))
            END_INLINE()
            GOTO(51)
        END_IF()
    END_IF()
    USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(3), LW(4), 256, LW(6))
    IF_EQUAL(LW(6), 1)
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_GetResultAC, LW(0))
        ELSE()
            SET(LW(0), 0x2)
        END_IF()

        IF_SMALL((int32_t)GSW_Battle_Multihit_GuardCount, 1)
            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(5))
            USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(5))
            SET((int32_t)GSW_Battle_Multihit_GuardCount, 1)
        END_IF()

        IF_FLAG(LW(0), 0x2)
            USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131328, LW(5))
        ELSE()
            USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 256, LW(5))
        END_IF()
        IF_LARGE_EQUAL(LW(13), 2)
            SUB(LW(13), 1)
        END_IF()
    ELSE()
        IF_EQUAL(LW(6), 3)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
        END_IF()
        IF_EQUAL(LW(6), 6)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
        END_IF()
        IF_EQUAL(LW(6), 2)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
        END_IF()
    END_IF()
    GOTO(50)
LBL(90)
LBL(92)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, -1, -1, 2, LW(10))
    USER_FUNC(btlevtcmd_GetStageSize, LW(11), EVT_NULLPTR, EVT_NULLPTR)
    MUL(LW(11), LW(10))
    IF_LARGE_EQUAL(LW(11), 0)
        IF_SMALL_EQUAL(LW(7), LW(11))
            WAIT_FRM(6)
            ADD(LW(7), LW(12))
            INLINE_EVT()
                USER_FUNC(_fire_wave, LW(7), LW(8), LW(9), LW(13))
            END_INLINE()
            GOTO(92)
        END_IF()
    ELSE()
        IF_LARGE_EQUAL(LW(7), LW(11))
            WAIT_FRM(6)
            ADD(LW(7), LW(12))
            INLINE_EVT()
                USER_FUNC(_fire_wave, LW(7), LW(8), LW(9), LW(13))
            END_INLINE()
            GOTO(92)
        END_IF()
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(marioAttackEvent_FireNaguri)
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    RUN_CHILD_EVT(PTR(&marioAttackEvent_MajinaiPowerUpCheck))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
        USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
        USER_FUNC(btlevtcmd_AcSetParamAll, 0, 83, 63, 83, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AcSetFlag, 16)
        USER_FUNC(btlevtcmd_SetupAC, -2, 11, 1, 0)
    END_IF()
    WAIT_FRM(2)
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -2)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(evt_btl_camera_set_posoffset, 0, 40, 0, 0)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 25)
    USER_FUNC(btlevtcmd_GetStageSize, EVT_NULLPTR, LW(1), EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetPartsPos, -2, 3, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 3, PTR("M_H_8"))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 3, 0, 1)
    BROTHER_EVT()
        USER_FUNC(_mario_fire_ball_controll, -2, 3)
    END_BROTHER()
    USER_FUNC(btlevtcmd_GetPos, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
    ADD(LW(1), 140)
    BROTHER_EVT_ID(LW(15))
        USER_FUNC(btlevtcmd_DivePartsPosition, -2, 3, LW(0), LW(1), LW(2), 60, 0, 5, 0, -1)
    END_BROTHER()
LBL(10)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_PadCheckNow, 0, 262144, LW(0))
        IF_EQUAL(LW(0), 0)
            WAIT_FRM(1)
            GOTO(10)
        END_IF()
    END_IF()
    CHK_EVT(LW(15), LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        DELETE_EVT(LW(15))
    END_IF()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_POWER2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(_get_mario_hammer_lv, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL(2)
            SET(LW(0), PTR("M_H_18A"))
            SET(LW(1), PTR("M_H_18B"))
            SET(LW(2), PTR("M_H_18C"))
        CASE_EQUAL(3)
            SET(LW(0), PTR("M_H_19A"))
            SET(LW(1), PTR("M_H_19B"))
            SET(LW(2), PTR("M_H_19C"))
        CASE_ETC()
            SET(LW(0), PTR("M_H_17A"))
            SET(LW(1), PTR("M_H_17B"))
            SET(LW(2), PTR("M_H_17C"))
    END_SWITCH()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, LW(0))
    USER_FUNC(_get_mario_hammer_lv, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL(2)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_MARIO_HAMMER_HOLD2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        CASE_EQUAL(1)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_MARIO_HAMMER_HOLD1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        CASE_ETC()
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_MARIO_HAMMER_HOLD3"), EVT_NULLPTR, 0, EVT_NULLPTR)
    END_SWITCH()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_MARIO_HAMMER_FIRE1"), EVT_NULLPTR, 0, LW(15))
    INLINE_EVT_ID(LW(14))
        USER_FUNC(btlevtcmd_GetPartsPos, -2, 3, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 3, 0, FLOAT(4.0), FLOAT(0.20))
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_MARIO_HAMMER_FIRE3"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_FallPartsPosition, -2, 3, LW(0), LW(1), LW(2), 75)
    END_INLINE()
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_StartAC, 1)
    END_IF()
    WAIT_FRM(20)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, LW(1))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_ResultAC)
        USER_FUNC(btlevtcmd_GetResultAC, LW(0))
    ELSE()
        WAIT_FRM(55)
        SET(LW(0), 0x2)
    END_IF()
    IF_NOT_FLAG(LW(0), 0x2)
        USER_FUNC(btlevtcmd_AcGetOutputParam, 0, LW(0))
        BROTHER_EVT()
            USER_FUNC(_get_mario_hammer_lv, LW(0))
            SWITCH(LW(0))
                CASE_EQUAL(2)
                    USER_FUNC(evt_snd_sfxon, PTR("SFX_MARIO_HAMMER_SWING2"), 0)
                CASE_EQUAL(1)
                    USER_FUNC(evt_snd_sfxon, PTR("SFX_MARIO_HAMMER_SWING1"), 0)
                CASE_ETC()
                    USER_FUNC(evt_snd_sfxon, PTR("SFX_MARIO_HAMMER_SWING3"), 0)
            END_SWITCH()
            USER_FUNC(evt_snd_sfxoff, LW(15))
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_MARIO_HAMMER_FIRE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, LW(2))
            WAIT_FRM(7)
            WAIT_FRM(60)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_S_1"))
        END_BROTHER()
        USER_FUNC(btlevtcmd_WaitEventEnd, LW(14))
        USER_FUNC(btlevtcmd_SetPartsWork, -2, 3, 0, 0)
        USER_FUNC(evt_audience_ap_recovery)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
        USER_FUNC(btlevtcmd_GetPartsPos, -2, 3, LW(0), LW(1), LW(2))
        USER_FUNC(evt_eff64, PTR(""), PTR("fire_spark_n64"), 0, LW(0), LW(1), LW(2), 30, 40, FLOAT(1.0), 12, 100, FLOAT(1.0), FLOAT(1.0), 0)
        ADD(LW(1), 5)
        USER_FUNC(evt_eff, PTR(""), PTR("kemuri_test"), 11, LW(0), LW(1), LW(2), FLOAT(1.0), 0, 0, 0, 0, 0, 0, 0)
        USER_FUNC(evt_btl_camera_set_mode, 0, 1)
        USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
        USER_FUNC(evt_btl_camera_set_zoom, 0, 0)
        USER_FUNC(evt_btl_camera_off_posoffset_manual, 0)
        USER_FUNC(btlevtcmd_GetUnitId, -2, LW(6))
        USER_FUNC(btlevtcmd_GetEnemyBelong, LW(6), LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -5, LW(0), PTR(&customWeapon_FireDriveFailed))
        USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
LBL(15)
        IF_EQUAL(LW(3), -1)
            GOTO(17)
        END_IF()
        IF_EQUAL(LW(3), LW(6))
            GOTO(16)
        END_IF()
LBL(16)
        USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
        GOTO(15)
LBL(17)
        USER_FUNC(btlevtcmd_GetUnitId, -2, LW(6))
        USER_FUNC(btlevtcmd_GetEnemyBelong, LW(6), LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -5, LW(0), PTR(&customWeapon_FireDriveFailed))
        USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
LBL(20)
        IF_EQUAL(LW(3), -1)
            GOTO(22)
        END_IF()
        IF_NOT_EQUAL(LW(3), LW(6))
            GOTO(21)
        END_IF()
        USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(3), LW(4), 256, LW(5))
LBL(21)
        USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
        GOTO(20)
LBL(22)
        WAIT_FRM(30)
        GOTO(99)
    END_IF()
    USER_FUNC(_get_mario_hammer_lv, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL(2)
            USER_FUNC(evt_snd_sfxon, PTR("SFX_MARIO_HAMMER_SWING2"), 0)
        CASE_EQUAL(1)
            USER_FUNC(evt_snd_sfxon, PTR("SFX_MARIO_HAMMER_SWING1"), 0)
        CASE_ETC()
            USER_FUNC(evt_snd_sfxon, PTR("SFX_MARIO_HAMMER_SWING3"), 0)
    END_SWITCH()
    USER_FUNC(evt_snd_sfxoff, LW(15))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_MARIO_HAMMER_FIRE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, LW(2))
    WAIT_FRM(7)
    DELETE_EVT(LW(14))
    USER_FUNC(evt_btl_camera_set_mode, 0, 1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 0)
    USER_FUNC(evt_btl_camera_off_posoffset_manual, 0)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_MARIO_HAMMER_FIRE4"), EVT_NULLPTR, 0, EVT_NULLPTR)
    RUN_EVT_ID(PTR(&marioAttackEvent_FireNaguri_object), LW(15))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_ACRStart, -2, 0, 15, 15, 0)
        USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
    ELSE()
        SET(LW(6), 0)
    END_IF()
    SWITCH(LW(6))
        CASE_LARGE_EQUAL(2)
            DO(20)
                USER_FUNC(btlevtcmd_AddRotate, -2, 0, 36, 0)
                WAIT_FRM(1)
            WHILE()
            USER_FUNC(evtTot_LogActiveMoveStylish, 0)
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
            USER_FUNC(btlevtcmd_snd_voice, -2, 1)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_V_2"))
            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
            USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
            WAIT_MSEC(500)
        CASE_ETC()
            IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
                USER_FUNC(evt_audience_acrobat_notry)
            END_IF()
            WAIT_FRM(60)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_S_1"))
    END_SWITCH()
    USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(evt_audience_ap_recovery)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
    END_IF()
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_W_1"))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

// Weapon definitions.

BattleWeapon customWeapon_Jump = {
    .name = "btl_cmd_act_jump",
    .icon = IconType::BOOTS,
    .item_id = 0,
    .description = "msg_normal_jump",
    .base_accuracy = 100,
    .base_fp_cost = 0,
    .base_sp_cost = 0,
    .superguards_allowed = 1,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 1,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 2, 2, 3, 3, 4, 4, 0, MoveType::JUMP_BASE },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags = 
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::JUMPLIKE |
        AttackTargetProperty_Flags::CANNOT_TARGET_CEILING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_jump",
    .special_property_flags =
        AttackSpecialProperty_Flags::TOT_PARTY_UNGUARDABLE |
        AttackSpecialProperty_Flags::USABLE_IF_CONFUSED |
        AttackSpecialProperty_Flags::GROUNDS_WINGED |
        AttackSpecialProperty_Flags::FLIPS_SHELLED |
        AttackSpecialProperty_Flags::MAKES_ATTACK_FX_SOUND |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = 
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY |
        AttackCounterResistance_Flags::FRONT_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // .status_chances = all 0,
    
    .attack_evt_code = (void*)marioAttackEvent_NormalJump,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 4,
    .nozzle_fire_chance = 2,
    .ceiling_fall_chance = 1,
    .object_fall_chance = 1,
};

BattleWeapon customWeapon_SpinJump = {
    .name = "btl_wn_mario_kururin_jump",
    .icon = IconType::SUPER_BOOTS,
    .item_id = 0,
    .description = "msg_kururin_jump",
    .base_accuracy = 100,
    .base_fp_cost = 2,
    .base_sp_cost = 0,
    .superguards_allowed = 1,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 2, 2, 3, 3, 4, 4, 0, MoveType::JUMP_SPIN },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags = 
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::JUMPLIKE |
        AttackTargetProperty_Flags::CANNOT_TARGET_CEILING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_k_jump",
    .special_property_flags =
        AttackSpecialProperty_Flags::TOT_PARTY_UNGUARDABLE |
        AttackSpecialProperty_Flags::GROUNDS_WINGED |
        AttackSpecialProperty_Flags::FLIPS_SHELLED |
        AttackSpecialProperty_Flags::MAKES_ATTACK_FX_SOUND |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = 
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY |
        AttackCounterResistance_Flags::FRONT_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // .status_chances = all 0,
    
    .attack_evt_code = (void*)marioAttackEvent_KururinJump,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 4,
    .nozzle_fire_chance = 5,
    .ceiling_fall_chance = 1,
    .object_fall_chance = 1,
};

BattleWeapon customWeapon_SpinJump2 = {
    .name = "btl_wn_mario_kururin_jump",
    .icon = IconType::SUPER_BOOTS,
    .item_id = 0,
    .description = "msg_kururin_jump",
    .base_accuracy = 100,
    .base_fp_cost = 0,
    .base_sp_cost = 0,
    .superguards_allowed = 1,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 2, 4, 3, 6, 4, 8, 0, MoveType::JUMP_SPIN },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags = 
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::JUMPLIKE |
        AttackTargetProperty_Flags::CANNOT_TARGET_CEILING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_k_jump",
    .special_property_flags =
        AttackSpecialProperty_Flags::TOT_PARTY_UNGUARDABLE |
        AttackSpecialProperty_Flags::GROUNDS_WINGED |
        AttackSpecialProperty_Flags::FLIPS_SHELLED |
        AttackSpecialProperty_Flags::MAKES_ATTACK_FX_SOUND |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = 
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY |
        AttackCounterResistance_Flags::FRONT_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // .status_chances = all 0,
    
    .attack_evt_code = (void*)marioAttackEvent_KururinJump,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 5,
    .bg_a2_fall_weight = 5,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 20,
    .nozzle_fire_chance = 10,
    .ceiling_fall_chance = 5,
    .object_fall_chance = 5,
};

BattleWeapon customWeapon_SpringJump = {
    .name = "btl_wn_mario_jyabara_jump",
    .icon = IconType::ULTRA_BOOTS,
    .item_id = 0,
    .description = "msg_jyabara_jump",
    .base_accuracy = 100,
    .base_fp_cost = 4,
    .base_sp_cost = 0,
    .superguards_allowed = 1,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 2, 2, 3, 3, 4, 4, 0, MoveType::JUMP_SPRING },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags = 
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::JUMPLIKE |
        AttackTargetProperty_Flags::CANNOT_TARGET_CEILING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_j_jump",
    .special_property_flags =
        AttackSpecialProperty_Flags::TOT_PARTY_UNGUARDABLE |
        AttackSpecialProperty_Flags::GROUNDS_WINGED |
        AttackSpecialProperty_Flags::FLIPS_SHELLED |
        AttackSpecialProperty_Flags::MAKES_ATTACK_FX_SOUND |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = 
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY |
        AttackCounterResistance_Flags::FRONT_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // .status_chances = all 0,
    
    .attack_evt_code = (void*)marioAttackEvent_JyabaraJump,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 5,
    .bg_a2_fall_weight = 5,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 20,
    .nozzle_fire_chance = 10,
    .ceiling_fall_chance = 5,
    .object_fall_chance = 5,
};

BattleWeapon customWeapon_SpringJump2 = {
    .name = "btl_wn_mario_jyabara_jump",
    .icon = IconType::ULTRA_BOOTS,
    .item_id = 0,
    .description = "msg_jyabara_jump",
    .base_accuracy = 100,
    .base_fp_cost = 4,
    .base_sp_cost = 0,
    .superguards_allowed = 1,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 2, 4, 3, 6, 4, 8, 0, MoveType::JUMP_SPRING },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags = 
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::JUMPLIKE |
        AttackTargetProperty_Flags::CANNOT_TARGET_CEILING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_j_jump",
    .special_property_flags =
        AttackSpecialProperty_Flags::TOT_PARTY_UNGUARDABLE |
        AttackSpecialProperty_Flags::GROUNDS_WINGED |
        AttackSpecialProperty_Flags::FLIPS_SHELLED |
        AttackSpecialProperty_Flags::MAKES_ATTACK_FX_SOUND |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = 
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY |
        AttackCounterResistance_Flags::FRONT_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // .status_chances = all 0,
    
    .attack_evt_code = (void*)marioAttackEvent_JyabaraJump,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 5,
    .bg_a2_fall_weight = 5,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 20,
    .nozzle_fire_chance = 10,
    .ceiling_fall_chance = 5,
    .object_fall_chance = 5,
};

BattleWeapon customWeapon_SpringJumpFailed = {
    .name = "btl_wn_mario_jyabara_jump",
    .icon = IconType::ULTRA_BOOTS,
    .item_id = 0,
    .description = "msg_jyabara_jump",
    .base_accuracy = 100,
    .base_fp_cost = 4,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 1,
    .unk_1b = 50,
    .damage_function = nullptr,
    .damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags = 
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::JUMPLIKE |
        AttackTargetProperty_Flags::CANNOT_TARGET_CEILING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::GROUNDS_WINGED |
        AttackSpecialProperty_Flags::FLIPS_SHELLED |
        AttackSpecialProperty_Flags::MAKES_ATTACK_FX_SOUND |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = 
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY |
        AttackCounterResistance_Flags::FRONT_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // .status_chances = all 0,
    
    .attack_evt_code = (void*)marioAttackEvent_JyabaraJump,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 50,
    .bg_a2_fall_weight = 50,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 0,
    .nozzle_fire_chance = 0,
    .ceiling_fall_chance = 50,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_Hammer = {
    .name = "btl_cmd_act_hammer",
    .icon = IconType::HAMMER,
    .item_id = 0,
    .description = "msg_normal_hammer",
    .base_accuracy = 100,
    .base_fp_cost = 0,
    .base_sp_cost = 0,
    .superguards_allowed = 2,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 1,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 2, 4, 3, 6, 4, 8, 0, MoveType::HAMMER_BASE },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags = 
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::ONLY_FRONT |
        AttackTargetProperty_Flags::HAMMERLIKE,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_hammer",
    .special_property_flags =
        AttackSpecialProperty_Flags::TOT_PARTY_UNGUARDABLE |
        AttackSpecialProperty_Flags::USABLE_IF_CONFUSED |
        AttackSpecialProperty_Flags::MAKES_ATTACK_FX_SOUND |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = 
        AttackCounterResistance_Flags::ALL &
        ~AttackCounterResistance_Flags::PREEMPTIVE_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // .status_chances = all 0,
    
    .attack_evt_code = (void*)marioAttackEvent_NormalHammer,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 3,
    .bg_a2_fall_weight = 3,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 3,
    .nozzle_turn_chance = 6,
    .nozzle_fire_chance = 2,
    .ceiling_fall_chance = 3,
    .object_fall_chance = 3,
};

BattleWeapon customWeapon_SuperHammer = {
    .name = "btl_wn_mario_kaiten_hammer",
    .icon = IconType::SUPER_HAMMER,
    .item_id = 0,
    .description = "msg_kaiten_hammer",
    .base_accuracy = 100,
    .base_fp_cost = 2,
    .base_sp_cost = 0,
    .superguards_allowed = 2,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 4, 4, 6, 6, 8, 8, 0, MoveType::HAMMER_SUPER },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags = 
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::ONLY_FRONT |
        AttackTargetProperty_Flags::HAMMERLIKE,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0xe,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_kaiten_hammer",
    .special_property_flags =
        AttackSpecialProperty_Flags::TOT_PARTY_UNGUARDABLE |
        AttackSpecialProperty_Flags::MAKES_ATTACK_FX_SOUND |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = 
        AttackCounterResistance_Flags::ALL &
        ~AttackCounterResistance_Flags::PREEMPTIVE_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // .status_chances = all 0,
    
    .attack_evt_code = (void*)marioAttackEvent_KaitenHammer,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 6,
    .nozzle_fire_chance = 5,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_SuperHammerRecoil = {
    .name = nullptr,
    .icon = 0,
    .item_id = 0,
    .description = nullptr,
    .base_accuracy = 100,
    .base_fp_cost = 0,
    .base_sp_cost = 0,
    .superguards_allowed = 1,
    .unk_14 = 0.0,
    .stylish_multiplier = 0,
    .unk_19 = 0,
    .bingo_card_chance = 0,
    .unk_1b = 0,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 2, 2, 3, 3, 5, 5, 0, MoveType::HAMMER_SUPER },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags = 
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::RECOIL_DAMAGE |
        AttackTargetProperty_Flags::HAMMERLIKE,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0xf,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = 0,
        
    // .status_chances = all 0,
    
    .attack_evt_code = nullptr,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 0,
    .nozzle_fire_chance = 0,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_UltraHammer = {
    .name = "btl_wn_mario_ultra_hammer",
    .icon = IconType::ULTRA_HAMMER,
    .item_id = 0,
    .description = "msg_ultra_hammer",
    .base_accuracy = 100,
    .base_fp_cost = 4,
    .base_sp_cost = 0,
    .superguards_allowed = 2,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 4, 4, 6, 6, 8, 8, 0, MoveType::HAMMER_ULTRA },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::ONLY_FRONT |
        AttackTargetProperty_Flags::HAMMERLIKE,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0x10,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_ultra_hammer",
    .special_property_flags =
        AttackSpecialProperty_Flags::TOT_PARTY_UNGUARDABLE |
        AttackSpecialProperty_Flags::MAKES_ATTACK_FX_SOUND |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = 
        AttackCounterResistance_Flags::ALL &
        ~AttackCounterResistance_Flags::PREEMPTIVE_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // .status_chances = all 0,
    
    .attack_evt_code = (void*)marioAttackEvent_KaitenHammer,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 6,
    .nozzle_fire_chance = 5,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_UltraHammerRecoil = {
    .name = nullptr,
    .icon = 0,
    .item_id = 0,
    .description = nullptr,
    .base_accuracy = 100,
    .base_fp_cost = 0,
    .base_sp_cost = 0,
    .superguards_allowed = 1,
    .unk_14 = 0.0,
    .stylish_multiplier = 0,
    .unk_19 = 0,
    .bingo_card_chance = 0,
    .unk_1b = 0,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 2, 2, 3, 3, 5, 5, 0, MoveType::HAMMER_ULTRA },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags = 
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::RECOIL_DAMAGE |
        AttackTargetProperty_Flags::HAMMERLIKE,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0x11,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = 0,
        
    // .status_chances = all 0,
    
    .attack_evt_code = nullptr,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 0,
    .nozzle_fire_chance = 0,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_UltraHammerFinisher = {
    .name = "btl_wn_mario_ultra_hammer",
    .icon = IconType::ULTRA_HAMMER,
    .item_id = 0,
    .description = "msg_ultra_hammer",
    .base_accuracy = 100,
    .base_fp_cost = 0,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 4, 4, 6, 6, 8, 8, 0, MoveType::HAMMER_ULTRA },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::TOT_PARTY_UNGUARDABLE |
        AttackSpecialProperty_Flags::MAKES_ATTACK_FX_SOUND |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::CANNOT_MISS |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // .status_chances = all 0,
    
    .attack_evt_code = (void*)marioAttackEvent_KaitenHammer,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 0,
    .nozzle_fire_chance = 0,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_FSSuperHammer = {
    .name = nullptr,
    .icon = 0,
    .item_id = 0,
    .description = nullptr,
    .base_accuracy = 100,
    .base_fp_cost = 0,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 1,
    .bingo_card_chance = 1,
    .unk_1b = 1,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 4, 4, 6, 6, 8, 8, 0, MoveType::HAMMER_SUPER },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags = 
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::ONLY_FRONT |
        AttackTargetProperty_Flags::HAMMERLIKE,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0xe,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::MAKES_ATTACK_FX_SOUND |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // .status_chances = all 0,
    
    .attack_evt_code = (void*)marioAttackEvent_FirstAttackKaitenHammer,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 0,
    .nozzle_fire_chance = 0,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_FSUltraHammer = {
    .name = nullptr,
    .icon = 0,
    .item_id = 0,
    .description = nullptr,
    .base_accuracy = 100,
    .base_fp_cost = 0,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 1,
    .bingo_card_chance = 1,
    .unk_1b = 1,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 4, 4, 6, 6, 8, 8, 0, MoveType::HAMMER_ULTRA },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags = 
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::ONLY_FRONT |
        AttackTargetProperty_Flags::HAMMERLIKE,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0x10,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::MAKES_ATTACK_FX_SOUND |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // .status_chances = all 0,
    
    .attack_evt_code = (void*)marioAttackEvent_FirstAttackKaitenHammer,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 0,
    .nozzle_fire_chance = 0,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_PowerSoftStomp = {
    .name = "in_gatsun_jump",
    .icon = IconType::POWER_JUMP,
    .item_id = ItemType::POWER_JUMP,
    .description = nullptr,
    .base_accuracy = 100,
    .base_fp_cost = 2,
    .base_sp_cost = 0,
    .superguards_allowed = 1,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 5, 6, 7, 9, 9, 12, 0, MoveType::JUMP_POWER_JUMP },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::JUMPLIKE |
        AttackTargetProperty_Flags::CANNOT_TARGET_CEILING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_jump",
    .special_property_flags =
        AttackSpecialProperty_Flags::TOT_PARTY_UNGUARDABLE |
        AttackSpecialProperty_Flags::GROUNDS_WINGED |
        AttackSpecialProperty_Flags::FLIPS_SHELLED |
        AttackSpecialProperty_Flags::MAKES_ATTACK_FX_SOUND |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = 
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY |
        AttackCounterResistance_Flags::FRONT_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .def_change_chance = 100,
    .def_change_time = 3,
    .def_change_strength = -3,
    
    .attack_evt_code = (void*)marioAttackEvent_GatsunJump,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 25,
    .bg_a2_fall_weight = 25,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 25,
    .nozzle_turn_chance = 25,
    .nozzle_fire_chance = 25,
    .ceiling_fall_chance = 5,
    .object_fall_chance = 25,
};

BattleWeapon customWeapon_Multibounce = {
    .name = "in_tugitugi_jump",
    .icon = IconType::MULTIBOUNCE,
    .item_id = ItemType::MULTIBOUNCE,
    .description = nullptr,
    .base_accuracy = 100,
    .base_fp_cost = 2,
    .base_sp_cost = 0,
    .superguards_allowed = 1,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 2, 2, 3, 3, 5, 5, 0, MoveType::JUMP_MULTIBOUNCE },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::JUMPLIKE |
        AttackTargetProperty_Flags::CANNOT_TARGET_CEILING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_jump",
    .special_property_flags =
        AttackSpecialProperty_Flags::TOT_PARTY_UNGUARDABLE |
        AttackSpecialProperty_Flags::GROUNDS_WINGED |
        AttackSpecialProperty_Flags::FLIPS_SHELLED |
        AttackSpecialProperty_Flags::MAKES_ATTACK_FX_SOUND |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = 
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY |
        AttackCounterResistance_Flags::FRONT_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)marioAttackEvent_TugiTugiJump,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 10,
    .nozzle_fire_chance = 5,
    .ceiling_fall_chance = 2,
    .object_fall_chance = 2,
};

BattleWeapon customWeapon_PowerBounce = {
    .name = "in_renzoku_jump",
    .icon = IconType::POWER_BOUNCE,
    .item_id = ItemType::POWER_BOUNCE,
    .description = nullptr,
    .base_accuracy = 100,
    .base_fp_cost = 3,
    .base_sp_cost = 0,
    .superguards_allowed = 1,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 1, 1, 2, 2, 3, 3, 0, MoveType::JUMP_POWER_BOUNCE },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::JUMPLIKE |
        AttackTargetProperty_Flags::CANNOT_TARGET_CEILING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_jump",
    .special_property_flags =
        AttackSpecialProperty_Flags::TOT_PARTY_UNGUARDABLE |
        AttackSpecialProperty_Flags::GROUNDS_WINGED |
        AttackSpecialProperty_Flags::FLIPS_SHELLED |
        AttackSpecialProperty_Flags::MAKES_ATTACK_FX_SOUND |
        AttackSpecialProperty_Flags::DIMINISHING_BY_HIT |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = 
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY |
        AttackCounterResistance_Flags::FRONT_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)marioAttackEvent_RenzokuJump,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 10,
    .nozzle_fire_chance = 5,
    .ceiling_fall_chance = 2,
    .object_fall_chance = 2,
};

BattleWeapon customWeapon_SleepyStomp = {
    .name = "in_nemurase_fumi",
    .icon = IconType::SLEEPY_STOMP,
    .item_id = ItemType::SLEEPY_STOMP,
    .description = nullptr,
    .base_accuracy = 100,
    .base_fp_cost = 2,
    .base_sp_cost = 0,
    .superguards_allowed = 1,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 2, 2, 2, 2, 2, 2, 0, MoveType::JUMP_SLEEPY_STOMP },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::JUMPLIKE |
        AttackTargetProperty_Flags::CANNOT_TARGET_CEILING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_jump",
    .special_property_flags =
        AttackSpecialProperty_Flags::TOT_PARTY_UNGUARDABLE |
        AttackSpecialProperty_Flags::GROUNDS_WINGED |
        AttackSpecialProperty_Flags::FLIPS_SHELLED |
        AttackSpecialProperty_Flags::MAKES_ATTACK_FX_SOUND |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = 
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY |
        AttackCounterResistance_Flags::FRONT_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .sleep_chance = 100,
    .sleep_time = 3,
    
    .attack_evt_code = (void*)marioAttackEvent_NemuraseFumi,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 6,
    .nozzle_fire_chance = 5,
    .ceiling_fall_chance = 1,
    .object_fall_chance = 1,
};

BattleWeapon customWeapon_TornadoJump = {
    .name = "in_tamatsuki_jump",
    .icon = IconType::TORNADO_JUMP,
    .item_id = ItemType::TORNADO_JUMP,
    .description = nullptr,
    .base_accuracy = 100,
    .base_fp_cost = 2,
    .base_sp_cost = 0,
    .superguards_allowed = 1,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 2, 4, 3, 6, 4, 8, 0, MoveType::JUMP_TORNADO_JUMP },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::JUMPLIKE |
        AttackTargetProperty_Flags::CANNOT_TARGET_CEILING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_tatsumaki_jump",
    .special_property_flags =
        AttackSpecialProperty_Flags::TOT_PARTY_UNGUARDABLE |
        AttackSpecialProperty_Flags::GROUNDS_WINGED |
        AttackSpecialProperty_Flags::FLIPS_SHELLED |
        AttackSpecialProperty_Flags::MAKES_ATTACK_FX_SOUND |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = 
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY |
        AttackCounterResistance_Flags::FRONT_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)marioAttackEvent_TatsumakiJump,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 0,
    .nozzle_fire_chance = 0,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_TornadoJumpRecoil = {
    .name = nullptr,
    .icon = IconType::TORNADO_JUMP,
    .item_id = ItemType::TORNADO_JUMP,
    .description = nullptr,
    .base_accuracy = 100,
    .base_fp_cost = 0,
    .base_sp_cost = 0,
    .superguards_allowed = 2,
    .unk_14 = 0.0,
    .stylish_multiplier = 0,
    .unk_19 = 0,
    .bingo_card_chance = 0,
    .unk_1b = 0,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 2, 2, 3, 3, 5, 5, 0, MoveType::JUMP_TORNADO_JUMP },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        // Can hit all enemies, including grounded.
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::TOT_PARTY_UNGUARDABLE |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = 0,
        
    // status chances (only for flying enemies)
    .dizzy_chance = 100,
    .dizzy_time = 2,
    
    .attack_evt_code = nullptr,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 0,
    .nozzle_fire_chance = 0,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_PowerPiercingSmash = {
    .name = "in_gatsun_naguri",
    .icon = IconType::POWER_SMASH,
    .item_id = ItemType::POWER_SMASH,
    .description = nullptr,
    .base_accuracy = 100,
    .base_fp_cost = 2,
    .base_sp_cost = 0,
    .superguards_allowed = 2,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 5, 6, 7, 9, 9, 12, 0, MoveType::HAMMER_POWER_SMASH },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags = 
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::ONLY_FRONT |
        AttackTargetProperty_Flags::HAMMERLIKE,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_hammer",
    .special_property_flags =
        AttackSpecialProperty_Flags::TOT_PARTY_UNGUARDABLE |
        AttackSpecialProperty_Flags::MAKES_ATTACK_FX_SOUND |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = 
        AttackCounterResistance_Flags::ALL &
        ~AttackCounterResistance_Flags::PREEMPTIVE_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // .status_chances = all 0,
    
    .attack_evt_code = (void*)marioAttackEvent_NormalHammer,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 25,
    .bg_a2_fall_weight = 25,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 25,
    .nozzle_turn_chance = 25,
    .nozzle_fire_chance = 25,
    .ceiling_fall_chance = 5,
    .object_fall_chance = 25,
};

BattleWeapon customWeapon_ShrinkSmash = {
    .name = "in_konran_hammer",
    .icon = IconType::HEAD_RATTLE,
    .item_id = ItemType::HEAD_RATTLE,
    .description = nullptr,
    .base_accuracy = 100,
    .base_fp_cost = 2,
    .base_sp_cost = 0,
    .superguards_allowed = 2,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 2, 4, 2, 4, 2, 4, 0, MoveType::HAMMER_SHRINK_SMASH },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags = 
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::ONLY_FRONT |
        AttackTargetProperty_Flags::HAMMERLIKE,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_hammer",
    .special_property_flags =
        AttackSpecialProperty_Flags::TOT_PARTY_UNGUARDABLE |
        AttackSpecialProperty_Flags::MAKES_ATTACK_FX_SOUND |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = 
        AttackCounterResistance_Flags::ALL &
        ~AttackCounterResistance_Flags::PREEMPTIVE_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .size_change_chance = 100,
    .size_change_time = 2,
    .size_change_strength = -2,
    
    .attack_evt_code = (void*)marioAttackEvent_NormalHammer,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 5,
    .bg_a2_fall_weight = 5,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 10,
    .nozzle_fire_chance = 5,
    .ceiling_fall_chance = 5,
    .object_fall_chance = 5,
};

BattleWeapon customWeapon_IceSmash = {
    .name = "in_ice_naguri",
    .icon = IconType::ICE_SMASH,
    .item_id = ItemType::ICE_SMASH,
    .description = nullptr,
    .base_accuracy = 100,
    .base_fp_cost = 3,
    .base_sp_cost = 0,
    .superguards_allowed = 2,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 2, 4, 2, 4, 2, 4, 0, MoveType::HAMMER_ICE_SMASH },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags = 
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::ONLY_FRONT |
        AttackTargetProperty_Flags::HAMMERLIKE,
    .element = AttackElement::ICE,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_hammer",
    .special_property_flags =
        AttackSpecialProperty_Flags::TOT_PARTY_UNGUARDABLE |
        AttackSpecialProperty_Flags::MAKES_ATTACK_FX_SOUND |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = 
        AttackCounterResistance_Flags::ALL &
        ~AttackCounterResistance_Flags::PREEMPTIVE_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .freeze_chance = 100,
    .freeze_time = 2,
    
    .attack_evt_code = (void*)marioAttackEvent_NormalHammer,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 5,
    .bg_a2_fall_weight = 5,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 10,
    .nozzle_fire_chance = 5,
    .ceiling_fall_chance = 5,
    .object_fall_chance = 5,
};

BattleWeapon customWeapon_QuakeHammer = {
    .name = "in_jishin_attack",
    .icon = IconType::QUAKE_HAMMER,
    .item_id = ItemType::QUAKE_HAMMER,
    .description = nullptr,
    .base_accuracy = 100,
    .base_fp_cost = 3,
    .base_sp_cost = 0,
    .superguards_allowed = 2,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 1, 2, 2, 4, 3, 6, 0, MoveType::HAMMER_QUAKE_HAMMER },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::CANNOT_TARGET_FLOATING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_hammer",
    .special_property_flags =
        AttackSpecialProperty_Flags::TOT_PARTY_UNGUARDABLE |
        AttackSpecialProperty_Flags::FLIPS_SHELLED |
        AttackSpecialProperty_Flags::FLIPS_BOMB |
        AttackSpecialProperty_Flags::MAKES_ATTACK_FX_SOUND |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)marioAttackEvent_JishinHammer,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 50,
    .bg_a2_fall_weight = 50,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 50,
    .nozzle_turn_chance = 25,
    .nozzle_fire_chance = 25,
    .ceiling_fall_chance = 5,
    .object_fall_chance = 25,
};

BattleWeapon customWeapon_FireDrive = {
    .name = "in_fire_naguri",
    .icon = IconType::FIRE_DRIVE,
    .item_id = ItemType::FIRE_DRIVE,
    .description = nullptr,
    .base_accuracy = 100,
    .base_fp_cost = 3,
    .base_sp_cost = 0,
    .superguards_allowed = 2,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 3, 3, 5, 5, 7, 7, 0, MoveType::HAMMER_FIRE_DRIVE },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::HAMMERLIKE,
    .element = AttackElement::FIRE,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_fire_naguri",
    .special_property_flags =
        AttackSpecialProperty_Flags::TOT_PARTY_UNGUARDABLE |
        AttackSpecialProperty_Flags::MAKES_ATTACK_FX_SOUND |
        AttackSpecialProperty_Flags::DIMINISHING_BY_TARGET |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .burn_chance = 100,
    .burn_time = 3,
    
    .attack_evt_code = (void*)marioAttackEvent_FireNaguri,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 10,
    .nozzle_fire_chance = 5,
    .ceiling_fall_chance = 5,
    .object_fall_chance = 5,
};

BattleWeapon customWeapon_FireDriveFailed = {
    .name = nullptr,
    .icon = 0,
    .item_id = ItemType::FIRE_DRIVE,
    .description = nullptr,
    .base_accuracy = 100,
    .base_fp_cost = 3,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 1,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 1, 2, 2, 4, 3, 6, 0, MoveType::HAMMER_FIRE_DRIVE },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_OPPOSING_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_SAME_ALLIANCE_DIR |
        AttackTargetProperty_Flags::HAMMERLIKE,
    .element = AttackElement::FIRE,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_fire_naguri",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .burn_chance = 100,
    .burn_time = 3,
    
    .attack_evt_code = (void*)marioAttackEvent_FireNaguri,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 10,
    .nozzle_fire_chance = 5,
    .ceiling_fall_chance = 5,
    .object_fall_chance = 5,
};

// Throw variants of single-target Hammer moves, if Hammerman is equipped.

BattleWeapon customWeapon_HammerThrow = {
    .name = "btl_cmd_act_hammer",
    .icon = IconType::HAMMER,
    .item_id = 0,
    .description = "msg_normal_hammer",
    .base_accuracy = 100,
    .base_fp_cost = 0,
    .base_sp_cost = 0,
    .superguards_allowed = 2,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 1,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 2, 4, 3, 6, 4, 8, 0, MoveType::HAMMER_BASE },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags = 
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_hammer_nageru",
    .special_property_flags =
        AttackSpecialProperty_Flags::TOT_PARTY_UNGUARDABLE |
        AttackSpecialProperty_Flags::USABLE_IF_CONFUSED |
        AttackSpecialProperty_Flags::MAKES_ATTACK_FX_SOUND |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // .status_chances = all 0,
    
    .attack_evt_code = (void*)marioAttackEvent_HammerNageru,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 0,
    .nozzle_fire_chance = 0,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_PowerPiercingSmashThrow = {
    .name = "in_gatsun_naguri",
    .icon = IconType::POWER_SMASH,
    .item_id = ItemType::POWER_SMASH,
    .description = nullptr,
    .base_accuracy = 100,
    .base_fp_cost = 2,
    .base_sp_cost = 0,
    .superguards_allowed = 2,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 5, 6, 7, 9, 9, 12, 0, MoveType::HAMMER_POWER_SMASH },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags = 
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_hammer_nageru",
    .special_property_flags =
        AttackSpecialProperty_Flags::TOT_PARTY_UNGUARDABLE |
        AttackSpecialProperty_Flags::MAKES_ATTACK_FX_SOUND |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // .status_chances = all 0,
    
    .attack_evt_code = (void*)marioAttackEvent_HammerNageru,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 0,
    .nozzle_fire_chance = 0,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_ShrinkSmashThrow = {
    .name = "in_konran_hammer",
    .icon = IconType::HEAD_RATTLE,
    .item_id = ItemType::HEAD_RATTLE,
    .description = nullptr,
    .base_accuracy = 100,
    .base_fp_cost = 2,
    .base_sp_cost = 0,
    .superguards_allowed = 2,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 2, 4, 2, 4, 2, 4, 0, MoveType::HAMMER_SHRINK_SMASH },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags = 
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_hammer_nageru",
    .special_property_flags =
        AttackSpecialProperty_Flags::TOT_PARTY_UNGUARDABLE |
        AttackSpecialProperty_Flags::MAKES_ATTACK_FX_SOUND |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .size_change_chance = 100,
    .size_change_time = 2,
    .size_change_strength = -2,
    
    .attack_evt_code = (void*)marioAttackEvent_HammerNageru,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 0,
    .nozzle_fire_chance = 0,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_IceSmashThrow = {
    .name = "in_ice_naguri",
    .icon = IconType::ICE_SMASH,
    .item_id = ItemType::ICE_SMASH,
    .description = nullptr,
    .base_accuracy = 100,
    .base_fp_cost = 3,
    .base_sp_cost = 0,
    .superguards_allowed = 2,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 2, 4, 2, 4, 2, 4, 0, MoveType::HAMMER_ICE_SMASH },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags = 
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::ICE,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_hammer_nageru",
    .special_property_flags =
        AttackSpecialProperty_Flags::TOT_PARTY_UNGUARDABLE |
        AttackSpecialProperty_Flags::MAKES_ATTACK_FX_SOUND |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .freeze_chance = 100,
    .freeze_time = 2,
    
    .attack_evt_code = (void*)marioAttackEvent_HammerNageru,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 0,
    .nozzle_fire_chance = 0,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

}  // namespace mod::tot::party_mario