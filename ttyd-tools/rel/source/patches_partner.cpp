#include "patches_partner.h"

#include "common_functions.h"
#include "evt_cmd.h"
#include "mod.h"
#include "patch.h"
#include "patches_battle.h"
#include "tot_generate_enemy.h"
#include "tot_gsw.h"
#include "tot_party_bobbery.h"
#include "tot_party_flurrie.h"
#include "tot_party_goombella.h"
#include "tot_party_koops.h"
#include "tot_party_mowz.h"
#include "tot_party_vivian.h"
#include "tot_party_yoshi.h"
#include "tot_state.h"

#include <gc/types.h>
#include <ttyd/battle.h>
#include <ttyd/battle_damage.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_monosiri.h>
#include <ttyd/battle_sub.h>
#include <ttyd/battle_unit.h>
#include <ttyd/battle_weapon_power.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/icondrv.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/swdrv.h>
#include <ttyd/system.h>
#include <ttyd/unit_bomzou.h>
#include <ttyd/unit_koura.h>
#include <ttyd/unit_mario.h>
#include <ttyd/unit_party_christine.h>
#include <ttyd/unit_party_chuchurina.h>
#include <ttyd/unit_party_clauda.h>
#include <ttyd/unit_party_nokotarou.h>
#include <ttyd/unit_party_sanders.h>
#include <ttyd/unit_party_vivian.h>
#include <ttyd/unit_party_yoshi.h>

#include <cstdint>
#include <cstring>

// Assembly patch functions.
extern "C" {
    // tattle_disp_patches.s
    void StartDispTattleStats();
    void BranchBackDispTattleStats();
    
    void dispTattleStats(
        gc::mtx34* matrix, int32_t number, int32_t is_small, uint32_t* color,
        ttyd::battle_unit::BattleWorkUnit* unit) {
        mod::tot::patch::partner::DisplayTattleStats(
            matrix, number, is_small, color, unit);
    }
}

namespace mod::tot::patch {

namespace {
    
// Include whole namespace for convenience.
using namespace ::ttyd::battle_event_cmd;

using ::ttyd::battle_database_common::BattleWeapon;
using ::ttyd::battle_database_common::BattleWeapon;
using ::ttyd::battle_unit::BattleWorkUnit;
using ::ttyd::battle_weapon_power::weaponGetPowerDefault;
using ::ttyd::evtmgr::EvtEntry;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

namespace BattleUnitType = ::ttyd::battle_database_common::BattleUnitType;
namespace ItemType = ::ttyd::item_data::ItemType;

}

// Function hooks.
extern int32_t (*g_btlevtcmd_get_monosiri_msg_no_trampoline)(EvtEntry*, bool);
// Patch addresses.
extern const int32_t g_BattleDrawEnemyHP_DrawEnemyHPText_BH;
extern const int32_t g_koura_pose_tbl_reset_Patch_HeavyDmg;
extern const int32_t g_koura_pose_tbl_reset_Patch_LightDmg;
extern const int32_t g_koura_damage_core_Patch_HeavyDmg;
extern const int32_t g_koura_damage_core_Patch_LightDmg;
extern const int32_t g_subsetevt_blow_dead_Patch_GetRewards;
extern const int32_t g_BattleSetStatusDamage_Patch_GaleLevelFactor;
extern const int32_t g_acShot_dispAfterimage_Patch_numBombs;
extern const int32_t g_acShot_dispAfterimage_Patch_targetVar2_1;
extern const int32_t g_acShot_dispAfterimage_Patch_targetVar2_2;
extern const int32_t g_acShot_dispAfterimage_Patch_targetVar3_1;
extern const int32_t g_acShot_dispAfterimage_Patch_targetVar3_2;
extern const int32_t g_acShot_dispAfterimage_Patch_targetVar3_3;
extern const int32_t g_acShot_dispAfterimage_Patch_targetVar3_4;
extern const int32_t g_acShot_main_Patch_numBombs;
extern const int32_t g_acShot_main_Patch_targetVar2_1;
extern const int32_t g_acShot_main_Patch_targetVar2_2;
extern const int32_t g_acShot_main_Patch_targetVar2_3;
extern const int32_t g_acShot_main_Patch_targetVar2_4;
extern const int32_t g_acShot_main_Patch_targetVar2_5;
extern const int32_t g_acShot_main_Patch_targetVar2_6;
extern const int32_t g_acShot_main_Patch_targetVar3_1;
extern const int32_t g_acShot_main_Patch_targetVar3_2;
extern const int32_t g_acShot_main_Patch_targetVar3_3;
extern const int32_t g_acShot_main_Patch_targetVar3_4;
extern const int32_t g_acShot_main_Patch_targetVar3_5;
extern const int32_t g_acShot_main_Patch_sfxId_1;
extern const int32_t g_acShot_main_Patch_sfxId_2;
extern const int32_t g_acShot_main_Patch_sfxId_3;
extern const int32_t g_partyClauda_makeTechMenuFuncPtr;
extern const int32_t g_partyYoshi_makeTechMenuFuncPtr;
extern const int32_t g_partyChuchurina_makeTechMenuFuncPtr;
extern const int32_t g_partySanders_makeTechMenuFuncPtr;
extern const int32_t g_partyVivian_makeTechMenuFuncPtr;
extern const int32_t g_partyNokotarou_makeTechMenuFuncPtr;
extern const int32_t g_partyChristine_makeTechMenuFuncPtr;
extern const int32_t g_partyNokotarou_Patch_InitWaitPhase;
extern const int32_t g_ac_power_gauge_lv2_LipLockPower;
extern const int32_t g_ac_air_gauge_FlurrieGaleForceResistance;

namespace partner {
    
namespace {
    
EVT_BEGIN(KoopsCustomInitEvt)
// Select idle pose based on whether in 'invincible' state from Withdraw.
USER_FUNC(btlevtcmd_CheckPartsAttribute, -2, 1, int(0xe0000000), LW(0))
IF_EQUAL(LW(0), 0)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
ELSE()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_A_1"))
END_IF()
RETURN()
EVT_END()

EVT_BEGIN(KoopsCustomPhaseEvt)
// Break out of 'invincible' state at beginning of player movement phase.
USER_FUNC(btlevtcmd_CheckPhase, LW(0), 0x4000002)
IF_EQUAL(LW(0), 1)
    USER_FUNC(btlevtcmd_CheckPartsAttribute, -2, 1, int(0xe0000000), LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
        USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, int(0xe0000000))
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 1, int(0x1000))
    END_IF()
END_IF()
RETURN()
EVT_END()
    
EVT_BEGIN(KoopsInitWaitPhase)
USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&KoopsCustomInitEvt))
USER_FUNC(btlevtcmd_SetEventPhase, -2, PTR(&KoopsCustomPhaseEvt))
RETURN()
EVT_END()

EVT_BEGIN(KoopsInitWaitPhaseHook)
RUN_CHILD_EVT(KoopsInitWaitPhase) DEBUG_REM(0)
EVT_PATCH_END()
static_assert(sizeof(KoopsInitWaitPhaseHook) == 0x10);

// Patch to disable the coins / EXP from Gale Force for most enemies.
// (Replaces the code with no-ops).  Needs to be patched into enemies' specific
// death events in some cases as well.
EVT_BEGIN(GaleForceKillPatch)
DEBUG_REM(0) DEBUG_REM(0) DEBUG_REM(0) DEBUG_REM(0)
DEBUG_REM(0) DEBUG_REM(0) DEBUG_REM(0)
EVT_PATCH_END()
static_assert(sizeof(GaleForceKillPatch) == 0x38);

// Patch to disable getting Star Power early from certain attacks;
// battle::AwardStarPowerAndResetFaceDirection will be used to award it
// at the end of the attack instead, to make sure Stylishes are counted.
EVT_BEGIN(DeclareStarPowerPatch)
DEBUG_REM(0) DEBUG_REM(0)
EVT_PATCH_END()
static_assert(sizeof(DeclareStarPowerPatch) == 0x10);

}
    
void ApplyFixedPatches() {
    
    // Tattle returns a custom message based on the enemy's stats.
    g_btlevtcmd_get_monosiri_msg_no_trampoline = mod::hookFunction(
        ttyd::unit_party_christine::btlevtcmd_get_monosiri_msg_no,
        [](EvtEntry* evt, bool isFirstCall) {
            auto* battleWork = ttyd::battle::g_BattleWork;
            int32_t unit_idx = evtGetValue(evt, evt->evtArguments[0]);
            unit_idx = ttyd::battle_sub::BattleTransID(evt, unit_idx);
            auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, unit_idx);
            
            // Get original pointer to Tattle string.
            g_btlevtcmd_get_monosiri_msg_no_trampoline(evt, isFirstCall);
            const char* tattle_msg = 
                reinterpret_cast<const char*>(
                    evtGetValue(evt, evt->evtArguments[2]));
            // Build a custom tattle, if the enemy has stats to pull from.
            tattle_msg = SetCustomTattle(unit, tattle_msg);
            evtSetValue(evt, evt->evtArguments[2], PTR(tattle_msg));
            return 2;
        });

    // Calls a custom function to display ATK / DEF under HP if a unit has
    // previously been Tattled (and if it's currently the player's turn to act).
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_BattleDrawEnemyHP_DrawEnemyHPText_BH),
        reinterpret_cast<void*>(StartDispTattleStats),
        reinterpret_cast<void*>(BranchBackDispTattleStats));
        
    // Replace Koops' init and phase scripts to handle Withdraw status.
    mod::writePatch(
        reinterpret_cast<void*>(g_partyNokotarou_Patch_InitWaitPhase),
        KoopsInitWaitPhaseHook, sizeof(KoopsInitWaitPhaseHook));
        
    // Set HP thresholds for different Shell Shield disrepair animation states:
    // - On initialization (pose_tbl_reset)
    mod::writePatch(
        reinterpret_cast<void*>(g_koura_pose_tbl_reset_Patch_HeavyDmg), 1);
    mod::writePatch(
        reinterpret_cast<void*>(g_koura_pose_tbl_reset_Patch_LightDmg), 2);
    // - On damage (damage_core)
    mod::writePatch(
        reinterpret_cast<void*>(g_koura_damage_core_Patch_HeavyDmg), 1);
    mod::writePatch(
        reinterpret_cast<void*>(g_koura_damage_core_Patch_LightDmg), 2);

    // Change the strength of Gale Force / Blizzard's resistance to be
    // more manageable at higher difficulties.
    float kGaleForceResistanceArr[] = {
        0.30f, 0.34f, 0.37f, 0.40f, 0.42f, 0.435f, 0.45f
    };
    mod::writePatch(
        reinterpret_cast<void*>(g_ac_air_gauge_FlurrieGaleForceResistance),
        kGaleForceResistanceArr, sizeof(kGaleForceResistanceArr));

    // Change the strength of Lip Lock's power to scale down less harshly at
    // higher difficulties and up less hilariously at lower ones.
    int32_t kLipLockPowerArr[] = { 124, 116, 108, 100, 92, 84, 76 };
    mod::writePatch(
        reinterpret_cast<void*>(g_ac_power_gauge_lv2_LipLockPower),
        kLipLockPowerArr, sizeof(kLipLockPowerArr));
    
    
    // Disable getting coins and experience from a successful Gale Force.
    mod::writePatch(
        reinterpret_cast<void*>(g_subsetevt_blow_dead_Patch_GetRewards),
        GaleForceKillPatch, sizeof(GaleForceKillPatch));
    // Remove the (Mario - enemy level) adjustment to Gale Force's chance.
    mod::writePatch(
        reinterpret_cast<void*>(g_BattleSetStatusDamage_Patch_GaleLevelFactor),
        0x60000000U /* nop */);
        
    // Patch a bunch of Bomb Squad's AC logic that assumes a max of 3 bombs;
    // all patches are just changing the offset in the AC extra-work struct.
    constexpr const int8_t k_MaxBombs = 4;
    constexpr const int8_t k_TargetVar1Offset = 0x40;
    constexpr const int8_t
        k_TargetVar2Offset = k_TargetVar1Offset + k_MaxBombs * 4;
    constexpr const int8_t
        k_TargetVar3Offset = k_TargetVar1Offset + k_MaxBombs * 8;
    constexpr const int8_t
        k_SfxIdOffset = k_TargetVar1Offset + k_MaxBombs * 12;
    mod::writePatch(
        reinterpret_cast<void*>(g_acShot_dispAfterimage_Patch_numBombs), 
        &k_MaxBombs, sizeof(int8_t));
    mod::writePatch(
        reinterpret_cast<void*>(g_acShot_dispAfterimage_Patch_targetVar2_1),
        &k_TargetVar2Offset, sizeof(int8_t));
    mod::writePatch(
        reinterpret_cast<void*>(g_acShot_dispAfterimage_Patch_targetVar2_2),
        &k_TargetVar2Offset, sizeof(int8_t));
    mod::writePatch(
        reinterpret_cast<void*>(g_acShot_dispAfterimage_Patch_targetVar3_1),
        &k_TargetVar3Offset, sizeof(int8_t));
    mod::writePatch(
        reinterpret_cast<void*>(g_acShot_dispAfterimage_Patch_targetVar3_2),
        &k_TargetVar3Offset, sizeof(int8_t));
    mod::writePatch(
        reinterpret_cast<void*>(g_acShot_dispAfterimage_Patch_targetVar3_3),
        &k_TargetVar3Offset, sizeof(int8_t));
    mod::writePatch(
        reinterpret_cast<void*>(g_acShot_dispAfterimage_Patch_targetVar3_4),
        &k_TargetVar3Offset, sizeof(int8_t));
    mod::writePatch(
        reinterpret_cast<void*>(g_acShot_main_Patch_numBombs),
        &k_MaxBombs, sizeof(int8_t));
    mod::writePatch(
        reinterpret_cast<void*>(g_acShot_main_Patch_targetVar2_1),
        &k_TargetVar2Offset, sizeof(int8_t));
    mod::writePatch(
        reinterpret_cast<void*>(g_acShot_main_Patch_targetVar2_2),
        &k_TargetVar2Offset, sizeof(int8_t));
    mod::writePatch(
        reinterpret_cast<void*>(g_acShot_main_Patch_targetVar2_3),
        &k_TargetVar2Offset, sizeof(int8_t));
    mod::writePatch(
        reinterpret_cast<void*>(g_acShot_main_Patch_targetVar2_4),
        &k_TargetVar2Offset, sizeof(int8_t));
    mod::writePatch(
        reinterpret_cast<void*>(g_acShot_main_Patch_targetVar2_5),
        &k_TargetVar2Offset, sizeof(int8_t));
    mod::writePatch(
        reinterpret_cast<void*>(g_acShot_main_Patch_targetVar2_6),
        &k_TargetVar2Offset, sizeof(int8_t));
    mod::writePatch(
        reinterpret_cast<void*>(g_acShot_main_Patch_targetVar3_1),
        &k_TargetVar3Offset, sizeof(int8_t));
    mod::writePatch(
        reinterpret_cast<void*>(g_acShot_main_Patch_targetVar3_2),
        &k_TargetVar3Offset, sizeof(int8_t));
    mod::writePatch(
        reinterpret_cast<void*>(g_acShot_main_Patch_targetVar3_3),
        &k_TargetVar3Offset, sizeof(int8_t));
    mod::writePatch(
        reinterpret_cast<void*>(g_acShot_main_Patch_targetVar3_4),
        &k_TargetVar3Offset, sizeof(int8_t));
    mod::writePatch(
        reinterpret_cast<void*>(g_acShot_main_Patch_targetVar3_5),
        &k_TargetVar3Offset, sizeof(int8_t));
    mod::writePatch(
        reinterpret_cast<void*>(g_acShot_main_Patch_sfxId_1),
        &k_SfxIdOffset, sizeof(int8_t));
    mod::writePatch(
        reinterpret_cast<void*>(g_acShot_main_Patch_sfxId_2),
        &k_SfxIdOffset, sizeof(int8_t));
    mod::writePatch(
        reinterpret_cast<void*>(g_acShot_main_Patch_sfxId_3),
        &k_SfxIdOffset, sizeof(int8_t));
        
    // TOT: Replace party member weapon selection functions.
    mod::writePatch(
        reinterpret_cast<void*>(g_partyChristine_makeTechMenuFuncPtr),
        reinterpret_cast<int32_t>(tot::party_goombella::MakeSelectWeaponTable));
    mod::writePatch(
        reinterpret_cast<void*>(g_partyNokotarou_makeTechMenuFuncPtr),
        reinterpret_cast<int32_t>(tot::party_koops::MakeSelectWeaponTable));
    mod::writePatch(
        reinterpret_cast<void*>(g_partyClauda_makeTechMenuFuncPtr),
        reinterpret_cast<int32_t>(tot::party_flurrie::MakeSelectWeaponTable));
    mod::writePatch(
        reinterpret_cast<void*>(g_partyYoshi_makeTechMenuFuncPtr),
        reinterpret_cast<int32_t>(tot::party_yoshi::MakeSelectWeaponTable));
    mod::writePatch(
        reinterpret_cast<void*>(g_partyVivian_makeTechMenuFuncPtr),
        reinterpret_cast<int32_t>(tot::party_vivian::MakeSelectWeaponTable));
    mod::writePatch(
        reinterpret_cast<void*>(g_partySanders_makeTechMenuFuncPtr),
        reinterpret_cast<int32_t>(tot::party_bobbery::MakeSelectWeaponTable));
    mod::writePatch(
        reinterpret_cast<void*>(g_partyChuchurina_makeTechMenuFuncPtr),
        reinterpret_cast<int32_t>(tot::party_mowz::MakeSelectWeaponTable));
    
    // Make Shell Shield only take one damage per hit.
    for (int32_t i = 0; i < 5; ++i) {
        ttyd::unit_koura::unitKoura_defense_attr[i] = 4;
    }

    // Replace Vivian's unveiling event with a custom one.
    ttyd::unit_party_vivian::data_table_Party_Vivian[0].data =
        tot::party_vivian::GetVivianUnhideEvt();

    // Give party a higher resistance to Power Bounce / Multibonk.
    ttyd::unit_mario::unitdata_Mario.pb_soft_cap = 5;
    ttyd::unit_party_christine::unitdata_Party_Christine.pb_soft_cap = 5;
    ttyd::unit_party_nokotarou::unitdata_Party_Nokotarou.pb_soft_cap = 5;
    ttyd::unit_party_clauda::unitdata_Party_Clauda.pb_soft_cap = 5;
    ttyd::unit_party_yoshi::unitdata_Party_Yoshi.pb_soft_cap = 5;
    ttyd::unit_party_vivian::unitdata_Party_Vivian.pb_soft_cap = 5;
    ttyd::unit_party_sanders::unitdata_Party_Sanders.pb_soft_cap = 5;
    ttyd::unit_party_chuchurina::unitdata_Party_Chuchurina.pb_soft_cap = 5;

    // Allow party to be susceptible to Lickety Spit.
    ttyd::unit_mario::unitdata_Mario.swallow_attributes = 2;
    ttyd::unit_party_christine::unitdata_Party_Christine.swallow_attributes = 2;
    ttyd::unit_party_nokotarou::unitdata_Party_Nokotarou.swallow_attributes = 2;
    ttyd::unit_party_clauda::unitdata_Party_Clauda.swallow_attributes = 2;
    ttyd::unit_party_yoshi::unitdata_Party_Yoshi.swallow_attributes = 2;
    ttyd::unit_party_vivian::unitdata_Party_Vivian.swallow_attributes = 2;
    ttyd::unit_party_sanders::unitdata_Party_Sanders.swallow_attributes = 2;
    ttyd::unit_party_chuchurina::unitdata_Party_Chuchurina.swallow_attributes = 2;
    ttyd::unit_mario::unitdata_Mario.swallow_chance = 0;
    ttyd::unit_party_christine::unitdata_Party_Christine.swallow_chance = 0;
    ttyd::unit_party_nokotarou::unitdata_Party_Nokotarou.swallow_chance = 0;
    ttyd::unit_party_clauda::unitdata_Party_Clauda.swallow_chance = 0;
    ttyd::unit_party_yoshi::unitdata_Party_Yoshi.swallow_chance = 0;
    ttyd::unit_party_vivian::unitdata_Party_Vivian.swallow_chance = 0;
    ttyd::unit_party_sanders::unitdata_Party_Sanders.swallow_chance = 0;
    ttyd::unit_party_chuchurina::unitdata_Party_Chuchurina.swallow_chance = 0;
}

void DisplayTattleStats(
    gc::mtx34* matrix, int32_t number, int32_t is_small, uint32_t* color,
    BattleWorkUnit* unit) {

    // If enemy has been Tattled (regular Peekaboo does not count), 
    // display the enemy's ATK and DEF underneath their HP.
    bool show_atk_def =
        (ttyd::swdrv::swGet(0x117a + unit->true_kind) ||
        ttyd::swdrv::swGet(0x117a + unit->current_kind));
    // If option is enabled, always display.
    if (GetSWF(GSWF_SuperPeekabooEnabled)) {
        show_atk_def = true;
    }
    // Hide ATK / DEF outside player action phase.
    if (!(ttyd::battle::g_BattleWork->battle_flags & 0x80)) {
        show_atk_def = false;
    }
       
    if (show_atk_def) {
        int32_t atk, def;
        
        // If the enemy's ATK and DEF can't be fetched, just draw HP normally.
        if (!GetTattleDisplayStats(unit->current_kind, &atk, &def)) {
            ttyd::icondrv::iconNumberDispGx(matrix, number, is_small, color);
            return;
        }
        
        // Get current ATK and DEF, including statuses, badges, etc.
        atk = battle::GetCurrentEnemyAtk(unit);
        def = battle::GetCurrentEnemyDef(unit);
        
        atk = Clamp(atk, 0, 99);
        def = Clamp(def, 0, 99);

        // Undo alignment adjustment the game normally does for small numbers.
        if (number < 100) matrix->m[0][3] += 4.0f;
        if (number < 10) matrix->m[0][3] += 4.0f;
        ttyd::icondrv::iconNumberDispGx(matrix, number, is_small, color);
        
        // Draw ATK and DEF numbers.
        uint32_t color_atk = 0xffa0a0ffU;
        uint32_t color_def = 0xc0c0ffffU;
        matrix->m[1][3] -= 20.0f;
        ttyd::icondrv::iconNumberDispGx(matrix, def, is_small, &color_def);
        matrix->m[0][3] -= 16.0f * (def > 9 ? 3 : 2) - 4.0f;
        ttyd::icondrv::iconNumberDispGx(matrix, atk, is_small, &color_atk);
        // Draw slash in-between ATK and DEF.
        matrix->m[0][3] += 14.0f;
        ttyd::icondrv::iconDispGxCol(matrix, 0x10, 0x1e0, color);
    } else {
        // Otherwise, just draw HP.
        ttyd::icondrv::iconNumberDispGx(matrix, number, is_small, color);
    }
}

}  // namespace partner
}  // namespace mod::tot::patch