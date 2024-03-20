#include "tot_gon.h"

#include "evt_cmd.h"
#include "tot_gon_lobby.h"
#include "tot_gon_tower.h"

#include <ttyd/battle_database_common.h>
#include <ttyd/battle_weapon_power.h>
#include <ttyd/database.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/mapdata.h>

namespace mod::tot::gon {

namespace {
    
// For convenience.
using namespace ::ttyd::battle_database_common;

using ::ttyd::battle_weapon_power::weaponGetPowerDefault;

using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

}  // namespace

#define DEFAULT_WALL_WEAPON {                                       \
    .base_accuracy = 100,                                           \
    .superguards_allowed = 2,                                       \
    .damage_function = (void*)weaponGetPowerDefault,                \
    .damage_function_params = { 1, 0, 0, 0, 0, 0, 0, 0, },          \
    .target_class_flags =                                           \
        AttackTargetClass_Flags::MULTIPLE_TARGET |                  \
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |      \
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |       \
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,      \
    .target_property_flags =                                        \
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,   \
    .element = AttackElement::NORMAL,                               \
    .damage_pattern = 26,                                           \
    .weapon_ac_level = 3,                                           \
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL, \
    .bg_no_a_fall_weight = 100,                                     \
}

#define DEFAULT_WALL_WEAPON_NO_DAMAGE {                             \
    .base_accuracy = 100,                                           \
    .superguards_allowed = 2,                                       \
    .target_class_flags =                                           \
        AttackTargetClass_Flags::MULTIPLE_TARGET |                  \
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |      \
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |       \
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,      \
    .target_property_flags =                                        \
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,   \
    .element = AttackElement::NORMAL,                               \
    .damage_pattern = 26,                                           \
    .weapon_ac_level = 3,                                           \
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL, \
    .bg_no_a_fall_weight = 100,                                     \
}

BattleStageObjectData g_StageObjectData[] = {
    {
        .name = "B_0",
        .type = 2,
        .layer = 2,  // B
        .position = { 0.0f, 0.0f, 0.0f },
        .num_frames_before_falling = 0,
        .num_frames_to_fall = 60,
    },
    {
        .name = "jon_0",
        .type = 2,
        .layer = 3,  // C
        .position = { 0.0f, 0.0f, 0.0f },
        .num_frames_before_falling = 0,
        .num_frames_to_fall = 60,
    },
};

BattleStageData g_StageData[] = {
    {
        .global_stage_data_dir = "bti_01",
        .current_stage_data_dir = "stg_00_3",
        .num_props = 2,
        .props = &g_StageObjectData[0],
        .bg_a_weapon = DEFAULT_WALL_WEAPON_NO_DAMAGE,
        .bg_b_weapon = DEFAULT_WALL_WEAPON,
    },
};

// Will be filled dynamically by tot_generate_enemy.h.
BattleGroupSetup g_NormalBattlePartyData;

BattleSetupWeightedLoadout g_NormalBattleLoadouts[] = {
    {
        .weight = 10,
        .group_data = &g_NormalBattlePartyData,
        .stage_data = &g_StageData[0],
    },
    {},
};

BattleSetupData g_SetupDataTbl[] = {
    {
        .battle_name = "normal_battle",
        .different_loadouts_flag = -1,
        .flag_on_loadouts = nullptr,
        .flag_off_loadouts = g_NormalBattleLoadouts,
        .battle_setup_flags = 0,    // First Attack allowed (unavailable anyway)
        .audience_setting_mode = 0, // no special audience
    },
    {},
};

// Not sure that this does anything?
ttyd::database::DatabaseDefinition g_SetupNoTbl[] = {
    { .name = "normal_battle", .id = 0, },
    {},
};

void Prolog() {
    ttyd::mapdata::relSetEvtAddr("gon_00", GetLobbyInitEvt());
    ttyd::mapdata::relSetEvtAddr("gon_01", GetTowerInitEvt());
    ttyd::mapdata::relSetBtlAddr("gon", g_SetupDataTbl, g_SetupNoTbl);
}

EVT_DEFINE_USER_FUNC(evtTot_GetGonBattleDatabasePtr) {
    evtSetValue(evt, evt->evtArguments[0], PTR(g_SetupDataTbl));
    return 2;
}

}  // namespace mod::tot::gon