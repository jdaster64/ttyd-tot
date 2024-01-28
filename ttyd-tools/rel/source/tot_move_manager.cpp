#include "tot_move_manager.h"

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::tot {
    
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
    { "Move 5",     { 1, 1, 1, }, },
    { "Move 6",     { 1, 1, 1, }, },
    
    { "Shell Toss", { 0, 2, 4, }, },
    { "Power Sh.",  { 2, 4, 6, }, },
    { "Shell Shield", { 4, 4, 4, }, },
    { "Shell Slam", { 4, 6, 8, }, },
    { "Move 5",     { 1, 1, 1, }, },
    { "Move 6",     { 1, 1, 1, }, },
    
    { "Body Slam",  { 0, 2, 4, }, },
    { "Gale Force", { 4, 4, 4, }, },
    { "Lip Lock",   { 3, 5, 7, }, },
    { "Dodgy Fog",  { 4, 4, 4, }, },
    { "Move 5",     { 1, 1, 1, }, },
    { "Move 6",     { 1, 1, 1, }, },
    
    { "Ground P.",  { 0, 2, 4, }, },
    { "Gulp",       { 3, 5, 7, }, },
    { "Mini-Egg",   { 3, 5, 7, }, },
    { "Stampede",   { 4, 6, 8, }, },
    { "Move 5",     { 1, 1, 1, }, },
    { "Move 6",     { 1, 1, 1, }, },
    
    { "Shade Fist", { 0, 2, 4, }, },
    { "Veil",       { 1, 1, 1, }, },
    { "Fiery Jinx", { 3, 5, 7, }, },
    { "Infatuate",  { 3, 5, 7, }, },
    { "Move 5",     { 1, 1, 1, }, },
    { "Move 6",     { 1, 1, 1, }, },
    
    { "Bomb",       { 0, 2, 4, }, },
    { "Bomb Sq.",   { 3, 3, 3, }, },
    { "Hold Fast",  { 4, 4, 4, }, },
    { "Bob-ombast", { 4, 6, 8, }, },
    { "Move 5",     { 1, 1, 1, }, },
    { "Move 6",     { 1, 1, 1, }, },
    
    { "Love Slap",  { 0, 2, 4, }, },
    { "Kiss Thief", { 4, 4, 4, }, },
    { "Tease",      { 4, 4, 4, }, },
    { "Smooch",     { 3, 5, 7, }, },
    { "Move 5",     { 1, 1, 1, }, },
    { "Move 6",     { 1, 1, 1, }, },
}; 

void MoveManager::Init() {
    // TODO: Implement.
    for (int32_t i = 0; i < MoveType::MOVE_TYPE_MAX; ++i) {
        level_unlocked_[i] = 3;
        level_selected_[i] = 1;
    }
}

int32_t MoveManager::GetUnlockedLevel(int32_t move_type) const {
    return level_unlocked_[move_type];
}

int32_t MoveManager::GetSelectedLevel(int32_t move_type) const {
    return level_selected_[move_type];
}

int32_t MoveManager::GetMoveCost(int32_t move_type) const {
    return g_MoveData[move_type].move_cost[level_selected_[move_type]-1];
}

void MoveManager::GetCurrentSelectionString(
    int32_t move_type, char* out_buf) const {
    sprintf(
        out_buf, "%s Lv. %" PRId8,
        g_MoveData[move_type].name_abbreviated, level_selected_[move_type]);
}

bool MoveManager::ChangeSelectedLevel(int32_t move_type, int32_t change) {
    int32_t old_level = level_selected_[move_type];
    int32_t new_level = old_level + change;
    
    // TODO: Implement maxes.
    if (new_level < 1) new_level = 1;
    if (new_level > 3) new_level = 3;
    
    level_selected_[move_type] = new_level;
    return new_level != old_level;
}

void MoveManager::ResetSelectedLevels() {
    // TODO: Set to min level for all unlocked moves?
    MoveManager::Init();
}

bool MoveManager::IsUnlockable(int32_t move_type) const {
    // TODO: Implement dependencies.
    return false;
}

bool MoveManager::IsUpgradable(int32_t move_type) const {
    // TODO: Implement maxes.
    return false;
}

}  // namespace mod::tot