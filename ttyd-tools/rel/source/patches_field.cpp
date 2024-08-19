#include "patches_field.h"

#include "evt_cmd.h"
#include "mod.h"
#include "patch.h"
#include "tot_manager_cosmetics.h"
#include "tot_state.h"

#include <gc/types.h>
#include <ttyd/battle_monosiri.h>
#include <ttyd/evt_npc.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/npc_data.h>
#include <ttyd/npc_event.h>
#include <ttyd/npcdrv.h>
#include <ttyd/pmario_sound.h>
#include <ttyd/system.h>

#include <cstring>

// Assembly patch functions.
extern "C" {
    // attack_fx_patches.s
    void StartPlayFieldHammerFX();
    void BranchBackPlayFieldHammerFX();
    
    void playFieldHammerFX(gc::vec3* position) {
        int32_t id = mod::tot::CosmeticsManager::PickActiveFX();
        if (id) {
            const char* fx_name = mod::tot::CosmeticsManager::GetFXName(id);
            int32_t id = ttyd::pmario_sound::psndSFXOn_3D(fx_name, position);
            // Play at one of a few random pitches.
            int16_t pitch = 0x400 * (ttyd::system::irand(3) - 1);
            ttyd::pmario_sound::psndSFX_pit(id, pitch);
        } else {
            // Play standard hammer impact sound.
            ttyd::pmario_sound::psndSFXOn_3D("SFX_MARIO_HAMMER_WOOD_DON1", position);
        }
    }
}

namespace mod::infinite_pit {

// Declarations of patches.
extern const int32_t g_mot_hammer_PickHammerFieldSfx_BH;
extern const int32_t g_mot_hammer_PickHammerFieldSfx_EH;
extern const int32_t g_mot_damage_Patch_DisableFallDamage;
extern const int32_t g_chorobon_move_event_Patch_SetScale;
extern const int32_t g_chorobon_find_event_Patch_SetScale;
extern const int32_t g_chorobon_lost_event_Patch_SetScale;
extern const int32_t g_chorobon_return_event_Patch_SetScale;
extern const int32_t g_zakowiz_find_event_Patch_ProjectileScaleHook;

namespace field {

namespace {

// For convenience.
using namespace ::ttyd::evt_npc;

using ::ttyd::npcdrv::NpcTribeDescription;

namespace BattleUnitType = ::ttyd::battle_database_common::BattleUnitType;
namespace NpcAiType = ::ttyd::npc_data::NpcAiType;

}

EVT_BEGIN(evtTot_HammerBroInitWrapper)
    RUN_CHILD_EVT(ttyd::npc_event::hbross_init_event)
    USER_FUNC(tot::evtTot_IsMidbossFloor, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_npc_set_scale, PTR("slave_0"), 2, 2, 2)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(evtTot_DryBonesInitWrapper)
    RUN_CHILD_EVT(ttyd::npc_event::karon_init_event)
    USER_FUNC(tot::evtTot_IsMidbossFloor, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_npc_set_scale, PTR("slave_0"), 2, 2, 2)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(evtTot_WizzerdInitWrapper)
    RUN_CHILD_EVT(ttyd::npc_event::mahoon_init_event)
    USER_FUNC(tot::evtTot_IsMidbossFloor, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_npc_set_scale, PTR("slave_0"), 2, 2, 2)
        USER_FUNC(evt_npc_set_scale, PTR("slave_1"), 2, 2, 2)
        USER_FUNC(evt_npc_set_scale, PTR("slave_2"), 2, 2, 2)
        USER_FUNC(evt_npc_set_scale, PTR("slave_3"), 2, 2, 2)
    END_IF()
    RETURN()
EVT_END()

// Runs passively when Wizzerd on field is in most movement states.
EVT_BEGIN(evtTot_WizzerdHandsIdle)
    USER_FUNC(evt_npc_get_dir, PTR("me"), LW(12))

    // Scale to 1x if non-midboss, or 2x if midboss.
    USER_FUNC(tot::evtTot_IsMidbossFloor, LW(13))
    ADD(LW(13), 1)

    SET(LW(14), 0)
    DO(4)
        SWITCH(LW(14))
            CASE_EQUAL(0)
                SET(LW(5), PTR("slave_0"))
                SET(LW(6), 25)
                SET(LW(7), 5)
            CASE_EQUAL(1)
                SET(LW(5), PTR("slave_1"))
                SET(LW(6), -25)
                SET(LW(7), 5)
            CASE_EQUAL(2)
                SET(LW(5), PTR("slave_2"))
                SET(LW(6), 25)
                SET(LW(7), 30)
            CASE_EQUAL(3)
                SET(LW(5), PTR("slave_3"))
                SET(LW(6), -25)
                SET(LW(7), 30)
        END_SWITCH()
        MUL(LW(6), LW(13))
        MUL(LW(7), LW(13))

        USER_FUNC(evt_npc_set_ry, LW(5), LW(12))
        USER_FUNC(evt_npc_get_position, PTR("me"), LW(0), LW(1), LW(2))
        USER_FUNC(evt_npc_add_dirdist, LW(0), LW(2), LW(12), LW(6))
        ADD(LW(1), LW(7))
        USER_FUNC(evt_npc_set_position, LW(5), LW(0), LW(1), LW(2))

        ADD(LW(14), 1)
    WHILE()

    // Pad out to length of original event section.
    0, 0, 0, 0, 0, 0, 0,

    // No return; patched over part of existing events.
EVT_PATCH_END()
static_assert(sizeof(evtTot_WizzerdHandsIdle) == 0x1a0);

EVT_BEGIN(evtTot_XNautPhdProjectilePosition)
    USER_FUNC(tot::evtTot_IsMidbossFloor, LW(4))
    ADD(LW(4), 1)
    USER_FUNC(evt_npc_set_scale, PTR("slave_0"), LW(4), LW(4), LW(4))
    MUL(LW(4), 25)
    USER_FUNC(evt_npc_add_dirdist, LW(0), LW(2), LW(3), LW(4))
    ADD(LW(1), LW(4))
    USER_FUNC(evt_npc_set_position, PTR("slave_0"), LW(0), LW(1), LW(2))
    RETURN()
EVT_END()

EVT_BEGIN(evtTot_XNautPhdProjectilePosition_Hook)
    RUN_CHILD_EVT(evtTot_XNautPhdProjectilePosition)
    // Pad out to length of original event section.
    0, 0, 0, 0, 0, 0, 0,
    // No return; patched over part of existing event.
EVT_PATCH_END()
static_assert(sizeof(evtTot_XNautPhdProjectilePosition_Hook) == 0x24);

void ApplyFixedPatches() {
    // Replaces logic for picking a FX to play on hammering in the field.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_mot_hammer_PickHammerFieldSfx_BH),
        reinterpret_cast<void*>(g_mot_hammer_PickHammerFieldSfx_EH),
        reinterpret_cast<void*>(StartPlayFieldHammerFX),
        reinterpret_cast<void*>(BranchBackPlayFieldHammerFX));

    // Disable damage after falling into water.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_mot_damage_Patch_DisableFallDamage),
        0x38600000U /* li r3, 0 */);

    // Add support for midboss scale to Wizzerd field AI.
    ttyd::npc_data::npc_ai_type_table[NpcAiType::WIZZERD].initEvtCode = 
        const_cast<int32_t*>(evtTot_WizzerdInitWrapper);
    mod::patch::writePatch(
        reinterpret_cast<void*>(
            reinterpret_cast<uintptr_t>(ttyd::npc_event::mahoon_move_event) + 0x28),
        evtTot_WizzerdHandsIdle, sizeof(evtTot_WizzerdHandsIdle));
    mod::patch::writePatch(
        reinterpret_cast<void*>(
            reinterpret_cast<uintptr_t>(ttyd::npc_event::mahoon_find_event) + 0x14),
        evtTot_WizzerdHandsIdle, sizeof(evtTot_WizzerdHandsIdle));
    mod::patch::writePatch(
        reinterpret_cast<void*>(
            reinterpret_cast<uintptr_t>(ttyd::npc_event::mahoon_lost_event) + 0x14),
        evtTot_WizzerdHandsIdle, sizeof(evtTot_WizzerdHandsIdle));
    mod::patch::writePatch(
        reinterpret_cast<void*>(
            reinterpret_cast<uintptr_t>(ttyd::npc_event::mahoon_return_event) + 0x14),
        evtTot_WizzerdHandsIdle, sizeof(evtTot_WizzerdHandsIdle));

    // Add support for midboss scale to X-Naut PhD's projectiles.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_zakowiz_find_event_Patch_ProjectileScaleHook),
        evtTot_XNautPhdProjectilePosition_Hook,
        sizeof(evtTot_XNautPhdProjectilePosition_Hook));

    // Add support for midboss scale to Hammer Bros., Dry Bones' projectiles.
    ttyd::npc_data::npc_ai_type_table[NpcAiType::DRY_BONES].initEvtCode = 
        const_cast<int32_t*>(evtTot_DryBonesInitWrapper);
    ttyd::npc_data::npc_ai_type_table[NpcAiType::HAMMER_BRO].initEvtCode = 
        const_cast<int32_t*>(evtTot_HammerBroInitWrapper);

    // Null out set scale events for Fuzzies' field AI, since they're unused.
    memset((void*)g_chorobon_move_event_Patch_SetScale, 0, 0x18);
    memset((void*)g_chorobon_find_event_Patch_SetScale, 0, 0x18);
    memset((void*)g_chorobon_lost_event_Patch_SetScale, 0, 0x18);
    memset((void*)g_chorobon_return_event_Patch_SetScale, 0, 0x18);

    // Correcting heights in NPC tribe description data.
    NpcTribeDescription* tribe_descs = ttyd::npc_data::npcTribe;
    // Shady Paratroopa
    tribe_descs[291].height = 30;
    // Fire Bro
    tribe_descs[293].height = 40;
    // Boomerang Bro
    tribe_descs[294].height = 40;
    // Craw
    tribe_descs[298].height = 40;
    // Atomic Boo
    tribe_descs[148].height = 100;
    
    // Copying tribe description data for Bob-omb, Atomic Boo over slots for
    // Bald + Hyper Bald Clefts, so they can be used for variants.
    memcpy(&tribe_descs[238], &tribe_descs[283], sizeof(NpcTribeDescription));
    memcpy(&tribe_descs[288], &tribe_descs[148], sizeof(NpcTribeDescription));
    // Set unique names + model filenames.
    tribe_descs[238].nameJp = "hyper_bomb";
    tribe_descs[238].modelName = "c_bomhey_h";
    tribe_descs[288].nameJp = "cosmic_boo";
    tribe_descs[288].modelName = "c_atmic_trs_p";
    
    // TODO: Move to patches_ui.h?
    // Fix captures / location information in Tattle menu.
    auto* tattle_inf = ttyd::battle_monosiri::battleGetUnitMonosiriPtr(0);
    tattle_inf[BattleUnitType::TOT_COSMIC_BOO].model_name = "c_atmic_trs_p";
    tattle_inf[BattleUnitType::TOT_COSMIC_BOO].pose_name = "Z_1";
    tattle_inf[BattleUnitType::TOT_COSMIC_BOO].location_name = "menu_monosiri_shiga";
    tattle_inf[BattleUnitType::TOT_HYPER_BOB_OMB].model_name = "c_bomhey_h";
    tattle_inf[BattleUnitType::TOT_HYPER_BOB_OMB].pose_name = "BOM_Z_1";
    tattle_inf[BattleUnitType::TOT_HYPER_BOB_OMB].location_name = "menu_monosiri_shiga";
    
}

}  // namespace field
}  // namespace mod::infinite_pit