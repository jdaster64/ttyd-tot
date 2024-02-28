#include "tot_generate_enemy.h"

#include "evt_cmd.h"
#include "mod.h"
#include "mod_debug.h"
#include "mod_state.h"
#include "tot_custom_rel.h"

#include <ttyd/battle.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_unit.h>
#include <ttyd/evt_npc.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/item_data.h>
#include <ttyd/npc_data.h>
#include <ttyd/npcdrv.h>
#include <ttyd/system.h>

#include <cstring>

namespace mod::tot {

namespace {

// Including entire namespace for convenience.
using namespace ::ttyd::battle_database_common;

using ::mod::infinite_pit::DebugManager;
using ::mod::infinite_pit::StateManager_v2;

using ::ttyd::battle_unit::BattleWorkUnit;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::npc_data::NpcAiTypeTable;
using ::ttyd::npcdrv::NpcBattleInfo;
using ::ttyd::npcdrv::NpcEntry;
using ::ttyd::npcdrv::NpcSetupInfo;
using ::ttyd::npcdrv::NpcTribeDescription;

namespace BattleUnitType = ::ttyd::battle_database_common::BattleUnitType;
namespace ItemType = ::ttyd::item_data::ItemType;

// Stats for a type of enemy (e.g. Hyper Goomba).
struct EnemyTypeInfo {
    // Pointer to enemy type's basic parameters (usually in custom.rel).
    BattleUnitKind* kind;
    // Indices into npc_data tribe and AI type tables.
    int16_t         npc_tribe_idx;
    int8_t          ai_type_idx;
    // Custom order for Tattle log.
    int8_t          tattle_idx;
    // The enemy's base HP, ATK, and DEF.
    int16_t         base_hp;
    int16_t         base_atk;
    int16_t         base_def;
    // The reference point used as the enemy's "base" attack power; other
    // attacks will have the same difference in power as in the original game.
    // (e.g. a Hyper Goomba will charge by its attack power + 4).
    int16_t         atk_reference;
    // The difference between vanilla 'base' ATK and the mod's atk_reference.
    int16_t         atk_offset;
    // The enemy's level; used for loadout weighting and coin drops.
    int8_t          level;
    // Makes a type of audience more likely to spawn (-1 = none).
    int8_t          audience_type_boosted;
    // The index of the enemy's HP and FP drop tables. (0-4 = 2-6 HP/FP, at max)
    int8_t          hp_drop_table_idx;
    int8_t          fp_drop_table_idx;
    // The actor's default Y position, in and out of battle.
    uint8_t         battle_y_pos;
    uint8_t         field_y_pos;
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

const EnemyTypeInfo kEnemyInfo[] = {
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { &custom::unit_Goomba, 214, 0x04, 1, 10, 6, 0, 1, 0, 2, 10, 0, 0, 0, 0 },						
    { &custom::unit_Paragoomba, 216, 0x06, 2, 10, 6, 0, 1, 0, 3, 10, 0, 0, 40, 50 },						
    { &custom::unit_SpikyGoomba, 215, 0x04, 3, 10, 6, 0, 1, 1, 3, 10, 0, 0, 0, 0 },						
    { &custom::unit_Spinia, 310, 0x28, 43, 13, 6, 0, 1, 0, 2, -1, 0, 0, 0, 0 },						
    { &custom::unit_Spania, 309, 0x28, 44, 13, 6, 0, 1, 0, 3, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { &custom::unit_Koopatrol, 205, 0x2d, 18, 15, 8, 3, 4, 0, 6, 8, 3, 0, 0, 0 },						
    { &custom::unit_Magikoopa, 313, 0x2a, 35, 15, 7, 0, 4, 0, 7, 3, 0, 3, 0, 0 },						
    { nullptr, -1, -1, -1, 15, 7, 0, 4, 0, 7, 3, 0, 3, 0, 0 },						
    { &custom::unit_KoopaTroopa, 242, 0x07, 10, 14, 7, 2, 2, 0, 3, 8, 0, 0, 0, 0 },						
    { &custom::unit_Paratroopa, 243, 0x08, 11, 14, 7, 2, 2, 0, 4, 8, 0, 0, 40, 50 },						
    { &custom::unit_Fuzzy, 248, 0x10, 46, 11, 5, 0, 1, 0, 2, -1, 0, 0, 0, 0 },						
    { &custom::unit_DullBones, 39, 0x0e, 20, 7, 5, 1, 1, 1, 2, 4, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { &custom::unit_Bristle, 258, 0x17, 73, 6, 6, 4, 1, 0, 4, -1, 0, 1, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { &custom::unit_RedBones, 36, 0x0e, 21, 10, 7, 2, 3, 0, 5, 4, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { &custom::unit_DarkPuff, 286, 0x1d, 61, 12, 7, 0, 2, 0, 3, -1, 0, 0, 40, 10 },						
    { &custom::unit_PalePiranha, 261, 0x1c, 51, 12, 7, 0, 2, 0, 4, 11, 0, 1, 0, 0 },						
    { &custom::unit_Cleft, 237, 0x16, 70, 8, 6, 5, 2, 0, 2, -1, 1, 0, 0, 0 },						
    { &custom::unit_Pider, 266, 0x1b, 56, 14, 6, 0, 2, 0, 5, -1, 0, 0, 40, 140 },						
    { &custom::unit_XNaut, 271, 0x04, 83, 12, 7, 0, 3, 0, 4, 1, 0, 0, 0, 0 },						
    { &custom::unit_Yux, 268, 0x19, 86, 7, 5, 0, 2, 0, 6, 1, 0, 0, 40, 30 },						
    { &custom::unit_MiniYux, -1, -1, 87, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, 10, 6, 0, 1, 0, 2, 10, 0, 0, 0, 0 },						
    { &custom::unit_KpKoopa, 246, 0x07, 12, 16, 7, 2, 2, 0, 4, 8, 0, 0, 0, 0 },						
    { &custom::unit_KpParatroopa, 247, 0x08, 13, 16, 7, 2, 2, 0, 5, 8, 0, 0, 40, 50 },						
    { &custom::unit_Pokey, 233, 0x14, 49, 12, 7, 0, 3, 0, 4, -1, 1, 0, 0, 0 },						
    { &custom::unit_Lakitu, 280, 0x24, 27, 13, 7, 0, 2, 0, 5, -1, 0, 1, 40, 20 },						
    { &custom::unit_Spiny, 287, -1, 29, 8, 7, 4, 2, 1, 1, -1, 0, 0, 0, 0 },						
    { nullptr, 288, 0x15, -1, 10, 6, 5, 3, 0, 5, -1, 1, 0, 0, 0 },						
    { &custom::unit_BobOmb, 283, 0x04, 75, 10, 7, 2, 2, 0, 5, 9, 1, 0, 0, 0 },						
    { &custom::unit_Bandit, 274, 0x04, 40, 12, 6, 0, 2, 0, 4, 5, 0, 0, 0, 0 },						
    { &custom::unit_BigBandit, 129, 0x04, 41, 15, 6, 0, 2, 1, 5, 5, 0, 0, 0, 0 },						
    { nullptr, 230, 0x0b, -1, 8, 6, 5, 3, 0, 6, 7, 1, 0, 0, 0 },						
    { &custom::unit_ShadyKoopa, 282, 0x07, 14, 18, 7, 2, 3, 0, 6, 8, 0, 0, 0, 0 },						
    { &custom::unit_ShadyParatroopa, 291, 0x08, 15, 18, 7, 2, 3, 0, 7, 8, 0, 0, 40, 50 },						
    { &custom::unit_RedMagikoopa, 314, -1, 36, 15, 7, 0, 3, 1, 8, 3, 0, 3, 0, 0 },						
    { nullptr, -1, -1, -1, 15, 7, 0, 3, 1, 8, 3, 0, 3, 0, 0 },						
    { &custom::unit_WhiteMagikoopa, 315, -1, 37, 18, 7, 0, 4, 0, 8, 3, 0, 3, 0, 0 },						
    { nullptr, -1, -1, -1, 18, 7, 0, 4, 0, 8, 3, 0, 3, 0, 0 },						
    { &custom::unit_GreenMagikoopa, 316, -1, 38, 15, 7, 1, 4, 0, 8, 3, 0, 3, 0, 0 },						
    { nullptr, -1, -1, -1, 15, 7, 1, 4, 0, 8, 3, 0, 3, 0, 0 },						
    { &custom::unit_DarkCraw, 308, 0x04, 39, 20, 9, 0, 6, 0, 8, -1, 3, 0, 0, 0 },						
    { &custom::unit_HammerBro, 206, 0x2b, 24, 16, 6, 2, 3, 1, 9, 3, 2, 1, 0, 0 },						
    { &custom::unit_BoomerangBro, 294, 0x04, 25, 16, 4, 2, 2, 0, 9, 3, 2, 1, 0, 0 },						
    { &custom::unit_FireBro, 293, 0x04, 26, 16, 4, 2, 1, 2, 9, 3, 2, 1, 0, 0 },						
    { &custom::unit_RedChomp, 306, 0x2c, 79, 12, 10, 5, 5, 0, 8, -1, 2, 0, 0, 0 },						
    { &custom::unit_DarkKoopatrol, 307, 0x2d, 19, 25, 10, 3, 5, 0, 10, 8, 3, 1, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { &custom::unit_HyperGoomba, 217, 0x04, 4, 15, 6, 0, 3, -1, 5, 10, 0, 0, 0, 0 },						
    { &custom::unit_HyperParagoomba, 219, 0x06, 5, 15, 6, 0, 3, -1, 6, 10, 0, 0, 40, 50 },						
    { &custom::unit_HyperSpikyGoomba, 218, 0x04, 6, 15, 6, 0, 3, 0, 6, 10, 0, 0, 0, 0 },						
    { &custom::unit_CrazeeDayzee, 252, 0x22, 55, 14, 5, 0, 2, 0, 6, 6, 0, 2, 0, 0 },						
    { &custom::unit_AmazyDayzee, 253, -1, 92, 20, 20, 1, 20, 0, 80, 6, 2, 4, 0, 0 },						
    { &custom::unit_HyperCleft, 236, 0x16, 71, 10, 6, 5, 3, 0, 6, -1, 1, 0, 0, 0 },						
    { &custom::unit_BuzzyBeetle, 225, 0x09, 31, 8, 6, 5, 3, 0, 4, 7, 1, 0, 0, 0 },						
    { &custom::unit_SpikeTop, 226, 0x0b, 32, 8, 6, 5, 3, 0, 6, 7, 1, 0, 0, 0 },						
    { &custom::unit_Swooper, 239, 0x20, 58, 14, 7, 0, 3, 0, 5, -1, 0, 0, 130, 80 },						
    { &custom::unit_Boo, 146, 0x21, 65, 13, 6, 0, 2, 1, 5, 2, 0, 1, 0, 30 },						
    { nullptr, 148, 0x21, 93, 100, 4, 0, 2, 2, 60, 2, 2, 2, 20, 30 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { &custom::unit_Ember, 159, 0x24, 68, 13, 6, 0, 3, 0, 6, 2, 0, 1, 40, 20 },						
    { &custom::unit_LavaBubble, 302, 0x24, 67, 10, 6, 0, 3, 1, 6, 2, 0, 1, 40, 20 },						
    { &custom::unit_GreenFuzzy, 249, 0x10, 47, 13, 6, 0, 2, 1, 4, -1, 0, 0, 0, 0 },						
    { &custom::unit_FlowerFuzzy, 250, 0x10, 48, 13, 6, 0, 2, 1, 6, -1, 0, 2, 0, 0 },						
    { &custom::unit_PutridPiranha, 262, 0x1c, 52, 14, 6, 0, 2, 1, 6, 11, 0, 2, 0, 0 },						
    { &custom::unit_Parabuzzy, 228, 0x0d, 33, 8, 6, 5, 3, 0, 5, 7, 1, 0, 40, 50 },						
    { nullptr, 254, 0x12, -1, 10, 0, 3, 0, 0, 6, 9, 1, 0, 0, 0 },						
    { nullptr, 255, -1, -1, 4, 7, 1, 4, 0, 0, 9, 0, 0, 40, 0 },						
    { &custom::unit_BulkyBobOmb, 304, 0x25, 76, 12, 4, 2, 2, 0, 5, 9, 1, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { &custom::unit_RuffPuff, 284, 0x1d, 62, 14, 8, 0, 4, 0, 4, -1, 0, 0, 40, 10 },						
    { &custom::unit_PoisonPokey, 234, 0x14, 50, 15, 7, 0, 3, 1, 6, -1, 1, 0, 0, 0 },						
    { &custom::unit_SpikyParabuzzy, 227, 0x0d, 34, 8, 6, 5, 3, 0, 7, 7, 2, 0, 40, 50 },						
    { &custom::unit_DarkBoo, 147, 0x21, 66, 17, 8, 0, 4, 1, 7, 2, 0, 1, 0, 30 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { &custom::unit_IcePuff, 285, 0x1d, 63, 16, 8, 0, 4, 0, 6, -1, 0, 0, 40, 10 },						
    { &custom::unit_FrostPiranha, 263, 0x1c, 53, 16, 7, 0, 4, 1, 7, 11, 0, 2, 0, 0 },						
    { &custom::unit_MoonCleft, 235, 0x16, 72, 12, 8, 5, 5, 0, 6, -1, 1, 0, 0, 0 },						
    { &custom::unit_ZYux, 269, 0x19, 88, 9, 6, 0, 4, 0, 8, 1, 1, 1, 40, 30 },						
    { &custom::unit_MiniZYux, -1, -1, 89, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },						
    { &custom::unit_XYux, 270, 0x19, 90, 11, 5, 2, 3, 0, 10, 1, 2, 2, 40, 30 },						
    { &custom::unit_MiniXYux, -1, -1, 91, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },						
    { &custom::unit_XNautPhD, 273, 0x27, 84, 14, 8, 0, 4, 0, 8, 1, 0, 2, 0, 0 },						
    { &custom::unit_EliteXNaut, 272, 0x04, 85, 16, 9, 2, 5, 0, 8, 1, 2, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { &custom::unit_Swoopula, 240, 0x20, 59, 14, 6, 0, 4, 0, 5, -1, 0, 0, 130, 80 },						
    { &custom::unit_PhantomEmber, 303, 0x24, 69, 16, 6, 0, 3, 2, 8, 2, 0, 2, 40, 20 },						
    { nullptr, 257, -1, -1, 6, 9, 2, 6, 0, 0, 9, 0, 0, 0, 0 },						
    { nullptr, 256, 0x12, -1, 15, 0, 5, 0, 0, 10, 9, 2, 0, 40, 0 },						
    { &custom::unit_ChainChomp, 301, 0x2c, 78, 10, 8, 4, 6, 0, 6, -1, 3, 0, 0, 0 },						
    { &custom::unit_DarkWizzerd, 296, 0x26, 81, 12, 8, 4, 5, 0, 8, -1, 2, 2, 0, 20 },						
    { nullptr, -1, -1, -1, 12, 8, 4, 5, 0, 8, -1, 2, 2, 0, 20 },						
    { &custom::unit_DryBones, 196, 0x0f, 22, 12, 7, 3, 5, 0, 7, 4, 0, 2, 0, 0 },						
    { &custom::unit_DarkBones, 197, 0x0f, 23, 20, 7, 3, 4, 1, 10, 4, 1, 2, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { nullptr, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },						
    { &custom::unit_Gloomba, 220, 0x04, 7, 20, 6, 0, 2, 1, 5, 10, 0, 0, 0, 0 },						
    { &custom::unit_Paragloomba, 222, 0x06, 8, 20, 6, 0, 2, 1, 6, 10, 0, 0, 40, 50 },						
    { &custom::unit_SpikyGloomba, 221, 0x04, 9, 20, 6, 0, 2, 2, 6, 10, 0, 0, 0, 0 },						
    { &custom::unit_DarkKoopa, 244, 0x07, 16, 20, 8, 3, 3, 1, 6, 8, 0, 0, 0, 0 },						
    { &custom::unit_DarkParatroopa, 245, 0x08, 17, 20, 8, 3, 3, 1, 7, 8, 0, 0, 40, 50 },						
    { &custom::unit_BadgeBandit, 275, 0x04, 42, 18, 6, 0, 3, 2, 6, 5, 0, 0, 0, 0 },						
    { &custom::unit_DarkLakitu, 281, 0x24, 28, 19, 9, 0, 5, 0, 8, -1, 2, 0, 40, 20 },						
    { &custom::unit_SkyBlueSpiny, -1, -1, 30, 10, 9, 4, 5, 1, 1, -1, 0, 0, 0, 0 },						
    { &custom::unit_Wizzerd, 295, 0x26, 80, 10, 8, 3, 7, -1, 7, -1, 1, 1, 0, 20 },						
    { &custom::unit_PiranhaPlant, 260, 0x1c, 54, 18, 8, 0, 7, 2, 9, 11, 0, 4, 0, 0 },						
    { &custom::unit_Spunia, 311, 0x28, 45, 16, 7, 2, 6, 1, 6, -1, 3, 0, 0, 0 },						
    { &custom::unit_Arantula, 267, 0x1b, 57, 18, 6, 0, 5, 2, 8, -1, 2, 2, 40, 140 },						
    { &custom::unit_DarkBristle, 259, 0x17, 74, 9, 9, 4, 8, 0, 8, -1, 0, 3, 0, 0 },						
    { &custom::unit_PoisonPuff, 265, 0x1d, 64, 18, 8, 0, 8, 0, 8, -1, 0, 0, 40, 10 },						
    { &custom::unit_Swampire, 241, 0x20, 60, 20, 8, 0, 6, 0, 8, -1, 0, 0, 130, 80 },						
    { &custom::unit_BobUlk, 305, 0x25, 77, 15, 5, 2, 4, 0, 7, 9, 3, 0, 0, 0 },						
    { &custom::unit_EliteWizzerd, 297, 0x26, 82, 14, 8, 5, 7, 1, 10, -1, 3, 3, 0, 20 },						
    { nullptr, -1, -1, -1, 14, 8, 5, 7, 1, 10, -1, 3, 3, 0, 20 },						
    { nullptr, 325, 0x11, 94, 200, 8, 2, 8, 0, 100, -1, 0, 0, 0, 0 },						
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

// TODO: Update for TOT.
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

// Structures for holding constructed battle information.
int32_t g_NumEnemies = 0;
int32_t g_Enemies[5] = { -1, -1, -1, -1, -1 };
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
/*
void SelectPresetLoadout(StateManager_v2& state) {
    int32_t sum_weights = 0;
    for (const auto& preset : kPresetLoadouts) sum_weights += preset.weight;
    
    int32_t weight = state.Rand(sum_weights, infinite_pit::RNG_ENEMY);
    const PresetLoadoutInfo* preset = kPresetLoadouts;
    for (; (weight -= preset->weight) >= 0; ++preset);
    
    int32_t start = 0;
    int32_t size = 0;
    for (const auto& enemy_type : preset->enemies) {
        if (enemy_type >= 0) ++size;
    }
    if (preset->alt_start_idx >= 0 && state.Rand(2, infinite_pit::RNG_ENEMY)) {
        // 50% chance of choosing the three-enemy subset, if possible.
        start = preset->alt_start_idx;
        size = 3;
    }
    
    // Copy the chosen preset to g_Enemies.
    for (int32_t i = 0; i < 5; ++i) {
        g_Enemies[i] = i < size ? preset->enemies[start+i] : -1;
    }
    if (preset->reversible && state.Rand(2, infinite_pit::RNG_ENEMY)) {
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
        if (static_cast<int32_t>(state.Rand(200, infinite_pit::RNG_ENEMY)) 
            < state.floor_) {
            g_Enemies[3] = g_Enemies[1];
            g_Enemies[4] = g_Enemies[0];
            size = 5;
        }
    }
    g_NumEnemies = size;
}
*/

void SelectEnemies(int32_t floor) {
    // TODO: Remove this; testing for now.
    g_NumEnemies = 2;
    g_Enemies[0] = BattleUnitType::GOOMBA;
    g_Enemies[1] = BattleUnitType::GOOMBA;
    return;
    
    // TODO: Add special cases for boss floors.
    
    // Check to see if there is a debug set of enemies, and use it if so.
    if (int32_t* debug_enemies = DebugManager::GetEnemies(); debug_enemies) {
        PopulateDebugEnemyLoadout(debug_enemies);
        return;
    }
    
    /*
    
    const auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
    auto& state = mod::infinite_pit::g_Mod->state_;
    
    // If floor > 50, determine whether to use one of the preset loadouts.
    if (floor >= 50 && state.Rand(100, infinite_pit::RNG_ENEMY) < 10) {
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
            int32_t idx = state.Rand(kNumEnemyTypes, infinite_pit::RNG_ENEMY);
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
            
            int32_t weight = state.Rand(sum_weights, infinite_pit::RNG_ENEMY);
            int32_t idx = 0;
            for (; (weight -= weights[slot][idx]) >= 0; ++idx);
            
            g_Enemies[slot] = idx;
            level_sum += kEnemyInfo[idx].level_offset;
            
            // If level_sum is sufficiently high for the floor and not on the
            // fifth enemy, decide whether to add any further enemies.
            if (level_sum >= target_sum / 2 && slot < 4) {
                const int32_t end_chance = level_sum * 100 / target_sum;
                if (static_cast<int32_t>(state.Rand(100, infinite_pit::RNG_ENEMY)) 
                    < end_chance) {
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
        if (floor >= 80 && state.Rand(100, infinite_pit::RNG_ENEMY) < 5) {
            int32_t idx = 1;
            for (; idx < 5; ++idx) {
                if (g_Enemies[idx] == -1) break;
            }
            if (idx > 1) {
                g_Enemies[state.Rand(idx - 1, infinite_pit::RNG_ENEMY) + 1] =
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
    */
}

void BuildBattle(
    BattleSetupData* battle, NpcSetupInfo* npc_setup_info, int32_t floor,
    NpcTribeDescription** out_npc_tribe_description, int32_t* out_lead_type) {

    const EnemyTypeInfo* enemy_info[5];
    
    for (int32_t i = 0; i < g_NumEnemies; ++i) {
        enemy_info[i] = kEnemyInfo + g_Enemies[i];        
    }
    
    // Construct the data for the NPC on the field from the lead enemy's info.
    NpcTribeDescription* npc_tribe =
        ttyd::npc_data::npcTribe + enemy_info[0]->npc_tribe_idx;
    NpcAiTypeTable* npc_ai =
        ttyd::npc_data::npc_ai_type_table + enemy_info[0]->ai_type_idx;

    NpcSetupInfo& npc = npc_setup_info[0];
    memset(&npc, 0, sizeof(NpcSetupInfo));
    npc.nameJp            = "\x93\x47";  // enemy
    npc.flags             = 0x1000000a;
    npc.reactionFlags     = 0;
    npc.initEvtCode       = npc_ai->initEvtCode;
    npc.regularEvtCode    = npc_ai->moveEvtCode;
    npc.talkEvtCode       = nullptr;
    npc.deadEvtCode       = npc_ai->deadEvtCode;
    npc.findEvtCode       = npc_ai->findEvtCode;
    npc.lostEvtCode       = npc_ai->lostEvtCode;
    npc.returnEvtCode     = npc_ai->returnEvtCode;
    npc.blowEvtCode       = npc_ai->blowEvtCode;
    npc.territoryType     = ttyd::npcdrv::NpcTerritoryType::kSquare;
    npc.territoryBase     = { 0.0f, 0.0f, 0.0f };
    npc.territoryLoiter   = { 150.0f, 100.0f, 100.0f };
    npc.searchRange       = 200.0f;
    npc.searchAngle       = 300.0f;
    npc.homingRange       = 1000.0f;
    npc.homingAngle       = 360.0f;
    npc.battleInfoId      = 1;
    
    // Set output variables.
    *out_npc_tribe_description = npc_tribe;
    *out_lead_type = g_Enemies[0];
    
    // Construct the BattleGroupSetup from previously selected enemies.
    
    // TODO: Return early for boss fights, if handling them elsewhere?
    
    auto& state = mod::infinite_pit::g_Mod->state_;
    
    for (int32_t i = 0; i < 12; ++i) g_CustomAudienceWeights[i] = 2;
    // Make Toads slightly likelier since they're never boosted.
    g_CustomAudienceWeights[0] = 3;
    for (int32_t i = 0; i < g_NumEnemies; ++i) {
        BattleUnitSetup& unit = g_CustomUnits[i];
        
        unit.unit_kind_params = enemy_info[i]->kind;
        unit.alliance = 1;
        unit.attack_phase = 0x400'0004;
        unit.addl_target_offset_x = 0;
        memset(unit.unit_work, 0, sizeof(unit.unit_work));
        unit.item_drop_table = nullptr;
        
        // Position the enemies in standard spacing.
        float offset = i - (g_NumEnemies - 1) * 0.5f;
        unit.position.x = kEnemyPartyCenterX + offset * kEnemyPartySepX;
        unit.position.z = offset * kEnemyPartySepZ;
        // Height depends on enemy.
        unit.position.y = enemy_info[i]->battle_y_pos;
        
        switch (g_Enemies[i]) {
            case BattleUnitType::SWOOPER:
            case BattleUnitType::SWOOPULA:
            case BattleUnitType::SWAMPIRE: {
                // Swoopers should always be the flying variant.
                unit.unit_work[0] = 1;
                break;
            }
            case BattleUnitType::MAGIKOOPA:
            case BattleUnitType::RED_MAGIKOOPA:
            case BattleUnitType::WHITE_MAGIKOOPA:
            case BattleUnitType::GREEN_MAGIKOOPA: {
                // Magikoopas in the back can randomly be flying or grounded.
                unit.unit_work[0] = state.Rand(
                    i > 0 ? 2 : 1, infinite_pit::RNG_ENEMY);
                break;
            }
        }
        
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
    g_CustomBattleParty.held_item_weight = state.Rand(
        g_NumEnemies, infinite_pit::RNG_ENEMY);
    
    // Point the passed-in battle setup's loadouts to the constructed party.
    battle->flag_off_loadouts[0].group_data = &g_CustomBattleParty;
    
    // Set the battle's audience weights based on the custom ones set above.
    for (int32_t i = 0; i < 12; ++i) {
        auto& weights = battle->audience_type_weights[i];
        weights.min_weight = g_CustomAudienceWeights[i];
        weights.max_weight = g_CustomAudienceWeights[i];
    }
    
    // TODO: Change music for mid-bosses, Atomic Boo fights?
    // if (floor % 100 == 48) {
    //     battle_setup->music_name = "BGM_CHUBOSS_BATTLE1";
    // }
}

}  // namespace

EVT_DEFINE_USER_FUNC(evtTot_GetEnemyNpcInfo) {
    auto* battle_db = (BattleSetupData*)evtGetValue(evt, evt->evtArguments[0]);
    auto* npc_setup = (NpcSetupInfo*)evtGetValue(evt, evt->evtArguments[1]);
    ttyd::npcdrv::NpcTribeDescription* npc_tribe_description;
    int32_t lead_enemy_type;
    
    // TODO: Incorporate floor number into generation.
    int32_t floor = 0;
    SelectEnemies(floor);
    BuildBattle(
        battle_db, npc_setup, floor, &npc_tribe_description, &lead_enemy_type);
    
    int32_t x_pos = ttyd::system::irand(50) + 80;
    x_pos *= (ttyd::system::irand(2) ? 1 : -1);
    int32_t z_pos = ttyd::system::irand(200) - 100;
    int32_t y_pos = kEnemyInfo[lead_enemy_type].field_y_pos;
    
    if (lead_enemy_type == BattleUnitType::CHAIN_CHOMP ||
        lead_enemy_type == BattleUnitType::RED_CHOMP) {
        npc_setup->territoryBase = { (float)x_pos, (float)y_pos, (float)z_pos };
    }
    
    evtSetValue(evt, evt->evtArguments[2], PTR(npc_tribe_description->modelName));
    evtSetValue(evt, evt->evtArguments[3], PTR(npc_tribe_description->nameJp));
    evtSetValue(evt, evt->evtArguments[4], x_pos);
    evtSetValue(evt, evt->evtArguments[5], y_pos);
    evtSetValue(evt, evt->evtArguments[6], z_pos);
    
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_SetEnemyNpcBattleInfo) {
    const char* name = (const char*)evtGetValue(evt, evt->evtArguments[0]);
    int32_t battle_id = evtGetValue(evt, evt->evtArguments[1]);
    NpcEntry* npc = ttyd::evt_npc::evtNpcNameToPtr(evt, name);
    ttyd::npcdrv::npcSetBattleInfo(npc, battle_id);
    
    // TODO: Set enemy held items, battle conditions.
    
    return 2;
}

}  // namespace mod::tot