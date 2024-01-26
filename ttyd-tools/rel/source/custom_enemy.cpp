#include "custom_enemy.h"

#include "common_functions.h"
#include "common_types.h"
#include "mod.h"
#include "mod_debug.h"
#include "mod_state.h"

#include <ttyd/battle.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_monosiri.h>
#include <ttyd/battle_unit.h>
#include <ttyd/mariost.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/msgdrv.h>
#include <ttyd/npcdrv.h>
#include <ttyd/npc_data.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::infinite_pit {

// Declaration of Pit table.
extern const int32_t g_jon_enemy_100_Offset;
extern const int32_t g_jon_btlsetup_jon_tbl_Offset;

namespace {

using ::ttyd::battle_unit::BattleWorkUnit;
using ::ttyd::npc_data::NpcAiTypeTable;
using ::ttyd::npcdrv::NpcSetupInfo;
using ::ttyd::npcdrv::NpcTribeDescription;
using namespace ::ttyd::battle_database_common;  // for convenience

// Stats for a particular kind of enemy (e.g. Hyper Goomba).
struct EnemyTypeInfo {
    // The module + offset of the reference BattleUnitSetup for custom loadouts.
    int32_t         battle_unit_setup_offset;
    ModuleId::e     module;
    // Indices into npc_data tribe and AI type tables.
    int16_t         npc_tribe_idx;
    int8_t          ai_type_idx;
    // Custom order for Tattle log.
    int8_t          tattle_idx;
    // How quickly the enemy's HP, ATK and DEF scale by the floor (base stats).
    int16_t         hp_scale;
    int16_t         atk_scale;
    int16_t         def_scale;
    // The reference point used as the enemy's "base" attack power; other
    // attacks will have the same difference in power as in the original game.
    // (e.g. a Hyper Goomba will charge by its attack power + 4).
    int16_t         atk_base;
    // The difference between the vanilla base ATK and the mod's atk_base.
    int16_t         atk_offset;
    // The enemy's level will be this much higher than Mario's at base.
    int16_t         level_offset;
    // Makes a type of audience member more likely to spawn (-1 = none).
    int16_t         audience_type_boosted;
    // The index of the enemy's HP and FP drop tables (not in BattleUnitSetup).
    int8_t          hp_drop_table_idx;
    int8_t          fp_drop_table_idx;
};

PointDropData* kHpTables[] = {
    &battle_heart_drop_param_default, &battle_heart_drop_param_default2,
    &battle_heart_drop_param_default3, &battle_heart_drop_param_default4,
    &battle_heart_drop_param_default5
};
PointDropData* kFpTables[] = {
    &battle_flower_drop_param_default, &battle_flower_drop_param_default2,
    &battle_flower_drop_param_default3, &battle_flower_drop_param_default4,
    &battle_flower_drop_param_default5
};
const float kEnemyPartyCenterX = 90.0f;
const float kEnemyPartySepX = 40.0f;
const float kEnemyPartySepZ = 10.0f;

// Enemy info for all types of enemies in BattleUnitType order (0x0 to 0xab).
const EnemyTypeInfo kEnemyInfo[] = {
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { 0x3c8b0, ModuleId::CUSTOM, 214, 0x04, 1, 10, 6, 0, 1, 0, 2, 10, 0, 0 },
    { 0x3c8e0, ModuleId::CUSTOM, 216, 0x06, 2, 10, 6, 0, 1, 0, 3, 10, 0, 0 },
    { 0x3c910, ModuleId::CUSTOM, 215, 0x04, 3, 10, 6, 0, 1, 1, 3, 10, 0, 0 },
    { 0x15b70, ModuleId::JON, 310, 0x28, 43, 13, 6, 0, 1, 0, 2, -1, 0, 0 },
    { 0x15e10, ModuleId::JON, 309, 0x28, 44, 13, 6, 0, 1, 0, 3, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { 0x3c850, ModuleId::CUSTOM, 205, 0x2d, 18, 15, 8, 3, 4, 0, 6, 8, 3, 0 },
    { 0x3c7f0, ModuleId::CUSTOM, 313, 0x2a, 35, 15, 7, 0, 4, 0, 7, 3, 0, 3 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, 15, 7, 0, 4, 0, 7, 3, 0, 3 },
    { 0x3c880, ModuleId::CUSTOM, 242, 0x07, 10, 14, 7, 2, 2, 0, 3, 8, 0, 0 },
    { 0x3c820, ModuleId::CUSTOM, 243, 0x08, 11, 14, 7, 2, 2, 0, 4, 8, 0, 0 },
    { 0x16260, ModuleId::JON, 248, 0x10, 46, 11, 5, 0, 1, 0, 2, -1, 0, 0 },
    { 0x16050, ModuleId::JON, 39, 0x0e, 20, 7, 5, 1, 1, 1, 2, 4, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { 0x493b0, ModuleId::CUSTOM, 258, 0x17, 73, 6, 6, 4, 1, 0, 4, -1, 0, 1 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { 0x2f8a8, ModuleId::CUSTOM, 36, 0x0e, 21, 10, 7, 2, 3, 0, 5, 4, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { 0x16b60, ModuleId::JON, 286, 0x1d, 61, 12, 7, 0, 2, 0, 3, -1, 0, 0 },
    { 0x49260, ModuleId::CUSTOM, 261, 0x1c, 51, 12, 7, 0, 2, 0, 4, 11, 0, 1 },
    { 0x166e0, ModuleId::JON, 237, 0x16, 70, 8, 6, 5, 2, 0, 2, -1, 1, 0 },
    { 0x16d40, ModuleId::JON, 266, 0x1b, 56, 14, 6, 0, 2, 0, 5, -1, 0, 0 },
    { 0x1420c, ModuleId::CUSTOM, 271, 0x04, 83, 12, 7, 0, 3, 0, 4, 1, 0, 0 },
    { 0x1438c, ModuleId::CUSTOM, 268, 0x19, 86, 7, 5, 0, 2, 0, 6, 1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, 87, 1, 0, 0, 0, 0, 0, 1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, 10, 6, 0, 1, 0, 2, 10, 0, 0 },
    { 0x491d0, ModuleId::CUSTOM, 246, 0x07, 12, 16, 7, 2, 2, 0, 4, 8, 0, 0 },
    { 0x49230, ModuleId::CUSTOM, 247, 0x08, 13, 16, 7, 2, 2, 0, 5, 8, 0, 0 },
    { 0x16920, ModuleId::JON, 233, 0x14, 49, 12, 7, 0, 3, 0, 4, -1, 1, 0 },
    { 0x17490, ModuleId::JON, 280, 0x24, 27, 13, 7, 0, 2, 0, 5, -1, 0, 1 },
    { -1, ModuleId::INVALID_MODULE, 287, -1, 29, 8, 7, 4, 2, 1, 1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, 288, 0x15, -1, 10, 6, 5, 3, 0, 5, -1, 1, 0 },
    { 0x176d0, ModuleId::JON, 283, 0x04, 75, 10, 7, 2, 2, 0, 5, 9, 1, 0 },
    { 0x171f0, ModuleId::JON, 274, 0x04, 40, 12, 6, 0, 2, 0, 4, 5, 0, 0 },
    { 0x49350, ModuleId::CUSTOM, 129, 0x04, 41, 15, 6, 0, 2, 1, 5, 5, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, 230, 0x0b, -1, 8, 6, 5, 3, 0, 6, 7, 1, 0 },
    { 0x181e0, ModuleId::JON, 282, 0x07, 14, 18, 7, 2, 3, 0, 6, 8, 0, 0 },
    { 0x49440, ModuleId::CUSTOM, 291, 0x08, 15, 18, 7, 2, 3, 0, 7, 8, 0, 0 },
    { 0x494a0, ModuleId::CUSTOM, 314, -1, 36, 15, 7, 0, 3, 1, 8, 3, 0, 3 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, 15, 7, 0, 3, 1, 8, 3, 0, 3 },
    { 0x494d0, ModuleId::CUSTOM, 315, -1, 37, 18, 7, 0, 4, 0, 8, 3, 0, 3 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, 18, 7, 0, 4, 0, 8, 3, 0, 3 },
    { 0x49500, ModuleId::CUSTOM, 316, -1, 38, 15, 7, 1, 4, 0, 8, 3, 0, 3 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, 15, 7, 1, 4, 0, 8, 3, 0, 3 },
    { 0x1a430, ModuleId::JON, 308, 0x04, 39, 20, 9, 0, 6, 0, 8, -1, 3, 0 },
    { 0x49530, ModuleId::CUSTOM, 206, 0x2b, 24, 16, 6, 2, 3, 1, 9, 3, 2, 1 },
    { 0x49590, ModuleId::CUSTOM, 294, 0x04, 25, 16, 4, 2, 2, 0, 9, 3, 2, 1 },
    { 0x49560, ModuleId::CUSTOM, 293, 0x04, 26, 16, 4, 2, 1, 2, 9, 3, 2, 1 },
    { 0x197d0, ModuleId::JON, 306, 0x2c, 79, 12, 10, 5, 5, 0, 8, -1, 2, 0 },
    { 0x1a7f0, ModuleId::JON, 307, 0x2d, 19, 25, 10, 3, 5, 0, 10, 8, 3, 1 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { 0x24f20, ModuleId::CUSTOM, 217, 0x04, 4, 15, 6, 0, 3, -1, 5, 10, 0, 0 },
    { 0x24fb0, ModuleId::CUSTOM, 219, 0x06, 5, 15, 6, 0, 3, -1, 6, 10, 0, 0 },
    { 0x25040, ModuleId::CUSTOM, 218, 0x04, 6, 15, 6, 0, 3, 0, 6, 10, 0, 0 },
    { 0x250d0, ModuleId::CUSTOM, 252, 0x22, 55, 14, 5, 0, 2, 0, 6, 6, 0, 2 },
    { 0x1c7a0, ModuleId::JON, 253, -1, 92, 20, 20, 1, 20, 0, 80, 6, 2, 4 },
    { 0x17d90, ModuleId::JON, 236, 0x16, 71, 10, 6, 5, 3, 0, 6, -1, 1, 0 },
    { 0x29b08, ModuleId::CUSTOM, 225, 0x09, 31, 8, 6, 5, 3, 0, 4, 7, 1, 0 },
    { 0x29b38, ModuleId::CUSTOM, 226, 0x0b, 32, 8, 6, 5, 3, 0, 6, 7, 1, 0 },
    { 0x495c0, ModuleId::CUSTOM, 239, 0x20, 58, 14, 7, 0, 3, 0, 5, -1, 0, 0 },
    { 0x17940, ModuleId::JON, 146, 0x21, 65, 13, 6, 0, 2, 1, 5, 2, 0, 1 },
    { 0x29ad8, ModuleId::CUSTOM, 148, 0x21, 93, 100, 4, 0, 2, 2, 60, 2, 2, 2 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { 0x37698, ModuleId::CUSTOM, 159, 0x24, 68, 13, 6, 0, 3, 0, 6, 2, 0, 1 },
    { 0x18ab0, ModuleId::JON, 302, 0x24, 67, 10, 6, 0, 3, 1, 6, 2, 0, 1 },
    { 0x37728, ModuleId::CUSTOM, 249, 0x10, 47, 13, 6, 0, 2, 1, 4, -1, 0, 0 },
    { 0x18420, ModuleId::JON, 250, 0x10, 48, 13, 6, 0, 2, 1, 6, -1, 0, 2 },
    { 0x37758, ModuleId::CUSTOM, 262, 0x1c, 52, 14, 6, 0, 2, 1, 6, 11, 0, 2 },
    { 0x17fa0, ModuleId::JON, 228, 0x0d, 33, 8, 6, 5, 3, 0, 5, 7, 1, 0 },
    { -1, ModuleId::INVALID_MODULE, 254, 0x12, -1, 10, 0, 3, 0, 0, 6, 9, 1, 0 },
    { -1, ModuleId::INVALID_MODULE, 255, -1, -1, 4, 7, 1, 4, 0, 0, 9, 0, 0 },
    { 0x188a0, ModuleId::JON, 304, 0x25, 76, 12, 4, 2, 2, 0, 5, 9, 1, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { 0x23700, ModuleId::CUSTOM, 284, 0x1d, 62, 14, 8, 0, 4, 0, 4, -1, 0, 0 },
    { 0x18cf0, ModuleId::JON, 234, 0x14, 50, 15, 7, 0, 3, 1, 6, -1, 1, 0 },
    { 0x18f30, ModuleId::JON, 227, 0x0d, 34, 8, 6, 5, 3, 0, 7, 7, 2, 0 },
    { 0x19590, ModuleId::JON, 147, 0x21, 66, 17, 8, 0, 4, 1, 7, 2, 0, 1 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { 0x19380, ModuleId::JON, 285, 0x1d, 63, 16, 8, 0, 4, 0, 6, -1, 0, 0 },
    { 0x1a220, ModuleId::JON, 263, 0x1c, 53, 16, 7, 0, 4, 1, 7, 11, 0, 2 },
    { 0x199e0, ModuleId::JON, 235, 0x16, 72, 12, 8, 5, 5, 0, 6, -1, 1, 0 },
    { 0x143ec, ModuleId::CUSTOM, 269, 0x19, 88, 9, 6, 0, 4, 0, 8, 1, 1, 1 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, 89, 2, 0, 0, 0, 0, 0, 1, 0, 0 },
    { 0x1429c, ModuleId::CUSTOM, 270, 0x19, 90, 11, 5, 2, 3, 0, 10, 1, 2, 2 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, 91, 1, 0, 0, 0, 0, 0, 1, 0, 0 },
    { 0x142fc, ModuleId::CUSTOM, 273, 0x27, 84, 14, 8, 0, 4, 0, 8, 1, 0, 2 },
    { 0x141ac, ModuleId::CUSTOM, 272, 0x04, 85, 16, 9, 2, 5, 0, 8, 1, 2, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { 0x1acd0, ModuleId::JON, 240, 0x20, 59, 14, 6, 0, 4, 0, 5, -1, 0, 0 },
    { 0x1aa00, ModuleId::JON, 303, 0x24, 69, 16, 6, 0, 3, 2, 8, 2, 0, 2 },
    { -1, ModuleId::INVALID_MODULE, 257, -1, -1, 6, 9, 2, 6, 0, 0, 9, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, 256, 0x12, -1, 15, 0, 5, 0, 0, 10, 9, 2, 0 },
    { 0x1af70, ModuleId::JON, 301, 0x2c, 78, 10, 8, 4, 6, 0, 6, -1, 3, 0 },
    { 0x1a010, ModuleId::JON, 296, 0x26, 81, 12, 8, 4, 5, 0, 8, -1, 2, 2 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, 12, 8, 4, 5, 0, 8, -1, 2, 2 },
    { 0x19e00, ModuleId::JON, 196, 0x0f, 22, 12, 7, 3, 5, 0, 7, 4, 0, 2 },
    { 0x2f908, ModuleId::CUSTOM, 197, 0x0f, 23, 20, 7, 3, 4, 1, 10, 4, 1, 2 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0 },
    { 0x15a20, ModuleId::JON, 220, 0x04, 7, 20, 6, 0, 2, 1, 5, 10, 0, 0 },
    { 0x16500, ModuleId::JON, 222, 0x06, 8, 20, 6, 0, 2, 1, 6, 10, 0, 0 },
    { 0x16f80, ModuleId::JON, 221, 0x04, 9, 20, 6, 0, 2, 2, 6, 10, 0, 0 },
    { 0x17bb0, ModuleId::JON, 244, 0x07, 16, 20, 8, 3, 3, 1, 6, 8, 0, 0 },
    { 0x18660, ModuleId::JON, 245, 0x08, 17, 20, 8, 3, 3, 1, 7, 8, 0, 0 },
    { 0x19170, ModuleId::JON, 275, 0x04, 42, 18, 6, 0, 3, 2, 6, 5, 0, 0 },
    { 0x19c20, ModuleId::JON, 281, 0x24, 28, 19, 9, 0, 5, 0, 8, -1, 2, 0 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, 30, 10, 9, 4, 5, 1, 1, -1, 0, 0 },
    { 0x1a5e0, ModuleId::JON, 295, 0x26, 80, 10, 8, 3, 7, -1, 7, -1, 1, 1 },
    { 0x1b870, ModuleId::JON, 260, 0x1c, 54, 18, 8, 0, 7, 2, 9, 11, 0, 4 },
    { 0x1b1b0, ModuleId::JON, 311, 0x28, 45, 16, 7, 2, 6, 1, 6, -1, 3, 0 },
    { 0x1b630, ModuleId::JON, 267, 0x1b, 57, 18, 6, 0, 5, 2, 8, -1, 2, 2 },
    { 0x1b420, ModuleId::JON, 259, 0x17, 74, 9, 9, 4, 8, 0, 8, -1, 0, 3 },
    { 0x1bff0, ModuleId::JON, 265, 0x1d, 64, 18, 8, 0, 8, 0, 8, -1, 0, 0 },
    { 0x1c500, ModuleId::JON, 241, 0x20, 60, 20, 8, 0, 6, 0, 8, -1, 0, 0 },
    { 0x1c290, ModuleId::JON, 305, 0x25, 77, 15, 5, 2, 4, 0, 7, 9, 3, 0 },
    { 0x1bd80, ModuleId::JON, 297, 0x26, 82, 14, 8, 5, 7, 1, 10, -1, 3, 3 },
    { -1, ModuleId::INVALID_MODULE, -1, -1, -1, 14, 8, 5, 7, 1, 10, -1, 3, 3 },
    { 0x159d0, ModuleId::JON, 325, 0x11, 94, 200, 8, 2, 8, 0, 100, -1, 0, 0 },
};

struct PresetLoadoutInfo {
    // The base set of enemies.
    int16_t enemies[5];
    // For longer presets, setting this to > -1 specifies that there is an
    // alternative form of the preset consisting of a subset of three enemies.
    int8_t alt_start_idx;
    // Whether or not the loadout or its subset can be reversed / mirrored.
    int8_t reversible;
    int8_t mirrorable;
    int32_t weight;
};

const PresetLoadoutInfo kPresetLoadouts[] = {
    { { BattleUnitType::GOOMBA, 
        BattleUnitType::HYPER_GOOMBA, 
        BattleUnitType::GLOOMBA,
        -1,
        -1, }, 
        -1, true, true, 5 },
    { { BattleUnitType::SPIKY_GOOMBA, 
        BattleUnitType::HYPER_SPIKY_GOOMBA, 
        BattleUnitType::SPIKY_GLOOMBA,
        -1,
        -1, }, 
        -1, true, true, 5 },
    { { BattleUnitType::PARAGOOMBA, 
        BattleUnitType::HYPER_PARAGOOMBA, 
        BattleUnitType::PARAGLOOMBA,
        -1,
        -1, }, 
        -1, true, true, 5 },
    { { BattleUnitType::HYPER_GOOMBA, 
        BattleUnitType::HYPER_PARAGOOMBA, 
        BattleUnitType::HYPER_SPIKY_GOOMBA,
        -1,
        -1, }, 
        -1, true, true, 5 },
    { { BattleUnitType::GLOOMBA,
        BattleUnitType::PARAGLOOMBA, 
        BattleUnitType::SPIKY_GLOOMBA,
        -1,
        -1, }, 
        -1, true, true, 5 },
    { { BattleUnitType::KOOPA_TROOPA,
        BattleUnitType::KP_KOOPA,
        BattleUnitType::SHADY_KOOPA,
        BattleUnitType::DARK_KOOPA,
        -1, },
        1, true, true, 15 },
    { { BattleUnitType::PARATROOPA,
        BattleUnitType::KP_PARATROOPA,
        BattleUnitType::SHADY_PARATROOPA,
        BattleUnitType::DARK_PARATROOPA,
        -1, },
        1, true, true, 15 },
    { { BattleUnitType::BUZZY_BEETLE,
        BattleUnitType::SPIKE_TOP,
        BattleUnitType::PARABUZZY,
        BattleUnitType::SPIKY_PARABUZZY,
        -1, },
        -1, true, true, 20 },
    { { BattleUnitType::DULL_BONES,
        BattleUnitType::RED_BONES,
        BattleUnitType::DRY_BONES,
        BattleUnitType::DARK_BONES,
        -1, },
        1, true, true, 15 },
    { { BattleUnitType::HAMMER_BRO,
        BattleUnitType::BOOMERANG_BRO,
        BattleUnitType::FIRE_BRO,
        -1,
        -1, },
        -1, true, true, 15 },
    { { BattleUnitType::LAKITU,
        BattleUnitType::DARK_LAKITU,
        BattleUnitType::LAKITU,
        BattleUnitType::DARK_LAKITU,
        -1, },
        0, true, true, 15 },
    { { BattleUnitType::KOOPATROL,
        BattleUnitType::DARK_KOOPATROL,
        BattleUnitType::KOOPATROL,
        BattleUnitType::DARK_KOOPATROL,
        -1, },
        0, true, true, 15 },
    { { BattleUnitType::MAGIKOOPA,
        BattleUnitType::RED_MAGIKOOPA,
        BattleUnitType::WHITE_MAGIKOOPA,
        BattleUnitType::GREEN_MAGIKOOPA,
        -1, },
        -1, false, false, 15 },
    { { BattleUnitType::KOOPATROL,
        BattleUnitType::MAGIKOOPA,
        BattleUnitType::HAMMER_BRO,
        -1,
        -1, },
        -1, true, true, 20 },
    { { BattleUnitType::PALE_PIRANHA,
        BattleUnitType::PUTRID_PIRANHA,
        BattleUnitType::FROST_PIRANHA,
        BattleUnitType::PIRANHA_PLANT,
        -1, },
        1, true, true, 20 },
    { { BattleUnitType::FUZZY,
        BattleUnitType::GREEN_FUZZY,
        BattleUnitType::FLOWER_FUZZY,
        -1,
        -1, },
        -1, true, true, 20 },
    { { BattleUnitType::LAVA_BUBBLE,
        BattleUnitType::EMBER,
        BattleUnitType::PHANTOM_EMBER,
        -1,
        -1, },
        -1, true, true, 20 },
    { { BattleUnitType::DARK_PUFF,
        BattleUnitType::RUFF_PUFF,
        BattleUnitType::ICE_PUFF,
        BattleUnitType::POISON_PUFF,
        -1, },
        1, true, true, 20 },
    { { BattleUnitType::SWOOPER,
        BattleUnitType::SWOOPULA,
        BattleUnitType::SWAMPIRE,
        -1,
        -1, },
        -1, true, true, 20 },
    { { BattleUnitType::BANDIT,
        BattleUnitType::BIG_BANDIT,
        BattleUnitType::BADGE_BANDIT,
        -1,
        -1, },
        -1, true, true, 20 },
    { { BattleUnitType::X_NAUT,
        BattleUnitType::X_NAUT_PHD,
        BattleUnitType::ELITE_X_NAUT,
        -1,
        -1, },
        -1, true, true, 20 },
    { { BattleUnitType::Z_YUX,
        BattleUnitType::ELITE_X_NAUT,
        BattleUnitType::YUX,
        BattleUnitType::X_NAUT_PHD,
        BattleUnitType::X_YUX, },
        -1, false, false, 10 },
    { { BattleUnitType::WIZZERD,
        BattleUnitType::DARK_WIZZERD,
        BattleUnitType::ELITE_WIZZERD,
        -1,
        -1, },
        -1, true, true, 20 },
    { { BattleUnitType::CLEFT,
        BattleUnitType::HYPER_CLEFT,
        BattleUnitType::MOON_CLEFT,
        -1,
        -1, },
        -1, true, true, 20 },
    { { BattleUnitType::POKEY,
        BattleUnitType::POISON_POKEY,
        BattleUnitType::POKEY,
        BattleUnitType::POISON_POKEY,
        -1, },
        0, true, true, 20 },
    { { BattleUnitType::PIDER,
        BattleUnitType::ARANTULA,
        BattleUnitType::PIDER,
        BattleUnitType::ARANTULA,
        -1, },
        0, true, true, 20 },
    { { BattleUnitType::CRAZEE_DAYZEE,
        BattleUnitType::AMAZY_DAYZEE,
        BattleUnitType::CRAZEE_DAYZEE,
        -1,
        -1, },
        -1, true, true, 15 },
    { { BattleUnitType::CHAIN_CHOMP,
        BattleUnitType::RED_CHOMP,
        BattleUnitType::CHAIN_CHOMP,
        BattleUnitType::RED_CHOMP,
        -1, },
        0, true, true, 20 },
    { { BattleUnitType::BULKY_BOB_OMB,
        BattleUnitType::BOB_ULK,
        BattleUnitType::BULKY_BOB_OMB,
        BattleUnitType::BOB_ULK,
        -1, },
        0, true, true, 20 },
    { { BattleUnitType::BOO,
        BattleUnitType::DARK_BOO,
        BattleUnitType::BOO,
        BattleUnitType::DARK_BOO,
        -1, },
        0, true, true, 20 },
    { { BattleUnitType::SPINIA,
        BattleUnitType::SPANIA,
        BattleUnitType::SPUNIA,
        -1,
        -1, },
        -1, true, true, 25 },
    { { BattleUnitType::BRISTLE,
        BattleUnitType::DARK_BRISTLE,
        BattleUnitType::BRISTLE,
        BattleUnitType::DARK_BRISTLE,
        -1, },
        0, true, true, 20 },
};

// Base weights per floor group (00s, 10s, ...) and level_offset (2, 3, ... 10).
const int8_t kBaseWeights[11][9] = {
    { 10, 10, 5, 3, 2, 0, 0, 0, 0 },
    { 5, 10, 5, 5, 3, 0, 0, 0, 0 },
    { 3, 5, 10, 7, 5, 1, 0, 0, 0 },
    { 1, 1, 7, 10, 10, 3, 2, 0, 0 },
    { 1, 1, 5, 10, 10, 5, 3, 0, 0 },
    { 1, 1, 2, 5, 10, 10, 5, 1, 1 },
    { 0, 0, 2, 3, 10, 10, 6, 4, 2 },
    { 0, 0, 2, 3, 7, 10, 10, 5, 4 },
    { 0, 0, 1, 3, 6, 8, 10, 10, 8 },
    { 0, 0, 1, 2, 5, 7, 10, 10, 10 },
    { 0, 0, 1, 2, 5, 7, 10, 10, 10 },
};
// The target sum of enemy level_offsets for each floor group.
const int8_t kTargetLevelSums[11] = {
    12, 15, 18, 22, 25, 28, 31, 34, 37, 40, 50
};

// Global structures for holding constructed battle information.
int32_t g_NumEnemies = 0;
int32_t g_Enemies[5] = { -1, -1, -1, -1, -1 };
NpcSetupInfo g_CustomNpc[2];
BattleUnitSetup g_CustomUnits[6];
BattleGroupSetup g_CustomBattleParty;
int8_t g_CustomAudienceWeights[12];

// Skips the enemy loadout selection process, using debug enemies instead.
void PopulateDebugEnemyLoadout(int32_t* debug_enemies) {
    g_NumEnemies = 0;
    for (int32_t i = 0; i < 5; ++i) {
        g_Enemies[i] = debug_enemies[i];
        if (g_Enemies[i] < 0) break;
        ++g_NumEnemies;
    }
    for (int32_t i = g_NumEnemies; i < 5; ++i) g_Enemies[i] = -1;
    // Reset debug enemies.
    for (int32_t i = 0; i < 5; ++i) debug_enemies[i] = -1;
}

// Selects one of the "preset" loadouts of related enemies.
void SelectPresetLoadout(StateManager_v2& state) {
    int32_t sum_weights = 0;
    for (const auto& preset : kPresetLoadouts) sum_weights += preset.weight;
    
    int32_t weight = state.Rand(sum_weights, RNG_ENEMY);
    const PresetLoadoutInfo* preset = kPresetLoadouts;
    for (; (weight -= preset->weight) >= 0; ++preset);
    
    int32_t start = 0;
    int32_t size = 0;
    for (const auto& enemy_type : preset->enemies) {
        if (enemy_type >= 0) ++size;
    }
    if (preset->alt_start_idx >= 0 && state.Rand(2, RNG_ENEMY)) {
        // 50% chance of choosing the three-enemy subset, if possible.
        start = preset->alt_start_idx;
        size = 3;
    }
    
    // Copy the chosen preset to g_Enemies.
    for (int32_t i = 0; i < 5; ++i) {
        g_Enemies[i] = i < size ? preset->enemies[start+i] : -1;
    }
    if (preset->reversible && state.Rand(2, RNG_ENEMY)) {
        // 50% chance of flipping the loadout left-to-right, if possible.
        for (int32_t i = 0; i < size/2; ++i) {
            int32_t tmp = g_Enemies[i];
            g_Enemies[i] = g_Enemies[size - i - 1];
            g_Enemies[size - i - 1] = tmp;
        }
    }
    // If only 3 enemies, occasionally mirror it across the center.
    if (size == 3 && preset->mirrorable) {
        // The higher the floor number, the more likely the 5-enemy version.
        if (static_cast<int32_t>(state.Rand(200, RNG_ENEMY)) < state.floor_) {
            g_Enemies[3] = g_Enemies[1];
            g_Enemies[4] = g_Enemies[0];
            size = 5;
        }
    }
    g_NumEnemies = size;
}

}

void SelectEnemies(int32_t floor) {
    // Special cases: Floor X00 (Bonetail), X49 (Atomic Boo).
    if (floor % 100 == 99) {
        g_NumEnemies = 1;
        g_Enemies[0] = BattleUnitType::BONETAIL;
        for (int32_t i = 1; i < 4; ++i) g_Enemies[i] = -1;
        return;
    }
    if (floor % 100 == 48) {
        g_NumEnemies = 1;
        g_Enemies[0] = BattleUnitType::ATOMIC_BOO;
        for (int32_t i = 1; i < 4; ++i) g_Enemies[i] = -1;
        return;
    }
    // If a reward floor, no enemies to spawn.
    if (floor % 10 == 9) return;
    
    // Check to see if there is a debug set of enemies, and use it if so.
    if (int32_t* debug_enemies = DebugManager::GetEnemies(); debug_enemies) {
        PopulateDebugEnemyLoadout(debug_enemies);
        return;
    }
    
    const auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
    auto& state = g_Mod->state_;
    
    // If floor > 50, determine whether to use one of the preset loadouts.
    if (floor >= 50 && state.Rand(100, RNG_ENEMY) < 10) {
        SelectPresetLoadout(state);
    } else {
        // Put together an array of weights, scaled by the floor number and
        // enemy's level offset (such that harder enemies appear more later on).
        int32_t kNumEnemyTypes = BattleUnitType::BONETAIL;
        
        int16_t weights[6][kNumEnemyTypes];
        for (int32_t i = 0; i < kNumEnemyTypes; ++i) {
            int32_t base_wt = 0;
            const EnemyTypeInfo& ei = kEnemyInfo[i];
            
            // If enemy does not have a BattleUnitSetup, or is a boss, ignore.
            if (ei.battle_unit_setup_offset >= 0 &&
                ei.level_offset >= 2 && ei.level_offset <= 10) {
                int32_t floor_group = floor < 110 ? floor / 10 : 10;
                base_wt = kBaseWeights[floor_group][ei.level_offset - 2];
            }
            
            // Halve base rate for all Yux types since they're so centralizing.
            switch (i) {
                case BattleUnitType::YUX:
                case BattleUnitType::Z_YUX:
                case BattleUnitType::X_YUX: {
                    base_wt /= 2;
                    break;
                }
            }
            
            // The 6th slot is used for reference as an unchanging base weight.
            for (int32_t slot = 0; slot < 6; ++slot) weights[slot][i] = base_wt;
            // Disable selecting enemies with no overworld behavior for slot 0.
            if (ei.ai_type_idx < 0) weights[0][i] = 0;
        }
        
        // Randomly upweight a handful of enemies by 100% or 50%.
        for (int32_t i = 0; i < 6; ++i) {
            int32_t idx = state.Rand(kNumEnemyTypes, RNG_ENEMY);
            for (int32_t slot = 0; slot < 5; ++slot) {  // don't change 6th slot
                weights[slot][idx] = weights[slot][idx] * (i < 3 ? 4 : 3) / 2;
            }
        }
        
        // Pick enemies in weighted fashion, with preference towards repeats.
        int32_t level_sum = 0;
        const int32_t target_sum =
            floor < 100 ? kTargetLevelSums[floor / 10] : kTargetLevelSums[10];
        for (int32_t slot = 0; slot < 5; ++slot) {
            int32_t sum_weights = 0;
            for (int32_t i = 0; i < kNumEnemyTypes; ++i) 
                sum_weights += weights[slot][i];
            
            int32_t weight = state.Rand(sum_weights, RNG_ENEMY);
            int32_t idx = 0;
            for (; (weight -= weights[slot][idx]) >= 0; ++idx);
            
            g_Enemies[slot] = idx;
            level_sum += kEnemyInfo[idx].level_offset;
            
            // If level_sum is sufficiently high for the floor and not on the
            // fifth enemy, decide whether to add any further enemies.
            if (level_sum >= target_sum / 2 && slot < 4) {
                const int32_t end_chance = level_sum * 100 / target_sum;
                if (static_cast<int32_t>(state.Rand(100, RNG_ENEMY)) < 
                    end_chance) {
                    for (++slot; slot < 5; ++slot) g_Enemies[slot] = -1;
                    break;
                }
            }
            
            // Add large additional weight for repeat enemy in subsequent slots,
            // scaled by how likely it was to be chosen to begin with.
            for (int32_t j = slot + 1; j < 5; ++j) {
                weights[j][idx] += weights[5][idx] * 20;
            }
            // If the enemy is any Yux, set the next weight for all Yuxes to 0,
            // since they appear too crowded if placed 40 units apart.
            switch (idx) {
                case BattleUnitType::YUX:
                case BattleUnitType::Z_YUX:
                case BattleUnitType::X_YUX: {
                    weights[slot + 1][BattleUnitType::YUX] = 0;
                    weights[slot + 1][BattleUnitType::Z_YUX] = 0;
                    weights[slot + 1][BattleUnitType::X_YUX] = 0;
                    break;
                }
            }
        }
        
        // If floor > 80, rarely insert an Amazy Dayzee in the loadout.
        if (floor >= 80 && state.Rand(100, RNG_ENEMY) < 5) {
            int32_t idx = 1;
            for (; idx < 5; ++idx) {
                if (g_Enemies[idx] == -1) break;
            }
            if (idx > 1) {
                g_Enemies[state.Rand(idx - 1, RNG_ENEMY) + 1] =
                    BattleUnitType::AMAZY_DAYZEE;
            }
        }
    }
    
    // Count how many enemies are in the final party.
    for (g_NumEnemies = 0; g_NumEnemies < 5; ++g_NumEnemies) {
        if (g_Enemies[g_NumEnemies] == -1) break;
    }
    
    // Change Yuxes to corresponding X-Naut types if partner is not present.
    bool has_partner = false;
    for (int32_t i = 0; i < 8; ++i) {
        if (pouch.party_data[i].flags & 1) {
            has_partner = true;
            break;
        }
    }
    if (!has_partner) {
        for (int32_t i = 0; i < g_NumEnemies; ++i) {
            switch (g_Enemies[i]) {
                case BattleUnitType::YUX: {
                    g_Enemies[i] = BattleUnitType::X_NAUT;
                    break;
                }
                case BattleUnitType::Z_YUX: {
                    g_Enemies[i] = BattleUnitType::X_NAUT_PHD;
                    break;
                }
                case BattleUnitType::X_YUX: {
                    g_Enemies[i] = BattleUnitType::ELITE_X_NAUT;
                    break;
                }
            }
        }
    }
}

void BuildBattle(
    uintptr_t pit_module_ptr, int32_t floor,
    NpcTribeDescription** out_npc_tribe_description,
    NpcSetupInfo** out_npc_setup_info, int32_t* out_lead_type) {

    const EnemyTypeInfo* enemy_info[5];
    const BattleUnitSetup* unit_info[5];
    
    for (int32_t i = 0; i < g_NumEnemies; ++i) {
        enemy_info[i] = kEnemyInfo + g_Enemies[i];
        uintptr_t module_ptr = (enemy_info[i]->module != ModuleId::JON)
            ? reinterpret_cast<uintptr_t>(ttyd::mariost::g_MarioSt->pMapAlloc)
            : pit_module_ptr;
        unit_info[i] = reinterpret_cast<const BattleUnitSetup*>(
            module_ptr + enemy_info[i]->battle_unit_setup_offset);
    }
    
    // Construct the data for the NPC on the field from the lead enemy's info.
    NpcTribeDescription* npc_tribe =
        ttyd::npc_data::npcTribe + enemy_info[0]->npc_tribe_idx;
    NpcAiTypeTable* npc_ai =
        ttyd::npc_data::npc_ai_type_table + enemy_info[0]->ai_type_idx;
        
    memset(g_CustomNpc, 0, sizeof(g_CustomNpc));
    g_CustomNpc[0].nameJp           = "\x93\x47";  // enemy
    g_CustomNpc[0].flags            = 0x1000000a;
    g_CustomNpc[0].reactionFlags    = 0;
    g_CustomNpc[0].initEvtCode      = npc_ai->initEvtCode;
    g_CustomNpc[0].regularEvtCode   = npc_ai->moveEvtCode;
    g_CustomNpc[0].talkEvtCode      = nullptr;
    g_CustomNpc[0].deadEvtCode      = npc_ai->deadEvtCode;
    g_CustomNpc[0].findEvtCode      = npc_ai->findEvtCode;
    g_CustomNpc[0].lostEvtCode      = npc_ai->lostEvtCode;
    g_CustomNpc[0].returnEvtCode    = npc_ai->returnEvtCode;
    g_CustomNpc[0].blowEvtCode      = npc_ai->blowEvtCode;
    g_CustomNpc[0].territoryType    = ttyd::npcdrv::NpcTerritoryType::kSquare;
    g_CustomNpc[0].territoryBase    = { 0.0f, 0.0f, 0.0f };
    g_CustomNpc[0].territoryLoiter  = { 150.0f, 100.0f, 100.0f };
    g_CustomNpc[0].searchRange      = 200.0f;
    g_CustomNpc[0].searchAngle      = 300.0f;
    g_CustomNpc[0].homingRange      = 1000.0f;
    g_CustomNpc[0].homingAngle      = 360.0f;
    g_CustomNpc[0].battleInfoId     = 1;
    
    // Set output variables.
    *out_npc_tribe_description = npc_tribe;
    *out_npc_setup_info = g_CustomNpc;
    *out_lead_type = g_Enemies[0];
    
    // Construct the BattleGroupSetup from the previously selected enemies.
    
    // Return early if this is a Bonetail fight, since it needs no changes.
    if (floor % 100 == 99) return;
    
    auto& state = g_Mod->state_;
    
    for (int32_t i = 0; i < 12; ++i) g_CustomAudienceWeights[i] = 2;
    // Make Toads slightly likelier since they're never boosted.
    g_CustomAudienceWeights[0] = 3;
    for (int32_t i = 0; i < g_NumEnemies; ++i) {
        BattleUnitSetup& custom_unit = g_CustomUnits[i];
        memcpy(&custom_unit, unit_info[i], sizeof(BattleUnitSetup));
        
        switch (g_Enemies[i]) {
            case BattleUnitType::SWOOPER:
            case BattleUnitType::SWOOPULA:
            case BattleUnitType::SWAMPIRE: {
                // Swoopers should always be the flying variant.
                custom_unit.unit_work[0] = 1;
                break;
            }
            case BattleUnitType::MAGIKOOPA:
            case BattleUnitType::RED_MAGIKOOPA:
            case BattleUnitType::WHITE_MAGIKOOPA:
            case BattleUnitType::GREEN_MAGIKOOPA: {
                // Magikoopas in the back can randomly be flying or grounded.
                custom_unit.unit_work[0] = state.Rand(i > 0 ? 2 : 1, RNG_ENEMY);
                break;
            }
        }
        
        // Position the enemies in standard spacing.
        float offset = i - (g_NumEnemies - 1) * 0.5f;
        custom_unit.position.x = kEnemyPartyCenterX + offset * kEnemyPartySepX;
        custom_unit.position.z = offset * kEnemyPartySepZ;
        
        // If the enemy is related to a particular type of audience member,
        // increase that audience type's weight.
        if (enemy_info[i]->audience_type_boosted >= 0) {
            g_CustomAudienceWeights[enemy_info[i]->audience_type_boosted] += 2;
        }
    }
    g_CustomBattleParty.num_enemies         = g_NumEnemies;
    g_CustomBattleParty.enemy_data          = g_CustomUnits;
    g_CustomBattleParty.random_item_weight  = 0;
    g_CustomBattleParty.no_item_weight      = 0;
    g_CustomBattleParty.hp_drop_table =
        kHpTables[enemy_info[0]->hp_drop_table_idx];
    g_CustomBattleParty.fp_drop_table = 
        kFpTables[enemy_info[0]->fp_drop_table_idx];
    
    // Actually used as the index of the enemy whose item should be dropped.
    g_CustomBattleParty.held_item_weight = state.Rand(g_NumEnemies, RNG_ENEMY);
    
    // Make the current floor's battle point to the constructed party setup.
    int8_t* enemy_100 =
        reinterpret_cast<int8_t*>(pit_module_ptr + g_jon_enemy_100_Offset);
    BattleSetupData* pit_battle_setups =
        reinterpret_cast<BattleSetupData*>(
            pit_module_ptr + g_jon_btlsetup_jon_tbl_Offset);
    BattleSetupData* battle_setup = pit_battle_setups + enemy_100[floor % 100];
    battle_setup->flag_off_loadouts[0].group_data = &g_CustomBattleParty;
    battle_setup->flag_off_loadouts[1].weight = 0;
    battle_setup->flag_off_loadouts[1].group_data = nullptr;
    battle_setup->flag_off_loadouts[1].stage_data = nullptr;
    // If floor > 100, fix the background to always display the floor 80+ bg.
    if (floor >= 100) {
        battle_setup->flag_off_loadouts[0].stage_data =
            pit_battle_setups[50].flag_off_loadouts[0].stage_data;
    }
    // Set the battle's audience weights based on the custom ones set above.
    for (int32_t i = 0; i < 12; ++i) {
        auto& weights = battle_setup->audience_type_weights[i];
        weights.min_weight = g_CustomAudienceWeights[i];
        weights.max_weight = g_CustomAudienceWeights[i];
    }
    // If Atomic Boo floor, change background music to miniboss theme.
    if (floor % 100 == 48) {
        battle_setup->music_name = "BGM_CHUBOSS_BATTLE1";
    }
}

// Stat weights as percentages for certain Pit floors (00s, 10s, 20s, ... 90s).
// After floor 100, HP, ATK and DEF rise by 5% every 10 floors.
const int8_t kStatPercents[10] = { 20, 25, 35, 40, 50, 55, 65, 75, 90, 100 };

bool GetEnemyStats(
    int32_t unit_type, int32_t* out_hp, int32_t* out_atk, int32_t* out_def,
    int32_t* out_level, int32_t* out_coinlvl, int32_t base_attack_power) {
    // Look up the enemy type info w/matching unit_type.
    if (unit_type > BattleUnitType::BONETAIL || unit_type < 0 ||
        kEnemyInfo[unit_type].hp_scale < 0) {
        // No stats to pull from; just use the original message.
        return false;
    }
    const StateManager_v2& state = g_Mod->state_;
    const EnemyTypeInfo& ei = kEnemyInfo[unit_type];
    
    int32_t floor_group = g_Mod->state_.floor_ / 10;
    int32_t hp_atk_scale =
        state.GetOptionNumericValue(OPT_FLOOR_100_SCALING) ? 10 : 5;
        
    int32_t base_hp_pct = 
        floor_group > 9 ?
            100 + (floor_group - 9) * hp_atk_scale : kStatPercents[floor_group];
    int32_t base_atk_pct = 
        floor_group > 9 ?
            100 + (floor_group - 9) * hp_atk_scale : kStatPercents[floor_group];
    int32_t base_def_pct = 
        floor_group > 9 ?
            100 + (floor_group - 9) * 5 : kStatPercents[floor_group];
    
    int32_t boss_scale_factor = 4;
    if (unit_type == BattleUnitType::ATOMIC_BOO ||
        unit_type == BattleUnitType::BONETAIL) {
        switch (g_Mod->state_.GetOptionValue(OPT_BOSS_SCALING)) {
            case OPTVAL_BOSS_SCALING_1_25X: {
                boss_scale_factor = 5;
                break;
            }
            case OPTVAL_BOSS_SCALING_1_50X: {
                boss_scale_factor = 6;
                break;
            }
            case OPTVAL_BOSS_SCALING_2_00X: {
                boss_scale_factor = 8;
                break;
            }
        }
    }
            
    if (out_hp) {
        int32_t hp = Min(ei.hp_scale * base_hp_pct, 1000000);
        hp *= state.GetOptionValue(OPTNUM_ENEMY_HP);
        hp = hp * boss_scale_factor / 4;
        *out_hp = Clamp((hp + 5000) / 10000, 1, 9999);
    }
    if (out_atk) {
        int32_t atk = Min(ei.atk_scale * base_atk_pct, 1000000);
        atk += (base_attack_power - ei.atk_base) * 100;
        atk *= state.GetOptionValue(OPTNUM_ENEMY_ATK);
        atk = atk * boss_scale_factor / 4;
        *out_atk = Clamp((atk + 5000) / 10000, 1, 99);
    }
    if (out_def) {
        if (ei.def_scale == 0) {
            *out_def = 0;
        } else {
            // Enemies with def_scale > 0 should always have at least 1 DEF.
            int32_t def = (ei.def_scale * base_def_pct + 50) / 100;
            def = def < 1 ? 1 : def;
            *out_def = def > 99 ? 99 : def;
        }
    }
    if (out_level) {
        if (ttyd::mario_pouch::pouchGetPtr()->level >= 99) {
            *out_level = 0;
        } else if (ei.level_offset == 0) {
            // Enemies like Mini-Yuxes should never grant EXP.
            *out_level = 0;
        } else {
            // Enemies' level will always be the same relative to than Mario,
            // typically giving 3 ~ 10 EXP depending on strength and group size.
            // (The EXP gained will reduce slightly after floor 100.)
            if (ei.level_offset <= 10) {
                int32_t level_offset = 
                    ei.level_offset + (state.floor_ < 100 ? 5 : 2);
                *out_level =
                    ttyd::mario_pouch::pouchGetPtr()->level + level_offset;
            } else {
                // Bosses / special enemies get fixed bonus Star Points instead.
                *out_level = -ei.level_offset / 2;
            }
        }
    }
    if (out_coinlvl) {
        // "Coin level" = expected number of coins x 2.
        // Return level_offset for normal enemies (lv 2 ~ 10 / 1 ~ 5 coins),
        // or 20 (10 coins) for boss / special enemies.
        *out_coinlvl = ei.level_offset > 10 ? 20 : ei.level_offset;
    }
    
    return true;
}

char g_TattleTextBuf[512];

const char* GetCustomTattle() { return g_TattleTextBuf; }

const char* SetCustomTattle(
    BattleWorkUnit* unit, const char* original_tattle_msg) {
    int32_t unit_type = unit->current_kind;
    if (unit_type > BattleUnitType::BONETAIL || unit_type < 0 ||
        kEnemyInfo[unit_type].hp_scale < 0) {
        // No stats to pull from; just use the original message.
        return original_tattle_msg;
    }
    const EnemyTypeInfo& ei = kEnemyInfo[unit_type];
    // Take the first paragraph from the original tattle 
    // (ignore the first few characters in case there's a <p> there).
    const char* original_tattle = ttyd::msgdrv::msgSearch(original_tattle_msg);
    const char* p1_end_ptr = strstr(original_tattle + 4, "<p>");
    int32_t p1_len =
        p1_end_ptr ? p1_end_ptr - original_tattle : strlen(original_tattle);
    strncpy(g_TattleTextBuf, original_tattle, p1_len);
    
    // Append a paragraph with the enemy's base stats.
    char* p2_ptr = g_TattleTextBuf + p1_len;
    char atk_offset_buf[8];
    sprintf(atk_offset_buf, " (%+" PRId16 ")", ei.atk_offset);
    sprintf(p2_ptr,
        "<p>Its base stats are:\n"
        "Max HP: %" PRId16 ", ATK: %" PRId16 "%s,\n"
        "DEF: %" PRId16 ", Level: %" PRId16 ".\n<k>",
        ei.hp_scale, ei.atk_scale, ei.atk_offset ? atk_offset_buf : "",
        ei.def_scale, ei.level_offset);
    
    // Append one more paragraph with the enemy's current stats
    // (using its standard attack's power as reference for ATK).
    int32_t hp, atk, def;
    int32_t base_atk_power = ei.atk_offset + ei.atk_base;
    if(GetEnemyStats(
        unit_type, &hp, &atk, &def, nullptr, nullptr, base_atk_power)) {
        char* p3_ptr = g_TattleTextBuf + strlen(g_TattleTextBuf);
        sprintf(p3_ptr,
            "<p>Currently, its stats are:\n"
            "Max HP: %" PRId32 ", ATK: %" PRId32 ", DEF: %" PRId32 ".\n<k>",
            hp, atk, def);
    }
    
    // Return a key that looks up g_TattleTextBuf from custom_strings.
    return "custom_tattle_battle";
}

const char* SetCustomMenuTattle(const char* original_tattle_msg) {
    const auto* kTattleInfo = 
        ttyd::battle_monosiri::battleGetUnitMonosiriPtr(0);
        
    // Look for the enemy type with a matching message name.
    bool found_match = false;
    int32_t unit_type = 1;
    for (; unit_type <= BattleUnitType::BONETAIL; ++unit_type) {
        const char* tattle = kTattleInfo[unit_type].menu_tattle;
        if (tattle && !strcmp(tattle, original_tattle_msg)) {
            found_match = true;
            break;
        }
    }
    if (!found_match) return original_tattle_msg;
    
    // Print a simple base stat string to g_TattleTextBuf.
    if (unit_type > BattleUnitType::BONETAIL || unit_type < 0 ||
        kEnemyInfo[unit_type].hp_scale < 0) {
        // No stats to pull from.
        sprintf(g_TattleTextBuf, "No info known on this enemy.");
    } else {
        const EnemyTypeInfo& ei = kEnemyInfo[unit_type];
        char atk_offset_buf[8];
        sprintf(atk_offset_buf, " (%+" PRId16 ")", ei.atk_offset);
        sprintf(g_TattleTextBuf,
            "Base HP: %" PRId16 ", Base ATK: %" PRId16 "%s,\n"
            "Base DEF: %" PRId16 ", Level: %" PRId16 "",
            ei.hp_scale, ei.atk_scale, ei.atk_offset ? atk_offset_buf : "",
            ei.def_scale, ei.level_offset);
    }
    
    // Return a key that looks up g_TattleTextBuf from custom_strings.
    return "custom_tattle_menu";
}

int8_t GetCustomTattleIndex(int32_t unit_type) {
    if (unit_type > BattleUnitType::BONETAIL || unit_type < 0) return -1;
    return kEnemyInfo[unit_type].tattle_idx;
}

bool GetTattleDisplayStats(int32_t unit_type, int32_t* atk, int32_t* def) {
    if (unit_type > BattleUnitType::BONETAIL || unit_type < 0 ||
        kEnemyInfo[unit_type].hp_scale < 0) {
        return false;
    }
    const EnemyTypeInfo& ei = kEnemyInfo[unit_type];
    int32_t base_atk_power = ei.atk_offset + ei.atk_base;
    return GetEnemyStats(
        unit_type, nullptr, atk, def, nullptr, nullptr, base_atk_power);
}

bool IsEligibleLoadoutEnemy(int32_t unit_type) {
    if (unit_type > BattleUnitType::BONETAIL || unit_type < 0) return false;
    return kEnemyInfo[unit_type].battle_unit_setup_offset >= 0;
}

bool IsEligibleFrontEnemy(int32_t unit_type) {
    if (unit_type > BattleUnitType::BONETAIL || unit_type < 0) return false;
    return kEnemyInfo[unit_type].ai_type_idx >= 0 &&
        kEnemyInfo[unit_type].battle_unit_setup_offset >= 0;
}

}
