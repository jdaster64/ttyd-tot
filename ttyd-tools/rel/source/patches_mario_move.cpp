#include "patches_mario_move.h"

#include "common_types.h"
#include "evt_cmd.h"
#include "mod.h"
#include "mod_state.h"
#include "patch.h"
#include "patches_battle.h"

#include <ttyd/battle.h>
#include <ttyd/battle_camera.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_mario.h>
#include <ttyd/battle_menu_disp.h>
#include <ttyd/battle_unit.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
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

namespace BattleUnitType = ::ttyd::battle_database_common::BattleUnitType;
namespace ItemType = ::ttyd::item_data::ItemType;

}

// Function hooks.
extern int32_t (*g_BtlUnit_GetWeaponCost_trampoline)(BattleWorkUnit*, BattleWeapon*);
extern int32_t (*g_pouchEquipCheckBadge_trampoline)(int16_t);
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
extern const int32_t g_marioAttackEvent_JyabaraJump_Patch_GetSp1;
extern const int32_t g_marioAttackEvent_JyabaraJump_Patch_GetSp2;
extern const int32_t g_marioAttackEvent_JyabaraJump_Patch_GetSp3;
extern const int32_t g_marioAttackEvent_JyabaraJump_Patch_GetSp4;
extern const int32_t g_marioAttackEvent_JyabaraJump_Patch_PrizeTier1;
extern const int32_t g_marioAttackEvent_JyabaraJump_Patch_PrizeTier2;
extern const int32_t g_marioAttackEvent_JyabaraJump_Patch_ResetFace;
extern const int32_t g_sac_genki_main_base_BlinkNumbers_BH;
extern const int32_t g_sac_genki_main_base_BlinkNumbers_EH;
extern const int32_t g_sac_genki_main_base_SetupTargets_BH;
extern const int32_t g_sac_genki_main_base_SetupTargets_EH;
extern const int32_t g_genki_evt_common_Patch_SweetTreatFeastResult;
extern const int32_t g_genki_evt_common_SweetTreatResultJumpPoint;
extern const int32_t g_sac_deka_main_base_GetNumberOfBars_BH;
extern const int32_t g_scissor_damage_sub_ArtAttackDamage_BH;
extern const int32_t g_scissor_damage_sub_ArtAttackDamage_EH;

namespace mario_move {

namespace {
    
// Global variables and constants.
bool                g_InBattle = false;
int8_t              g_MaxMoveBadgeCounts[18];
int8_t              g_CurMoveBadgeCounts[18];
int8_t              g_MaxSpecialMoveLvls[8];
int8_t              g_CurSpecialMoveLvls[8];
char                g_MoveBadgeTextBuffer[24];
const char*         kMoveBadgeAbbreviations[18] = {
    "Power J.", "Multib.", "Power B.", "Tor. J.", "Shrink S.",
    "Sleep S.", "Soft S.", "Power S.", "Quake H.", "H. Throw",
    "Pier. B.", "H. Rattle", "Fire Drive", "Ice Smash",
    "Charge", "Charge", "Tough. Up", "Tough. Up"
};
const char*         kSpecialMoveAbbreviations[8] = {
    "Sweet Tr.", "Earth Tr.", "Clock Out", "Power Lift",
    "Art Attack", "Sweet F.", "Showst.", "Supernova"
};
const int8_t        kSpCostLevels[] = {
    1, 3, 5,    1, 2, 3,    2, 3, 5,    3, 4, 6,
    4, 5, 7,    3, 4, 6,    2, 4, 6,    7, 8, 9,
};

// Patch to disable getting Star Power early from certain attacks;
// battle::AwardStarPowerAndResetFaceDirection will be used to award it
// at the end of the attack instead, to make sure Stylishes are counted.
EVT_BEGIN(DeclareStarPowerPatch)
DEBUG_REM(0) DEBUG_REM(0)
EVT_PATCH_END()
static_assert(sizeof(DeclareStarPowerPatch) == 0x10);

// Returns the max level of a move badge based on the number of copies equipped.
int32_t MaxLevelForMoveBadges(int32_t badge_count) {
    int32_t max_level = badge_count;
    switch (g_Mod->state_.GetOptionValue(OPT_BADGE_MOVE_LEVEL)) {
        case OPTVAL_BADGE_MOVE_1X: {
            break;
        }
        case OPTVAL_BADGE_MOVE_2X: {
            max_level *= 2;
            break;
        }
        case OPTVAL_BADGE_MOVE_RANK: {
            if (!badge_count) return 0;
            return ttyd::mario_pouch::pouchGetPtr()->rank + 1;
        }
        case OPTVAL_BADGE_MOVE_INFINITE: {
            if (!badge_count) return 0;
            return 99;
        }
    }
    if (max_level > 99) max_level = 99;
    return max_level;
}

// If the badge is one that can have its power level selected, returns the
// index of the value controlling its level; otherwise, returns -1.
int32_t GetWeaponLevelSelectionIndex(int16_t badge_id) {
    if (badge_id >= ItemType::POWER_JUMP && badge_id <= ItemType::ICE_SMASH) {
        return badge_id - ItemType::POWER_JUMP;
    }
    switch (badge_id) {
        case ItemType::CHARGE: return 14;
        case ItemType::CHARGE_P: return 15;
        case ItemType::SUPER_CHARGE: return 16;
        case ItemType::SUPER_CHARGE_P: return 17;
    }
    return -1;
}

// Gets the cost of badge moves that can have their power level selected.
// Returns -1 if the weapon is not one of these moves.
int32_t GetSelectedLevelWeaponCost(BattleWorkUnit* unit, BattleWeapon* weapon) {
    if (!weapon || !g_InBattle) return -1;
    if (int32_t idx = GetWeaponLevelSelectionIndex(weapon->item_id); idx >= 0) {
        const int32_t fp_cost =
            weapon->base_fp_cost * g_CurMoveBadgeCounts[idx] -
            unit->badges_equipped.flower_saver;
        return fp_cost < 1 ? 1 : fp_cost;
    }
    return -1;
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
            
            strats[i].cost = GetSelectedLevelWeaponCost(unit, weapon);
            strats[i].enabled =
                strats[i].cost <= ttyd::battle_unit::BtlUnit_GetFp(unit);
            strats[i].unk_08 = !strats[i].enabled;  // 1 if disabled: "no FP" msg
        }
        
        // Handle switch partner cost, if enabled.
        int32_t switch_fp_cost =
            g_Mod->state_.GetOptionValue(OPTNUM_SWITCH_PARTY_FP_COST);
        if (strats[0].type == 0 && switch_fp_cost) {
            // Reduce switch partner cost by Flower Savers.
            switch_fp_cost -= unit->badges_equipped.flower_saver;
            if (switch_fp_cost < 1) switch_fp_cost = 1;
            
            strats[0].cost = switch_fp_cost;
            strats[0].enabled =
                ttyd::battle_unit::BtlUnit_GetFp(unit) >= switch_fp_cost;
            strats[0].unk_08 = !strats[0].enabled;  // 1 if disabled: "no FP" msg
        }
    } else {
        auto* weapons = reinterpret_cast<BattleWorkCommandWeapon*>(win_data[2]);
        if (!weapons) return;
        for (int32_t i = 0; i < cursor->num_options; ++i) {
            if (!weapons[i].weapon) continue;
            auto* weapon = weapons[i].weapon;
            
            // Handle Special moves.
            if (weapon->base_sp_cost) {
                // Match the Star Power by its icon.
                int32_t idx = 0;
                switch (weapon->icon) {
                    case 0x19f: { idx = 1; break; }
                    case 0x1a0: { idx = 2; break; }
                    case 0x1a2: { idx = 3; break; }
                    case 0x1a4: { idx = 4; break; }
                    case 0x1a5: { idx = 5; break; }
                    case 0x1a1: { idx = 6; break; }
                    case 0x1a3: { idx = 7; break; }
                }
                
                // If current selection, and L/R pressed, change power level.
                if (i == cursor->abs_position) {
                    if (left_press && g_CurSpecialMoveLvls[idx] > 1) {
                        --g_CurSpecialMoveLvls[idx];
                        ttyd::sound::SoundEfxPlayEx(0x478, 0, 0x64, 0x40);
                    } else if (
                        right_press &&
                        g_CurSpecialMoveLvls[idx] < g_MaxSpecialMoveLvls[idx]) {
                        ++g_CurSpecialMoveLvls[idx];
                        ttyd::sound::SoundEfxPlayEx(0x478, 0, 0x64, 0x40);
                    }
                    
                    // Overwrite default text based on current power level.
                    sprintf(
                        g_MoveBadgeTextBuffer, "%s Lv. %" PRId8,
                        kSpecialMoveAbbreviations[idx],
                        g_CurSpecialMoveLvls[idx]);
                    weapons[i].name = g_MoveBadgeTextBuffer;
                } else {
                    weapons[i].name = ttyd::msgdrv::msgSearch(weapon->name);
                }
                
                // Update actual SP cost.
                int8_t new_cost = 
                    kSpCostLevels[idx * 3 + g_CurSpecialMoveLvls[idx] - 1];
                mod::patch::writePatch(
                    &ttyd::battle_mario::superActionTable[idx]->base_sp_cost,
                    &new_cost, sizeof(new_cost));
            }
            
            // Otherwise, must be an FP-costing move (assuming a badge move).
            const int32_t idx = GetWeaponLevelSelectionIndex(weapon->item_id);
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
                weapons[i].name = g_MoveBadgeTextBuffer;
            } else {
                weapons[i].name = ttyd::msgdrv::msgSearch(
                    itemDataTable[weapon->item_id].name);
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
    g_pouchEquipCheckBadge_trampoline = patch::hookFunction(
        ttyd::mario_pouch::pouchEquipCheckBadge, [](int16_t badge_id) {
            if (g_InBattle) {
                int32_t idx = GetWeaponLevelSelectionIndex(badge_id);
                if (idx >= 0) {
                    return static_cast<int32_t>(g_CurMoveBadgeCounts[idx]);
                }
            }
            return g_pouchEquipCheckBadge_trampoline(badge_id);
        });

    g_BtlUnit_GetWeaponCost_trampoline = patch::hookFunction(
        ttyd::battle_unit::BtlUnit_GetWeaponCost,
        [](BattleWorkUnit* unit, BattleWeapon* weapon) {
            int32_t cost = GetSelectedLevelWeaponCost(unit, weapon);
            if (cost >= 0) return cost;
            return g_BtlUnit_GetWeaponCost_trampoline(unit, weapon);
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
        
    // Increase Spring Jump's AC prize level by 1 to make it more desirable.
    mod::patch::writePatch(
        reinterpret_cast<void*>(
            g_marioAttackEvent_JyabaraJump_Patch_PrizeTier1), 1);
    mod::patch::writePatch(
        reinterpret_cast<void*>(
            g_marioAttackEvent_JyabaraJump_Patch_PrizeTier2), 2);
    // Move SP calculation to the end of the attack so the Stylish matters.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_marioAttackEvent_JyabaraJump_Patch_GetSp1),
        DeclareStarPowerPatch, sizeof(DeclareStarPowerPatch));
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_marioAttackEvent_JyabaraJump_Patch_GetSp2),
        DeclareStarPowerPatch, sizeof(DeclareStarPowerPatch));
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_marioAttackEvent_JyabaraJump_Patch_GetSp3),
        DeclareStarPowerPatch, sizeof(DeclareStarPowerPatch));
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_marioAttackEvent_JyabaraJump_Patch_GetSp4),
        DeclareStarPowerPatch, sizeof(DeclareStarPowerPatch));
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_marioAttackEvent_JyabaraJump_Patch_ResetFace),
        reinterpret_cast<uint32_t>(
            battle::AwardStarPowerAndResetFaceDirection));
    
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
            return static_cast<uint32_t>(g_CurSpecialMoveLvls[1] + bars_full);
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
                weapon.stop_time += (g_CurSpecialMoveLvls[2] - 2);
                if (weapon.stop_time < 1) weapon.stop_time = 1;
            }
            return 2;
        });

    // Change the rate at which Power Lift's gauges fill based on level.
    g_main_muki_trampoline = patch::hookFunction(
        ttyd::sac_muki::main_muki, [](EvtEntry* evt, bool isFirstCall) {
            // Change the amount of power gained per arrow hit on startup.
            if (isFirstCall) {
                float arrow_power = 0.20001;
                switch (g_CurSpecialMoveLvls[3]) {
                    case 2: arrow_power = 0.25001; break;
                    case 3: arrow_power = 0.33334; break;
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
            switch (g_CurSpecialMoveLvls[6]) {
                case 1: weapon.ohko_chance = 30 + bars_full * 7;  break;
                case 2: weapon.ohko_chance = 45 + bars_full * 9;  break;
                case 3: weapon.ohko_chance = 60 + bars_full * 11; break;
            }
            return 2;
        });
    
    // Change attack power of Supernova to 3/5/10x bars + 1, based on level.
    g_weaponGetPower_ZubaStar_trampoline = patch::hookFunction(
        ttyd::sac_zubastar::weaponGetPower_ZubaStar, [](
            BattleWorkUnit*, BattleWeapon*, 
            BattleWorkUnit*, BattleWorkUnitPart*) {
            intptr_t sac_work_addr = reinterpret_cast<intptr_t>(GetSacWorkPtr());
            int32_t level = *reinterpret_cast<int32_t*>(sac_work_addr + 0x10);
            int32_t multiplier = 3;
            switch (g_CurSpecialMoveLvls[7]) {
                case 2: multiplier = 5;     break;
                case 3: multiplier = 10;    break;
            }
            return static_cast<uint32_t>((level + 1) * multiplier);
        });
}

void OnEnterExitBattle(bool is_start) {
    if (is_start) {
        int8_t badge_count;
        int32_t max_level;
        for (int32_t i = 0; i < 14; ++i) {
            badge_count = ttyd::mario_pouch::pouchEquipCheckBadge(
                ItemType::POWER_JUMP + i);
            max_level = MaxLevelForMoveBadges(badge_count);
            g_MaxMoveBadgeCounts[i] = max_level;
            g_CurMoveBadgeCounts[i] = max_level < 99 ? max_level : 1;
        }
        for (int32_t i = 0; i < 2; ++i) {
            badge_count = ttyd::mario_pouch::pouchEquipCheckBadge(
                ItemType::CHARGE + i);
            max_level = MaxLevelForMoveBadges(badge_count);
            g_MaxMoveBadgeCounts[14 + i] = max_level;
            g_CurMoveBadgeCounts[14 + i] = max_level < 99 ? max_level : 1;
            badge_count = ttyd::mario_pouch::pouchEquipCheckBadge(
                ItemType::SUPER_CHARGE + i);
            max_level = MaxLevelForMoveBadges(badge_count);
            g_MaxMoveBadgeCounts[16 + i] = max_level;
            g_CurMoveBadgeCounts[16 + i] = max_level < 99 ? max_level : 1;
        }
        for (int32_t i = 0; i < 8; ++i) {
            g_MaxSpecialMoveLvls[i] = g_Mod->state_.GetStarPowerLevel(i);
            g_CurSpecialMoveLvls[i] = g_MaxSpecialMoveLvls[i];
        }
        g_InBattle = true;
    } else {
        g_InBattle = false;
    }
}

int8_t GetToughenUpLevel(bool is_mario) {
    return g_CurMoveBadgeCounts[17 - is_mario];
}

bool CanUnlockNextLevel(int32_t star_power) {
    int32_t current_level = g_Mod->state_.GetStarPowerLevel(star_power);
    if (current_level == 3) return false;
    // Get the max SP the player would have without using any Shine Sprites.
    int32_t max_sp = 50;
    for (int32_t i = 0; i < 8; ++i) {
        max_sp += g_Mod->state_.GetStarPowerLevel(i) * 50;
    }
    // See if the next level of this star power could be affordable.
    int32_t required_sp = kSpCostLevels[star_power * 3 + current_level];
    return max_sp + 50 >= required_sp * 100;
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
        start_idx = kNumTargetTypes * (g_CurSpecialMoveLvls[5] + 2);
    } else {
        start_idx = kNumTargetTypes * (g_CurSpecialMoveLvls[0] - 1);
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
    return g_CurSpecialMoveLvls[1] + 2;
}

int32_t GetArtAttackPower(int32_t circled_percent) {
    // Like vanilla, circling 90% or more = full damage;
    // the damage dealt is up to 2, 3, or 4 based on the level.
    return (g_CurSpecialMoveLvls[4] + 1) * circled_percent / 90;
}

}  // namespace mario_move
}  // namespace mod::infinite_pit