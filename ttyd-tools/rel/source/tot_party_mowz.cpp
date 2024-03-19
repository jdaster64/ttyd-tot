#include "tot_party_mowz.h"

#include "evt_cmd.h"
#include "mod.h"
#include "tot_generate_item.h"
#include "tot_manager_move.h"
#include "tot_state.h"

#include <ttyd/battle.h>
#include <ttyd/battle_camera.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_event_default.h>
#include <ttyd/battle_event_subset.h>
#include <ttyd/battle_weapon_power.h>
#include <ttyd/battle_icon.h>
#include <ttyd/battle_message.h>
#include <ttyd/battle_sub.h>
#include <ttyd/battle_unit.h>
#include <ttyd/evt_audience.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/icondrv.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/msgdrv.h>
#include <ttyd/npcdrv.h>
#include <ttyd/unit_party_chuchurina.h>

namespace mod::tot::party_mowz {

namespace {
    
// Including entire namespaces for convenience.
using namespace ::ttyd::battle_camera;
using namespace ::ttyd::battle_database_common;
using namespace ::ttyd::battle_event_cmd;
using namespace ::ttyd::battle_event_default;
using namespace ::ttyd::battle_event_subset;
using namespace ::ttyd::battle_icon;
using namespace ::ttyd::battle_message;
using namespace ::ttyd::battle_weapon_power;
using namespace ::ttyd::evt_audience;
using namespace ::ttyd::evt_eff;
using namespace ::ttyd::evt_snd;
using namespace ::ttyd::evt_sub;
using namespace ::ttyd::unit_party_chuchurina;

using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

namespace IconType = ::ttyd::icondrv::IconType;
namespace ItemType = ::ttyd::item_data::ItemType;

}  // namespace

// Declaration of weapon structs.
extern BattleWeapon customWeapon_MowzLoveSlapL;
extern BattleWeapon customWeapon_MowzLoveSlapR;
extern BattleWeapon customWeapon_MowzLoveSlapLFinal;
extern BattleWeapon customWeapon_MowzLoveSlapRFinal;
extern BattleWeapon customWeapon_MowzKissThief;
extern BattleWeapon customWeapon_MowzTease;
extern BattleWeapon customWeapon_MowzSmooch;
extern BattleWeapon customWeapon_MowzEmbargo;
extern BattleWeapon customWeapon_MowzSmokeBomb;

BattleWeapon* g_WeaponTable[] = {
    &customWeapon_MowzLoveSlapLFinal, &customWeapon_MowzKissThief, 
    &customWeapon_MowzTease, &customWeapon_MowzEmbargo,
    &customWeapon_MowzSmokeBomb, &customWeapon_MowzSmooch
};

void MakeSelectWeaponTable(
    ttyd::battle::BattleWorkCommand* command_work, int32_t* num_options) {
    for (int32_t i = 0; i < 6; ++i) {
        auto& weapon_entry = command_work->weapon_table[*num_options];
        BattleWeapon* weapon = g_WeaponTable[i];
        
        if (MoveManager::GetUnlockedLevel(MoveType::MOWZ_BASE + i)) {
            weapon_entry.index = MoveType::MOWZ_BASE + i;
            weapon_entry.item_id = 0;
            weapon_entry.weapon = weapon;
            weapon_entry.icon = weapon->icon;
            weapon_entry.unk_04 = 0;
            weapon_entry.unk_18 = 0;
            weapon_entry.name = ttyd::msgdrv::msgSearch(weapon->name);
            
            ++*num_options;
        }
    }
}

// Gets an enemy's item held position.
EVT_DECLARE_USER_FUNC(evtTot_GetHeldItemPosition, 4)
EVT_DEFINE_USER_FUNC(evtTot_GetHeldItemPosition) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, id);
    
    evtSetValue(evt, evt->evtArguments[1], 
        unit->position.x + unit->held_item_base_offset.x);
    evtSetValue(evt, evt->evtArguments[2], 
        unit->position.y + unit->held_item_base_offset.y);
    evtSetValue(evt, evt->evtArguments[3], 
        unit->position.z + unit->held_item_base_offset.z);
    
    return 2;
}

// Removes item from enemy and gets item id.
EVT_DECLARE_USER_FUNC(evtTot_ConfiscateItem, 2)
EVT_DEFINE_USER_FUNC(evtTot_ConfiscateItem) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, id);
    
    int32_t item = unit->held_item;
    unit->held_item = 0;
    evtSetValue(evt, evt->evtArguments[1], item);
    
    return 2;
}

// Dynamically sets the damage and status chance parameters based on AC success.
EVT_DECLARE_USER_FUNC(evtTot_MakeTeaseWeapon, 3)
EVT_DEFINE_USER_FUNC(evtTot_MakeTeaseWeapon) {
    auto* weapon = (BattleWeapon*)evtGetValue(evt, evt->evtArguments[0]);
    int32_t ac_result = evtGetValue(evt, evt->evtArguments[1]);
    int32_t move_type = evtGetValue(evt, evt->evtArguments[2]);
    int32_t move_level = MoveManager::GetSelectedLevel(move_type);
    
    // Make changes in place, since the parameters are unchanged between uses.
    if (move_type == MoveType::MOWZ_TEASE) {
        weapon->confuse_chance = ac_result * 1.27;
        weapon->confuse_time = 3;
    } else {  // Smoke Bomb
        int32_t success_level = 1 + (move_level + 2) * ac_result / 100;
        weapon->damage_function_params[0] = success_level;
        weapon->dizzy_chance = 20 * success_level;
        weapon->dizzy_time = 1;
    }
    
    return 2;
}

// Replaces vanilla logic for choosing items to steal.
// TODO: Rework as appropriate for TOT.
EVT_DECLARE_USER_FUNC(evtTot_GetKissThiefResult, 3)
EVT_DEFINE_USER_FUNC(evtTot_GetKissThiefResult) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, id);
    uint32_t ac_result = evtGetValue(evt, evt->evtArguments[2]);
    
    int32_t item = unit->held_item;
    // No held item; pick a random item to steal;
    // 30% chance of item (20% normal, 10% recipe), 10% badge, 60% coin.
    if (!item) {
        item = PickRandomItem(RNG_STOLEN_ITEM, 20, 10, 10, 60);
        if (!item) item = ItemType::COIN;
    }
    if ((ac_result & 2) == 0 || item == ItemType::STAR_PIECE ||
        !ttyd::mario_pouch::pouchGetItem(item)) {
        // Action command unsuccessful, item = Star Piece (can't be stolen),
        // or the player's inventory cannot hold the item.
        evtSetValue(evt, evt->evtArguments[1], 0);
    } else {
        // Remove the unit's held item.
        unit->held_item = 0;
        
        // Remove the corresponding held/stolen item from the NPC setup,
        // if this was one of the initial enemies in the loadout.
        if (!ttyd::battle_unit::BtlUnit_CheckUnitFlag(unit, 0x40000000)) {
            if (unit->group_index >= 0) {
                battleWork->fbat_info->wBattleInfo->wHeldItems
                    [unit->group_index] = 0;
            }
        } else {
            ttyd::battle_unit::BtlUnit_OffUnitFlag(unit, 0x40000000);
            if (unit->group_index >= 0) {
                auto* npc_battle_info = battleWork->fbat_info->wBattleInfo;
                npc_battle_info->wHeldItems[unit->group_index] = 0;
                npc_battle_info->wStolenItems[unit->group_index] = 0;
            }
        }
        
        // If a badge was stolen, re-equip the target unit's remaining badges.
        if (item >= ItemType::POWER_JUMP && item < ItemType::MAX_ITEM_TYPE) {
            int32_t kind = unit->current_kind;
            if (kind == BattleUnitType::MARIO) {
                ttyd::battle::BtlUnit_EquipItem(unit, 3, 0);
            } else if (kind >= BattleUnitType::GOOMBELLA) {
                ttyd::battle::BtlUnit_EquipItem(unit, 5, 0);
            } else {
                ttyd::battle::BtlUnit_EquipItem(unit, 1, 0);
            }
        }
        
        evtSetValue(evt, evt->evtArguments[1], item);
    }
    return 2;
}

EVT_BEGIN(partyChuchurinaAttack_NormalAttack)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.70))
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    USER_FUNC(evt_btl_camera_set_mode, 0, 11)
    USER_FUNC(evt_btl_camera_set_homing_unitparts, 0, -2, 1, LW(3), LW(4))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(3), LW(4), 256, LW(5))
    IF_EQUAL(LW(5), 5)
        USER_FUNC(btlevtcmd_GetStatusMg, LW(3), LW(6))
        MULF(LW(6), 40)
        USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), LW(6))
        USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
        USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
        USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 256, LW(5))
    END_IF()
    USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(5))
    MULF(LW(5), 12)
    USER_FUNC(_get_binta_hit_position, -2, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), LW(5))
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
    ADD(LW(2), 5)
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 350)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PCH_A1_1"))
    BROTHER_EVT_ID(LW(0))
        USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
        USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
        USER_FUNC(btlevtcmd_AcGetDifficulty, LW(0))
        SUB(LW(0), 3)
        SWITCH(LW(0))
            CASE_SMALL_EQUAL(-3)
                SET(LW(1), 25)
                SET(LW(2), 30)
            CASE_EQUAL(-2)
                SET(LW(1), 20)
                SET(LW(2), 30)
            CASE_EQUAL(-1)
                SET(LW(1), 17)
                SET(LW(2), 30)
            CASE_EQUAL(0)
                SET(LW(1), 15)
                SET(LW(2), 30)
            CASE_EQUAL(1)
                SET(LW(1), 14)
                SET(LW(2), 30)
            CASE_EQUAL(2)
                SET(LW(1), 13)
                SET(LW(2), 30)
            CASE_ETC()
                SET(LW(1), 12)
                SET(LW(2), 30)
        END_SWITCH()
        // Change gauge parameters based on move level (up to 2-4 extra hits).
        USER_FUNC(evtTot_GetMoveSelectedLevel, MoveType::MOWZ_BASE, LW(0))
        SWITCH(LW(0))
            CASE_EQUAL(1)
                USER_FUNC(btlevtcmd_AcSetParamAll, 14, 210, 178, LW(1), LW(2), 50, 51, EVT_NULLPTR)
                USER_FUNC(btlevtcmd_AcSetGaugeParam, 50, 100, 100, 100)
            CASE_EQUAL(2)
                USER_FUNC(btlevtcmd_AcSetParamAll, 14, 210, 178, LW(1), LW(2), 34, 35, EVT_NULLPTR)
                USER_FUNC(btlevtcmd_AcSetGaugeParam, 34, 67, 100, 100)
            CASE_ETC()
                USER_FUNC(btlevtcmd_AcSetParamAll, 14, 210, 178, LW(1), LW(2), 25, 26, EVT_NULLPTR)
                USER_FUNC(btlevtcmd_AcSetGaugeParam, 25, 50, 75, 100)
        END_SWITCH()
        // Start with 1 base hit.
        USER_FUNC(btlevtcmd_AcSetFlag, 64)
        USER_FUNC(btlevtcmd_SetupAC, -2, 6, 1, 0)
        WAIT_FRM(1)
        USER_FUNC(btlevtcmd_StartAC, 1)
    END_BROTHER()
    USER_FUNC(btlevtcmd_WaitEventEnd, LW(0))
    BROTHER_EVT_ID(LW(14))
        SET(LW(6), 0)
        SET(LW(5), 5)
        DO(LW(5))
            ADD(LW(6), 1)
            SET(LW(0), LW(6))
            DIV(LW(0), 2)
            MOD(LW(0), 2)
            IF_EQUAL(LW(0), 0)
                USER_FUNC(btlevtcmd_SetScale, -2, FLOAT(1.05), FLOAT(1.05), FLOAT(1.0))
            ELSE()
                USER_FUNC(btlevtcmd_SetScale, -2, FLOAT(1.0), FLOAT(1.0), FLOAT(1.0))
            END_IF()
            WAIT_FRM(1)
            SET(LW(5), 5)
        WHILE()
    END_BROTHER()
    BROTHER_EVT_ID(LW(15))
        SET(LW(5), 5)
        DO(LW(5))
            USER_FUNC(btlevtcmd_AcGetOutputParam, 1, LW(0))
            SET(LW(1), 255)
            SET(LW(2), 190)
            MUL(LW(2), LW(0))
            DIV(LW(2), 100)
            SUB(LW(1), LW(2))
            USER_FUNC(btlevtcmd_SetRGB, -2, 1, 255, LW(1), LW(1))
            WAIT_FRM(1)
            SET(LW(5), 5)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_ResultAC)
    DELETE_EVT(LW(14))
    DELETE_EVT(LW(15))
    // Number of hits based on AC output param 2, rather than move level.
    USER_FUNC(btlevtcmd_AcGetOutputParam, 2, LW(0))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 5, LW(0))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 300)
    USER_FUNC(btlevtcmd_SetRGB, -2, 1, 255, 255, 255)
    BROTHER_EVT_ID(LW(14))
        SET(LW(8), -75)
        USER_FUNC(btlevtcmd_GetRotate, -2, EVT_NULLPTR, LW(0), EVT_NULLPTR)
        IF_LARGE_EQUAL(LW(0), 180)
            SUB(LW(0), 360)
        END_IF()
        USER_FUNC(evt_sub_intpl_init, 5, LW(0), LW(8), 8)
        DO(8)
            USER_FUNC(evt_sub_intpl_get_value)
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, LW(0), 0)
            WAIT_FRM(1)
        WHILE()
        SET(LW(8), -60)
        SET(LW(9), -75)
        USER_FUNC(evt_sub_intpl_init, 11, LW(9), LW(8), 4)
        DO(4)
            USER_FUNC(evt_sub_intpl_get_value)
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, LW(0), 0)
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_WaitEventEnd, LW(14))
    SET(LW(13), 18)
    SET(LW(10), 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 4, 0)
    LBL(10)
    ADD(LW(10), 1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PCH_A1_2"))
    IF_LARGE_EQUAL(LW(10), 2)
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
    IF_NOT_EQUAL(LW(5), 1)
        DELETE_EVT(LW(14))
        SET(LW(14), 0)
        USER_FUNC(btlevtcmd_StopAC)
        IF_EQUAL(LW(5), 3)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
        END_IF()
        IF_EQUAL(LW(5), 6)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
        END_IF()
        IF_EQUAL(LW(5), 2)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
        END_IF()
        DO(20)
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 18, 0)
            WAIT_FRM(1)
        WHILE()
        DO(36)
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 10, 0)
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PCH_A2_4"))
        WAIT_FRM(52)
        // USER_FUNC(btlevtcmd_InviteApInfoReport)
        GOTO(95)
    END_IF()
    SET(LW(0), LW(10))
    MOD(LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        SET(LW(12), PTR(&customWeapon_MowzLoveSlapR))
    ELSE()
        SET(LW(12), PTR(&customWeapon_MowzLoveSlapL))
    END_IF()
    USER_FUNC(btlevtcmd_GetUnitWork, -2, 4, LW(15))
    IF_NOT_EQUAL(LW(15), 0)
        DELETE_EVT(LW(15))
        USER_FUNC(btlevtcmd_SetUnitWork, -2, 4, 0)
    END_IF()
    IF_NOT_EQUAL(LW(14), 0)
        DELETE_EVT(LW(14))
        SET(LW(14), 0)
    END_IF()
    USER_FUNC(btlevtcmd_GetUnitWork, -2, 5, LW(0))
    IF_LARGE_EQUAL(LW(10), LW(0))
        GOTO(20)
    ELSE()
        USER_FUNC(btlevtcmd_GetResultAC, LW(6))
        IF_NOT_FLAG(LW(6), 0x2)
            GOTO(20)
        END_IF()
    END_IF()
    GOTO(21)
    LBL(20)
    IF_EQUAL(LW(12), PTR(&customWeapon_MowzLoveSlapL))
        SET(LW(12), PTR(&customWeapon_MowzLoveSlapLFinal))
    ELSE()
        SET(LW(12), PTR(&customWeapon_MowzLoveSlapRFinal))
    END_IF()
    LBL(21)
    USER_FUNC(btlevtcmd_PreCheckCounter, -2, LW(3), LW(4), LW(12), 256, LW(0))
    SWITCH(LW(0))
        CASE_OR(7)
        CASE_OR(8)
        CASE_OR(9)
        CASE_OR(10)
        CASE_OR(11)
        CASE_OR(12)
        CASE_OR(13)
        CASE_OR(14)
        CASE_OR(15)
        CASE_OR(16)
        CASE_OR(17)
            IF_EQUAL(LW(12), PTR(&customWeapon_MowzLoveSlapL))
                SET(LW(12), PTR(&customWeapon_MowzLoveSlapLFinal))
            ELSE()
                SET(LW(12), PTR(&customWeapon_MowzLoveSlapRFinal))
            END_IF()
            BROTHER_EVT_ID(LW(15))
                SET(LW(7), LW(13))
                USER_FUNC(btlevtcmd_GetRotate, -2, EVT_NULLPTR, LW(0), EVT_NULLPTR)
                // Note: Missing end-if in original code makes execution jump
                // to the start of the next event!
                IF_LARGE_EQUAL(LW(0), 180)
                    SUB(LW(0), 360)
                END_IF()
                USER_FUNC(evt_sub_intpl_init, 11, LW(0), 0, 4)
                DO(4)
                    USER_FUNC(evt_sub_intpl_get_value)
                    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, LW(0))
                    WAIT_FRM(1)
                    IF_EQUAL(LW(1), 1)
                        DO_BREAK()
                    END_IF()
                WHILE()
                USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
            END_BROTHER()
            USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
            USER_FUNC(btlevtcmd_GetResultAC, LW(6))
            IF_FLAG(LW(6), 0x2)
                USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 131328, LW(5))
            ELSE()
                USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
            END_IF()
            RETURN()
        CASE_END()
    END_SWITCH()
    USER_FUNC(evt_btl_camera_shake_h, 0, 2, 0, 12, 13)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, 5, LW(0))
    IF_LARGE_EQUAL(LW(10), LW(0))
        USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_CHUCHURINA_ATTACK3"), 0)
        IF_FLAG(LW(6), 0x2)
            USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 131328, LW(5))
        ELSE()
            USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
        END_IF()
    ELSE()
        SET(LW(0), LW(10))
        AND(LW(0), 1)
        IF_EQUAL(LW(0), 0)
            USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_CHUCHURINA_ATTACK2"), 0)
        ELSE()
            USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_CHUCHURINA_ATTACK1"), 0)
        END_IF()
        IF_FLAG(LW(6), 0x2)
            USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 131072, LW(5))
        ELSE()
            USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
        END_IF()
    END_IF()
    USER_FUNC(btlevtcmd_GetResultAC, LW(6))
    IF_FLAG(LW(6), 0x2)
        USER_FUNC(btlevtcmd_AcGetOutputParam, 2, LW(0))
        // Changed Nice / Good / Great thresholds to 2, 4, 5 hits.
        IF_EQUAL(LW(0), 2)
            ADD(LW(0), 1)
        END_IF()
        SUB(LW(0), 3)
        IF_LARGE_EQUAL(LW(0), 0)
            USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), LW(0), LW(11))
            USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_ACSuccessEffect, LW(11), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(11))
        ELSE()
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
        END_IF()
    ELSE()
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
    END_IF()
    IF_FLAG(LW(6), 0x2)
        SWITCH(LW(10))
            CASE_SMALL_EQUAL(2)
                BROTHER_EVT_ID(LW(15))
                    SET(LW(7), LW(13))
                    IF_LARGE_EQUAL(LW(7), 0)
                        SET(LW(8), 75)
                    ELSE()
                        SET(LW(8), -75)
                    END_IF()
                    USER_FUNC(btlevtcmd_GetRotate, -2, EVT_NULLPTR, LW(0), EVT_NULLPTR)
                    IF_LARGE_EQUAL(LW(0), 180)
                        SUB(LW(0), 360)
                    END_IF()
                    USER_FUNC(evt_sub_intpl_init, 5, LW(0), LW(8), 14)
                    DO(14)
                        USER_FUNC(evt_sub_intpl_get_value)
                        USER_FUNC(btlevtcmd_SetRotate, -2, 0, LW(0), 0)
                        WAIT_FRM(1)
                    WHILE()
                    USER_FUNC(btlevtcmd_GetUnitWork, -2, 5, LW(0))
                    IF_SMALL(LW(10), LW(0))
                        SET(LW(7), LW(13))
                        IF_LARGE_EQUAL(LW(7), 0)
                            SET(LW(8), 75)
                            SET(LW(9), 75)
                        ELSE()
                            SET(LW(8), -75)
                            SET(LW(9), -75)
                        END_IF()
                        USER_FUNC(evt_sub_intpl_init, 11, LW(9), LW(8), 6)
                        DO(6)
                            USER_FUNC(evt_sub_intpl_get_value)
                            USER_FUNC(btlevtcmd_SetRotate, -2, 0, LW(0), 0)
                            WAIT_FRM(1)
                        WHILE()
                    ELSE()
                        SET(LW(7), LW(13))
                        IF_LARGE_EQUAL(LW(7), 0)
                            SET(LW(8), 0)
                            SET(LW(9), 75)
                        ELSE()
                            SET(LW(8), 0)
                            SET(LW(9), -75)
                        END_IF()
                        USER_FUNC(evt_sub_intpl_init, 11, LW(9), LW(8), 12)
                        DO(12)
                            USER_FUNC(evt_sub_intpl_get_value)
                            USER_FUNC(btlevtcmd_SetRotate, -2, 0, LW(0), 0)
                            WAIT_FRM(1)
                        WHILE()
                    END_IF()
                END_BROTHER()
                USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
            CASE_ETC()
                BROTHER_EVT_ID(LW(15))
                    USER_FUNC(btlevtcmd_GetRotate, -2, EVT_NULLPTR, LW(8), EVT_NULLPTR)
                    SET(LW(7), 360)
                    USER_FUNC(btlevtcmd_GetUnitWork, -2, 5, LW(0))
                    IF_LARGE_EQUAL(LW(10), LW(0))
                        SET(LW(0), LW(8))
                        IF_LARGE_EQUAL(LW(8), 180)
                            SUB(LW(0), 360)
                        END_IF()
                        IF_LARGE_EQUAL(LW(13), 0)
                            SUB(LW(7), LW(0))
                        ELSE()
                            ADD(LW(7), LW(0))
                        END_IF()
                    ELSE()
                        ADD(LW(7), 60)
                        SET(LW(0), LW(8))
                        IF_LARGE_EQUAL(LW(8), 180)
                            SUB(LW(0), 360)
                        END_IF()
                        IF_LARGE_EQUAL(LW(13), 0)
                            SUB(LW(7), LW(0))
                        ELSE()
                            ADD(LW(7), LW(0))
                        END_IF()
                    END_IF()
                    IF_SMALL(LW(13), 0)
                        MUL(LW(7), -1)
                    END_IF()
                    USER_FUNC(evt_sub_intpl_init, 5, 0, LW(7), 20)
                    DO(20)
                        USER_FUNC(evt_sub_intpl_get_value)
                        ADD(LW(0), LW(8))
                        USER_FUNC(btlevtcmd_SetRotate, -2, 0, LW(0), 0)
                        WAIT_FRM(1)
                    WHILE()
                END_BROTHER()
                USER_FUNC(btlevtcmd_SetUnitWork, -2, 4, LW(15))
                WAIT_FRM(10)
        END_SWITCH()
    ELSE()
        BROTHER_EVT_ID(LW(15))
            USER_FUNC(btlevtcmd_GetRotate, -2, EVT_NULLPTR, LW(8), EVT_NULLPTR)
            SET(LW(7), 720)
            IF_LARGE_EQUAL(LW(10), 1)
                SET(LW(0), LW(8))
                IF_LARGE_EQUAL(LW(8), 180)
                    SUB(LW(0), 360)
                END_IF()
                SUB(LW(7), LW(0))
            END_IF()
            IF_SMALL(LW(13), 0)
                MUL(LW(7), -1)
            END_IF()
            USER_FUNC(evt_sub_intpl_init, 0, 0, LW(7), 60)
            DO(60)
                USER_FUNC(evt_sub_intpl_get_value)
                ADD(LW(0), LW(8))
                USER_FUNC(btlevtcmd_SetRotate, -2, 0, LW(0), 0)
                WAIT_FRM(1)
            WHILE()
        END_BROTHER()
        USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PCH_X_1"))
        WAIT_FRM(60)
        USER_FUNC(btlevtcmd_StopAC)
        // USER_FUNC(btlevtcmd_InviteApInfoReport)
        GOTO(95)
    END_IF()
    MUL(LW(13), -1)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, 5, LW(0))
    IF_SMALL(LW(10), LW(0))
        GOTO(10)
    END_IF()
    LBL(90)
    USER_FUNC(btlevtcmd_StopAC)
    // USER_FUNC(btlevtcmd_InviteApInfoReport)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PCH_A1_3"))
    WAIT_FRM(40)
    LBL(95)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
    USER_FUNC(btlevtcmd_GetMoveFrame, -2, LW(0), LW(1), LW(2), 0, LW(14))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    BROTHER_EVT_ID(LW(15))
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), LW(14), -1, 0)
    END_BROTHER()
    SET(LW(11), LW(14))
    SUB(LW(11), 11)
    SUB(LW(14), 1)
    IF_LARGE(LW(14), LW(11))
        SET(LW(13), LW(14))
        SUB(LW(13), 10)
        IF_FLAG(LW(6), 0x2)
            USER_FUNC(btlevtcmd_ACRStart, -2, 0, LW(13), LW(14), 0)
            USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
            IF_LARGE_EQUAL(LW(6), 2)
                DELETE_EVT(LW(15))
                USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.5))
                USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 30)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PCH_O_1"))
                USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
                USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
                USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
                USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
                USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
                USER_FUNC(btlevtcmd_SetHomePos, -2, LW(0), LW(1), LW(2))
            ELSE()
                USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
            END_IF()
        ELSE()
            USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
        END_IF()
    ELSE()
        USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
    END_IF()
    LBL(99)
    // Move Star Power generation to the end so Stylishes affect it.
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(partyChuchurinaAttack_ItemSteal)
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    SET(LW(14), 0)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), PTR(&customWeapon_MowzKissThief), 256, LW(5))
    IF_EQUAL(LW(5), 5)
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
        USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
        USER_FUNC(btlevtcmd_GetWidth, LW(3), LW(15))
        DIV(LW(15), 2)
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), LW(15))
        USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 20)
        USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetMoveFrame, -2, LW(0), LW(1), LW(2), 0, LW(8))
        BROTHER_EVT_ID(LW(9))
            USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), LW(8), -1, 0)
        END_BROTHER()
        IF_LARGE(LW(8), 5)
            SUB(LW(8), 2)
            SET(LW(15), LW(8))
            DIV(LW(15), 3)
            SET(LW(8), LW(15))
            MUL(LW(8), 2)
            USER_FUNC(btlevtcmd_ACRStart, -2, LW(15), LW(8), LW(8), 0)
            USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
            SWITCH(LW(6))
                CASE_LARGE_EQUAL(2)
                    DELETE_EVT(LW(9))
                    USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
                    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PCH_Y_1"))
                    USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
                    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
                    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
                CASE_ETC()
                    USER_FUNC(evt_audience_acrobat_notry)
            END_SWITCH()
        ELSE()
            USER_FUNC(btlevtcmd_WaitEventEnd, LW(9))
        END_IF()
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&customWeapon_MowzKissThief), 256, LW(5))
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
    USER_FUNC(btlevtcmd_GetWidth, LW(3), LW(15))
    MUL(LW(15), 2)
    DIV(LW(15), 3)
    ADD(LW(15), 10)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), LW(15))
    ADD(LW(2), 8)
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetMoveFrame, -2, LW(0), LW(1), LW(2), 0, LW(8))
    BROTHER_EVT_ID(LW(9))
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), LW(8), -1, 0)
    END_BROTHER()
    IF_LARGE(LW(8), 5)
        SUB(LW(8), 2)
        SET(LW(15), LW(8))
        DIV(LW(15), 3)
        SET(LW(8), LW(15))
        MUL(LW(8), 2)
        USER_FUNC(btlevtcmd_ACRStart, -2, LW(15), LW(8), LW(8), 0)
        USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
        SWITCH(LW(6))
            CASE_LARGE_EQUAL(2)
                DELETE_EVT(LW(9))
                USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PCH_Y_1"))
                USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
                USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
                USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
            CASE_ETC()
                USER_FUNC(evt_audience_acrobat_notry)
                USER_FUNC(btlevtcmd_WaitEventEnd, LW(9))
        END_SWITCH()
    ELSE()
        USER_FUNC(btlevtcmd_WaitEventEnd, LW(9))
    END_IF()
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, 1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 350)
    WAIT_MSEC(200)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PCH_K_1"))
    WAIT_FRM(6)
    IF_NOT_EQUAL(LW(5), 1)
        WAIT_FRM(20)
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 43)
        SWITCH(LW(5))
            CASE_OR(4)
                CASE_END()
            CASE_EQUAL(3)
                USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
            CASE_EQUAL(6)
                USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
            CASE_EQUAL(2)
                USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
            CASE_EQUAL(1)
            CASE_ETC()
        END_SWITCH()
        WAIT_FRM(30)
        USER_FUNC(btlevtcmd_StopAC)
        GOTO(90)
    END_IF()
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 350)
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&customWeapon_MowzKissThief), 256, LW(5))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHeight, -2, LW(15))
    MUL(LW(15), 2)
    DIV(LW(15), 3)
    ADD(LW(1), LW(15))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 20)
    USER_FUNC(evt_eff, PTR(""), PTR("kiss"), 0, LW(0), LW(1), LW(2), 1, 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_CHURINA_KISS1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_OnUnitFlag, LW(3), 33554432)
    WAIT_FRM(20)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PCH_Z_1"))
    USER_FUNC(evt_btl_camera_set_mode, 0, 3)
    USER_FUNC(btlevtcmd_AnimeFlagOnOff, -2, 1, 256, 1)
    WAIT_FRM(1)
    USER_FUNC(mono_capture_event)
    RUN_EVT_ID(PTR(&main_evt), LW(13))
    INLINE_EVT()
        USER_FUNC(mono_on)
    END_INLINE()
    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
    USER_FUNC(_get_itemsteal_param, LW(3), LW(0), LW(1))
    USER_FUNC(btlevtcmd_AcSetParamAll, 1, LW(0), 2, 2, LW(1), EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AcSetFlag, 0)
    USER_FUNC(btlevtcmd_SetupAC, -2, 15, 1, 0)
    WAIT_FRM(22)
    USER_FUNC(btlevtcmd_StartAC, 1)
    USER_FUNC(btlevtcmd_ResultAC)
    USER_FUNC(btlevtcmd_GetResultAC, LW(6))
    IF_FLAG(LW(6), 0x2)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PCH_A4_1"))
    END_IF()
    WAIT_FRM(35)
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_CHURINA_EYE1"), 0)
    WAIT_FRM(47)
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetWidth, LW(3), LW(15))
    DIV(LW(15), 2)
    ADD(LW(0), LW(15))
    USER_FUNC(btlevtcmd_GetHeight, LW(3), LW(15))
    ADD(LW(1), LW(15))
    ADD(LW(1), 5)
    ADD(LW(2), 8)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 16, -1)
    USER_FUNC(btlevtcmd_GetResultAC, LW(6))
    USER_FUNC(evtTot_GetKissThiefResult, LW(3), LW(14), LW(6))
    IF_EQUAL(LW(14), 0)
        USER_FUNC(btlevtcmd_JumpContinue, -2)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PCH_F_1"))
        WAIT_FRM(22)
        INLINE_EVT()
            USER_FUNC(mono_off)
            DELETE_EVT(LW(13))
        END_INLINE()
        WAIT_FRM(20)
        USER_FUNC(btlevtcmd_OffUnitFlag, LW(3), 33554432)
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
        USER_FUNC(evt_btl_camera_set_mode, 0, 0)
        USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
        USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PCH_R_2"))
        USER_FUNC(btlevtcmd_AddRotate, -2, 0, 180, 0)
        USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 0, 0, 2, 0, -1)
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_GetResultPrizeLv, 0, 0, LW(6))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(6))
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_CHURINA_HEARTCATCH1"), 0)
    USER_FUNC(btlevtcmd_OffUnitFlag, LW(3), 33554432)
    INLINE_EVT()
        USER_FUNC(mono_off)
        DELETE_EVT(LW(13))
    END_INLINE()
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 50)
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.30))
    INLINE_EVT()
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PCH_A3_1"))
        USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 10, 0)
        DO(18)
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, 20)
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 0, 0)
    END_INLINE()
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 30, -1)
    LBL(90)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    IF_NOT_EQUAL(LW(14), 0)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetHeight, -2, LW(5))
        ADD(LW(1), LW(5))
        ADD(LW(1), 5)
        USER_FUNC(btlevtcmd_BtlIconEntryItemId, LW(14), LW(0), LW(1), LW(2), LW(8))
        BROTHER_EVT_ID(LW(15))
            USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
            USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 180, 0)
            USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 0, 0, 2, 0, -1)
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        END_BROTHER()
        LBL(91)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetHeight, -2, LW(5))
        ADD(LW(1), LW(5))
        ADD(LW(1), 5)
        USER_FUNC(btlevtcmd_BtlIconSetPosition, LW(8), LW(0), LW(1), LW(2))
        CHK_EVT(LW(15), LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            WAIT_FRM(1)
            GOTO(91)
        END_IF()
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PCH_A2_2"))
        WAIT_FRM(4)
        USER_FUNC(btlevtcmd_AnnounceSetParam, 0, LW(14))
        USER_FUNC(btlevtcmd_AnnounceMessage, 1, 0, 0, PTR("btl_msg_steal_item_get"), 90)
        USER_FUNC(btlevtcmd_BtlIconDelete, LW(8))
    ELSE()
        USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
        USER_FUNC(btlevtcmd_AddRotate, -2, 0, 180, 0)
        USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 0, 0, 2, 0, -1)
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    END_IF()
    LBL(99)
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(partyChuchurinaAttack_MadowaseAttack)
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
    USER_FUNC(btlevtcmd_AcSetFlag, 41)
    USER_FUNC(btlevtcmd_AcGetDifficulty, LW(0))
    SUB(LW(0), 3)
    SWITCH(LW(0))
        CASE_SMALL_EQUAL(-3)
            SET(LW(0), 3)
            SET(LW(1), 50)
        CASE_EQUAL(-2)
            SET(LW(0), 4)
            SET(LW(1), 80)
        CASE_EQUAL(-1)
            SET(LW(0), 5)
            SET(LW(1), 100)
        CASE_EQUAL(0)
            SET(LW(0), 7)
            SET(LW(1), 100)
        CASE_EQUAL(1)
            SET(LW(0), 8)
            SET(LW(1), 110)
            USER_FUNC(btlevtcmd_AcSetFlag, 49)
        CASE_EQUAL(2)
            SET(LW(0), 10)
            SET(LW(1), 120)
            USER_FUNC(btlevtcmd_AcSetFlag, 49)
        CASE_ETC()
            SET(LW(0), 12)
            SET(LW(1), 130)
            USER_FUNC(btlevtcmd_AcSetFlag, 49)
    END_SWITCH()
    
    // Set gauge colors + success params based on move used.
    SWITCH(LW(12))
        CASE_EQUAL(PTR(&customWeapon_MowzSmokeBomb))
            USER_FUNC(evtTot_GetMoveSelectedLevel, MoveType::MOWZ_SMOKE_BOMB, LW(2))
            SWITCH(LW(2))
                CASE_EQUAL(1)
                    USER_FUNC(btlevtcmd_AcSetGaugeParam, 67, 100, 100, 100)
                    SET(LW(2), 68)
                CASE_EQUAL(2)
                    USER_FUNC(btlevtcmd_AcSetGaugeParam, 50, 75, 100, 100)
                    SET(LW(2), 51)
                CASE_ETC()
                    USER_FUNC(btlevtcmd_AcSetGaugeParam, 40, 60, 80, 100)
                    SET(LW(2), 41)
            END_SWITCH()
        CASE_EQUAL(PTR(&customWeapon_MowzEmbargo))
            USER_FUNC(btlevtcmd_AcSetGaugeParam, 80, 100, 100, 100)
            SET(LW(2), 80)
        CASE_ETC()
            USER_FUNC(btlevtcmd_AcSetGaugeParam, 34, 68, 100, 100)
            SET(LW(2), 1)
    END_SWITCH()
    USER_FUNC(btlevtcmd_AcSetParamAll, 1, 240, 178, LW(0), LW(1), 100, LW(2), EVT_NULLPTR)
    
    USER_FUNC(btlevtcmd_SetupAC, -2, 6, 1, 0)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), EVT_NULLPTR, EVT_NULLPTR)
    USER_FUNC(evt_btl_camera_set_mode, 0, 3)
    USER_FUNC(evt_btl_camera_set_moveto, 1, LW(0), 73, 400, LW(0), 43, 0, 40, 5)
    WAIT_FRM(22)
    USER_FUNC(btlevtcmd_StartAC, 1)
    USER_FUNC(btlevtcmd_ResultAC)
    USER_FUNC(btlevtcmd_StopAC)
    WAIT_FRM(20)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PCH_I_1"))
    DO(40)
        USER_FUNC(btlevtcmd_AddRotate, -2, 0, 36, 0)
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PCH_A4_1"))
    USER_FUNC(btlevtcmd_SetWalkSound, -2, PTR("SFX_BTL_CHURINA_CONFUSE_MOVE1L"), PTR("SFX_BTL_CHURINA_CONFUSE_MOVE1R"), 0, 15, 13)
    USER_FUNC(btlevtcmd_SetRunSound, -2, PTR("SFX_BTL_CHURINA_CONFUSE_MOVE1L"), PTR("SFX_BTL_CHURINA_CONFUSE_MOVE1R"), 0, 7, 7)
    USER_FUNC(btlevtcmd_SetDiveSound, -2, PTR("SFX_BTL_CHURINA_CONFUSE_MOVE1L"), PTR("SFX_BTL_CHURINA_CONFUSE_MOVE1R"), 0, 7, 7)
    USER_FUNC(btlevtcmd_SetJumpSound, -2, PTR("SFX_BTL_CHURINA_CONFUSE_JUMP1"), PTR("SFX_BTL_CHURINA_CONFUSE_LANDING1"))
    USER_FUNC(btlevtcmd_SetPartsWalkSound, -2, 2, PTR("SFX_BTL_CHURINA_CONFUSE_MOVE1L"), PTR("SFX_BTL_CHURINA_CONFUSE_MOVE1R"), 0, 15, 13)
    USER_FUNC(btlevtcmd_SetPartsRunSound, -2, 2, PTR("SFX_BTL_CHURINA_CONFUSE_MOVE1L"), PTR("SFX_BTL_CHURINA_CONFUSE_MOVE1R"), 0, 7, 7)
    USER_FUNC(btlevtcmd_SetPartsDiveSound, -2, 2, PTR("SFX_BTL_CHURINA_CONFUSE_MOVE1L"), PTR("SFX_BTL_CHURINA_CONFUSE_MOVE1R"), 0, 7, 7)
    USER_FUNC(btlevtcmd_SetPartsJumpSound, -2, 2, PTR("SFX_BTL_CHURINA_CONFUSE_JUMP1"), PTR("SFX_BTL_CHURINA_CONFUSE_LANDING1"))
    USER_FUNC(btlevtcmd_SetPartsWalkSound, -2, 3, PTR("SFX_BTL_CHURINA_CONFUSE_MOVE1L"), PTR("SFX_BTL_CHURINA_CONFUSE_MOVE1R"), 0, 15, 13)
    USER_FUNC(btlevtcmd_SetPartsRunSound, -2, 3, PTR("SFX_BTL_CHURINA_CONFUSE_MOVE1L"), PTR("SFX_BTL_CHURINA_CONFUSE_MOVE1R"), 0, 7, 7)
    USER_FUNC(btlevtcmd_SetPartsDiveSound, -2, 3, PTR("SFX_BTL_CHURINA_CONFUSE_MOVE1L"), PTR("SFX_BTL_CHURINA_CONFUSE_MOVE1R"), 0, 7, 7)
    USER_FUNC(btlevtcmd_SetPartsJumpSound, -2, 3, PTR("SFX_BTL_CHURINA_CONFUSE_JUMP1"), PTR("SFX_BTL_CHURINA_CONFUSE_LANDING1"))
    USER_FUNC(btlevtcmd_SetPartsWalkSound, -2, 4, PTR("SFX_BTL_CHURINA_CONFUSE_MOVE1L"), PTR("SFX_BTL_CHURINA_CONFUSE_MOVE1R"), 0, 15, 13)
    USER_FUNC(btlevtcmd_SetPartsRunSound, -2, 4, PTR("SFX_BTL_CHURINA_CONFUSE_MOVE1L"), PTR("SFX_BTL_CHURINA_CONFUSE_MOVE1R"), 0, 7, 7)
    USER_FUNC(btlevtcmd_SetPartsDiveSound, -2, 4, PTR("SFX_BTL_CHURINA_CONFUSE_MOVE1L"), PTR("SFX_BTL_CHURINA_CONFUSE_MOVE1R"), 0, 7, 7)
    USER_FUNC(btlevtcmd_SetPartsJumpSound, -2, 4, PTR("SFX_BTL_CHURINA_CONFUSE_JUMP1"), PTR("SFX_BTL_CHURINA_CONFUSE_LANDING1"))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(1), 40)
    SUB(LW(2), 20)
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 20)
    USER_FUNC(btlevtcmd_SetPartsPos, -2, 2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetPartsRotate, -2, 2, 0, 180, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 2, PTR("PCH_A3_1"))
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 2, 50331648)
    USER_FUNC(evt_eff, PTR(""), PTR("bomb"), 0, LW(0), LW(1), LW(2), FLOAT(1.0), 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_CHURINA_CONFUSE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    BROTHER_EVT()
        WAIT_FRM(30)
        USER_FUNC(btlevtcmd_SetPartsFallAccel, -2, 2, FLOAT(0.30))
        USER_FUNC(btlevtcmd_GetPartsPos, -2, 2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FallPartsPosition, -2, 2, LW(0), LW(1), LW(2), 25)
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, 2, 0, 0, 0)
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 2, 43)
    END_BROTHER()
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(1), 20)
    SUB(LW(2), 10)
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 10)
    USER_FUNC(btlevtcmd_SetPartsPos, -2, 3, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetPartsRotate, -2, 3, 0, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 3, PTR("PCH_A3_2"))
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 3, 50331648)
    USER_FUNC(evt_eff, PTR(""), PTR("bomb"), 0, LW(0), LW(1), LW(2), FLOAT(1.0), 0, 0, 0, 0, 0, 0, 0)
    BROTHER_EVT()
        WAIT_FRM(30)
        USER_FUNC(btlevtcmd_SetPartsFallAccel, -2, 3, FLOAT(0.30))
        USER_FUNC(btlevtcmd_GetPartsPos, -2, 3, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FallPartsPosition, -2, 3, LW(0), LW(1), LW(2), 25)
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, 3, 0, 0, 0)
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 3, 43)
    END_BROTHER()
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(1), 30)
    ADD(LW(2), 10)
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), -10)
    USER_FUNC(btlevtcmd_SetPartsPos, -2, 4, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetPartsRotate, -2, 4, 0, 180, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 4, PTR("PCH_A3_3"))
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 4, 50331648)
    USER_FUNC(evt_eff, PTR(""), PTR("bomb"), 0, LW(0), LW(1), LW(2), FLOAT(1.0), 0, 0, 0, 0, 0, 0, 0)
    BROTHER_EVT()
        WAIT_FRM(30)
        USER_FUNC(btlevtcmd_SetPartsFallAccel, -2, 4, FLOAT(0.30))
        USER_FUNC(btlevtcmd_GetPartsPos, -2, 4, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FallPartsPosition, -2, 4, LW(0), LW(1), LW(2), 25)
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, 4, 0, 0, 0)
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 4, 43)
    END_BROTHER()
    WAIT_FRM(85)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 43)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 0, 1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 1, 1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 2, 1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 3, 1)
    USER_FUNC(btlevtcmd_SetPartsBlur, -2, 1, 255, 255, 255, 255, 100, 100, 100, 50, 1)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 1, 67108864)
    USER_FUNC(btlevtcmd_SetPartsBlur, -2, 2, 255, 255, 255, 255, 100, 100, 100, 50, 1)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 2, 67108864)
    USER_FUNC(btlevtcmd_SetPartsBlur, -2, 3, 255, 255, 255, 255, 100, 100, 100, 50, 1)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 3, 67108864)
    USER_FUNC(btlevtcmd_SetPartsBlur, -2, 4, 255, 255, 255, 255, 100, 100, 100, 50, 1)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 4, 67108864)
    BROTHER_EVT()
        WAIT_FRM(40)
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetStageSize, LW(5), EVT_NULLPTR, EVT_NULLPTR)
        DIV(LW(5), 3)
        SET(LW(0), 0)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), LW(5))
        USER_FUNC(btlevtcmd_GetMoveFrame, -2, LW(0), LW(1), LW(2), FLOAT(3.0), LW(7))
        USER_FUNC(evt_btl_camera_set_mode, 0, 3)
        USER_FUNC(evt_btl_camera_set_moveto, 1, LW(0), 90, 500, LW(0), 60, 0, LW(7), 0)
    END_BROTHER()
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
        WAIT_FRM(40)
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetStageSize, LW(5), EVT_NULLPTR, EVT_NULLPTR)
        DIV(LW(5), 3)
        SET(LW(0), 0)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), LW(5))
        USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
        USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 0, 0, 2, 0, -1)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        ADD(LW(0), 40)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
        SUB(LW(0), 60)
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 180, 0)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
        ADD(LW(0), 40)
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
        SUB(LW(0), 60)
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 180, 0)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
        ADD(LW(0), 40)
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
        USER_FUNC(evt_eff, PTR(""), PTR("bomb"), 0, LW(0), LW(1), LW(2), FLOAT(1.0), 0, 0, 0, 0, 0, 0, 0)
        USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 67108864)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, 0, 0)
    END_BROTHER()
    BROTHER_EVT()
        SET(LW(6), 2)
        USER_FUNC(btlevtcmd_SetPartsFallAccel, -2, LW(6), FLOAT(0.70))
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, LW(6), 42)
        USER_FUNC(btlevtcmd_GetPartsPos, -2, LW(6), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetStageSize, LW(5), EVT_NULLPTR, EVT_NULLPTR)
        DIV(LW(5), 3)
        SET(LW(0), 0)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), LW(5))
        USER_FUNC(btlevtcmd_SetPartsMoveSpeed, -2, LW(6), FLOAT(3.0))
        USER_FUNC(btlevtcmd_DivePartsPosition, -2, LW(6), LW(0), LW(1), LW(2), 0, 0, 2, 0, -1)
        USER_FUNC(btlevtcmd_GetPartsPos, -2, LW(6), LW(0), LW(1), LW(2))
        ADD(LW(0), 60)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(6), LW(0), LW(1), LW(2), 25, -1)
        SUB(LW(0), 90)
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, LW(6), 0, 180, 0)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(6), LW(0), LW(1), LW(2), 25, -1)
        ADD(LW(0), 60)
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, LW(6), 0, 0, 0)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(6), LW(0), LW(1), LW(2), 25, -1)
        SUB(LW(0), 90)
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, LW(6), 0, 180, 0)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(6), LW(0), LW(1), LW(2), 25, -1)
        SUB(LW(0), 30)
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, LW(6), 0, 180, 0)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(6), LW(0), LW(1), LW(2), 25, -1)
        ADD(LW(0), 30)
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, LW(6), 0, 0, 0)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(6), LW(0), LW(1), LW(2), 25, -1)
        USER_FUNC(evt_eff, PTR(""), PTR("bomb"), 0, LW(0), LW(1), LW(2), FLOAT(1.0), 0, 0, 0, 0, 0, 0, 0)
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 2, 50331648)
        USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 2, 67108864)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, 1, 0)
    END_BROTHER()
    BROTHER_EVT()
        SET(LW(6), 3)
        USER_FUNC(btlevtcmd_SetPartsFallAccel, -2, LW(6), FLOAT(0.70))
        WAIT_FRM(20)
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, LW(6), 42)
        USER_FUNC(btlevtcmd_GetPartsPos, -2, LW(6), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetStageSize, LW(5), EVT_NULLPTR, EVT_NULLPTR)
        DIV(LW(5), 3)
        SET(LW(0), 0)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), LW(5))
        USER_FUNC(btlevtcmd_SetPartsMoveSpeed, -2, LW(6), FLOAT(3.0))
        USER_FUNC(btlevtcmd_DivePartsPosition, -2, LW(6), LW(0), LW(1), LW(2), 0, 0, 2, 0, -1)
        USER_FUNC(btlevtcmd_GetPartsPos, -2, LW(6), LW(0), LW(1), LW(2))
        SUB(LW(0), 80)
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, LW(6), 0, 180, 0)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(6), LW(0), LW(1), LW(2), 25, -1)
        ADD(LW(0), 160)
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, LW(6), 0, 0, 0)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(6), LW(0), LW(1), LW(2), 40, -1)
        SUB(LW(0), 40)
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, LW(6), 0, 180, 0)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(6), LW(0), LW(1), LW(2), 15, -1)
        SUB(LW(0), 60)
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, LW(6), 0, 180, 0)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(6), LW(0), LW(1), LW(2), 20, -1)
        ADD(LW(0), 20)
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, LW(6), 0, 0, 0)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(6), LW(0), LW(1), LW(2), 20, -1)
        USER_FUNC(evt_eff, PTR(""), PTR("bomb"), 0, LW(0), LW(1), LW(2), FLOAT(1.0), 0, 0, 0, 0, 0, 0, 0)
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 3, 50331648)
        USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 3, 67108864)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, 2, 0)
    END_BROTHER()
    BROTHER_EVT()
        SET(LW(6), 4)
        USER_FUNC(btlevtcmd_SetPartsFallAccel, -2, LW(6), FLOAT(0.70))
        WAIT_FRM(60)
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, LW(6), 42)
        USER_FUNC(btlevtcmd_GetPartsPos, -2, LW(6), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetStageSize, LW(5), EVT_NULLPTR, EVT_NULLPTR)
        DIV(LW(5), 3)
        SET(LW(0), 0)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), LW(5))
        USER_FUNC(btlevtcmd_SetPartsMoveSpeed, -2, LW(6), FLOAT(3.0))
        USER_FUNC(btlevtcmd_DivePartsPosition, -2, LW(6), LW(0), LW(1), LW(2), 0, 0, 2, 0, -1)
        USER_FUNC(btlevtcmd_GetPartsPos, -2, LW(6), LW(0), LW(1), LW(2))
        ADD(LW(0), 30)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(6), LW(0), LW(1), LW(2), 10, -1)
        ADD(LW(0), 30)
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, LW(6), 0, 0, 0)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(6), LW(0), LW(1), LW(2), 10, -1)
        SUB(LW(0), 30)
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, LW(6), 0, 180, 0)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(6), LW(0), LW(1), LW(2), 10, -1)
        SUB(LW(0), 30)
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, LW(6), 0, 180, 0)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(6), LW(0), LW(1), LW(2), 10, -1)
        SUB(LW(0), 30)
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, LW(6), 0, 180, 0)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(6), LW(0), LW(1), LW(2), 10, -1)
        SUB(LW(0), 30)
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, LW(6), 0, 180, 0)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(6), LW(0), LW(1), LW(2), 10, -1)
        ADD(LW(0), 30)
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, LW(6), 0, 0, 0)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(6), LW(0), LW(1), LW(2), 10, -1)
        SUB(LW(0), 30)
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, LW(6), 0, 180, 0)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(6), LW(0), LW(1), LW(2), 10, -1)
        USER_FUNC(evt_eff, PTR(""), PTR("bomb"), 0, LW(0), LW(1), LW(2), FLOAT(1.0), 0, 0, 0, 0, 0, 0, 0)
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 4, 50331648)
        USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 4, 67108864)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, 3, 0)
    END_BROTHER()
    LBL(9)
    SET(LW(0), 0)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, 0, LW(1))
    ADD(LW(0), LW(1))
    USER_FUNC(btlevtcmd_GetUnitWork, -2, 1, LW(1))
    ADD(LW(0), LW(1))
    USER_FUNC(btlevtcmd_GetUnitWork, -2, 2, LW(1))
    ADD(LW(0), LW(1))
    USER_FUNC(btlevtcmd_GetUnitWork, -2, 3, LW(1))
    ADD(LW(0), LW(1))
    IF_NOT_EQUAL(LW(0), 0)
        WAIT_FRM(1)
        GOTO(9)
    END_IF()
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_CHURINA_CONFUSE1"), 0)
    SET(LW(10), 0)
    // Count total number of enemies hit successfully.
    SET(LW(15), 0)
    LBL(10)
    
    // Clear fog and have bigger smoke effect than usual.
    IF_EQUAL(LW(12), PTR(&customWeapon_MowzSmokeBomb))
        USER_FUNC(btlevtcmd_StageDispellFog)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(evt_eff, PTR(""), PTR("bomb"), 0, LW(0), LW(1), LW(2), FLOAT(4.0), 0, 0, 0, 0, 0, 0, 0)
    END_IF()
    
    USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(3), LW(4), 256, LW(6))
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
        GOTO(50)
    END_IF()

    // Make weapons and set AC success level.
    SET(LW(13), -1)
    USER_FUNC(btlevtcmd_AcGetOutputParam, 1, LW(0))
    SWITCH(LW(12))
        CASE_EQUAL(PTR(&customWeapon_MowzSmokeBomb))
            USER_FUNC(evtTot_MakeTeaseWeapon, LW(12), LW(0), MoveType::MOWZ_SMOKE_BOMB)
            USER_FUNC(evtTot_GetMoveSelectedLevel, MoveType::MOWZ_SMOKE_BOMB, LW(1))
            ADD(LW(1), 2)
            MUL(LW(0), LW(1))
            DIV(LW(0), 100)
            SUB(LW(0), 2)
            IF_SMALL(LW(0), 0)
                SET(LW(0), -1)
            END_IF()
            SET(LW(13), LW(0))
        CASE_EQUAL(PTR(&customWeapon_MowzTease))
            USER_FUNC(evtTot_MakeTeaseWeapon, LW(12), LW(0), MoveType::MOWZ_TEASE)
            IF_LARGE(LW(0), 0)
                SET(LW(13), 0)
            END_IF()
        CASE_ETC()
            IF_LARGE_EQUAL(LW(0), 80)
                SET(LW(13), 0)
            END_IF()
    END_SWITCH()
    
    // By default, consider action unsuccessful.
    SET(LW(11), -1)
    
    IF_NOT_EQUAL(LW(12), PTR(&customWeapon_MowzEmbargo))
        IF_LARGE_EQUAL(LW(13), 0)
            USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 131328, LW(5))
            // Only considered successful if damaged or status procs.
            USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), LW(13), LW(11))
        ELSE()
            GOTO(30)
        END_IF()
    ELSE()
        IF_LARGE_EQUAL(LW(13), 0)
            // For Embargo, mark as successful if the enemy had an item.
            USER_FUNC(evtTot_ConfiscateItem, LW(3), LW(14))
            IF_NOT_EQUAL(LW(14), 0)
                SET(LW(11), 0)
                // If the enemy has an item, have it get tossed away in a random dir.
                BROTHER_EVT()          
                    USER_FUNC(evtTot_GetHeldItemPosition, LW(3), LW(0), LW(1), LW(2))
                    USER_FUNC(btlevtcmd_BtlIconEntryItemId, LW(14), LW(0), LW(1), LW(2), LW(8))

                    // Initial movement vector.
                    USER_FUNC(evt_sub_random, 200, LW(5))
                    SUB(LW(5), 100)
                    DIVF(LW(5), FLOAT(50.0))
                    USER_FUNC(evt_sub_random, 17, LW(6))
                    ADD(LW(6), 36)
                    DIVF(LW(6), FLOAT(10.0))
                    
                    // Update movement.
                    DO(75)
                        ADDF(LW(0), LW(5))
                        ADDF(LW(1), LW(6))
                        ADDF(LW(6), FLOAT(-0.1))
                        WAIT_FRM(1)
                        USER_FUNC(btlevtcmd_BtlIconSetPosition, LW(8), LW(0), LW(1), LW(2))
                    WHILE()
                    
                    // Puff of smoke on disappearing.
                    WAIT_FRM(1)
                    USER_FUNC(btlevtcmd_BtlIconDelete, LW(8))
                    USER_FUNC(evt_eff, PTR(""), PTR("bomb"), 0, LW(0), LW(1), LW(2), FLOAT(1.0), 0, 0, 0, 0, 0, 0, 0)
                END_BROTHER()
                
                // Have enemy react with "damaged" animation.
                BROTHER_EVT()
                    WAIT_FRM(1)
                    USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(3), LW(4), 39)
                    WAIT_FRM(100)
                    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, LW(3), 1)
                END_BROTHER()
            ELSE()
                GOTO(30)
            END_IF()
        ELSE()
            GOTO(30)
        END_IF()
    END_IF()
    GOTO(40)
        
    LBL(30)
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
    LBL(40)
    
    // Increment number of successful hits if appropriate.
    IF_LARGE_EQUAL(LW(11), 0)
        ADD(LW(15), 1)
    END_IF()
    
    LBL(50)
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        ADD(LW(10), 1)
        GOTO(10)
    END_IF()
    // If there were any successful hits, give the appropriate reward.
    IF_LARGE(LW(15), 0)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetResultPrizeLv, -1, LW(13), LW(11))
        USER_FUNC(btlevtcmd_ACSuccessEffect, LW(11), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(11))
    ELSE()
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
    END_IF()
        
    LBL(80)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PCH_B_1"))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 80)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.15))
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_CHURINA_CONFUSE1_2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 5, 0)
        USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(5))
        MUL(LW(5), 9)
        DO(40)
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, LW(5))
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(5))
        MUL(LW(5), 36)
        DO(20)
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, LW(5))
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    END_BROTHER()
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_ACRStart, -2, 40, 59, 59, 0)
    END_BROTHER()
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 60, -1)
    USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
    SWITCH(LW(6))
        CASE_LARGE_EQUAL(2)
            USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PCH_Y_1"))
            USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
            WAIT_MSEC(300)
            USER_FUNC(evt_audience_ap_recovery)
            USER_FUNC(btlevtcmd_InviteApInfoReport)
            USER_FUNC(evt_btl_camera_set_mode, 0, 0)
            USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
            USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 180, 0)
            USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 0, 0, 2, 0, -1)
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        CASE_ETC()
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
            USER_FUNC(evt_audience_acrobat_notry)
            USER_FUNC(evt_audience_ap_recovery)
            USER_FUNC(btlevtcmd_InviteApInfoReport)
            USER_FUNC(evt_btl_camera_set_mode, 0, 0)
            USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PCH_R_2"))
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 180, 0)
            USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 0, 0, 2, 0, -1)
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    END_SWITCH()
    LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_SetWalkSound, -2, PTR("SFX_PARTY_BATTLE_MOVE1L"), PTR("SFX_PARTY_BATTLE_MOVE1R"), 0, 15, 13)
    USER_FUNC(btlevtcmd_SetRunSound, -2, PTR("SFX_PARTY_BATTLE_MOVE1L"), PTR("SFX_PARTY_BATTLE_MOVE1R"), 0, 7, 7)
    USER_FUNC(btlevtcmd_SetDiveSound, -2, PTR("SFX_PARTY_BATTLE_MOVE1L"), PTR("SFX_PARTY_BATTLE_MOVE1R"), 0, 7, 7)
    USER_FUNC(btlevtcmd_SetJumpSound, -2, PTR("SFX_PARTY_BATTLE_JUMP1"), PTR("SFX_PARTY_BATTLE_LANDING1"))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(partyChuchurinaAttack_Kiss)
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -2)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    USER_FUNC(btlevtcmd_GetUnitId, -2, LW(0))
    IF_NOT_EQUAL(LW(0), LW(3))
        USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
        USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    END_IF()
    RUN_CHILD_EVT(PTR(&unk_evt_803537c4))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_CalculateFaceDirection, LW(3), LW(4), -2, -1, 16, LW(15))
        USER_FUNC(btlevtcmd_ChangeFaceDirection, LW(3), LW(15))
    END_IF()
    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
    USER_FUNC(btlevtcmd_AcGetDifficulty, LW(0))
    SUB(LW(0), 3)
    SWITCH(LW(0))
        CASE_SMALL_EQUAL(-3)
            SET(LW(0), 300)
            SET(LW(1), 20)
            SET(LW(2), 20)
        CASE_EQUAL(-2)
            SET(LW(0), 270)
            SET(LW(1), 18)
            SET(LW(2), 22)
        CASE_EQUAL(-1)
            SET(LW(0), 240)
            SET(LW(1), 17)
            SET(LW(2), 24)
        CASE_EQUAL(0)
            SET(LW(0), 210)
            SET(LW(1), 16)
            SET(LW(2), 25)
        CASE_EQUAL(1)
            SET(LW(0), 195)
            SET(LW(1), 14)
            SET(LW(2), 26)
        CASE_EQUAL(2)
            SET(LW(0), 180)
            SET(LW(1), 13)
            SET(LW(2), 27)
        CASE_ETC()
            SET(LW(0), 150)
            SET(LW(1), 12)
            SET(LW(2), 30)
    END_SWITCH()
    // Change gauge parameters based on move level.
    // Heal up to 5, 10, 15 HP with a full bar.
    USER_FUNC(evtTot_GetMoveSelectedLevel, MoveType::MOWZ_SMOOCH, LW(6))
    SWITCH(LW(6))
        CASE_EQUAL(1)
            USER_FUNC(btlevtcmd_AcSetParamAll, 11, LW(0), 178, LW(1), LW(2), 20, 100, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_AcSetGaugeParam, 100, 100, 100, 100)
        CASE_EQUAL(2)
            USER_FUNC(btlevtcmd_AcSetParamAll, 11, LW(0), 178, LW(1), LW(2), 10, 50, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_AcSetGaugeParam, 50, 100, 100, 100)
        CASE_ETC()
            USER_FUNC(btlevtcmd_AcSetParamAll, 11, LW(0), 178, LW(1), LW(2), 7, 36, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_AcSetGaugeParam, 35, 70, 100, 100)
    END_SWITCH()
    // Disable the vanilla 1 HP base.
    USER_FUNC(btlevtcmd_AcSetFlag, 1)
    USER_FUNC(btlevtcmd_SetupAC, -2, 6, 1, 0)
    WAIT_FRM(22)
    USER_FUNC(btlevtcmd_StartAC, 1)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
        USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(6))
        USER_FUNC(btlevtcmd_GetStatusMg, LW(3), LW(7))
        MULF(LW(6), 12)
        MULF(LW(7), 7)
        USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), LW(6))
        USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), LW(7))
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 235, -1, 0)
        USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
        USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    END_BROTHER()
    USER_FUNC(btlevtcmd_ResultAC)
    USER_FUNC(btlevtcmd_AcGetOutputParam, 2, LW(5))
    // Action command success: 5 - Nice, 10 - Good, 15 - Great.
    SWITCH(LW(5))
        CASE_LARGE_EQUAL(15)
            SET(LW(5), 2)
        CASE_LARGE_EQUAL(10)
            SET(LW(5), 1)
        CASE_LARGE_EQUAL(5)
            SET(LW(5), 0)
        CASE_ETC()
            SET(LW(5), -1)
    END_SWITCH()
    IF_LARGE_EQUAL(LW(5), 0)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetResultPrizeLv, -5, LW(5), LW(6))
        USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(6))
    ELSE()
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
    END_IF()
    USER_FUNC(btlevtcmd_GetResultAC, LW(6))
    USER_FUNC(btlevtcmd_StopAC)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 350)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PCH_K_1"))
    WAIT_FRM(6)
    IF_LARGE_EQUAL(LW(5), 0)
        RUN_CHILD_EVT(PTR(&unk_evt_803537c4))
        IF_NOT_EQUAL(LW(0), 0)
            USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(3), LW(4), 59)
        END_IF()
    END_IF()
    USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(6))
    MULF(LW(6), 16)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), LW(6))
    USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(6))
    MULF(LW(6), 40)
    ADD(LW(1), LW(6))
    USER_FUNC(btlevtcmd_GetFaceDirection, LW(3), LW(6))
    MUL(LW(6), 45)
    USER_FUNC(evt_eff, 0, PTR("kiss"), 0, LW(0), LW(1), LW(2), LW(6), 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_CHURINA_KISS1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    WAIT_FRM(40)
    DIV(LW(6), 5)
    
    // Heal target (Mario).
    USER_FUNC(btlevtcmd_GetPartsPos, LW(3), LW(4), LW(12), LW(13), LW(14))
    USER_FUNC(btlevtcmd_GetHeight, LW(3), LW(5))
    USER_FUNC(btlevtcmd_GetStatusMg, LW(3), LW(7))
    MULF(LW(5), LW(7))
    ADD(LW(13), LW(5))
    SUB(LW(12), LW(6))  // move hearts apart in X dir; only if healing both
    USER_FUNC(btlevtcmd_AcGetOutputParam, 2, LW(5))
    USER_FUNC(evt_eff, PTR("eff"), PTR("recovery"), 0, LW(12), LW(13), LW(14), LW(5), 0, 0, 0, 0, 0, 0, 0)
    INLINE_EVT_ID(LW(15))
        WAIT_FRM(120)
        USER_FUNC(btlevtcmd_RecoverHp, LW(3), LW(4), LW(5))
    END_INLINE()
    
    // Also heal self.
    USER_FUNC(btlevtcmd_GetPartsPos, -2, 1, LW(12), LW(13), LW(14))
    USER_FUNC(btlevtcmd_GetHeight, LW(3), LW(5))
    USER_FUNC(btlevtcmd_GetStatusMg, LW(3), LW(7))
    MULF(LW(5), LW(7))
    ADD(LW(13), LW(5))
    SUB(LW(14), 10)     // to prevent z-fighting
    ADD(LW(12), LW(6))  // move hearts apart in X dir; only if healing both
    USER_FUNC(btlevtcmd_AcGetOutputParam, 2, LW(5))
    USER_FUNC(evt_eff, PTR("eff"), PTR("recovery"), 0, LW(12), LW(13), LW(14), LW(5), 0, 0, 0, 0, 0, 0, 0)
    INLINE_EVT_ID(LW(15))
        WAIT_FRM(120)
        USER_FUNC(btlevtcmd_RecoverHp, -2, 1, LW(5))
    END_INLINE()
    
    USER_FUNC(btlevtcmd_ACRStart, -2, 0, 15, 15, 20)
    USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
    SWITCH(LW(6))
        CASE_LARGE_EQUAL(2)
            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
            USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PCH_Y_1"))
            USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
            USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 43)
            WAIT_MSEC(300)
        CASE_ETC()
            USER_FUNC(evt_audience_acrobat_notry)
    END_SWITCH()
    USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    RUN_CHILD_EVT(PTR(&unk_evt_803537c4))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_ResetFaceDirection, LW(3))
        USER_FUNC(btlevtcmd_StartWaitEvent, LW(3))
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

BattleWeapon customWeapon_MowzLoveSlapL = {
    .name = "btl_wn_pcr_normal",
    .icon = IconType::PARTNER_MOVE_0,
    .item_id = 0,
    .description = "msg_pch_binta",
    .base_accuracy = 100,
    .base_fp_cost = 0,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 1,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)weaponGetPowerDefault,
    .damage_function_params = { 1, 0, 0, 0, 0, 0, 0, 0 },
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
    .damage_pattern = 0x15,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_binta",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::USABLE_IF_CONFUSED |
        AttackSpecialProperty_Flags::GROUNDS_WINGED |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::DIMINISHING_BY_HIT |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::TOP_SPIKY |
        // Additional resistances (and removed Payback).
        AttackCounterResistance_Flags::FRONT_SPIKY |
        AttackCounterResistance_Flags::FIERY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)partyChuchurinaAttack_NormalAttack,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 4,
    .nozzle_fire_chance = 3,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_MowzLoveSlapR = {
    .name = "btl_wn_pcr_normal",
    .icon = IconType::PARTNER_MOVE_0,
    .item_id = 0,
    .description = "msg_pch_binta",
    .base_accuracy = 100,
    .base_fp_cost = 0,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 1,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)weaponGetPowerDefault,
    .damage_function_params = { 1, 0, 0, 0, 0, 0, 0, 0 },
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
    .damage_pattern = 0x16,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_binta",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::USABLE_IF_CONFUSED |
        AttackSpecialProperty_Flags::GROUNDS_WINGED |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::DIMINISHING_BY_HIT |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::TOP_SPIKY |
        // Additional resistances (and removed Payback).
        AttackCounterResistance_Flags::FRONT_SPIKY |
        AttackCounterResistance_Flags::FIERY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)partyChuchurinaAttack_NormalAttack,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 4,
    .nozzle_fire_chance = 3,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_MowzLoveSlapLFinal = {
    .name = "btl_wn_pcr_normal",
    .icon = IconType::PARTNER_MOVE_0,
    .item_id = 0,
    .description = "msg_pch_binta",
    .base_accuracy = 100,
    .base_fp_cost = 0,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 1,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)weaponGetPowerDefault,
    .damage_function_params = { 1, 0, 0, 0, 0, 0, 0, 0 },
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
    .damage_pattern = 0x17,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_binta",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::USABLE_IF_CONFUSED |
        AttackSpecialProperty_Flags::GROUNDS_WINGED |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::DIMINISHING_BY_HIT |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::TOP_SPIKY |
        // Additional resistances.
        AttackCounterResistance_Flags::FRONT_SPIKY |
        AttackCounterResistance_Flags::FIERY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)partyChuchurinaAttack_NormalAttack,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 4,
    .nozzle_fire_chance = 2,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_MowzLoveSlapRFinal = {
    .name = "btl_wn_pcr_normal",
    .icon = IconType::PARTNER_MOVE_0,
    .item_id = 0,
    .description = "msg_pch_binta",
    .base_accuracy = 100,
    .base_fp_cost = 0,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 1,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)weaponGetPowerDefault,
    .damage_function_params = { 1, 0, 0, 0, 0, 0, 0, 0 },
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
    .damage_pattern = 0x18,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_binta",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::USABLE_IF_CONFUSED |
        AttackSpecialProperty_Flags::GROUNDS_WINGED |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::DIMINISHING_BY_HIT |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::TOP_SPIKY |
        // Additional resistances.
        AttackCounterResistance_Flags::FRONT_SPIKY |
        AttackCounterResistance_Flags::FIERY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)partyChuchurinaAttack_NormalAttack,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 4,
    .nozzle_fire_chance = 2,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_MowzKissThief = {
    .name = "btl_wn_pcr_steal",
    .icon = IconType::PARTNER_MOVE_1,
    .item_id = 0,
    .description = "msg_pch_heart_catch",
    .base_accuracy = 100,
    .base_fp_cost = 2,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = nullptr,
    .damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
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
        // Removed Hammer-like range check.
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::ONLY_FRONT,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0x14,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_heart_catch",
    .special_property_flags = AttackSpecialProperty_Flags::UNGUARDABLE,
    .counter_resistance_flags =
        // Ignores all contact hazards, aside from lit Bob-ombs.
        AttackCounterResistance_Flags::ALL &
        ~AttackCounterResistance_Flags::VOLATILE_EXPLOSIVE,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)partyChuchurinaAttack_ItemSteal,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 0,
    .nozzle_fire_chance = 5,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_MowzTease = {
    .name = "btl_wn_pcr_madowase",
    .icon = IconType::PARTNER_MOVE_1,
    .item_id = 0,
    .description = "msg_pch_madowaseru",
    .base_accuracy = 100,
    .base_fp_cost = 3,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = nullptr,
    .damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
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
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_madowaseru",
    .special_property_flags = AttackSpecialProperty_Flags::UNGUARDABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .confuse_chance = 100,
    .confuse_time = 1,
    
    .attack_evt_code = (void*)partyChuchurinaAttack_MadowaseAttack,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 0,
    .nozzle_fire_chance = 5,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_MowzSmooch = {
    .name = "btl_wn_pcr_kiss",
    .icon = IconType::PARTNER_MOVE_3,
    .item_id = 0,
    .description = "msg_pch_kiss",
    .base_accuracy = 100,
    .base_fp_cost = 10,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = nullptr,
    .damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_OPPOSING_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH |
        // Explicitly add this to exclude Infatuated targets.
        AttackTargetClass_Flags::ONLY_TARGET_MARIO,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_SAME_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_chuchurina_kiss",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::CANNOT_MISS,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_IN_PERIL |
        AttackTargetWeighting_Flags::PREFER_LOWER_HP |
        AttackTargetWeighting_Flags::PREFER_LESS_HEALTHY,
        
    // status chances
    
    .attack_evt_code = (void*)partyChuchurinaAttack_Kiss,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 0,
    .nozzle_fire_chance = 5,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_MowzEmbargo = {
    .name = "tot_ptr7_embargo",
    .icon = IconType::PARTNER_MOVE_1,
    .item_id = 0,
    .description = "tot_ptr7_embargo_desc",
    .base_accuracy = 100,
    .base_fp_cost = 3,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = nullptr,
    .damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
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
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_madowaseru",
    .special_property_flags = AttackSpecialProperty_Flags::UNGUARDABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)partyChuchurinaAttack_MadowaseAttack,
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

BattleWeapon customWeapon_MowzSmokeBomb = {
    .name = "tot_ptr7_smokebomb",
    .icon = IconType::PARTNER_MOVE_2,
    .item_id = 0,
    .description = "tot_ptr7_smokebomb_desc",
    .base_accuracy = 100,
    .base_fp_cost = 3,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)weaponGetPowerDefault,
    .damage_function_params = { 1, 0, 0, 0, 0, 0, 0, 0 },
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
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::EXPLOSION,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_madowaseru",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .dizzy_chance = 60,
    .dizzy_time = 1,
    
    .attack_evt_code = (void*)partyChuchurinaAttack_MadowaseAttack,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 10,
    .bg_a2_fall_weight = 10,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 10,
    .nozzle_turn_chance = 10,
    .nozzle_fire_chance = 10,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

}  // namespace mod::tot::party_mowz