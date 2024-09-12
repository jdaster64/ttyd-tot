#include "tot_generate_enemy.h"

#include "common_functions.h"
#include "evt_cmd.h"
#include "mod.h"
#include "tot_custom_rel.h"
#include "tot_generate_condition.h"
#include "tot_generate_item.h"
#include "tot_manager_achievements.h"
#include "tot_manager_debug.h"

#include <ttyd/battle.h>
#include <ttyd/battle_camera.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_monosiri.h>
#include <ttyd/battle_sub.h>
#include <ttyd/battle_unit.h>
#include <ttyd/evt_npc.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/msgdrv.h>
#include <ttyd/npc_data.h>
#include <ttyd/npcdrv.h>
#include <ttyd/system.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::tot {

namespace {

// Including entire namespace for convenience.
using namespace ::ttyd::battle_camera;
using namespace ::ttyd::battle_database_common;
using namespace ::ttyd::battle_event_cmd;
using namespace ::ttyd::evt_sub;

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
namespace NpcAiType = ::ttyd::npc_data::NpcAiType;

// Stats for a type of enemy (e.g. Hyper Goomba).
struct EnemyTypeInfo {
    // Pointer to enemy type's basic parameters (usually in custom.rel).
    BattleUnitKind* kind;
    // Whether the enemy can be used as a midboss.
    bool            midboss_eligible;
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
    int8_t          atk_reference;
    // The difference between vanilla 'base' ATK and the mod's atk_reference.
    int8_t          atk_offset;
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
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { &custom::unit_Goomba, 1, 214, 0x04, 1, 10, 6, 0, 1, 0, 2, 10, 0, 0, 0, 0 },
    { &custom::unit_Paragoomba, 1, 216, 0x06, 2, 10, 6, 0, 1, 0, 3, 10, 0, 0, 40, 50 },
    { &custom::unit_SpikyGoomba, 1, 215, 0x04, 3, 10, 6, 0, 1, 1, 3, 10, 0, 0, 0, 0 },
    { &custom::unit_Spinia, 1, 310, 0x28, 44, 13, 6, 0, 1, 0, 2, -1, 0, 0, 0, 0 },
    { &custom::unit_Spania, 1, 309, 0x28, 45, 13, 6, 0, 1, 0, 3, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { &custom::unit_Craw, 1, 298, 0x04, 39, 14, 7, 0, 6, 0, 5, -1, 1, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { &custom::unit_Koopatrol, 1, 205, 0x2d, 18, 15, 8, 3, 4, 0, 6, 8, 3, 0, 0, 0 },
    { &custom::unit_Magikoopa, 1, 313, 0x2a, 35, 15, 7, 0, 4, 0, 7, 3, 0, 3, 0, 0 },
    { nullptr, 0, -1, -1, -1, 15, 7, 0, 4, 0, 7, 3, 0, 3, 0, 0 },
    { &custom::unit_KoopaTroopa, 1, 242, 0x07, 10, 14, 7, 2, 2, 0, 3, 8, 0, 0, 0, 0 },
    { &custom::unit_Paratroopa, 1, 243, 0x08, 11, 14, 7, 2, 2, 0, 4, 8, 0, 0, 40, 50 },
    { &custom::unit_Fuzzy, 1, 248, 0x10, 47, 11, 5, 0, 1, 0, 2, -1, 0, 0, 0, 0 },
    { &custom::unit_DullBones, 1, 39, 0x0e, 20, 7, 5, 1, 1, 1, 2, 4, 0, 0, 0, 0 },
    { &custom::unit_HyperBobOmb, 0, 238, 0x04, 78, 15, 7, 2, 2, 0, 7, 9, 1, 2, 0, 0 },
    { &custom::unit_Bristle, 1, 258, 0x17, 75, 6, 6, 4, 1, 0, 4, -1, 0, 1, 0, 0 },
    { &custom::unit_GoldFuzzy, 0, 251, 0x10, 102, 150, 7, 0, 7, 0, 50, -1, 3, 3, 0, 0 },
    { &custom::unit_FuzzyHorde, 0, -1, -1, 101, 200, 7, 0, 7, 0, 50, -1, 3, 3, 0, 0 },
    { &custom::unit_RedBones, 1, 36, 0x0e, 21, 10, 7, 2, 3, 0, 5, 4, 0, 0, 0, 0 },
    { &custom::unit_Hooktail, 0, 33, 0x11, 96, 80, 6, 1, 8, 0, 100, -1, 0, 0, 0, 0 },
    { &custom::unit_DarkPuff, 1, 286, 0x1d, 62, 12, 7, 0, 2, 0, 3, -1, 0, 0, 40, 10 },
    { &custom::unit_PalePiranha, 1, 261, 0x1c, 52, 12, 7, 0, 5, 0, 4, 11, 0, 1, 0, 0 },
    { &custom::unit_Cleft, 1, 237, 0x16, 71, 8, 6, 5, 3, 0, 2, -1, 1, 0, 0, 0 },
    { &custom::unit_Pider, 1, 266, 0x1b, 57, 14, 6, 0, 2, 0, 5, -1, 0, 0, 40, 140 },
    { &custom::unit_XNaut, 1, 271, 0x04, 86, 12, 7, 0, 3, 0, 4, 1, 0, 0, 0, 0 },
    { &custom::unit_Yux, 0, 268, 0x19, 89, 7, 5, 0, 2, 0, 6, 1, 0, 0, 40, 30 },
    { &custom::unit_MiniYux, 0, -1, -1, 90, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, 10, 6, 0, 1, 0, 2, 10, 0, 0, 0, 0 },
    { &custom::unit_KpKoopa, 1, 246, 0x07, 12, 16, 7, 2, 2, 0, 4, 8, 0, 0, 0, 0 },
    { &custom::unit_KpParatroopa, 1, 247, 0x08, 13, 16, 7, 2, 2, 0, 5, 8, 0, 0, 40, 50 },
    { &custom::unit_Pokey, 1, 233, 0x14, 50, 12, 7, 0, 3, 0, 4, -1, 1, 0, 0, 0 },
    { &custom::unit_Lakitu, 1, 280, 0x24, 27, 13, 7, 0, 2, 0, 5, -1, 0, 1, 40, 20 },
    { &custom::unit_Spiny, 0, 287, -1, 29, 8, 7, 4, 2, 1, 1, -1, 0, 0, 0, 0 },
    { &custom::unit_CosmicBoo, 0, 288, 0x21, 99, 75, 6, 0, 5, 0, 50, 2, 3, 3, 20, 30 },
    { &custom::unit_BobOmb, 0, 283, 0x04, 77, 10, 7, 2, 2, 0, 5, 9, 1, 0, 0, 0 },
    { &custom::unit_Bandit, 0, 274, 0x04, 41, 12, 6, 0, 2, 0, 4, 5, 0, 0, 0, 0 },
    { &custom::unit_BigBandit, 0, 129, 0x04, 42, 15, 6, 0, 2, 1, 5, 5, 0, 0, 0, 0 },
    { nullptr, 0, 230, 0x0b, -1, 8, 6, 5, 3, 0, 6, 7, 1, 0, 0, 0 },
    { &custom::unit_ShadyKoopa, 1, 282, 0x07, 14, 18, 7, 2, 3, 0, 6, 8, 0, 0, 0, 0 },
    { &custom::unit_ShadyParatroopa, 1, 291, 0x08, 15, 18, 7, 2, 3, 0, 7, 8, 0, 0, 40, 50 },
    { &custom::unit_RedMagikoopa, 0, 314, -1, 36, 15, 7, 0, 3, 1, 8, 3, 0, 3, 0, 0 },
    { nullptr, 0, -1, -1, -1, 15, 7, 0, 3, 1, 8, 3, 0, 3, 0, 0 },
    { &custom::unit_WhiteMagikoopa, 0, 315, -1, 37, 18, 7, 0, 4, 0, 8, 3, 0, 3, 0, 0 },
    { nullptr, 0, -1, -1, -1, 18, 7, 0, 4, 0, 8, 3, 0, 3, 0, 0 },
    { &custom::unit_GreenMagikoopa, 0, 316, -1, 38, 15, 7, 1, 4, 0, 8, 3, 0, 3, 0, 0 },
    { nullptr, 0, -1, -1, -1, 15, 7, 1, 4, 0, 8, 3, 0, 3, 0, 0 },
    { &custom::unit_DarkCraw, 1, 308, 0x04, 40, 20, 9, 0, 6, 0, 8, -1, 3, 0, 0, 0 },
    { &custom::unit_HammerBro, 1, 206, 0x2b, 24, 16, 6, 2, 3, 1, 9, 3, 2, 1, 0, 0 },
    { &custom::unit_BoomerangBro, 1, 294, 0x04, 25, 16, 4, 2, 2, 0, 9, 3, 2, 1, 0, 0 },
    { &custom::unit_FireBro, 1, 293, 0x04, 26, 16, 4, 2, 1, 2, 9, 3, 2, 1, 0, 0 },
    { &custom::unit_RedChomp, 1, 306, 0x2c, 82, 12, 10, 5, 5, 0, 8, -1, 2, 0, 0, 0 },
    { &custom::unit_DarkKoopatrol, 1, 307, 0x2d, 19, 25, 10, 3, 5, 0, 10, 8, 3, 1, 0, 0 },
    { &custom::unit_IronCleft, 0, 289, 0x16, 74, 8, 8, 1, 3, 0, 9, -1, 2, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { &custom::unit_HyperGoomba, 1, 217, 0x04, 4, 15, 6, 0, 3, -1, 5, 10, 0, 0, 0, 0 },
    { &custom::unit_HyperParagoomba, 1, 219, 0x06, 5, 15, 6, 0, 3, -1, 6, 10, 0, 0, 40, 50 },
    { &custom::unit_HyperSpikyGoomba, 1, 218, 0x04, 6, 15, 6, 0, 3, 0, 6, 10, 0, 0, 0, 0 },
    { &custom::unit_CrazeeDayzee, 1, 252, 0x22, 56, 14, 5, 0, 2, 0, 6, 6, 0, 2, 0, 0 },
    { &custom::unit_AmazyDayzee, 0, 253, -1, 95, 20, 20, 1, 20, 0, 40, 6, 2, 4, 0, 0 },
    { &custom::unit_HyperCleft, 1, 236, 0x16, 72, 10, 6, 5, 3, 0, 6, -1, 1, 0, 0, 0 },
    { &custom::unit_BuzzyBeetle, 1, 225, 0x09, 31, 8, 6, 5, 3, 0, 4, 7, 1, 0, 0, 0 },
    { &custom::unit_SpikeTop, 1, 226, 0x0b, 32, 8, 6, 5, 3, 0, 6, 7, 1, 0, 0, 0 },
    { &custom::unit_Swooper, 1, 239, 0x20, 59, 14, 7, 0, 3, 0, 5, -1, 0, 0, 130, 80 },
    { &custom::unit_Boo, 1, 146, 0x21, 66, 13, 6, 0, 2, 1, 5, 2, 0, 1, 0, 30 },
    { &custom::unit_AtomicBoo, 0, 148, 0x21, 97, 50, 5, 0, 5, 0, 30, 2, 2, 2, 20, 30 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { &custom::unit_Ember, 1, 159, 0x24, 69, 13, 6, 0, 3, 0, 6, 2, 0, 1, 40, 20 },
    { &custom::unit_LavaBubble, 1, 302, 0x24, 68, 10, 6, 0, 3, 1, 6, 2, 0, 1, 40, 20 },
    { &custom::unit_GreenFuzzy, 1, 249, 0x10, 48, 13, 6, 0, 2, 1, 4, -1, 0, 0, 0, 0 },
    { &custom::unit_FlowerFuzzy, 1, 250, 0x10, 49, 13, 6, 0, 2, 1, 6, -1, 0, 2, 0, 0 },
    { &custom::unit_PutridPiranha, 1, 262, 0x1c, 53, 14, 6, 0, 4, 1, 6, 11, 0, 2, 0, 0 },
    { &custom::unit_Parabuzzy, 1, 228, 0x0d, 33, 8, 6, 5, 3, 0, 5, 7, 1, 0, 40, 50 },
    { nullptr, 0, 254, 0x12, -1, 10, 0, 3, 0, 0, 6, 9, 1, 0, 0, 0 },
    { nullptr, 0, 255, -1, -1, 4, 7, 1, 4, 0, 0, 9, 0, 0, 40, 0 },
    { &custom::unit_BulkyBobOmb, 0, 304, 0x25, 79, 12, 4, 2, 2, 0, 5, 9, 1, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { &custom::unit_RuffPuff, 1, 284, 0x1d, 63, 14, 8, 0, 4, 0, 4, -1, 0, 0, 40, 10 },
    { &custom::unit_PoisonPokey, 1, 234, 0x14, 51, 15, 7, 0, 3, 1, 6, -1, 1, 0, 0, 0 },
    { &custom::unit_SpikyParabuzzy, 1, 227, 0x0d, 34, 8, 6, 5, 3, 0, 7, 7, 2, 0, 40, 50 },
    { &custom::unit_DarkBoo, 1, 147, 0x21, 67, 17, 8, 0, 4, 1, 7, 2, 0, 1, 0, 30 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { &custom::unit_IcePuff, 1, 285, 0x1d, 64, 16, 8, 0, 4, 0, 6, -1, 0, 0, 40, 10 },
    { &custom::unit_FrostPiranha, 1, 263, 0x1c, 54, 16, 7, 0, 4, 1, 7, 11, 0, 2, 0, 0 },
    { &custom::unit_MoonCleft, 1, 235, 0x16, 73, 12, 8, 5, 3, 0, 6, -1, 1, 0, 0, 0 },
    { &custom::unit_ZYux, 0, 269, 0x19, 91, 9, 6, 0, 4, 0, 8, 1, 1, 1, 40, 30 },
    { &custom::unit_MiniZYux, 0, -1, -1, 92, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
    { &custom::unit_XYux, 0, 270, 0x19, 93, 11, 5, 2, 3, 0, 10, 1, 2, 2, 40, 30 },
    { &custom::unit_MiniXYux, 0, -1, -1, 94, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
    { &custom::unit_XNautPhD, 1, 273, 0x27, 87, 14, 8, 0, 4, 0, 8, 1, 0, 2, 0, 0 },
    { &custom::unit_EliteXNaut, 1, 272, 0x04, 88, 16, 9, 2, 5, 0, 8, 1, 2, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { &custom::unit_Swoopula, 1, 240, 0x20, 60, 14, 6, 0, 4, 0, 5, -1, 0, 0, 130, 80 },
    { &custom::unit_PhantomEmber, 1, 303, 0x24, 70, 16, 6, 0, 3, 2, 8, 2, 0, 2, 40, 20 },
    { nullptr, 0, 257, -1, -1, 6, 9, 2, 6, 0, 0, 9, 0, 0, 0, 0 },
    { nullptr, 0, 256, 0x12, -1, 15, 0, 5, 0, 0, 10, 9, 2, 0, 40, 0 },
    { &custom::unit_ChainChomp, 1, 301, 0x2c, 81, 10, 8, 4, 6, 0, 6, -1, 3, 0, 0, 0 },
    { &custom::unit_DarkWizzerd, 1, 296, 0x26, 84, 12, 8, 4, 5, 0, 8, -1, 2, 2, 0, 20 },
    { nullptr, 0, -1, -1, -1, 12, 8, 4, 5, 0, 8, -1, 2, 2, 0, 20 },
    { &custom::unit_DryBones, 1, 196, 0x0f, 22, 12, 7, 3, 5, 0, 7, 4, 0, 2, 0, 0 },
    { &custom::unit_DarkBones, 1, 197, 0x0f, 23, 20, 7, 3, 4, 1, 10, 4, 1, 2, 0, 0 },
    { &custom::unit_Gloomtail, 0, 198, 0x11, 98, 160, 8, 2, 8, 0, 100, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { nullptr, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0 },
    { &custom::unit_Gloomba, 1, 220, 0x04, 7, 20, 6, 0, 2, 1, 5, 10, 0, 0, 0, 0 },
    { &custom::unit_Paragloomba, 1, 222, 0x06, 8, 20, 6, 0, 2, 1, 6, 10, 0, 0, 40, 50 },
    { &custom::unit_SpikyGloomba, 1, 221, 0x04, 9, 20, 6, 0, 2, 2, 6, 10, 0, 0, 0, 0 },
    { &custom::unit_DarkKoopa, 1, 244, 0x07, 16, 20, 8, 3, 3, 1, 6, 8, 0, 0, 0, 0 },
    { &custom::unit_DarkParatroopa, 1, 245, 0x08, 17, 20, 8, 3, 3, 1, 7, 8, 0, 0, 40, 50 },
    { &custom::unit_BadgeBandit, 0, 275, 0x04, 43, 18, 6, 0, 3, 2, 6, 5, 0, 0, 0, 0 },
    { &custom::unit_DarkLakitu, 1, 281, 0x24, 28, 19, 9, 0, 5, 0, 8, -1, 2, 0, 40, 20 },
    { &custom::unit_SkyBlueSpiny, 0, -1, -1, 30, 10, 9, 4, 5, 1, 1, -1, 0, 0, 0, 0 },
    { &custom::unit_Wizzerd, 1, 295, 0x26, 83, 10, 8, 3, 7, -1, 7, -1, 1, 1, 0, 20 },
    { &custom::unit_PiranhaPlant, 1, 260, 0x1c, 55, 18, 9, 0, 4, 1, 9, 11, 0, 4, 0, 0 },
    { &custom::unit_Spunia, 1, 311, 0x28, 46, 16, 7, 2, 6, 1, 6, -1, 3, 0, 0, 0 },
    { &custom::unit_Arantula, 1, 267, 0x1b, 58, 18, 6, 0, 5, 2, 8, -1, 2, 2, 40, 140 },
    { &custom::unit_DarkBristle, 1, 259, 0x17, 76, 9, 9, 4, 8, 0, 8, -1, 0, 3, 0, 0 },
    { &custom::unit_PoisonPuff, 1, 265, 0x1d, 65, 18, 8, 0, 8, 0, 8, -1, 0, 0, 40, 10 },
    { &custom::unit_Swampire, 1, 241, 0x20, 61, 20, 8, 0, 6, 0, 8, -1, 0, 0, 130, 80 },
    { &custom::unit_BobUlk, 0, 305, 0x25, 80, 15, 5, 2, 4, 0, 7, 9, 3, 0, 0, 0 },
    { &custom::unit_EliteWizzerd, 1, 297, 0x26, 85, 14, 8, 5, 7, 1, 10, -1, 3, 3, 0, 20 },
    { nullptr, 0, -1, -1, -1, 14, 8, 5, 7, 1, 10, -1, 3, 3, 0, 20 },
    { &custom::unit_Bonetail, 0, 325, 0x11, 100, 160, 10, 2, 8, 0, 100, -1, 0, 0, 0, 0 },
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
    { { BattleUnitType::GUS,
        BattleUnitType::DARK_CRAW,
        BattleUnitType::GUS,
        BattleUnitType::DARK_CRAW,
        -1, },
        0, true, true, 20 },
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
        BattleUnitType::IRON_CLEFT_RED,
        -1, },
        1, true, true, 20 },
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
    { { BattleUnitType::BOB_OMB,
        BattleUnitType::TOT_HYPER_BOB_OMB,
        BattleUnitType::BOB_OMB,
        BattleUnitType::TOT_HYPER_BOB_OMB,
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

// Structures for holding constructed battle information.
int32_t g_NumEnemies = 0;
int32_t g_Enemies[5] = { -1, -1, -1, -1, -1 };
BattleUnitSetup g_CustomUnits[6];
BattleUnitSetup g_MidbossMinionUnits[2];
BattleGroupSetup g_CustomBattleParty;
int8_t g_CustomAudienceWeights[12];
NpcTribeDescription* g_LastMidbossTribe = nullptr;

// Base enemy weights at level 2 ~ 10, per difficulty level.
const int8_t kBaseWeights[21][9] = {
    // Tutorial 
    { 10, 10, 5, 3, 2, 0, 0, 0, 0 },
    // Half
    { 5, 10, 5, 5, 3, 0, 0, 0, 0 },
    { 2, 5, 10, 8, 6, 2, 0, 0, 0 },
    { 1, 1, 5, 10, 10, 5, 3, 0, 0 },
    { 1, 1, 2, 5, 10, 10, 5, 1, 1 },
    // Full
    { 5, 10, 5, 5, 3, 0, 0, 0, 0 },
    { 3, 5, 10, 7, 5, 1, 0, 0, 0 },
    { 1, 1, 7, 10, 10, 3, 2, 0, 0 },
    { 1, 1, 5, 10, 10, 5, 3, 0, 0 },
    { 1, 1, 2, 5, 10, 10, 5, 2, 1 },
    { 0, 0, 2, 3, 7, 10, 10, 6, 3 },
    { 0, 0, 1, 3, 6, 8, 10, 10, 6 },
    { 0, 0, 1, 2, 5, 7, 10, 10, 10 },
    // Full EX
    { 3, 5, 10, 7, 5, 1, 0, 0, 0 },
    { 1, 1, 7, 10, 10, 3, 2, 0, 0 },
    { 1, 1, 5, 10, 10, 5, 3, 0, 0 },
    { 1, 1, 2, 5, 10, 10, 5, 1, 1 },
    { 0, 0, 2, 3, 7, 10, 10, 5, 3 },
    { 0, 0, 1, 3, 6, 8, 10, 10, 8 },
    { 0, 0, 1, 2, 5, 7, 10, 10, 10 },
    { 0, 0, 1, 2, 5, 7, 10, 10, 10 },
};
// The target sum of enemy levels for each difficulty.
const int8_t kTargetLevelSums[21] = {
    // Tutorial
    12,
    // Half
    14, 18, 22, 26,
    // Full
    14, 18, 22, 26, 31, 34, 37, 40,
    // Full EX
    18, 22, 26, 30, 34, 38, 42, 46,
};
// Percentage of enemy base stats for each difficulty.
const int8_t kStatPercents[21] = {
    // Tutorial
    20,
    // Half
    25, 35, 45, 55,
    // Full
    25, 35, 45, 55, 65, 75, 90, 100,
    // Full EX
    30, 40, 50, 65, 75, 90, 105, 120,
};
// Starting difficulty level offset for each tower.
const int8_t kBaseDifficulty[4] = { 0, 1, 5, 13 };


// Gets the difficulty level for the given tower / floor combination.
int32_t GetDifficulty() {
    StateManager& state = g_Mod->state_;
    int32_t tower = state.GetOption(OPT_DIFFICULTY);
    int32_t floor = state.floor_;
    return kBaseDifficulty[tower] + (floor - 1) / 8;
}

// Every 8 floors, except 32 and 64, which are regular bosses.
int32_t IsMidbossFloor(int32_t floor) {
    return floor % 8 == 0 && floor % 32 != 0;
}

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
void SelectPresetLoadout(StateManager& state) {
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
        if (static_cast<int32_t>(state.Rand(64, RNG_ENEMY)) < state.floor_) {
            g_Enemies[3] = g_Enemies[1];
            g_Enemies[4] = g_Enemies[0];
            size = 5;
        }
    }
    g_NumEnemies = size;
}

void SelectEnemies() {
    auto& state = g_Mod->state_;
    int32_t difficulty = GetDifficulty();
    int32_t target_level_sum = kTargetLevelSums[difficulty];
    int32_t* debug_enemies = DebugManager::GetEnemies();
    
    // Handle dragon / Atomic Boo bosses.
    if (state.IsFinalBossFloor()) {
        int32_t enemy_type;
        switch (state.GetOptionValue(OPT_DIFFICULTY)) {
            case OPTVAL_DIFFICULTY_HALF:
                enemy_type = BattleUnitType::HOOKTAIL;
                break;
            default:
                // On EX, you fight Gloomtail, then Bonetail.
                enemy_type = BattleUnitType::GLOOMTAIL;
                break;
        }

        // Determine whether to use Gold Fuzzy alternate boss.
        switch (state.GetOptionValue(OPT_SECRET_BOSS)) {
            case OPTVAL_SECRET_BOSS_ON:
                enemy_type = BattleUnitType::GOLD_FUZZY;
                break;
            case OPTVAL_SECRET_BOSS_RANDOM: {

                // Base chance of using the alternate boss, out of 100.
                int32_t boss_chance = 20;

                // Don't allow secret boss on the 1st clear of a difficulty,
                // unless forced by player by turning the option 'on'.
                if (state.CheckOptionValue(OPTVAL_DIFFICULTY_HALF) && 
                    !state.GetOption(STAT_PERM_HALF_FINISHES)) break;
                if (state.CheckOptionValue(OPTVAL_DIFFICULTY_FULL) && 
                    !state.GetOption(STAT_PERM_FULL_FINISHES)) break;
                if (state.CheckOptionValue(OPTVAL_DIFFICULTY_FULL_EX) && 
                    !state.GetOption(STAT_PERM_EX_FINISHES)) break;

                if (!state.GetOption(
                    FLAGS_ACHIEVEMENT, AchievementId::META_SECRET_BOSS)) {

                    int32_t total_clears =
                        state.GetOption(STAT_PERM_HALF_FINISHES) +
                        state.GetOption(STAT_PERM_FULL_FINISHES) +
                        state.GetOption(STAT_PERM_EX_FINISHES);

                    // Don't allow the alternate boss before 5 total clears.
                    if (total_clears < 5) break;

                    // If secret boss not beaten in over 10 total clears,
                    // double the chance of it appearing.
                    if (total_clears > 10) boss_chance = 40;
                }

                if (static_cast<int32_t>(state.Rand(100, RNG_ALTERNATE_BOSS)) 
                    < boss_chance) {
                    enemy_type = BattleUnitType::GOLD_FUZZY;
                }
                break;
            }
        }

        for (int32_t i = 1; i < 5; ++i) g_Enemies[i] = -1;
        g_Enemies[0] = enemy_type;
        g_NumEnemies = 1;
        return;
    } else if (state.floor_ == 32) {
        int32_t enemy_type;
        switch (state.GetOptionValue(OPT_DIFFICULTY)) {
            case OPTVAL_DIFFICULTY_FULL_EX:
                enemy_type = BattleUnitType::TOT_COSMIC_BOO;
                break;
            default:
                enemy_type = BattleUnitType::ATOMIC_BOO;
                break;
        }
        for (int32_t i = 1; i < 5; ++i) g_Enemies[i] = -1;
        g_Enemies[0] = enemy_type;
        g_NumEnemies = 1;
        return;
    }
    
    // Select a mid-boss when floor is a multiple of 8.
    if (IsMidbossFloor(state.floor_)) {
        // If debug set of enemies, take the first enemy from the list.
        if (debug_enemies) {
            PopulateDebugEnemyLoadout(debug_enemies);
            for (int32_t i = 1; i < 5; ++i) g_Enemies[i] = -1;
            g_NumEnemies = 1;
            return;
        }
        
        int32_t kNumEnemyTypes = BattleUnitType::BONETAIL + 1;
        int16_t weights[kNumEnemyTypes];
        for (int32_t i = 0; i < kNumEnemyTypes; ++i) {
            int32_t base_wt = 0;
            const EnemyTypeInfo& ei = kEnemyInfo[i];
            
            // If enemy is not mid-boss eligible, ignore.
            if (ei.midboss_eligible && ei.level >= 2 && ei.level <= 10) {
                base_wt = kBaseWeights[difficulty][ei.level - 2];
            }
            weights[i] = base_wt;
        }
        
        // Disable picking midbosses that were already used.
        for (int32_t i = 0; i < 7; ++i) {
            int32_t enemy = state.GetOption(STAT_RUN_MIDBOSSES_USED, i);
            weights[enemy] = 0;
        }
        
        // Pick a single enemy to make into a mid-boss.
        int32_t sum_weights = 0;
        for (int32_t i = 0; i < kNumEnemyTypes; ++i) sum_weights += weights[i];
        
        int32_t weight = state.Rand(sum_weights, RNG_ENEMY);
        int32_t idx = 0;
        for (; (weight -= weights[idx]) >= 0; ++idx);
        
        g_Enemies[0] = idx;        
        for (int32_t i = 1; i < 5; ++i) g_Enemies[i] = -1;
        g_NumEnemies = 1;
        
        // Save the enemy selected, so it won't show up again in the same run.
        int32_t boss_num = (state.floor_ / 8) - 1;
        state.SetOption(STAT_RUN_MIDBOSSES_USED, idx, boss_num);
        
        return;
    }
    
    // Check to see if there is a debug set of enemies, and use it if so.
    if (debug_enemies) {
        PopulateDebugEnemyLoadout(debug_enemies);
        return;
    }
    
    // After enemy loadouts are strong enough, chance to use a preset loadout.
    if (target_level_sum > 30 && state.Rand(100, RNG_ENEMY) < 10) {
        SelectPresetLoadout(state);
    } else {
        // Put together an array of weights, scaled by the floor number and
        // enemy's level offset (such that harder enemies appear more later on).
        int32_t kNumEnemyTypes = BattleUnitType::BONETAIL + 1;
        
        int16_t weights[6][kNumEnemyTypes];
        for (int32_t i = 0; i < kNumEnemyTypes; ++i) {
            int32_t base_wt = 0;
            const EnemyTypeInfo& ei = kEnemyInfo[i];
            
            // If enemy is unimplemented, or is not a standard enemy, ignore.
            if (ei.kind && ei.level >= 2 && ei.level <= 10) {
                base_wt = kBaseWeights[difficulty][ei.level - 2];
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
        for (int32_t slot = 0; slot < 5; ++slot) {
            int32_t sum_weights = 0;
            for (int32_t i = 0; i < kNumEnemyTypes; ++i) 
                sum_weights += weights[slot][i];
            
            int32_t weight = state.Rand(sum_weights, RNG_ENEMY);
            int32_t idx = 0;
            for (; (weight -= weights[slot][idx]) >= 0; ++idx);
            
            g_Enemies[slot] = idx;
            level_sum += kEnemyInfo[idx].level;
            
            // If level_sum is sufficiently high for the floor and not on the
            // fifth enemy, decide whether to add any further enemies.
            if (level_sum >= target_level_sum / 2 && slot < 4) {
                const int32_t end_chance = level_sum * 100 / target_level_sum;
                if (static_cast<int32_t>(state.Rand(100, RNG_ENEMY)) 
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
        
        // If enemy strength is high enough, small chance to add Amazy Dayzees.
        if (static_cast<int32_t>(state.Rand(100, RNG_ENEMY)) 
            < target_level_sum - 33) {
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
    if (GetNumActivePartners() <= 0) {
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

void FillBattleUnitSetup(
    BattleUnitSetup& unit, const EnemyTypeInfo* type_info,
    float x_position, float z_position, bool back_enemy = true) {
    unit.unit_kind_params = type_info->kind;
    unit.alliance = 1;
    unit.attack_phase = 0x400'0004;
    unit.addl_target_offset_x = 0;
    memset(unit.unit_work, 0, sizeof(unit.unit_work));
    unit.item_drop_table = nullptr;
    
    // Height depends on enemy.
    unit.position.x = x_position;
    unit.position.y = type_info->battle_y_pos;
    unit.position.z = z_position;
    
    switch (type_info - kEnemyInfo) {
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
            unit.unit_work[0] = g_Mod->state_.Rand(back_enemy ? 2 : 1, RNG_ENEMY);
            break;
        }
    }
}

void BuildBattle(
    BattleSetupData* battle, NpcSetupInfo* npc_setup_info,
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
    
    auto& state = g_Mod->state_;
        
    int32_t num_npcs = 1;
    if (!state.IsFinalBossFloor()) {
        // Special case overworld NPC for dragons, for testing purposes.
        switch (g_Enemies[0]) {
            case BattleUnitType::HOOKTAIL:
            case BattleUnitType::GLOOMTAIL:
            case BattleUnitType::BONETAIL:
                // Use a Goomba on the overworld; don't first strike it!
                npc_tribe = 
                    ttyd::npc_data::npcTribe + kEnemyInfo[1].npc_tribe_idx;
                npc_ai =
                    ttyd::npc_data::npc_ai_type_table +
                    kEnemyInfo[1].ai_type_idx;
                break;
        }
    } else {
        // Make a copy of NPC so type can be swapped to Bonetail after fight.
        num_npcs = 2;
        // Always set bosses to basic "Gold Fuzzy" AI type.
        npc_ai = &ttyd::npc_data::npc_ai_type_table[NpcAiType::GOLD_FUZZY];
    }
    
    memset(npc_setup_info, 0, sizeof(NpcSetupInfo) * 3);
    for (int32_t i = 0; i < num_npcs; ++i) {
        NpcSetupInfo& npc = npc_setup_info[i];
        npc.name              = i == 0 ? "npc_enemy" : "npc_enemy_2";
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
        npc.battleInfoId      = 2;      /* Initialize to invalid battle */
    }
    
    // Set output variables.
    *out_npc_tribe_description = npc_tribe;
    *out_lead_type = g_Enemies[0];
    
    // Construct the BattleGroupSetup from previously selected enemies.
    
    for (int32_t i = 0; i < 12; ++i) g_CustomAudienceWeights[i] = 2;
    // Make Toads slightly likelier since they're never boosted.
    g_CustomAudienceWeights[0] = 3;
    for (int32_t i = 0; i < g_NumEnemies; ++i) {
        BattleUnitSetup& unit = g_CustomUnits[i];
        
        // Position the unit in standard spacing.
        float offset = i - (g_NumEnemies - 1) * 0.5f;
        float x = kEnemyPartyCenterX + offset * kEnemyPartySepX;
        float z = offset * kEnemyPartySepZ;
        
        // Make sure bosses appear in the right position.
        switch (g_Enemies[i]) {
            case BattleUnitType::GOLD_FUZZY:
                x = 125.0f;
                break;
            case BattleUnitType::ATOMIC_BOO:
            case BattleUnitType::TOT_COSMIC_BOO:
                x = 70.0f;
                break;
            case BattleUnitType::HOOKTAIL:
            case BattleUnitType::GLOOMTAIL:
            case BattleUnitType::BONETAIL:
                x = 365.0f;
                break;
        }
        
        FillBattleUnitSetup(unit, enemy_info[i], x, z, /* back_enemy */ i > 0);
        
        // If a midboss floor, set unit work 3 to 1 as sentinel.
        if (IsMidbossFloor(state.floor_)) unit.unit_work[3] = 1;
        
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
    
    // Point the passed-in battle setup's loadouts to the constructed party.
    battle->flag_off_loadouts[0].group_data = &g_CustomBattleParty;

    // Update the background based on the current floor, if not a boss battle.
    static const char* kStageBgDirs[] = {
        "stg_01_3", "stg_01_7", "stg_01_8", "stg_01_9"
    };
    if (!state.IsFinalBossFloor()) {
        battle->flag_off_loadouts[0].stage_data->current_stage_data_dir =
            kStageBgDirs[(state.floor_ - 1) / 16];
    }
    
    // Set the battle's audience weights based on the custom ones set above.
    for (int32_t i = 0; i < 12; ++i) {
        auto& weights = battle->audience_type_weights[i];
        weights.min_weight = g_CustomAudienceWeights[i];
        weights.max_weight = g_CustomAudienceWeights[i];
    }
    
    if (state.IsFinalBossFloor()) {
        if (g_Enemies[0] != BattleUnitType::GOLD_FUZZY) {
            battle->music_name = "BGM_BOSS_STG1_GONBABA1";
        } else {
            battle->music_name = "BGM_CHUBOSS_BATTLE1";
        }
    } else if (state.floor_ == 32) {
        battle->music_name = "BGM_BOSS_STG4_RUNPELL1";
    } else if (IsMidbossFloor(state.floor_)) {
        battle->music_name = "BGM_KOBOSS_BATTLE1";
    } else {
        battle->music_name = "BGM_ZAKO_BATTLE1";
    }
    
    // Disallow running from the final boss fight.
    if (state.IsFinalBossFloor()) {
        battle->battle_setup_flags |= 0x10;
    } else {
        battle->battle_setup_flags &= ~0x10;
    }

    // Set flag to suppress Mario and party's entry event for dragon fights.
    switch (g_Enemies[0]) {
        case BattleUnitType::HOOKTAIL:
        case BattleUnitType::GLOOMTAIL:
        case BattleUnitType::BONETAIL:
            battle->battle_setup_flags |= 0x10'0000;
            break;
        default:
            battle->battle_setup_flags &= ~0x10'0000;
            break;
    }
}

}  // namespace

EVT_DEFINE_USER_FUNC(evtTot_GetEnemyNpcInfo) {
    auto* battle_db = (BattleSetupData*)evtGetValue(evt, evt->evtArguments[0]);
    auto* npc_setup = (NpcSetupInfo*)evtGetValue(evt, evt->evtArguments[1]);

    // Clear double-height multiplier from previous midboss NPC, if any.
    if (g_LastMidbossTribe) {
        g_LastMidbossTribe->height /= 2.0f;
        g_LastMidbossTribe = nullptr;
    }

    ttyd::npcdrv::NpcTribeDescription* npc_tribe_description;
    int32_t lead_enemy_type;
    
    SelectEnemies();

    // Use different battle setup for final boss.
    if (g_Mod->state_.IsFinalBossFloor()) battle_db += 1;
    BuildBattle(
        battle_db, npc_setup, &npc_tribe_description, &lead_enemy_type);
    
    int32_t x_pos = ttyd::system::irand(100) + 20;
    int32_t z_pos = ttyd::system::irand(200) - 100;
    int32_t y_pos = kEnemyInfo[lead_enemy_type].field_y_pos;
    
    if (lead_enemy_type == BattleUnitType::PIDER ||
        lead_enemy_type == BattleUnitType::ARANTULA) {
        // Need extra vertical range to detect Mario from the ceiling.
        npc_setup->territoryBase.y = y_pos;
        npc_setup->territoryLoiter.y = 300.f;
    }
    
    if (lead_enemy_type == BattleUnitType::CHAIN_CHOMP ||
        lead_enemy_type == BattleUnitType::RED_CHOMP) {
        npc_setup->territoryBase = { (float)x_pos, (float)y_pos, (float)z_pos };
    }
    
    evtSetValue(evt, evt->evtArguments[2], PTR(npc_tribe_description->modelName));
    evtSetValue(evt, evt->evtArguments[3], PTR(npc_tribe_description->nameJp));
    evtSetValue(evt, evt->evtArguments[4], x_pos);
    evtSetValue(evt, evt->evtArguments[5], y_pos);
    evtSetValue(evt, evt->evtArguments[6], z_pos);
    evtSetValue(evt, evt->evtArguments[7], g_Enemies[0] == BattleUnitType::GOLD_FUZZY);

    // Double effective height of midboss field NPCs (for "!" indicator, mostly).
    if (IsMidbossFloor(g_Mod->state_.floor_)) {
        g_LastMidbossTribe = npc_tribe_description;
        npc_tribe_description->height *= 2.0f;
    }
    
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_SetEnemyNpcBattleInfo) {
    const char* name = (const char*)evtGetValue(evt, evt->evtArguments[0]);
    int32_t battle_id = evtGetValue(evt, evt->evtArguments[1]);
    NpcEntry* npc = ttyd::evt_npc::evtNpcNameToPtr(evt, name);
    ttyd::npcdrv::npcSetBattleInfo(npc, battle_id);
    
    const auto& state = g_Mod->state_;
    NpcBattleInfo* battle_info = &npc->battleInfo;

    // Always have Pity Flower equipped on Gold Fuzzy.
    if (g_Enemies[0] == BattleUnitType::GOLD_FUZZY) {
        battle_info->wHeldItems[0] = ItemType::PITY_FLOWER;
    }
    
    if (state.IsFinalBossFloor()) return 2;
    
    const int32_t reward_mode = state.GetOptionValue(OPT_BATTLE_DROPS);
    if (reward_mode == OPTVAL_DROP_NO_HELD_W_BONUS) {
        for (int32_t i = 0; i < battle_info->pConfiguration->num_enemies; ++i) {
            battle_info->wHeldItems[i] = 0;
        }
    } else {
        // Base weight of Star Pieces; goes from 0 ~ 14 for non-midboss floors.
        int32_t sp_rate = state.floor_ % 8 ? (state.floor_ / 8) * 2 : 0;
        // Make Star Pieces more likely if they can't be given by conditions.
        if (reward_mode == OPTVAL_DROP_HELD_FROM_BONUS) sp_rate += 10;
        for (int32_t i = 0; i < battle_info->pConfiguration->num_enemies; ++i) {
            int32_t item = PickRandomItem(RNG_ENEMY_ITEM, 50, 10, 30, sp_rate);            
            if (!item) item = ItemType::STAR_PIECE;
            battle_info->wHeldItems[i] = item;
        }
    }
    
    // Occasionally, set a battle condition for an optional bonus reward.
    SetBattleCondition(&npc->battleInfo);
    
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_ClearEnemyInfo) {
    for (int32_t i = 0; i < g_NumEnemies; ++i) g_Enemies[i] = -1;
    g_NumEnemies = 0;
    return 2;
}

bool GetEnemyStats(
    int32_t unit_type, int32_t* out_hp, int32_t* out_atk, int32_t* out_def,
    int32_t* out_level, int32_t* out_coinlvl, int32_t base_attack_power) {
    // Look up the enemy type info w/matching unit_type.
    if (unit_type > BattleUnitType::BONETAIL || unit_type < 0 ||
        kEnemyInfo[unit_type].base_hp < 0) {
        // No stats to pull from; just use the original message.
        return false;
    }
    const auto& state = mod::g_Mod->state_;
    const EnemyTypeInfo& ei = kEnemyInfo[unit_type];
    
    int32_t difficulty = GetDifficulty();
        
    int32_t base_hp_pct = kStatPercents[difficulty];
    int32_t base_atk_pct = kStatPercents[difficulty];
    int32_t base_def_pct = kStatPercents[difficulty];
    
    // Exception: for major boss fights, use base stats directly.
    if (state.floor_ % 32 == 0) {
        base_hp_pct = 100;
        base_atk_pct = 100;
        base_def_pct = 100;

        // For Gold Fuzzy / Fuzzy Horde, scale stats by difficulty.
        // TODO: Stats should probably be a little lower across the board?
        if (unit_type == BattleUnitType::GOLD_FUZZY ||
            unit_type == BattleUnitType::FUZZY_HORDE) {
            switch (state.GetOptionValue(OPT_DIFFICULTY)) {
                case OPTVAL_DIFFICULTY_HALF:
                    // 75 HP, 100 HP horde; 5 ATK
                    base_hp_pct = 50;
                    base_atk_pct = 70;
                    break;
                case OPTVAL_DIFFICULTY_FULL_EX:
                    // 225 HP, 300 HP horde; 9 ATK
                    base_hp_pct = 150;
                    base_atk_pct = 130;
                    break;
                // default: 150 HP, 200 HP horde; 7 ATK
            }
        }
    }
    
    // Change this if adding back a boss scaling option.
    int32_t boss_scale_factor = 4;

    // Buff regular enemies' HP and ATK by 20% if Doopliss's effect is active.
    int32_t doopliss_scale_factor = 5;
    int32_t doopliss_floor = state.GetOption(STAT_RUN_NPC_DOOPLISS_FLOOR);
    if (doopliss_floor && state.floor_ - doopliss_floor < 8) {
        if (state.floor_ % 8 != 0) {
            doopliss_scale_factor = 6;
        }
    }
            
    if (out_hp) {
        int32_t hp = Min(ei.base_hp * base_hp_pct, 1000000);
        hp *= state.GetOption(OPTNUM_ENEMY_HP);
        hp = hp * boss_scale_factor / 4;
        hp = hp * doopliss_scale_factor / 5;
        *out_hp = Clamp((hp + 5000) / 10000, 1, 9999);
    }
    if (out_atk) {
        int32_t atk = Min(ei.base_atk * base_atk_pct, 1000000);
        atk += (base_attack_power - ei.atk_reference) * 100;
        atk *= state.GetOption(OPTNUM_ENEMY_ATK);
        atk = atk * boss_scale_factor / 4;
        atk = atk * doopliss_scale_factor / 5;
        *out_atk = Clamp((atk + 5000) / 10000, 1, 99);
    }
    if (out_def) {
        if (ei.base_def == 0) {
            *out_def = 0;
        } else {
            // Enemies with base_def > 0 should always have at least 1 DEF.
            int32_t def = (ei.base_def * base_def_pct + 50) / 100;
            def = def < 1 ? 1 : def;
            *out_def = def > 99 ? 99 : def;
        }
    }
    if (out_level) {
        // Practically doesn't matter anymore, aside from audience calculations
        // and displaying coins when enemies are defeated.
        // Return level 0 for minion enemies (e.g. mini-Yuxes),
        // or enemy's level + Mario's fixed level 20 otherwise.
        *out_level = ei.level == 0 ? 0 : ei.level + 20 + 1;
    }
    if (out_coinlvl) {
        // Return the level directly.
        *out_coinlvl = ei.level;
    }
    
    return true;
}

EVT_DEFINE_USER_FUNC(evtTot_GetEnemyStats) {
    int32_t unit_type = evtGetValue(evt, evt->evtArguments[0]);
    int32_t base_atk = evtGetValue(evt, evt->evtArguments[6]);
    int32_t hp, atk, def, level, coinlvl;
    GetEnemyStats(unit_type, &hp, &atk, &def, &level, &coinlvl, base_atk);
    
    evtSetValue(evt, evt->evtArguments[1], hp);
    evtSetValue(evt, evt->evtArguments[2], atk);
    evtSetValue(evt, evt->evtArguments[3], def);
    evtSetValue(evt, evt->evtArguments[4], level);
    evtSetValue(evt, evt->evtArguments[5], coinlvl);
    return 2;
}

int32_t GetBattleRewardTier() {
    int32_t floor = g_Mod->state_.floor_;
    // For the starter floor, you only have one choice.
    if (floor == 0) return 1;

    int32_t num_chests = g_Mod->state_.GetOption(OPT_NUM_CHESTS);

    // If set to default, determine the number of chests based on the tower
    // difficulty, tower progression, and enemy loadouts' sum of levels.
    if (num_chests == 0) {
        int32_t level_target_sum = kTargetLevelSums[GetDifficulty()];
        int32_t level_sum = 0;
        for (int32_t i = 0; i < 5; ++i) {
            if (g_Enemies[i] == -1) break;
            level_sum += kEnemyInfo[g_Enemies[i]].level;
        }
        level_sum *= 100;

        num_chests = 1;
        if (floor > 0 && floor % 8 == 0) {
            // For midboss or boss floors, always 3 options.
            num_chests = 3;
        } else if (floor % 8 == 7) {
            // For rest floors, always 1 option.
            num_chests = 1;
        } else if (g_Mod->state_.CheckOptionValue(OPTVAL_DIFFICULTY_FULL_EX)) {
            // Increased number of chests in EX mode, min of 2 even on rest floors.
            if (level_sum / level_target_sum >= 0) ++num_chests;
            if (level_sum / level_target_sum >= 62) ++num_chests;
            // Exceptionally hard layouts can give 4 chests later in the Pit.
            if (floor > 16 && level_sum / level_target_sum >= 83) ++num_chests;
        } else {
            if (level_sum / level_target_sum >= 55) ++num_chests;
            if (level_sum / level_target_sum >= 70) ++num_chests;
        }
    }

    // Give an extra chest if Doopliss's effect is active (up to the max of 4).
    int32_t doopliss_floor = g_Mod->state_.GetOption(STAT_RUN_NPC_DOOPLISS_FLOOR);
    if (doopliss_floor && floor - doopliss_floor < 8) {
        ++num_chests;
    }

    if (num_chests > 4) num_chests = 4;
    return num_chests;
}

EVT_DECLARE_USER_FUNC(evtTot_GetMinionEntries, 2)
EVT_DEFINE_USER_FUNC(evtTot_GetMinionEntries) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t boss_id = ttyd::battle_sub::BattleTransID(evt, -2);
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, boss_id);
    int32_t unit_kind = unit->current_kind;
    
    int32_t position_offsets[] = { -1, 1 };
    
    for (int32_t i = 0; i < 2; ++i) {
        BattleUnitSetup* setup = nullptr;
    
        float x =
            kEnemyPartyCenterX + position_offsets[i] * kEnemyPartySepX;
        float z = position_offsets[i] * kEnemyPartySepZ;
        
        // Check to make sure no other enemies are at the given X position.
        bool position_free = true;
        for (int32_t idx = 0; idx < 64; ++idx) {
            auto* unit = battleWork->battle_units[idx];
            if (unit && !ttyd::battle_unit::BtlUnit_CheckStatus(unit, 27) &&
                unit->home_position.x == x) {
                position_free = false;
                break;
            }
        }
        
        if (position_free) {
            setup = &g_MidbossMinionUnits[i];
            FillBattleUnitSetup(*setup, &kEnemyInfo[unit_kind], x  + 250.0f, z);
        }
        
        evtSetValue(evt, evt->evtArguments[i], PTR(setup));
    }
    return 2;
}

EVT_DECLARE_USER_FUNC(evtTot_GetMinionSpawnPos, 4)
EVT_DEFINE_USER_FUNC(evtTot_GetMinionSpawnPos) {
    int32_t idx = evtGetValue(evt, evt->evtArguments[0]);
    auto& unit_setup = g_MidbossMinionUnits[idx];
    evtSetValue(evt, evt->evtArguments[1], unit_setup.position.x);
    evtSetValue(evt, evt->evtArguments[2], unit_setup.position.y);
    evtSetValue(evt, evt->evtArguments[3], unit_setup.position.z);
    return 2;
}

EVT_BEGIN(MidbossEvt)
    // If the battle has started (not a First Strike), can call for backup.
    USER_FUNC(btlevtcmd_get_turn, LW(0))
    USER_FUNC(evt_sub_random, 99, LW(1))
    IF_LARGE_EQUAL(LW(0), 1)
        // Backup call chance = 50%, diminishing by 5% per turn.
        MUL(LW(0), 5)
        SUB(LW(0), 55)
        MUL(LW(0), -1)
        IF_SMALL(LW(1), LW(0))
            SET(LW(0), 1)
        ELSE()
            SET(LW(0), 0)
        END_IF()
    END_IF()
    IF_LARGE(LW(0), 0)
        USER_FUNC(evtTot_GetMinionEntries, LW(10), LW(11))
        SET(LW(2), 0)
        
        // Independently run spawning events for enemies.
        IF_NOT_EQUAL(LW(10), 0)
            ADD(LW(2), 1)
            
            INLINE_EVT()
                WAIT_MSEC(600)
                USER_FUNC(btlevtcmd_SpawnUnit, LW(3), LW(10), 0)
                WAIT_FRM(2)
                USER_FUNC(btlevtcmd_GetHomePos, LW(3), LW(0), LW(1), LW(2))
                SUB(LW(0), 250)
                WAIT_FRM(10)
                USER_FUNC(btlevtcmd_SetMoveSpeed, LW(3), 8)
                USER_FUNC(btlevtcmd_MovePosition, LW(3), LW(0), LW(1), LW(2), 0, -1, 0)
                USER_FUNC(btlevtcmd_SetHomePos, LW(3), LW(0), LW(1), LW(2))
            END_INLINE()
        END_IF()
        IF_NOT_EQUAL(LW(11), 0)
            ADD(LW(2), 1)
            
            INLINE_EVT()
                WAIT_MSEC(600)
                USER_FUNC(btlevtcmd_SpawnUnit, LW(3), LW(11), 0)
                WAIT_FRM(2)
                USER_FUNC(btlevtcmd_GetHomePos, LW(3), LW(0), LW(1), LW(2))
                SUB(LW(0), 250)
                WAIT_FRM(10)
                USER_FUNC(btlevtcmd_SetMoveSpeed, LW(3), 8)
                USER_FUNC(btlevtcmd_MovePosition, LW(3), LW(0), LW(1), LW(2), 0, -1, 0)
                USER_FUNC(btlevtcmd_SetHomePos, LW(3), LW(0), LW(1), LW(2))
            END_INLINE()
        END_IF()
        
        // If at least one spot was open, wait for spawn event to finish.
        IF_NOT_EQUAL(LW(2), 0)
            USER_FUNC(evt_btl_camera_set_mode, 0, 7)
            USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -1)
            USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
            USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
            USER_FUNC(
                btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGENOKO_CALL1"),
                EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_MSEC(600)
            // Rough length of spawning animation.
            WAIT_MSEC(1500)
            USER_FUNC(evt_btl_camera_set_mode, 0, 0)
            WAIT_MSEC(300)
            
            // Unless on EX difficulty, skip attack after spawning.
            USER_FUNC(evtTot_GetDifficulty, LW(0))
            IF_NOT_EQUAL(LW(0), (int32_t)OPTVAL_DIFFICULTY_FULL_EX)
                USER_FUNC(btlevtcmd_StartWaitEvent, -2)
                RETURN()
            END_IF()
        END_IF()
    END_IF()
    // Fall back to running normal attack event.
    // (The pointer to the event is patched in via GetMidbossAttackScript!)
    RUN_CHILD_EVT(0)
    RETURN()
EVT_END()

void* GetMidbossAttackScript(void* original_script) {
    // Patch over the "RUN_CHILD_EVT" script pointer.
    uintptr_t patch_location = reinterpret_cast<uintptr_t>(MidbossEvt);
    // Third op/arg from the end (0, RETURN(), END())
    patch_location += sizeof(MidbossEvt) - sizeof(int32_t) * 3;
    *(void**)patch_location = original_script;
    return (void*)MidbossEvt;
}

char g_TattleTextBuf[512];

const char* GetCustomTattle() { return g_TattleTextBuf; }

const char* SetCustomTattle(
    BattleWorkUnit* unit, const char* original_tattle_msg) {
    int32_t unit_type = unit->current_kind;
    if (unit_type > BattleUnitType::BONETAIL || unit_type < 0 ||
        kEnemyInfo[unit_type].base_hp < 0) {
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
        ei.base_hp, ei.base_atk, ei.atk_offset ? atk_offset_buf : "",
        ei.base_def, ei.level);
    
    // Append one more paragraph with the enemy's current stats
    // (using its standard attack's power as reference for ATK).
    int32_t hp, atk, def;
    int32_t base_atk_power = ei.atk_offset + ei.atk_reference;
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

const char* SetCustomMenuTattle(int32_t unit_type) {
    // Print a simple base stat string to g_TattleTextBuf.
    if (unit_type > BattleUnitType::BONETAIL || unit_type < 0 ||
        kEnemyInfo[unit_type].base_hp < 0) {
        // No stats to pull from.
        sprintf(g_TattleTextBuf, "No info known on this enemy.");
    } else {
        const EnemyTypeInfo& ei = kEnemyInfo[unit_type];
        char atk_offset_buf[8];
        sprintf(atk_offset_buf, " (%+" PRId16 ")", ei.atk_offset);
        sprintf(g_TattleTextBuf,
            "Base HP: %" PRId16 ", Base ATK: %" PRId16 "%s,\n"
            "Base DEF: %" PRId16 ", Level: %" PRId16 "",
            ei.base_hp, ei.base_atk, ei.atk_offset ? atk_offset_buf : "",
            ei.base_def, ei.level);
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
        kEnemyInfo[unit_type].base_hp < 0) {
        return false;
    }
    const EnemyTypeInfo& ei = kEnemyInfo[unit_type];
    int32_t base_atk_power = ei.atk_offset + ei.atk_reference;
    return GetEnemyStats(
        unit_type, nullptr, atk, def, nullptr, nullptr, base_atk_power);
}

bool IsEligibleLoadoutEnemy(int32_t unit_type) {
    if (unit_type > BattleUnitType::BONETAIL || unit_type < 0) return false;
    return kEnemyInfo[unit_type].kind != nullptr;
}

bool IsEligibleFrontEnemy(int32_t unit_type) {
    if (unit_type > BattleUnitType::BONETAIL || unit_type < 0) return false;
    return kEnemyInfo[unit_type].ai_type_idx >= 0 &&
           kEnemyInfo[unit_type].kind != nullptr;
}

}  // namespace mod::tot