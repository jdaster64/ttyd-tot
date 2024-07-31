#include "tot_manager_cosmetics.h"

#include "tot_gsw.h"

#include "ttyd/battle.h"
#include "ttyd/icondrv.h"
#include "ttyd/item_data.h"
#include "ttyd/mario_pouch.h"
#include "ttyd/system.h"

#include <cstdint>

namespace mod::tot {

namespace {

namespace IconType = ttyd::icondrv::IconType;
namespace ItemType = ttyd::item_data::ItemType;

}

CosmeticData g_AttackFxData[] = {
    { 0, nullptr, nullptr },
    { IconType::ATTACK_FX_Y, "tot_cos0_01", "tot_cos0_01_h" },  // Ding
    { IconType::ATTACK_FX_G, "tot_cos0_02", "tot_cos0_02_h" },  // Frog
    { IconType::ATTACK_FX_B, "tot_cos0_03", "tot_cos0_03_h" },  // Squeak
    { IconType::ATTACK_FX_P, "tot_cos0_04", "tot_cos0_04_h" },  // Peach
    { IconType::ATTACK_FX_R, "tot_cos0_05", "tot_cos0_05_h" },  // Bowser
};

const CosmeticData* CosmeticsManager::GetData(int32_t type, int32_t id) {
    if (type == 0) return &g_AttackFxData[id];
    return nullptr;
}
    
int32_t CosmeticsManager::PickActiveFX() {
    int32_t sounds[5] = { 0 };
    int32_t num_sounds = 0;
    
    for (int32_t i = 1; i <= 30; ++i) {
        if (GetSWF(GSWF_AttackFxFlags + i)) {
            sounds[num_sounds++] = i;
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
        "SFX_MARIO_HAMMER_PIKKYO_Y1",   // Ding
        "SFX_MARIO_HAMMER_PIKKYO_R1",   // Frog
        "SFX_MARIO_HAMMER_PIKKYO_B1",   // Squeak
        "SFX_MARIO_HAMMER_PIKKYO_G1",   // Peach
        "SFX_MARIO_HAMMER_PIKKYO_P1",   // Bowser
    };
    return kFxNames[id];
}

}