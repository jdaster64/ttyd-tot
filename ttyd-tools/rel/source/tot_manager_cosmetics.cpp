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

const CosmeticData g_AttackFxData[] = {
    { 0, nullptr, nullptr },
    { IconType::ATTACK_FX_Y, "tot_cos0_01", "tot_cos0_01_h" },  // Ding
    { IconType::ATTACK_FX_G, "tot_cos0_02", "tot_cos0_02_h" },  // Frog
    { IconType::ATTACK_FX_B, "tot_cos0_03", "tot_cos0_03_h" },  // Squeak
    { IconType::ATTACK_FX_P, "tot_cos0_04", "tot_cos0_04_h" },  // Peach
    { IconType::ATTACK_FX_R, "tot_cos0_05", "tot_cos0_05_h" },  // Bowser
    // Placeholders.
    { IconType::ATTACK_FX_Y, "tot_cos0_06", "tot_cos0_06_h" },
    { IconType::ATTACK_FX_G, "tot_cos0_07", "tot_cos0_07_h" },
    { IconType::ATTACK_FX_B, "tot_cos0_08", "tot_cos0_08_h" },
    { IconType::ATTACK_FX_P, "tot_cos0_09", "tot_cos0_09_h" },
    { IconType::ATTACK_FX_R, "tot_cos0_10", "tot_cos0_10_h" },
    { IconType::ATTACK_FX_Y, "tot_cos0_11", "tot_cos0_11_h" },
    { IconType::ATTACK_FX_G, "tot_cos0_12", "tot_cos0_12_h" },
    { IconType::ATTACK_FX_B, "tot_cos0_13", "tot_cos0_13_h" },
    { IconType::ATTACK_FX_P, "tot_cos0_14", "tot_cos0_14_h" },
    { IconType::ATTACK_FX_R, "tot_cos0_15", "tot_cos0_15_h" },
};

const CosmeticData g_MarioCostumeData[] = {
    { 0, nullptr, nullptr },
    { IconType::MARIO_HEAD, "tot_cos1_01", "tot_cos1_01_h" },  // Mario
    { IconType::MARIO_HEAD, "tot_cos1_02", "tot_cos1_02_h" },  // Luigi
    { IconType::MARIO_HEAD, "tot_cos1_03", "tot_cos1_03_h" },  // Wario
    { IconType::MARIO_HEAD, "tot_cos1_04", "tot_cos1_04_h" },  // Waluigi
    // Placeholders.
    { IconType::MARIO_HEAD, "tot_cos1_05", "tot_cos1_05_h" },
    { IconType::MARIO_HEAD, "tot_cos1_06", "tot_cos1_06_h" },
    { IconType::MARIO_HEAD, "tot_cos1_07", "tot_cos1_07_h" },
    { IconType::MARIO_HEAD, "tot_cos1_08", "tot_cos1_08_h" },
    { IconType::MARIO_HEAD, "tot_cos1_09", "tot_cos1_09_h" },
    { IconType::MARIO_HEAD, "tot_cos1_10", "tot_cos1_10_h" },
    { IconType::MARIO_HEAD, "tot_cos1_11", "tot_cos1_11_h" },
    { IconType::MARIO_HEAD, "tot_cos1_12", "tot_cos1_12_h" },
    { IconType::MARIO_HEAD, "tot_cos1_13", "tot_cos1_13_h" },
    { IconType::MARIO_HEAD, "tot_cos1_14", "tot_cos1_14_h" },
    { IconType::MARIO_HEAD, "tot_cos1_15", "tot_cos1_15_h" },
    { IconType::MARIO_HEAD, "tot_cos1_16", "tot_cos1_16_h" },
    { IconType::MARIO_HEAD, "tot_cos1_17", "tot_cos1_17_h" },
    { IconType::MARIO_HEAD, "tot_cos1_18", "tot_cos1_18_h" },
    { IconType::MARIO_HEAD, "tot_cos1_19", "tot_cos1_19_h" },
    { IconType::MARIO_HEAD, "tot_cos1_20", "tot_cos1_20_h" },
    { IconType::MARIO_HEAD, "tot_cos1_21", "tot_cos1_21_h" },
    { IconType::MARIO_HEAD, "tot_cos1_22", "tot_cos1_22_h" },
    { IconType::MARIO_HEAD, "tot_cos1_23", "tot_cos1_23_h" },
    { IconType::MARIO_HEAD, "tot_cos1_24", "tot_cos1_24_h" },
    { IconType::MARIO_HEAD, "tot_cos1_25", "tot_cos1_25_h" },
    { IconType::MARIO_HEAD, "tot_cos1_26", "tot_cos1_26_h" },
};

const CosmeticData g_YoshiCostumeData[] = {
    { 0, nullptr, nullptr },
    { IconType::YOSHI_GREEN,    "tot_cos2_01", "tot_cos2_01_h" },
    { IconType::YOSHI_RED,      "tot_cos2_02", "tot_cos2_02_h" },
    { IconType::YOSHI_BLUE,     "tot_cos2_03", "tot_cos2_03_h" },
    { IconType::YOSHI_ORANGE,   "tot_cos2_04", "tot_cos2_04_h" },
    { IconType::YOSHI_PINK,     "tot_cos2_05", "tot_cos2_05_h" },
    // Placeholders.
    { IconType::YOSHI_BLACK,    "tot_cos2_06", "tot_cos2_06_h" },
    { IconType::YOSHI_WHITE,    "tot_cos2_07", "tot_cos2_07_h" },
    { IconType::YOSHI_GREEN,    "tot_cos2_08", "tot_cos2_08_h" },
    { IconType::YOSHI_GREEN,    "tot_cos2_09", "tot_cos2_09_h" },
    { IconType::YOSHI_GREEN,    "tot_cos2_10", "tot_cos2_10_h" },
    { IconType::YOSHI_GREEN,    "tot_cos2_11", "tot_cos2_11_h" },
    { IconType::YOSHI_GREEN,    "tot_cos2_12", "tot_cos2_12_h" },
    { IconType::YOSHI_GREEN,    "tot_cos2_13", "tot_cos2_13_h" },
    { IconType::YOSHI_GREEN,    "tot_cos2_14", "tot_cos2_14_h" },
    { IconType::YOSHI_GREEN,    "tot_cos2_15", "tot_cos2_15_h" },
    { IconType::YOSHI_GREEN,    "tot_cos2_16", "tot_cos2_16_h" },
    { IconType::YOSHI_GREEN,    "tot_cos2_17", "tot_cos2_17_h" },
    { IconType::YOSHI_GREEN,    "tot_cos2_18", "tot_cos2_18_h" },
    { IconType::YOSHI_GREEN,    "tot_cos2_19", "tot_cos2_19_h" },
    { IconType::YOSHI_GREEN,    "tot_cos2_20", "tot_cos2_20_h" },
};

const CosmeticData* CosmeticsManager::GetData(int32_t type, int32_t id) {
    switch (type) {
        case CosmeticType::ATTACK_FX:       return &g_AttackFxData[id];
        case CosmeticType::MARIO_COSTUME:   return &g_MarioCostumeData[id];
        case CosmeticType::YOSHI_COSTUME:   return &g_YoshiCostumeData[id];
    }
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