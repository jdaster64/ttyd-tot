#include "tot_move_manager.h"

#include "evt_cmd.h"
#include "mod.h"

#include <ttyd/battle.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_unit.h>
#include <ttyd/evtmgr_cmd.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::tot {
    
namespace {
    
using ::mod::infinite_pit::g_Mod;
using ::ttyd::battle::g_BattleWork;
using ::ttyd::battle_database_common::BattleWeapon;
using ::ttyd::battle_unit::BattleWorkUnit;
using ::ttyd::battle_unit::BattleWorkUnitPart;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;
    
}
    
struct MoveData {
    const char* name_abbreviated;
    int8_t move_cost[3];
};
const MoveData g_MoveData[] = {
    { "Jump",       { 0, 2, 4, }, },
    { "Spin Jump",  { 2, 4, 6, }, },
    { "Spring J.",  { 4, 6, 8, }, },
    { "Power J.",   { 2, 4, 6, }, },
    { "Multib.",    { 2, 4, 6, }, },
    { "Power B.",   { 3, 5, 7, }, },
    { "Sleep S.",   { 2, 4, 6, }, },
    { "Tor. J.",    { 2, 4, 6, }, },
    
    { "Hammer",     { 0, 2, 4, }, },
    { "Super H.",   { 2, 4, 6, }, },
    { "Ultra H.",   { 4, 6, 8, }, },
    { "Power S.",   { 2, 4, 6, }, },
    { "H. Rattle",  { 2, 4, 6, }, },
    { "Ice Smash",  { 2, 4, 6, }, },
    { "Quake H.",   { 3, 5, 7, }, },
    { "Fire Drive", { 3, 5, 7, }, },
    
    { "Sweet Tr.",  { 1, 2, 3, }, },
    { "Earth Tr.",  { 1, 2, 3, }, },
    { "Clock Out",  { 2, 3, 4, }, },
    { "Power Lift", { 2, 3, 4, }, },
    { "Art Attack", { 3, 4, 5, }, },
    { "Sweet F.",   { 2, 3, 4, }, },
    { "Showst.",    { 3, 4, 5, }, },
    { "Supernova",  { 4, 5, 6, }, },
    
    { "Headbonk",   { 0, 2, 4, }, },
    { "Tattle",     { 0, 0, 0, }, },
    { "Multibonk",  { 3, 5, 7, }, },
    { "Rally Wink", { 4, 4, 4, }, },
    { "Move 5",     { 5, 5, 5, }, },
    { "Move 6",     { 1, 1, 1, }, },
    
    { "Shell Toss", { 0, 2, 4, }, },
    { "Power Sh.",  { 2, 4, 6, }, },
    { "Shell Shield", { 4, 6, 8, }, },
    { "Shell Slam", { 4, 6, 8, }, },
    { "Move 5",     { 3, 5, 7, }, },
    { "Move 6",     { 5, 5, 5, }, },
    
    { "Body Slam",  { 0, 2, 4, }, },
    { "Gale Force", { 4, 4, 4, }, },
    { "Lip Lock",   { 3, 5, 7, }, },
    { "Dodgy Fog",  { 4, 4, 4, }, },
    { "Move 5",     { 3, 5, 7, }, },
    { "Move 6",     { 9, 9, 9, }, },
    
    { "Ground P.",  { 0, 2, 4, }, },
    { "Gulp",       { 3, 5, 7, }, },
    { "Mini-Egg",   { 3, 5, 7, }, },
    { "Stampede",   { 4, 6, 8, }, },
    { "Move 5",     { 3, 5, 7, }, },
    { "Move 6",     { 5, 5, 5, }, },
    
    { "Shade Fist", { 0, 2, 4, }, },
    { "Veil",       { 1, 1, 1, }, },
    { "Fiery Jinx", { 3, 5, 7, }, },
    { "Infatuate",  { 5, 5, 5, }, },
    { "Move 5",     { 1, 1, 1, }, },
    { "Move 6",     { 1, 1, 1, }, },
    
    { "Bomb",       { 0, 2, 4, }, },
    { "Bomb Sq.",   { 2, 4, 6, }, },
    { "Hold Fast",  { 4, 4, 4, }, },
    { "Bob-ombast", { 4, 6, 8, }, },
    { "Move 5",     { 2, 4, 6, }, },
    { "Move 6",     { 10, 10, 10, }, },
    
    { "Love Slap",  { 0, 2, 4, }, },
    { "Kiss Thief", { 5, 5, 5, }, },
    { "Tease",      { 4, 4, 4, }, },
    { "Smooch",     { 3, 5, 7, }, },
    { "Move 5",     { 5, 5, 5, }, },
    { "Move 6",     { 3, 5, 7, }, },
}; 

void MoveManager::Init() {
    // TODO: Implement.
    for (int32_t i = 0; i < MoveType::MOVE_TYPE_MAX; ++i) {
        g_Mod->tot_state_.level_unlocked_[i] = 3;
        g_Mod->tot_state_.level_selected_[i] = 1;
    }
}

int32_t MoveManager::GetUnlockedLevel(int32_t move_type) {
    return g_Mod->tot_state_.level_unlocked_[move_type];
}

int32_t MoveManager::GetSelectedLevel(int32_t move_type) {
    return g_Mod->tot_state_.level_selected_[move_type];
}

int32_t MoveManager::GetMoveCost(int32_t move_type) {
    return g_MoveData[move_type].move_cost[
        g_Mod->tot_state_.level_selected_[move_type]-1];
}

void MoveManager::GetCurrentSelectionString(int32_t move_type, char* out_buf) {
    sprintf(
        out_buf, "%s Lv. %" PRId8,
        g_MoveData[move_type].name_abbreviated, GetSelectedLevel(move_type));
}

bool MoveManager::ChangeSelectedLevel(int32_t move_type, int32_t change) {
    int32_t old_level = g_Mod->tot_state_.level_selected_[move_type];
    int32_t new_level = old_level + change;
    
    // TODO: Implement maxes.
    if (new_level < 1) new_level = 1;
    if (new_level > 3) new_level = 3;
    
    g_Mod->tot_state_.level_selected_[move_type] = new_level;
    return new_level != old_level;
}

void MoveManager::ResetSelectedLevels() {
    // TODO: Set to min level for all unlocked moves?
    MoveManager::Init();
}

bool MoveManager::IsUnlockable(int32_t move_type) {
    // TODO: Implement dependencies.
    return false;
}

bool MoveManager::IsUpgradable(int32_t move_type) {
    // TODO: Implement maxes.
    return false;
}

uint32_t GetWeaponPowerFromSelectedLevel(
    BattleWorkUnit* unit1, BattleWeapon* weapon, BattleWorkUnit* unit2,
    BattleWorkUnitPart* part) {
    const int32_t move = weapon->damage_function_params[7];
    const int32_t level = MoveManager::GetSelectedLevel(move);
    const int32_t ac_success =
        g_BattleWork->ac_manager_work.ac_result == 2 ? 1 : 0;
        
    int32_t power = weapon->damage_function_params[level * 2 - 2 + ac_success];
    if ((move & ~7) == MoveType::JUMP_BASE) {
        power += unit1->badges_equipped.jumpman;
    } else if ((move & ~7) == MoveType::HAMMER_BASE) {
        power += unit1->badges_equipped.hammerman;
    }
    return power;
}

uint32_t GetWeaponPowerFromMaxLevel(
    BattleWorkUnit* unit1, BattleWeapon* weapon, BattleWorkUnit* unit2,
    BattleWorkUnitPart* part) {
    const int32_t move = weapon->damage_function_params[7];
    const int32_t level = MoveManager::GetUnlockedLevel(move);
    const int32_t ac_success =
        g_BattleWork->ac_manager_work.ac_result == 2 ? 1 : 0;
        
    int32_t power = weapon->damage_function_params[level * 2 - 2 + ac_success];
    if ((move & ~7) == MoveType::JUMP_BASE) {
        power += unit1->badges_equipped.jumpman;
    } else if ((move & ~7) == MoveType::HAMMER_BASE) {
        power += unit1->badges_equipped.hammerman;
    }
    return power;
}

uint32_t GetWeaponPowerFromUnitWorkVariable(
    ttyd::battle_unit::BattleWorkUnit* unit1,
    ttyd::battle_database_common::BattleWeapon* weapon,
    ttyd::battle_unit::BattleWorkUnit* unit2,
    ttyd::battle_unit::BattleWorkUnitPart* part) {
    return unit1->unit_work[weapon->damage_function_params[0]];
}

EVT_DEFINE_USER_FUNC(evtTot_GetMoveSelectedLevel) {
    int32_t move = evtGetValue(evt, evt->evtArguments[0]);
    evtSetValue(
        evt, evt->evtArguments[1], MoveManager::GetSelectedLevel(move));
    return 2;
}

}  // namespace mod::tot