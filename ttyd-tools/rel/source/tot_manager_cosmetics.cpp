#include "tot_manager_cosmetics.h"

#include "tot_gsw.h"

#include "ttyd/battle.h"
#include "ttyd/item_data.h"
#include "ttyd/mario_pouch.h"
#include "ttyd/system.h"

#include <cstdint>

namespace mod::tot {

namespace {

namespace ItemType = ttyd::item_data::ItemType;

}
    
int32_t CosmeticsManager::PickActiveFX(bool in_battle) {
    int32_t sounds[5] = { 0 };
    int32_t num_sounds = 0;
    
    // TODO: Move to GSWF bits, set by a dialog from a new key item.
    if (in_battle) {
        uint32_t badge_flags = ttyd::battle::g_BattleWork->badge_equipped_flags;
        if (badge_flags & 0x100) {
            sounds[num_sounds++] = 1;
        }
        if (badge_flags & 0x200) {
            sounds[num_sounds++] = 2;
        }
        if (badge_flags & 0x400) {
            sounds[num_sounds++] = 3;
        }
        if (badge_flags & 0x800) {
            sounds[num_sounds++] = 4;
        }
        if (badge_flags & 0x1000) {
            sounds[num_sounds++] = 5;
        }
    } else {
        if (ttyd::mario_pouch::pouchEquipCheckBadge(ItemType::ATTACK_FX_R)) {
            sounds[num_sounds++] = 1;
        }
        if (ttyd::mario_pouch::pouchEquipCheckBadge(ItemType::ATTACK_FX_Y)) {
            sounds[num_sounds++] = 2;
        }
        if (ttyd::mario_pouch::pouchEquipCheckBadge(ItemType::ATTACK_FX_B)) {
            sounds[num_sounds++] = 3;
        }
        if (ttyd::mario_pouch::pouchEquipCheckBadge(ItemType::ATTACK_FX_G)) {
            sounds[num_sounds++] = 4;
        }
        if (ttyd::mario_pouch::pouchEquipCheckBadge(ItemType::ATTACK_FX_P)) {
            sounds[num_sounds++] = 5;
        }
    }
    if (num_sounds) {
        return sounds[ttyd::system::irand(num_sounds)];
    }
    return 0;
}

const char* CosmeticsManager::GetFXName(int32_t id) {
    constexpr const char* kFxNames[] = {
        nullptr,
        "SFX_MARIO_HAMMER_PIKKYO_R1",
        "SFX_MARIO_HAMMER_PIKKYO_Y1",
        "SFX_MARIO_HAMMER_PIKKYO_B1",
        "SFX_MARIO_HAMMER_PIKKYO_G1",
        "SFX_MARIO_HAMMER_PIKKYO_P1",
    };
    return kFxNames[id];
}

}