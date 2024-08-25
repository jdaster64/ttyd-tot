#include "tot_manager_cosmetics.h"

#include "mod.h"
#include "tot_gsw.h"
#include "tot_state.h"
#include "tot_manager_achievements.h"

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

// For easier declaration of models' filepaths. (Thanks, Diagamma)
#define _MODEL_PATH_R(s, ...) \
    __VA_ARGS__ + 0 > 0 ? ("a/a_mario_" s "r") : ("a/a_mario_" s "_r")
#define MARIO_MODEL_PATHS(s, ...) \
	{"a/a_mario_" s, _MODEL_PATH_R(s, __VA_ARGS__), "a/b_mario_" s, "a/e_mario_" s}
#define YOSHI_MODEL_PATHS(s, ...) \
    {"a/c_babyyoshi" s, "a/EFF_m_yoshi" s, "a/c_tamag" s}

const CosmeticGroupData g_AttackFxGroupData[] = {
    // Not unlocked by achievements.
    { 0, nullptr },
    { IconType::ATTACK_FX_Y, "tot_cos0_01"},
    { IconType::ATTACK_FX_G, "tot_cos0_02"},
    { IconType::ATTACK_FX_B, "tot_cos0_03"},
    { IconType::ATTACK_FX_P, "tot_cos0_04"},
    { IconType::ATTACK_FX_R, "tot_cos0_05"},
    // Placeholders.
    { IconType::ATTACK_FX_Y, "tot_cos0_06_g" },
    { IconType::ATTACK_FX_G, "tot_cos0_07_g" },
    { IconType::ATTACK_FX_B, "tot_cos0_08_g" },
    { IconType::ATTACK_FX_P, "tot_cos0_09_g" },
    { IconType::ATTACK_FX_R, "tot_cos0_10_g" },
    { IconType::ATTACK_FX_Y, "tot_cos0_11_g" },
    { IconType::ATTACK_FX_G, "tot_cos0_12_g" },
    { IconType::ATTACK_FX_B, "tot_cos0_13_g" },
    { IconType::ATTACK_FX_P, "tot_cos0_14_g" },
};

const AttackFxData g_AttackFxData[] = {
    {
        .group_id = -1,
    },
    { 
        .name_msg = "tot_cos0_01", 
        .help_msg = "tot_cos0_01_h",
        .sounds = { "SFX_MARIO_HAMMER_PIKKYO_Y1" },
        .num_sounds = 1,
        .randomize_pitch = 1,
        .icon = IconType::ATTACK_FX_Y,
        .group_id = 0,
    },
    { 
        .name_msg = "tot_cos0_02", 
        .help_msg = "tot_cos0_02_h",
        .sounds = { "SFX_MARIO_HAMMER_PIKKYO_R1" },
        .num_sounds = 1,
        .randomize_pitch = 1,
        .icon = IconType::ATTACK_FX_G,
        .group_id = 0,
    },
    { 
        .name_msg = "tot_cos0_03", 
        .help_msg = "tot_cos0_03_h",
        .sounds = { "SFX_MARIO_HAMMER_PIKKYO_B1" },
        .num_sounds = 1,
        .randomize_pitch = 1,
        .icon = IconType::ATTACK_FX_B,
        .group_id = 0,
    },
    { 
        .name_msg = "tot_cos0_04", 
        .help_msg = "tot_cos0_04_h",
        .sounds = { "SFX_MARIO_HAMMER_PIKKYO_G1" },
        .num_sounds = 1,
        .randomize_pitch = 1,
        .icon = IconType::ATTACK_FX_P,
        .group_id = 0,
    },
    { 
        .name_msg = "tot_cos0_05", 
        .help_msg = "tot_cos0_05_h",
        .sounds = { "SFX_MARIO_HAMMER_PIKKYO_P1" },
        .num_sounds = 1,
        .randomize_pitch = 1,
        .icon = IconType::ATTACK_FX_R,
        .group_id = 0,
    },
};
const int32_t g_NumAttackFx = sizeof(g_AttackFxData) / sizeof(AttackFxData);

const CosmeticGroupData g_MarioCostumeGroupData[] = {
    // Not unlocked by achievements.
    { 0, nullptr },
    { IconType::MARIO_HEAD, "tot_cos1_01" },  // Mario
    { IconType::MARIO_HEAD, "tot_cos1_02" },  // Luigi
    { IconType::MARIO_HEAD, "tot_cos1_03" },  // Wario
    { IconType::MARIO_HEAD, "tot_cos1_04" },  // Waluigi
    // Placeholders.
    { IconType::MARIO_HEAD, "tot_cos1_05" },
    { IconType::MARIO_HEAD, "tot_cos1_06" },
    { IconType::MARIO_HEAD, "tot_cos1_07" },
    { IconType::MARIO_HEAD, "tot_cos1_08" },
    { IconType::MARIO_HEAD, "tot_cos1_09" },
    { IconType::MARIO_HEAD, "tot_cos1_10" },
    { IconType::MARIO_HEAD, "tot_cos1_11" },
    { IconType::MARIO_HEAD, "tot_cos1_12" },
    { IconType::MARIO_HEAD, "tot_cos1_13" },
    { IconType::MARIO_HEAD, "tot_cos1_14" },
    { IconType::MARIO_HEAD, "tot_cos1_15" },
    { IconType::MARIO_HEAD, "tot_cos1_16" },
    { IconType::MARIO_HEAD, "tot_cos1_17" },
    { IconType::MARIO_HEAD, "tot_cos1_18" },
    { IconType::MARIO_HEAD, "tot_cos1_19" },
    { IconType::MARIO_HEAD, "tot_cos1_20" },
    { IconType::MARIO_HEAD, "tot_cos1_21" },
    { IconType::MARIO_HEAD, "tot_cos1_22" },
    { IconType::MARIO_HEAD, "tot_cos1_23" },
    { IconType::MARIO_HEAD, "tot_cos1_24" },
    { IconType::MARIO_HEAD, "tot_cos1_25" },
    { IconType::MARIO_HEAD, "tot_cos1_26" },
};

const MarioCostumeData g_MarioCostumeData[] = {
    {
        .name_msg = "tot_cos1_01",
        .help_msg = "tot_cos1_01_h",
        .models = { "a/a_mario", "a/a_mario_r", "a/b_mario", "a/e_mario" },
        .icon = IconType::MARIO_HEAD,
        .group_id = 0,
    },
    {
        .name_msg = "tot_cos1_02",
        .help_msg = "tot_cos1_02_h",
        .models = MARIO_MODEL_PATHS("l", true),
        .icon = IconType::MARIO_HEAD,
        .group_id = 0,
    },
    {
        .name_msg = "tot_cos1_03",
        .help_msg = "tot_cos1_03_h",
        .models = MARIO_MODEL_PATHS("w", true),
        .icon = IconType::MARIO_HEAD,
        .group_id = 0,
    },
    {
        .name_msg = "tot_cos1_04",
        .help_msg = "tot_cos1_04_h",
        .models = MARIO_MODEL_PATHS("wl", true),
        .icon = IconType::MARIO_HEAD,
        .group_id = 0,
    },
};
const int32_t g_NumMarioCostumes =
    sizeof(g_MarioCostumeData) / sizeof(MarioCostumeData);

const CosmeticGroupData g_YoshiCostumeGroupData[] = {
    // Not unlocked by achievements.
    { 0, nullptr },
    { IconType::YOSHI_GREEN,    "tot_cos2_01" },
    { IconType::YOSHI_RED,      "tot_cos2_02" },
    { IconType::YOSHI_BLUE,     "tot_cos2_03" },
    { IconType::YOSHI_ORANGE,   "tot_cos2_04" },
    { IconType::YOSHI_PINK,     "tot_cos2_05" },
    // Placeholders.
    { IconType::YOSHI_BLACK,    "tot_cos2_06" },
    { IconType::YOSHI_WHITE,    "tot_cos2_07" },
    { IconType::YOSHI_GREEN,    "tot_cos2_08" },
    { IconType::YOSHI_GREEN,    "tot_cos2_09" },
    { IconType::YOSHI_GREEN,    "tot_cos2_10" },
    { IconType::YOSHI_GREEN,    "tot_cos2_11" },
    { IconType::YOSHI_GREEN,    "tot_cos2_12" },
    { IconType::YOSHI_GREEN,    "tot_cos2_13" },
    { IconType::YOSHI_GREEN,    "tot_cos2_14" },
    { IconType::YOSHI_GREEN,    "tot_cos2_15" },
    { IconType::YOSHI_GREEN,    "tot_cos2_16" },
    { IconType::YOSHI_GREEN,    "tot_cos2_17" },
    { IconType::YOSHI_GREEN,    "tot_cos2_18" },
    { IconType::YOSHI_GREEN,    "tot_cos2_19" },
    { IconType::YOSHI_GREEN,    "tot_cos2_20" },
    { IconType::YOSHI_GREEN,    "tot_cos2_21" },
};

const YoshiCostumeData g_YoshiCostumeData[] = {
    {
        .name_msg = "tot_cos2_01",
        .help_msg = "tot_cos2_01_h",
        .models = { "a/c_babyyoshi", "a/EFF_m_yoshi", "a/c_tamago" },
        .icon = IconType::YOSHI_GREEN,
        .icon_hud = IconType::HUD_YOSHI_GREEN,
        .group_id = 0,
    },
    {
        .name_msg = "tot_cos2_02",
        .help_msg = "tot_cos2_02_h",
        .models = YOSHI_MODEL_PATHS("2"),
        .icon = IconType::YOSHI_RED,
        .icon_hud = IconType::HUD_YOSHI_RED,
        .group_id = 0,
    },
    {
        .name_msg = "tot_cos2_03",
        .help_msg = "tot_cos2_03_h",
        .models = YOSHI_MODEL_PATHS("3"),
        .icon = IconType::YOSHI_BLUE,
        .icon_hud = IconType::HUD_YOSHI_BLUE,
        .group_id = 0,
    },
    {
        .name_msg = "tot_cos2_04",
        .help_msg = "tot_cos2_04_h",
        .models = YOSHI_MODEL_PATHS("4"),
        .icon = IconType::YOSHI_ORANGE,
        .icon_hud = IconType::HUD_YOSHI_ORANGE,
        .group_id = 0,
    },
    {
        .name_msg = "tot_cos2_05",
        .help_msg = "tot_cos2_05_h",
        .models = YOSHI_MODEL_PATHS("5"),
        .icon = IconType::YOSHI_PINK,
        .icon_hud = IconType::HUD_YOSHI_PINK,
        .group_id = 0,
    },
    {
        .name_msg = "tot_cos2_06",
        .help_msg = "tot_cos2_06_h",
        .models = YOSHI_MODEL_PATHS("6"),
        .icon = IconType::YOSHI_BLACK,
        .icon_hud = IconType::HUD_YOSHI_BLACK,
        .group_id = 0,
    },
    {
        .name_msg = "tot_cos2_07",
        .help_msg = "tot_cos2_07_h",
        .models = YOSHI_MODEL_PATHS("7"),
        .icon = IconType::YOSHI_WHITE,
        .icon_hud = IconType::HUD_YOSHI_WHITE,
        .group_id = 0,
    },
    {
        .name_msg = "tot_cos2_08",
        .help_msg = "tot_cos2_08_h",
        .models = YOSHI_MODEL_PATHS("H"),
        .icon = IconType::YOSHI_GREEN,
        .icon_hud = IconType::HUD_YOSHI_GREEN,
        .group_id = 0,
    },
};
const int32_t g_NumYoshiCostumes =
    sizeof(g_YoshiCostumeData) / sizeof(YoshiCostumeData);

const CosmeticGroupData* CosmeticsManager::GetGroupData(
    int32_t type, int32_t group_id) {
    switch (type) {
        case CosmeticType::ATTACK_FX:
            return &g_AttackFxGroupData[group_id];
        case CosmeticType::MARIO_COSTUME:
            return &g_MarioCostumeGroupData[group_id];
        case CosmeticType::YOSHI_COSTUME:
            return &g_YoshiCostumeGroupData[group_id];
    }
    return nullptr;
}

bool CosmeticsManager::IsEquipped(int32_t type, int32_t id) {
    switch (type) {
        case CosmeticType::ATTACK_FX:
            return GetSWF(GSWF_AttackFxFlags + id);
        case CosmeticType::MARIO_COSTUME:
            return GetSWByte(GSW_MarioCostume) == id;
        case CosmeticType::YOSHI_COSTUME:
            return GetSWF(GSWF_YoshiColors + id);
    }
    return false;
}

bool CosmeticsManager::ToggleEquipped(int32_t type, int32_t id) {
    switch (type) {
        case CosmeticType::ATTACK_FX:
            return ToggleSWF(GSWF_AttackFxFlags + id);
        case CosmeticType::MARIO_COSTUME:
            SetSWByte(GSW_MarioCostume, id);
            return true;
        case CosmeticType::YOSHI_COSTUME:
            return ToggleSWF(GSWF_YoshiColors + id);
    }
    return false;
}

void CosmeticsManager::MarkAsPurchased(int32_t type, int32_t id) {
    g_Mod->state_.SetOption(FLAGS_COSMETIC_PURCHASED, type * 32 + id);
}

bool CosmeticsManager::IsAvailable(int32_t type, int32_t id) {
    switch (type) {
        case CosmeticType::ATTACK_FX: {
            auto* data = GetAttackFxData(id);
            if (!data) return false;
            // TODO: For testing only!
            if (data->group_id == 0) return true;
            
            if (data->group_id == 0 || 
                AchievementsManager::CheckCosmeticGroupUnlocked(
                    AchievementRewardType::ATTACK_FX, data->group_id)) {
                return g_Mod->state_.GetOption(FLAGS_COSMETIC_PURCHASED, id);
            }
            break;
        }
        case CosmeticType::MARIO_COSTUME: {
            auto* data = GetMarioCostumeData(id);
            if (!data) return false;
            // TODO: For testing only!
            if (data->group_id == 0) return true;

            if (data->group_id == 0 || 
                AchievementsManager::CheckCosmeticGroupUnlocked(
                    AchievementRewardType::MARIO_COSTUME, data->group_id)) {
                return g_Mod->state_.GetOption(FLAGS_COSMETIC_PURCHASED, id + 32);
            }
            break;
        }
        case CosmeticType::YOSHI_COSTUME: {
            auto* data = GetYoshiCostumeData(id);
            if (!data) return false;
            // TODO: For testing only!
            if (data->group_id == 0) return true;
            
            if (data->group_id == 0 || 
                AchievementsManager::CheckCosmeticGroupUnlocked(
                    AchievementRewardType::YOSHI_COSTUME, data->group_id)) {
                return g_Mod->state_.GetOption(FLAGS_COSMETIC_PURCHASED, id + 64);
            }
            break;
        }
    }
    return false;
}

const AttackFxData* CosmeticsManager::GetAttackFxData(int32_t id) {
    // Attack FX id 0 is not used, since it's not compatible with the way
    // bits are set in battle.
    if (id < 1 || id >= g_NumAttackFx) return nullptr;
    return &g_AttackFxData[id];
}

const MarioCostumeData* CosmeticsManager::GetMarioCostumeData(int32_t id) {
    if (id < 0 || id >= g_NumMarioCostumes) return nullptr;
    return &g_MarioCostumeData[id];
}

const YoshiCostumeData* CosmeticsManager::GetYoshiCostumeData(int32_t id) {
    if (id < 0 || id >= g_NumYoshiCostumes) return nullptr;
    return &g_YoshiCostumeData[id];
}

int32_t CosmeticsManager::GetCosmeticCount(int32_t type) {
    switch (type) {
        case CosmeticType::ATTACK_FX:       return g_NumAttackFx;
        case CosmeticType::MARIO_COSTUME:   return g_NumMarioCostumes;
        case CosmeticType::YOSHI_COSTUME:   return g_NumYoshiCostumes;
    }
    return 0;
}
    
void CosmeticsManager::PickYoshiColor() {
    int32_t colors[30] = { 0 };
    int32_t num_colors = 0;
    int32_t selected_color = 0;
    
    for (int32_t i = 0; i < 30; ++i) {
        if (GetSWF(GSWF_YoshiColors + i)) {
            colors[num_colors++] = i;
        }
    }
    if (num_colors) {
        selected_color = colors[ttyd::system::irand(num_colors)];
    }
    ttyd::mario_pouch::pouchSetPartyColor(4, selected_color);
}
    
int32_t CosmeticsManager::PickActiveFX() {
    int32_t sounds[30] = { 0 };
    int32_t num_sounds = 0;
    
    for (int32_t i = 1; i < 30; ++i) {
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
    // TODO: Randomize, and also support random pitch.
    return g_AttackFxData[id].sounds[0];
}

}