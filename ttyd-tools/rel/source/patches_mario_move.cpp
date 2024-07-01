#include "patches_mario_move.h"

#include "common_types.h"
#include "evt_cmd.h"
#include "mod.h"
#include "mod_state.h"
#include "patch.h"
#include "patches_battle.h"
#include "tot_manager_move.h"
#include "tot_party_mario.h"
#include "tot_party_yoshi.h"
#include "tot_state.h"

#include <ttyd/battle.h>
#include <ttyd/battle_camera.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_mario.h>
#include <ttyd/battle_menu_disp.h>
#include <ttyd/battle_unit.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/icondrv.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/msgdrv.h>
#include <ttyd/sac_bakugame.h>
#include <ttyd/sac_common.h>
#include <ttyd/sac_deka.h>
#include <ttyd/sac_genki.h>
#include <ttyd/sac_muki.h>
#include <ttyd/sac_suki.h>
#include <ttyd/sac_zubastar.h>
#include <ttyd/sound.h>
#include <ttyd/system.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

// Assembly patch functions.
extern "C" {
    // special_move_patches.s
    void StartSweetTreatSetupTargets();
    void BranchBackSweetTreatSetupTargets();
    void StartSweetTreatBlinkNumbers();
    void BranchBackSweetTreatBlinkNumbers();
    void StartEarthTremorNumberOfBars();
    void BranchBackEarthTremorNumberOfBars();
    void StartArtAttackCalculateDamage();
    void BranchBackArtAttackCalculateDamage();
    
    void sweetTreatSetupTargets() {
        mod::infinite_pit::mario_move::SweetTreatSetUpTargets();
    }
    void sweetTreatBlinkNumbers() {
        mod::infinite_pit::mario_move::SweetTreatBlinkNumbers();
    }
    int32_t getEarthTremorNumberOfBars() {
        return mod::infinite_pit::mario_move::GetEarthTremorNumberOfBars();
    }
    int32_t getArtAttackPower(int32_t circled_percent) {
        return mod::infinite_pit::mario_move::GetArtAttackPower(circled_percent);
    }
}

namespace mod::infinite_pit {

namespace {

using ::ttyd::battle::BattleWorkCommandCursor;
using ::ttyd::battle::BattleWorkCommandOperation;
using ::ttyd::battle::BattleWorkCommandWeapon;
using ::ttyd::battle_database_common::BattleWeapon;
using ::ttyd::battle_unit::BattleWorkUnit;
using ::ttyd::battle_unit::BattleWorkUnitPart;
using ::ttyd::evtmgr::EvtEntry;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::item_data::itemDataTable;

namespace AttackTargetClass_Flags =
    ::ttyd::battle_database_common::AttackTargetClass_Flags;
namespace BattleUnitType = ::ttyd::battle_database_common::BattleUnitType;
namespace IconType = ::ttyd::icondrv::IconType;
namespace ItemType = ::ttyd::item_data::ItemType;

}

// Function hooks.
extern int32_t (*g_BtlUnit_GetWeaponCost_trampoline)(BattleWorkUnit*, BattleWeapon*);
extern void (*g_DrawOperationWin_trampoline)();
extern void (*g_DrawWeaponWin_trampoline)();
extern int32_t (*g_sac_genki_get_score_trampoline)(EvtEntry*, bool);
extern uint32_t (*g_weaponGetPower_Deka_trampoline)(
    BattleWorkUnit*, BattleWeapon*, BattleWorkUnit*, BattleWorkUnitPart*);
extern int32_t (*g_bakuGameDecideWeapon_trampoline)(EvtEntry*, bool);
extern int32_t (*g_main_muki_trampoline)(EvtEntry*, bool);
extern int32_t (*g_sac_suki_set_weapon_trampoline)(EvtEntry*, bool);
extern uint32_t (*g_weaponGetPower_ZubaStar_trampoline)(
    BattleWorkUnit*, BattleWeapon*, BattleWorkUnit*, BattleWorkUnitPart*);
// Patch addresses.
extern const int32_t g_subsetevt_shot_damage_Patch_SuperInvolvedWeapon;
extern const int32_t g_subsetevt_shot_damage_Patch_UltraInvolvedWeapon;
extern const int32_t g_subsetevt_swallow_shot_damage_Patch_InvolvedWeapon;
extern const int32_t g_sac_genki_main_base_BlinkNumbers_BH;
extern const int32_t g_sac_genki_main_base_BlinkNumbers_EH;
extern const int32_t g_sac_genki_main_base_SetupTargets_BH;
extern const int32_t g_sac_genki_main_base_SetupTargets_EH;
extern const int32_t g_genki_evt_common_Patch_SweetTreatFeastResult;
extern const int32_t g_genki_evt_common_SweetTreatResultJumpPoint;
extern const int32_t g_sac_deka_main_base_GetNumberOfBars_BH;
extern const int32_t g_scissor_damage_sub_ArtAttackDamage_BH;
extern const int32_t g_scissor_damage_sub_ArtAttackDamage_EH;
extern const int32_t g_scissor_damage_Patch_ArtAttackCheckImmunity;

namespace mario_move {

namespace {
    
// Global variables and constants.
bool                g_InBattle = false;
int8_t              g_MaxMoveBadgeCounts[4];
int8_t              g_CurMoveBadgeCounts[4];
char                g_MoveBadgeTextBuffer[24];
const char*         kMoveBadgeAbbreviations[4] = {
    "Charge", "Charge", "Tough. Up", "Tough. Up"
};

// Patch to disable getting Star Power early from certain attacks;
// battle::AwardStarPowerAndResetFaceDirection will be used to award it
// at the end of the attack instead, to make sure Stylishes are counted.
EVT_BEGIN(DeclareStarPowerPatch)
DEBUG_REM(0) DEBUG_REM(0)
EVT_PATCH_END()
static_assert(sizeof(DeclareStarPowerPatch) == 0x10);

// If the badge is one that can have its power level selected, returns the
// index of the value controlling its level; otherwise, returns -1.
int32_t GetWeaponLevelSelectionIndex(int16_t badge_id) {
    switch (badge_id) {
        case ItemType::CHARGE: return 0;
        case ItemType::CHARGE_P: return 1;
        case ItemType::TOT_TOUGHEN_UP: return 2;
        case ItemType::TOT_TOUGHEN_UP_P: return 3;
    }
    return -1;
}

// Gets the FP cost of a move, factoring in stackability of Charge / Toughen Up
// and Flower Savers if necessary.
int32_t GetWeaponCost(BattleWorkUnit* unit, BattleWeapon* weapon) {
    int32_t cost = weapon->base_fp_cost;
    if (int32_t idx = GetWeaponLevelSelectionIndex(weapon->item_id); idx >= 0) {
        cost *= g_CurMoveBadgeCounts[idx];
    }
    if (cost > 0) {
        cost -= unit->badges_equipped.flower_saver;
        if (cost < 1) cost = 1;
    }
    return cost;
}

// Extra code for the battle menus that allows selecting level of badge moves.
// Assumes the menu is one of the standard weapon menus (Jump, Hammer, etc.)
// if is_strategies_menu = false.
void CheckForSelectingWeaponLevel(bool is_strategies_menu) {
    const uint16_t buttons = ttyd::system::keyGetButtonTrg(0);
    const uint32_t dir_trg = ttyd::system::keyGetDirTrg(0); 
    const bool left_press =
        (buttons & (ButtonId::L | ButtonId::DPAD_LEFT)) ||
        (dir_trg == DirectionInputId::ANALOG_LEFT);
    const bool right_press =
        (buttons & (ButtonId::R | ButtonId::DPAD_RIGHT)) ||
        (dir_trg == DirectionInputId::ANALOG_RIGHT);
    
    void** win_data = reinterpret_cast<void**>(
        ttyd::battle::g_BattleWork->command_work.window_work);
    if (!win_data || !win_data[0]) return;
    
    auto* cursor = reinterpret_cast<BattleWorkCommandCursor*>(win_data[0]);
    if (is_strategies_menu) {
        auto* battleWork = ttyd::battle::g_BattleWork;
        auto* strats = battleWork->command_work.operation_table;
        BattleWorkUnit* unit =
            battleWork->battle_units[battleWork->active_unit_idx];
        BattleWeapon* weapon = nullptr;
        int32_t idx = 0;
        for (int32_t i = 0; i < cursor->num_options; ++i) {
            // Not selecting Charge or Super Charge.
            if (strats[i].type < 1 || strats[i].type > 2) continue;
            
            if (strats[i].type == 1) {
                if (unit->current_kind == BattleUnitType::MARIO) {
                    weapon = &ttyd::battle_mario::badgeWeapon_Charge;
                } else {
                    weapon = &ttyd::battle_mario::badgeWeapon_ChargeP;
                }
            } else {
                if (unit->current_kind == BattleUnitType::MARIO) {
                    weapon = &ttyd::battle_mario::badgeWeapon_SuperCharge;
                } else {
                    weapon = &ttyd::battle_mario::badgeWeapon_SuperChargeP;
                }
            }
            idx = GetWeaponLevelSelectionIndex(weapon->item_id);
            if (idx < 0 || g_MaxMoveBadgeCounts[idx] <= 1) continue;
            
            // If current selection, and L/R pressed, change power level.
            if (i == cursor->abs_position) {
                if (left_press && g_CurMoveBadgeCounts[idx] > 1) {
                    --g_CurMoveBadgeCounts[idx];
                    ttyd::sound::SoundEfxPlayEx(0x478, 0, 0x64, 0x40);
                } else if (
                    right_press && 
                    g_CurMoveBadgeCounts[idx] < g_MaxMoveBadgeCounts[idx]) {
                    ++g_CurMoveBadgeCounts[idx];
                    ttyd::sound::SoundEfxPlayEx(0x478, 0, 0x64, 0x40);
                }
                
                // Overwrite default text based on current power level.
                sprintf(
                    g_MoveBadgeTextBuffer, "%s Lv. %" PRId8,
                    kMoveBadgeAbbreviations[idx], g_CurMoveBadgeCounts[idx]);
                strats[i].name = g_MoveBadgeTextBuffer;
            } else {
                strats[i].name = ttyd::msgdrv::msgSearch(
                    itemDataTable[weapon->item_id].name);
            }
            
            strats[i].cost = GetWeaponCost(unit, weapon);
            strats[i].enabled =
                strats[i].cost <= ttyd::battle_unit::BtlUnit_GetFp(unit);
            strats[i].unk_08 = !strats[i].enabled;  // 1 if disabled: "no FP" msg
        }
        
        // Handle switch partner cost, if enabled.
        int32_t switch_fp_cost = battle::GetPartySwitchCost();
        if (strats[0].type == 0 && switch_fp_cost) {
            strats[0].cost = switch_fp_cost;
            strats[0].enabled =
                ttyd::battle_unit::BtlUnit_GetFp(unit) >= switch_fp_cost;
            strats[0].unk_08 = !strats[0].enabled;  // 1 if disabled: "no FP" msg
        }
    } else {
        auto* weapons = reinterpret_cast<BattleWorkCommandWeapon*>(win_data[2]);
        if (!weapons) return;
        for (int32_t i = 0; i < cursor->num_options; ++i) {
            // Skip non-weapon selections / items.
            if (!weapons[i].weapon || weapons[i].item_id) continue;
            
            auto* weapon = weapons[i].weapon;
            int32_t move_type = weapons[i].index;
            
            // Handle Special moves.
            if (weapon->base_sp_cost) {
                // If current selection, and L/R pressed, change power level.
                if (i == cursor->abs_position) {
                    if (left_press && 
                        tot::MoveManager::ChangeSelectedLevel(move_type, -1)) {
                        ttyd::sound::SoundEfxPlayEx(0x478, 0, 0x64, 0x40);
                    } else if (
                        right_press &&
                        tot::MoveManager::ChangeSelectedLevel(move_type, 1)) {
                        ttyd::sound::SoundEfxPlayEx(0x478, 0, 0x64, 0x40);
                    }
                    
                    // Overwrite default text based on current power level.
                    if (tot::MoveManager::GetCurrentSelectionString(
                        move_type, g_MoveBadgeTextBuffer)) {
                        weapons[i].name = g_MoveBadgeTextBuffer;
                    }
                } else {
                    weapons[i].name = ttyd::msgdrv::msgSearch(weapon->name);
                }
                
                // Update actual SP cost.
                int32_t idx = move_type - tot::MoveType::SP_SWEET_TREAT;
                int8_t new_cost = tot::MoveManager::GetMoveCost(move_type);
                mod::patch::writePatch(
                    &ttyd::battle_mario::superActionTable[idx]->base_sp_cost,
                    &new_cost, sizeof(new_cost));
            } else if (weapons[i].icon != IconType::DO_NOTHING) {
                // Otherwise, must be a free / FP-costing move.            
                // If current selection, and L/R pressed, change power level.
                if (i == cursor->abs_position) {
                    if (left_press && 
                        tot::MoveManager::ChangeSelectedLevel(move_type, -1)) {
                        ttyd::sound::SoundEfxPlayEx(0x478, 0, 0x64, 0x40);
                    } else if (
                        right_press && 
                        tot::MoveManager::ChangeSelectedLevel(move_type, 1)) {
                        ttyd::sound::SoundEfxPlayEx(0x478, 0, 0x64, 0x40);
                    }
                    
                    // Overwrite default text based on current power level.
                    if (tot::MoveManager::GetCurrentSelectionString(
                        move_type, g_MoveBadgeTextBuffer)) {
                        weapons[i].name = g_MoveBadgeTextBuffer;
                    }
                } else {
                    weapons[i].name = ttyd::msgdrv::msgSearch(weapon->name);
                }
                
                // Update single / multi-target for Vivian's extra moves.
                if (move_type == tot::MoveType::VIVIAN_CURSE ||
                    move_type == tot::MoveType::VIVIAN_NEUTRALIZE) {
                    int32_t move_level = 
                        tot::MoveManager::GetSelectedLevel(move_type);
                    if (move_level == 1) {
                        weapons[i].weapon->target_class_flags &=
                            ~AttackTargetClass_Flags::MULTIPLE_TARGET;
                        weapons[i].weapon->target_class_flags |=
                            AttackTargetClass_Flags::SINGLE_TARGET;
                    } else {
                        weapons[i].weapon->target_class_flags &=
                            ~AttackTargetClass_Flags::SINGLE_TARGET;
                        weapons[i].weapon->target_class_flags |=
                            AttackTargetClass_Flags::MULTIPLE_TARGET;
                    }
                }
                
                // Update actual FP cost.
                int8_t new_cost = tot::MoveManager::GetMoveCost(move_type);
                mod::patch::writePatch(
                    &weapon->base_fp_cost, &new_cost, sizeof(new_cost));
            }
        }
    }
}

// Returns a pointer to the common work area for all special action commands.
void* GetSacWorkPtr() {
    return reinterpret_cast<void*>(
        reinterpret_cast<intptr_t>(ttyd::battle::g_BattleWork) + 0x1f4c);
}

// Declarations for USER_FUNCs.
EVT_DECLARE_USER_FUNC(SetSweetFeastWeapon, 3)

// Event for the results of Sweet Treat/Feast.
EVT_BEGIN(SweetTreatFeastResultEvt)
SET(LW(9), LW(10))
ADD(LW(9), LW(11))
ADD(LW(9), LW(12))
IF_LARGE_EQUAL(LW(9), 1)
    // Cheer if at least 1 pickup was collected.
    USER_FUNC(ttyd::sac_common::sac_wao)
END_IF()
IF_EQUAL(LW(15), 0)
    // If Sweet Treat, run the original code to restore HP / FP.
    RUN_CHILD_EVT(g_genki_evt_common_SweetTreatResultJumpPoint)
ELSE()
    // If Sweet Feast, apply HP/FP-Regen status.
    INLINE_EVT()
        // Run end of original event code to properly end the attack.
        // (Mario inflicting status on himself ends the script prematurely.)
        USER_FUNC(ttyd::sac_genki::end_genki)
        WAIT_MSEC(1000)
        USER_FUNC(ttyd::sac_common::sac_enemy_slide_return)
        WAIT_MSEC(1000)
        USER_FUNC(ttyd::battle_camera::evt_btl_camera_set_mode, 0, 0)
        WAIT_MSEC(1000)
        USER_FUNC(ttyd::battle_event_cmd::btlevtcmd_StartWaitEvent, -2)
    END_INLINE()
    INLINE_EVT()
        // Apply FP-Regen status a bit later than HP-Regen.
        WAIT_MSEC(400)
        IF_LARGE_EQUAL(LW(12), 1)
            USER_FUNC(SetSweetFeastWeapon, 2, LW(12), LW(9))
            USER_FUNC(
                ttyd::battle_event_cmd::btlevtcmd_CheckDamage,
                -2, -2, 1, LW(9), 256, LW(5))
        END_IF()
    END_INLINE()
    IF_NOT_EQUAL(LW(13), -1)
        // Heal partner status ailments.
        SET(LW(9), LW(13))
        RUN_CHILD_EVT(ttyd::sac_genki::status_recover_evt)
        IF_LARGE_EQUAL(LW(11), 1)
            // Apply HP-Regen status to partner.
            USER_FUNC(SetSweetFeastWeapon, 1, LW(11), LW(9))
            USER_FUNC(
                ttyd::battle_event_cmd::btlevtcmd_CheckDamage,
                -2, LW(13), LW(14), LW(9), 256, LW(5))
        END_IF()
    END_IF()
    // Heal Mario status ailments.
    SET(LW(9), -2)
    RUN_CHILD_EVT(ttyd::sac_genki::status_recover_evt)
    IF_LARGE_EQUAL(LW(10), 1)
        // Apply HP-Regen status to Mario.
        USER_FUNC(SetSweetFeastWeapon, 0, LW(10), LW(9))
        USER_FUNC(
            ttyd::battle_event_cmd::btlevtcmd_CheckDamage,
            -2, -2, 1, LW(9), 256, LW(5))
    END_IF()
END_IF()
RETURN()
EVT_END()

// Wrapper for custom Sweet Treat/Feast results event.
EVT_BEGIN(SweetTreatFeastResultEvtHook)
RUN_CHILD_EVT(SweetTreatFeastResultEvt)
RETURN()
EVT_END()

// Constructs a weapon granting HP, partner HP or FP regen status.
EVT_DEFINE_USER_FUNC(SetSweetFeastWeapon) {
    int32_t weapon_type = evtGetValue(evt, evt->evtArguments[0]);
    int32_t strength    = evtGetValue(evt, evt->evtArguments[1]);
    // Build a weapon based on the base weapon for Power Lift.
    static BattleWeapon weapon[3];
    memcpy(&weapon[weapon_type], &ttyd::sac_muki::weapon_muki, 
        sizeof(BattleWeapon));
    if (weapon_type < 2) {
        weapon[weapon_type].hp_regen_time = 5;
        weapon[weapon_type].hp_regen_strength = strength;
    } else {
        weapon[weapon_type].fp_regen_time = 5;
        weapon[weapon_type].fp_regen_strength = strength;
    }
    evtSetValue(evt, evt->evtArguments[2], PTR(&weapon[weapon_type]));
    return 2;
}

}
    
void ApplyFixedPatches() {
    g_BtlUnit_GetWeaponCost_trampoline = patch::hookFunction(
        ttyd::battle_unit::BtlUnit_GetWeaponCost,
        [](BattleWorkUnit* unit, BattleWeapon* weapon) {
            // Replaces existing logic.
            return GetWeaponCost(unit, weapon);
        });

    g_DrawOperationWin_trampoline = patch::hookFunction(
        ttyd::battle_menu_disp::DrawOperationWin, []() {
            CheckForSelectingWeaponLevel(/* is_strategies_menu = */ true);
            g_DrawOperationWin_trampoline();
        });
        
    g_DrawWeaponWin_trampoline = patch::hookFunction(
        ttyd::battle_menu_disp::DrawWeaponWin, []() {
            CheckForSelectingWeaponLevel(/* is_strategies_menu = */ false);
            g_DrawWeaponWin_trampoline();
        });
            
    // Change "involved" weapons for Super/Ultra Hammer and Yoshi's Gulp.
    mod::patch::writePatch(
        reinterpret_cast<void*>(
            g_subsetevt_shot_damage_Patch_SuperInvolvedWeapon),
        reinterpret_cast<uint32_t>(
            &tot::party_mario::customWeapon_SuperHammerRecoil));
    mod::patch::writePatch(
        reinterpret_cast<void*>(
            g_subsetevt_shot_damage_Patch_UltraInvolvedWeapon),
        reinterpret_cast<uint32_t>(
            &tot::party_mario::customWeapon_UltraHammerRecoil));
    mod::patch::writePatch(
        reinterpret_cast<void*>(
            g_subsetevt_swallow_shot_damage_Patch_InvolvedWeapon),
        reinterpret_cast<uint32_t>(
            &tot::party_yoshi::customWeapon_YoshiGulp_Recoil));
    
    // Change the Sweet Treat/Feast target type counts based on level.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_sac_genki_main_base_SetupTargets_BH),
        reinterpret_cast<void*>(g_sac_genki_main_base_SetupTargets_EH),
        reinterpret_cast<void*>(StartSweetTreatSetupTargets),
        reinterpret_cast<void*>(BranchBackSweetTreatSetupTargets));
    // Change the displayed numbers for Feast to 1/5 the target value.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_sac_genki_main_base_BlinkNumbers_BH),
        reinterpret_cast<void*>(g_sac_genki_main_base_BlinkNumbers_EH),
        reinterpret_cast<void*>(StartSweetTreatBlinkNumbers),
        reinterpret_cast<void*>(BranchBackSweetTreatBlinkNumbers));
    
    // Hook the Sweet Treat/Feast results function to return the right values.
    g_sac_genki_get_score_trampoline = patch::hookFunction(
        ttyd::sac_genki::get_score, [](EvtEntry* evt, bool isFirstCall) {
            intptr_t sac_work_addr = reinterpret_cast<intptr_t>(GetSacWorkPtr());
            int32_t is_feast = *reinterpret_cast<int32_t*>(sac_work_addr + 0xc);
            // For Sweet Feast, return the target value / 5, rounded up.
            int32_t hp = *reinterpret_cast<int32_t*>(sac_work_addr + 0x10);
            int32_t php = *reinterpret_cast<int32_t*>(sac_work_addr + 0x14);
            int32_t fp = *reinterpret_cast<int32_t*>(sac_work_addr + 0x18);
            evtSetValue(evt, evt->evtArguments[0], is_feast ? (hp +4) / 5 : hp);
            evtSetValue(evt, evt->evtArguments[1], is_feast ? (php+4) / 5 : php);
            evtSetValue(evt, evt->evtArguments[2], is_feast ? (fp +4) / 5 : fp);
            return 2;
        });

    // Patch Sweet Treat common event to apply Regen status for Feast.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_genki_evt_common_Patch_SweetTreatFeastResult),
        SweetTreatFeastResultEvtHook, sizeof(SweetTreatFeastResultEvtHook));

    // Change the number of action command bars to fill for Earth Tremor.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_sac_deka_main_base_GetNumberOfBars_BH),
        reinterpret_cast<void*>(StartEarthTremorNumberOfBars),
        reinterpret_cast<void*>(BranchBackEarthTremorNumberOfBars));

    // Change the attack power for Earth Tremor to "level + bars full".
    g_weaponGetPower_Deka_trampoline = patch::hookFunction(
        ttyd::sac_deka::weaponGetPower_Deka, [](
            BattleWorkUnit*, BattleWeapon*,
            BattleWorkUnit*, BattleWorkUnitPart*) {
            intptr_t sac_work_addr = reinterpret_cast<intptr_t>(GetSacWorkPtr());
            int32_t bars_full = *reinterpret_cast<int32_t*>(sac_work_addr + 0x44);
            return static_cast<uint32_t>(
                tot::MoveManager::GetSelectedLevel(
                    tot::MoveType::SP_EARTH_TREMOR)
                + bars_full);
        });
        
    // Change Clock Out's turn count based on power level.
    g_bakuGameDecideWeapon_trampoline = patch::hookFunction(
        ttyd::sac_bakugame::bakuGameDecideWeapon,
        [](EvtEntry* evt, bool isFirstCall) {
            // Call vanilla logic.
            g_bakuGameDecideWeapon_trampoline(evt, isFirstCall);
            
            intptr_t sac_work_addr = reinterpret_cast<intptr_t>(GetSacWorkPtr());
            BattleWeapon& weapon = 
                *reinterpret_cast<BattleWeapon*>(sac_work_addr + 0xf8);
            // Modify turn count (-1 for level 1, +1 for level 3; min 1 turn).
            if (weapon.stop_chance) {
                weapon.stop_time += (
                    tot::MoveManager::GetSelectedLevel(
                        tot::MoveType::SP_CLOCK_OUT)
                    - 2);
                if (weapon.stop_time < 1) weapon.stop_time = 1;
            }
            return 2;
        });

    // Change the rate at which Power Lift's gauges fill based on level.
    g_main_muki_trampoline = patch::hookFunction(
        ttyd::sac_muki::main_muki, [](EvtEntry* evt, bool isFirstCall) {
            // Change the amount of power gained per arrow hit on startup.
            if (isFirstCall) {
                float arrow_power = 0.16667;
                switch (
                    tot::MoveManager::GetSelectedLevel(
                        tot::MoveType::SP_POWER_LIFT)) {
                    case 2: arrow_power = 0.20001; break;
                    case 3: arrow_power = 0.25001; break;
                }
                mod::patch::writePatch(
                    &ttyd::sac_muki::_sac_muki_power_per_arrow,
                    &arrow_power, sizeof(float));
            }
            // Call vanilla logic.
            return g_main_muki_trampoline(evt, isFirstCall);
        });

    // Change the damage dealt by Art Attack to 2/3/4 max based on level.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_scissor_damage_sub_ArtAttackDamage_BH),
        reinterpret_cast<void*>(g_scissor_damage_sub_ArtAttackDamage_EH),
        reinterpret_cast<void*>(StartArtAttackCalculateDamage),
        reinterpret_cast<void*>(BranchBackArtAttackCalculateDamage));
    // Make Art Attack not ignore enemies with "Gulp immunity".
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_scissor_damage_Patch_ArtAttackCheckImmunity),
            0x2c000002U  /* cmpwi r0, 0x2 - check normal immunity again */);

    // Change Showstopper's OHKO rate based on power level.
    g_sac_suki_set_weapon_trampoline = patch::hookFunction(
        ttyd::sac_suki::sac_suki_set_weapon,
        [](EvtEntry* evt, bool isFirstCall) {
            // Call vanilla logic.
            g_sac_suki_set_weapon_trampoline(evt, isFirstCall);
            
            intptr_t sac_work_addr = reinterpret_cast<intptr_t>(GetSacWorkPtr());
            BattleWeapon& weapon = 
                *reinterpret_cast<BattleWeapon*>(sac_work_addr + 0x284);
            int32_t bars_full = evtGetValue(evt, evt->evtArguments[0]) - 1;
            switch (tot::MoveManager::GetSelectedLevel(
                tot::MoveType::SP_SHOWSTOPPER)) {
                case 1: weapon.ohko_chance = 30 + bars_full * 7;  break;
                case 2: weapon.ohko_chance = 45 + bars_full * 9;  break;
                case 3: weapon.ohko_chance = 60 + bars_full * 11; break;
            }
            return 2;
        });
    
    // Change attack power of Supernova to 2/4/6x # of bars, based on level.
    g_weaponGetPower_ZubaStar_trampoline = patch::hookFunction(
        ttyd::sac_zubastar::weaponGetPower_ZubaStar, [](
            BattleWorkUnit*, BattleWeapon*, 
            BattleWorkUnit*, BattleWorkUnitPart*) {
            intptr_t sac_work_addr = reinterpret_cast<intptr_t>(GetSacWorkPtr());
            int32_t level = *reinterpret_cast<int32_t*>(sac_work_addr + 0x10);
            int32_t multiplier = 2 * tot::MoveManager::GetSelectedLevel(
                tot::MoveType::SP_SUPERNOVA);
            return static_cast<uint32_t>(level * multiplier);
        });
}

void OnEnterExitBattle(bool is_start) {
    if (is_start) {
        int32_t badge_count;
        for (int32_t i = 0; i < 2; ++i) {
            // Charge.
            badge_count = ttyd::mario_pouch::pouchEquipCheckBadge(
                ItemType::CHARGE + i);
            g_MaxMoveBadgeCounts[i] = badge_count;
            g_CurMoveBadgeCounts[i] = badge_count < 99 ? badge_count : 1;
            // Toughen Up (move); currently no longer used.
            badge_count = ttyd::mario_pouch::pouchEquipCheckBadge(
                ItemType::SUPER_CHARGE + i);
            g_MaxMoveBadgeCounts[2 + i] = badge_count;
            g_CurMoveBadgeCounts[2 + i] = badge_count < 99 ? badge_count : 1;
        }
        g_InBattle = true;
    } else {
        g_InBattle = false;
    }
}

int8_t GetStrategyBadgeLevel(bool is_charge, bool is_mario) {
    return g_CurMoveBadgeCounts[!is_charge * 2 + !is_mario];
}

void SweetTreatSetUpTargets() {
    // Count of each type of target (HP, 3xHP, PHP, 3xPHP, FP, 3xFP, poison)
    // for the three levels of Sweet Treat and Sweet Feast.
    static constexpr const int8_t kTargetCounts[] = {
        // Sweet Treat (up to 25 targets total)
        7,  0,  7,  0,  8,  0,  3,
        3,  4,  3,  4,  5,  3,  3,
        0,  8,  0,  8,  0,  7,  2,
        // Sweet Feast (up to 50 targets total)
        14, 2,  14, 2,  14, 2,  2,
        9,  7,  9,  7,  9,  7,  2,
        4,  12, 4,  12, 4,  12, 2,
    };
    const int32_t kNumTargetTypes = 7;
    
    intptr_t sac_work_addr = reinterpret_cast<intptr_t>(GetSacWorkPtr());
    int32_t is_feast = *reinterpret_cast<int32_t*>(sac_work_addr + 0xc);
    int32_t* target_arr = *reinterpret_cast<int32_t**>(sac_work_addr + 0x5c);
    
    // Select which set of target counts to use.
    int32_t start_idx;
    if (is_feast) {
        start_idx = kNumTargetTypes * (
            tot::MoveManager::GetSelectedLevel(tot::MoveType::SP_SWEET_FEAST) 
            + 2);
    } else {
        start_idx = kNumTargetTypes * (
            tot::MoveManager::GetSelectedLevel(tot::MoveType::SP_SWEET_TREAT) 
            - 1);
    }
    
    // Determine whether Mario's partner is dead or nonexistent.
    BattleWorkUnit* unit =
        ttyd::battle::BattleGetPartyPtr(ttyd::battle::g_BattleWork);
    bool mario_alone = !unit || ttyd::battle_unit::BtlUnit_CheckStatus(unit, 27);
    
    // Fill the array of targets.
    int32_t num_targets = 0;
    for (int32_t type = 0; type < kNumTargetTypes; ++type) {
        int32_t num_of_type = kTargetCounts[start_idx + type];
        if (mario_alone) {
            // If partner is dead / not present, give Mario 50% more HP targets,
            // and skip the partner targets.
            if (type == 0 || type == 1) num_of_type = num_of_type * 3 / 2;
            if (type == 2 || type == 3) continue;
        }
        for (int32_t i = 0; i < num_of_type; ++i) {
            target_arr[num_targets++] = type;
        }
    }
    // Shuffle with random swaps.
    for (int32_t i = 0; i < 200; ++i) {
        int32_t idx_a = ttyd::system::irand(num_targets);
        int32_t idx_b = ttyd::system::irand(num_targets);
        int32_t tmp = target_arr[idx_a];
        target_arr[idx_a] = target_arr[idx_b];
        target_arr[idx_b] = tmp;
    }
    
    // Set duration of attack based on the number of targets used.
    int32_t timer = (is_feast ? 0x12 : 0x25) * num_targets;
    *reinterpret_cast<int32_t*>(sac_work_addr + 0x40) = timer;
}

void SweetTreatBlinkNumbers() {
    const intptr_t sac_work_addr = reinterpret_cast<intptr_t>(GetSacWorkPtr());
    const int32_t is_feast = *reinterpret_cast<int32_t*>(sac_work_addr + 0xc);
    
    for (int32_t i = 0; i < 12; i += 4) {
        int32_t target_number =
            *reinterpret_cast<int32_t*>(sac_work_addr + 0x10 + i);
        // Sweet Feast should only give 1/5 of the target value, rounded up,
        // since it now gives Regen status for 5 turns.
        if (is_feast) target_number = (target_number + 4) / 5;
        
        float& disp_number = *reinterpret_cast<float*>(sac_work_addr + 0x1c + i);
        if (disp_number < target_number) {
            ++disp_number;
            // Refresh blinking timer.
            *reinterpret_cast<int32_t*>(sac_work_addr + 0x28 + i) = 60;
        } else if (disp_number > target_number) {
            --disp_number;
            // Refresh blinking timer.
            *reinterpret_cast<int32_t*>(sac_work_addr + 0x28 + i) = 60;
        }
    }
}

int32_t GetEarthTremorNumberOfBars() {
    // The level 1 and 2 versions have shorter minigames, at 3 and 4 bars.
    return tot::MoveManager::GetSelectedLevel(tot::MoveType::SP_EARTH_TREMOR)
        + 2;
}

int32_t GetArtAttackPower(int32_t circled_percent) {
    // Like vanilla, circling 90% or more = full damage;
    // the damage dealt is up to 2, 3, or 4 based on the level.
    return (tot::MoveManager::GetSelectedLevel(tot::MoveType::SP_ART_ATTACK) 
        + 1) * circled_percent / 90;
}

}  // namespace mario_move
}  // namespace mod::infinite_pit