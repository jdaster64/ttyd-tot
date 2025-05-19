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
    // Unlockables.
    { IconType::ATTACK_FX_LIME,     "tot_cos0_06_g" },  // Group 1 - Animals
    { IconType::ATTACK_FX_G,        "tot_cos0_07_g" },  // Group 2 - Characters
    { IconType::ATTACK_FX_AQUA,     "tot_cos0_08_g" },  // Group 3 - Crowd
    { IconType::ATTACK_FX_B,        "tot_cos0_09_g" },  // Group 4 - Retro
    { IconType::ATTACK_FX_PURPLE,   "tot_cos0_10_g" },  // Group 5 - Cartoon 1
    { IconType::ATTACK_FX_FUCHSIA,  "tot_cos0_11_g" },  // Group 6 - Cartoon 2
    { IconType::ATTACK_FX_P,        "tot_cos0_12_g" },  // Group 7 - Cartoon 3
    { IconType::ATTACK_FX_R,        "tot_cos0_13_g" },  // Group 8 - Misc.
    { IconType::ATTACK_FX_BROWN,    "tot_cos0_14_g" },  // Group 9 - Peach, Bowser
};

const AttackFxData g_AttackFxData[] = {
    // Dummy 0th entry; never unlockable.
    {
        .group_id = 99,
    },
    // Group 0 (available by default):
    { 
        // Ding FX
        .name_msg = "tot_cos0_01", 
        .help_msg = "tot_cos0_01_h",
        .sounds = { "SFX_MARIO_HAMMER_PIKKYO_Y1" },
        .num_sounds = 1,
        .randomize_pitch = 1,
        .price = 3,
        .icon = IconType::ATTACK_FX_Y,
        .group_id = 0,
    },
    {
        // Froggy FX
        .name_msg = "tot_cos0_02", 
        .help_msg = "tot_cos0_02_h",
        .sounds = { "SFX_MARIO_HAMMER_PIKKYO_R1" },
        .num_sounds = 1,
        .randomize_pitch = 1,
        .price = 3,
        .icon = IconType::ATTACK_FX_Y,
        .group_id = 0,
    },
    {
        // Squeaky FX
        .name_msg = "tot_cos0_03", 
        .help_msg = "tot_cos0_03_h",
        .sounds = { "SFX_MARIO_HAMMER_PIKKYO_B1" },
        .num_sounds = 1,
        .randomize_pitch = 1,
        .price = 3,
        .icon = IconType::ATTACK_FX_Y,
        .group_id = 0,
    },
    // Group 1: Animals
    {
        // Tweet FX
        .name_msg = "tot_cos0_04", 
        .help_msg = "tot_cos0_04_h",
        .sounds = {
            "SFX_AMB_BIRD1",
            "SFX_AMB_BIRD2",
            "SFX_AMB_BIRD3",
        },
        .num_sounds = 3,
        .randomize_pitch = 0,
        .price = 5,
        .icon = IconType::ATTACK_FX_LIME,
        .group_id = 6,
    },
    {
        // Crow FX
        .name_msg = "tot_cos0_05", 
        .help_msg = "tot_cos0_05_h",
        .sounds = { "SFX_STG4_CROW_HOWL1" },
        .num_sounds = 1,
        .randomize_pitch = 1,
        .price = 5,
        .icon = IconType::ATTACK_FX_LIME,
        .group_id = 6,
    },
    {
        // Pig FX
        .name_msg = "tot_cos0_06", 
        .help_msg = "tot_cos0_06_h",
        .sounds = { "SFX_STG4_VOICE_PIG1" },
        .num_sounds = 1,
        .randomize_pitch = 1,
        .price = 5,
        .icon = IconType::ATTACK_FX_LIME,
        .group_id = 6,
    },
    // Group 2: Minor characters
    {
        // Smorg FX
        .name_msg = "tot_cos0_07", 
        .help_msg = "tot_cos0_07_h",
        .sounds = { "SFX_STG6_MOAMOA_VOICE1" },
        .num_sounds = 1,
        .randomize_pitch = 1,
        .price = 5,
        .icon = IconType::ATTACK_FX_G,
        .group_id = 7,
    },
    {
        // Doopliss FX
        .name_msg = "tot_cos0_08", 
        .help_msg = "tot_cos0_08_h",
        .sounds = { "SFX_BOSS_RNPL_LAUGH1" },
        .num_sounds = 1,
        .randomize_pitch = 1,
        .price = 5,
        .icon = IconType::ATTACK_FX_G,
        .group_id = 7,
    },
    {
        // Bandit FX
        .name_msg = "tot_cos0_09", 
        .help_msg = "tot_cos0_09_h",
        .sounds = { "SFX_ENM_BORODO_LAUGH1" },
        .num_sounds = 1,
        .randomize_pitch = 1,
        .price = 5,
        .icon = IconType::ATTACK_FX_G,
        .group_id = 7,
    },
    // Group 3: Crowd noises
    {
        // Scream FX
        .name_msg = "tot_cos0_10",
        .help_msg = "tot_cos0_10_h",
        .sounds = { "SFX_AUDIENCE_PANIC_SCREAM2" },
        .num_sounds = 1,
        .randomize_pitch = 1,
        .price = 7,
        .icon = IconType::ATTACK_FX_AQUA,
        .group_id = 8,
    },
    {
        // Whistle FX
        .name_msg = "tot_cos0_11",
        .help_msg = "tot_cos0_11_h",
        .sounds = {
            "SFX_AUDIENCE_WHISTLE1",
            "SFX_AUDIENCE_WHISTLE2",
            "SFX_AUDIENCE_WHISTLE3",
        },
        .num_sounds = 3,
        .randomize_pitch = 0,
        .price = 7,
        .icon = IconType::ATTACK_FX_AQUA,
        .group_id = 8,
    },
    // Group 4: Digital stuff
    {
        // Techy FX
        .name_msg = "tot_cos0_12", 
        .help_msg = "tot_cos0_12_h",
        .sounds = {
            "SFX_PEACH_MAIL_SOUSIN2",
            "SFX_PEACH_SCREEN_OPEN1",
            "SFX_PEACH_ELEVATOR_ACCESS1",
        },
        .num_sounds = 3,
        .randomize_pitch = 0,
        .price = 7,
        .icon = IconType::ATTACK_FX_B,
        .group_id = 9,
    },
    {
        // Retro FX
        .name_msg = "tot_cos0_13",
        .help_msg = "tot_cos0_13_h",
        .sounds = {
            "SFX_PEACH_DISC_SET1",
            "SFX_ITEM_RANK_UP_ENEMY1",
            "SFX_MOBJ_JUMP_STAND1",
            "SFX_ENM_FIREB_ATTACK1",
            "SFX_MESSAGE_SAVE_OK1",
        },
        .num_sounds = 5,
        .randomize_pitch = 0,
        .price = 7,
        .icon = IconType::ATTACK_FX_B,
        .group_id = 9,
    },
    {
        // 1-UP FX
        .name_msg = "tot_cos0_14",
        .help_msg = "tot_cos0_14_h",
        .sounds = { "SFX_BTL_LVUP_BLOCK_HIT1" },
        .num_sounds = 1,
        .randomize_pitch = 0,
        .price = 7,
        .icon = IconType::ATTACK_FX_B,
        .group_id = 9,
    },
    // Group 5: Cartoon sounds 1
    {
        // Pop FX
        .name_msg = "tot_cos0_15",
        .help_msg = "tot_cos0_15_h",
        .sounds = {
            "SFX_BTL_CHURINA_CONFUSE1",
            "SFX_ENM_3RD1_BOTTLE5",
            "SFX_ENM_SAMBO_SEND_GROUP3",
        },
        .num_sounds = 3,
        .randomize_pitch = 0,
        .price = 7,
        .icon = IconType::ATTACK_FX_PURPLE,
        .group_id = 10,
    },
    {
        // Zip FX
        .name_msg = "tot_cos0_16",
        .help_msg = "tot_cos0_16_h",
        .sounds = {
            "SFX_ENM_PANSY_ESCAPE2",
            "SFX_ENM_BORODO_ESCAPE1",
            "SFX_ENM_JUGEM_THROW1",
        },
        .num_sounds = 3,
        .randomize_pitch = 0,
        .price = 7,
        .icon = IconType::ATTACK_FX_PURPLE,
        .group_id = 10,
    },
    // Group 6: Cartoon sounds 2
    {
        // Boom FX
        .name_msg = "tot_cos0_17",
        .help_msg = "tot_cos0_17_h",
        .sounds = { "SFX_STG5_CANNON_FIRING1" },
        .num_sounds = 1,
        .randomize_pitch = 1,
        .price = 7,
        .icon = IconType::ATTACK_FX_FUCHSIA,
        .group_id = 11,
    },
    {
        // Boing FX
        .name_msg = "tot_cos0_18",
        .help_msg = "tot_cos0_18_h",
        .sounds = {
            "SFX_STG6_OBJ_SWITCH_APPEAR1",
            "SFX_BTL_CLAUD_ATTACK2",
            "SFX_STG3_AIRDUCT_SHAKE1",
        },
        .num_sounds = 3,
        .randomize_pitch = 0,
        .price = 7,
        .icon = IconType::ATTACK_FX_FUCHSIA,
        .group_id = 11,
    },
    {
        // Crash FX
        .name_msg = "tot_cos0_19",
        .help_msg = "tot_cos0_19_h",
        .sounds = {
            "SFX_STG6_SWITCH_MOVE1",
            "SFX_MOBJ_BLOCK_OJAMA1",
            "SFX_BTL_SAC_ZUBA_STAR3",
        },
        .num_sounds = 3,
        .randomize_pitch = 0,
        .price = 7,
        .icon = IconType::ATTACK_FX_FUCHSIA,
        .group_id = 11,
    },
    // Group 7: Cartoon sounds 3
    {
        // Punch FX
        .name_msg = "tot_cos0_20",
        .help_msg = "tot_cos0_20_h",
        .sounds = { "SFX_BOSS_GANSU_FIST1" },
        .num_sounds = 1,
        .randomize_pitch = 1,
        .price = 10,
        .icon = IconType::ATTACK_FX_P,
        .group_id = 12,
    },
    {
        // Cartoon FX
        .name_msg = "tot_cos0_21",
        .help_msg = "tot_cos0_21_h",
        .sounds = {
            "SFX_BTL_ACROBAT_MISS1",
            "SFX_ENM_JUGEM_HOLD1",
            "SFX_HIT_TREE1",
            "SFX_BTL_STAGE_DAMAGE_FORK1",
        },
        .num_sounds = 4,
        .randomize_pitch = 0,
        .price = 10,
        .icon = IconType::ATTACK_FX_P,
        .group_id = 12,
    },
    {
        // Raspberry FX
        .name_msg = "tot_cos0_22",
        .help_msg = "tot_cos0_22_h",
        .sounds = { "SFX_STG5_SAILER_FART1" },
        .num_sounds = 1,
        .randomize_pitch = 1,
        .price = 10,
        .icon = IconType::ATTACK_FX_P,
        .group_id = 12,
    },
    // Group 8: Miscellaneous
    {
        // Jingle FX
        .name_msg = "tot_cos0_23",
        .help_msg = "tot_cos0_23_h",
        .sounds = {
            "SFX_QUIZ_START1",
            "SFX_STG6_DOUBTFUL_POINT1",
        },
        .num_sounds = 2,
        .randomize_pitch = 0,
        .price = 10,
        .icon = IconType::ATTACK_FX_R,
        .group_id = 13,
    },
    {
        // Crystal FX
        .name_msg = "tot_cos0_24",
        .help_msg = "tot_cos0_24_h",
        .sounds = {
            "SFX_STG8_STAR_LIGHTUP2",
            "SFX_KUPPA_STONE_BREAK1",
            "SFX_STG6_ELEVATOR_BELL1",
        },
        .num_sounds = 3,
        .randomize_pitch = 0,
        .price = 10,
        .icon = IconType::ATTACK_FX_R,
        .group_id = 13,
    },
    {
        // Warp FX
        .name_msg = "tot_cos0_25",
        .help_msg = "tot_cos0_25_h",
        .sounds = { "SFX_STG7_WARP2" },
        .num_sounds = 1,
        .randomize_pitch = 1,
        .price = 10,
        .icon = IconType::ATTACK_FX_R,
        .group_id = 13,
    },
    // Group 9: Peach & Bowser
    {
        .name_msg = "tot_cos0_26", 
        .help_msg = "tot_cos0_26_h",
        .sounds = {
            "SFX_MARIO_HAMMER_PIKKYO_G1",
            "SFX_VOICE_PEACH_HAPPY1_1",
            "SFX_VOICE_PEACH_QUESTION1",
            "SFX_VOICE_PEACH_LAUGH2_1",
        },
        .num_sounds = 4,
        .randomize_pitch = 0,
        .price = 15,
        .icon = IconType::ATTACK_FX_BROWN,
        .group_id = 14,
    },
    { 
        .name_msg = "tot_cos0_27",
        .help_msg = "tot_cos0_27_h",
        .sounds = {
            "SFX_MARIO_HAMMER_PIKKYO_P1",
            "SFX_VOICE_KOOPA_ANGRY2_1",
            "SFX_VOICE_KOOPA_LAUGH1_2",
            "SFX_VOICE_KOOPA_HAPPY1_1",
            "SFX_VOICE_KOOPA_SURPRISED1_1",
        },
        .num_sounds = 5,
        .randomize_pitch = 0,
        .price = 15,
        .icon = IconType::ATTACK_FX_BROWN,
        .group_id = 14,
    },
};
const int32_t g_NumAttackFx = sizeof(g_AttackFxData) / sizeof(AttackFxData);

const CosmeticGroupData g_MarioCostumeGroupData[] = {
    // Not unlocked by achievements.
    { 0, nullptr },
    { IconType::MARIO_COSTUME_ICONS +  0,   "tot_cos1_01" },  // Mario
    { IconType::MARIO_COSTUME_ICONS +  1,   "tot_cos1_02" },  // Luigi
    { IconType::MARIO_COSTUME_ICONS +  2,   "tot_cos1_03" },  // Wario
    { IconType::MARIO_COSTUME_ICONS +  3,   "tot_cos1_04" },  // Waluigi
    // Unlockables.
    { IconType::MARIO_COSTUME_ICONS +  4,   "tot_cos1_05" },
    { IconType::MARIO_COSTUME_ICONS +  5,   "tot_cos1_06" },
    { IconType::MARIO_COSTUME_ICONS +  6,   "tot_cos1_07" },
    { IconType::MARIO_COSTUME_ICONS +  7,   "tot_cos1_08" },
    { IconType::MARIO_COSTUME_ICONS +  8,   "tot_cos1_09" },
    { IconType::MARIO_COSTUME_ICONS +  9,   "tot_cos1_10" },
    { IconType::MARIO_COSTUME_ICONS + 10,   "tot_cos1_11" },
    { IconType::MARIO_COSTUME_ICONS + 11,   "tot_cos1_12" },
    { IconType::MARIO_COSTUME_ICONS + 12,   "tot_cos1_13" },
    { IconType::MARIO_COSTUME_ICONS + 13,   "tot_cos1_14" },
    { IconType::MARIO_COSTUME_ICONS + 14,   "tot_cos1_15" },
    { IconType::MARIO_COSTUME_ICONS + 15,   "tot_cos1_16" },
    { IconType::MARIO_COSTUME_ICONS + 16,   "tot_cos1_17" },
    { IconType::MARIO_COSTUME_ICONS + 17,   "tot_cos1_18" },
    { IconType::MARIO_COSTUME_ICONS + 18,   "tot_cos1_19" },
    { IconType::MARIO_COSTUME_ICONS + 19,   "tot_cos1_20" },
    { IconType::MARIO_COSTUME_ICONS + 20,   "tot_cos1_21" },
    { IconType::MARIO_COSTUME_ICONS + 21,   "tot_cos1_22" },
    { IconType::MARIO_COSTUME_ICONS + 22,   "tot_cos1_23" },
    { IconType::MARIO_COSTUME_ICONS + 23,   "tot_cos1_24" },
    { IconType::MARIO_COSTUME_ICONS + 24,   "tot_cos1_25" },
    { IconType::MARIO_COSTUME_ICONS + 25,   "tot_cos1_26" },
    { IconType::MARIO_COSTUME_26,           "tot_cos1_27" },
    { IconType::MARIO_COSTUME_27,           "tot_cos1_28" },
};

const MarioCostumeData g_MarioCostumeData[] = {
    {
        // Default (always unlocked).
        .name_msg = "tot_cos1_01",
        .help_msg = "tot_cos1_01_h",
        .models = { "a/a_mario", "a/a_mario_r", "a/b_mario", "a/e_mario" },
        .icon = IconType::MARIO_COSTUME_ICONS + 0,
        .group_id = -1,
        .sort_order = 1,
    },
    {
        // Luigi
        .name_msg = "tot_cos1_02",
        .help_msg = "tot_cos1_02_h",
        .models = MARIO_MODEL_PATHS("l", true),
        .icon = IconType::MARIO_COSTUME_ICONS + 1,
        .group_id = 0,
        .sort_order = 2,
    },
    {
        // Wario
        .name_msg = "tot_cos1_03",
        .help_msg = "tot_cos1_03_h",
        .models = MARIO_MODEL_PATHS("w", true),
        .icon = IconType::MARIO_COSTUME_ICONS + 2,
        .group_id = 0,
        .sort_order = 3,
    },
    {
        // Waluigi
        .name_msg = "tot_cos1_04",
        .help_msg = "tot_cos1_04_h",
        .models = MARIO_MODEL_PATHS("wl", true),
        .icon = IconType::MARIO_COSTUME_ICONS + 3,
        .group_id = 0,
        .sort_order = 4,
    },
    {
        // Fire Mario
        .name_msg = "tot_cos1_05",
        .help_msg = "tot_cos1_05_h",
        .models = MARIO_MODEL_PATHS("A", true),
        .price = 7,
        .icon = IconType::MARIO_COSTUME_ICONS + 4,
        .group_id = 5,
        .sort_order = 5,
    },
    {
        // Ice Mario
        .name_msg = "tot_cos1_06",
        .help_msg = "tot_cos1_06_h",
        .models = MARIO_MODEL_PATHS("B", true),
        .price = 7,
        .icon = IconType::MARIO_COSTUME_ICONS + 5,
        .group_id = 6,
        .sort_order = 6,
    },
    {
        // Bubble Mario
        .name_msg = "tot_cos1_07",
        .help_msg = "tot_cos1_07_h",
        .models = MARIO_MODEL_PATHS("C", true),
        .price = 7,
        .icon = IconType::MARIO_COSTUME_ICONS + 6,
        .group_id = 7,
        .sort_order = 7,
    },
    {
        // Superball Mario
        .name_msg = "tot_cos1_08",
        .help_msg = "tot_cos1_08_h",
        .models = MARIO_MODEL_PATHS("D", true),
        .price = 7,
        .icon = IconType::MARIO_COSTUME_ICONS + 7,
        .group_id = 8,
        .sort_order = 8,
    },
    {
        // Flying Mario
        .name_msg = "tot_cos1_09",
        .help_msg = "tot_cos1_09_h",
        .models = MARIO_MODEL_PATHS("E", true),
        .price = 10,
        .icon = IconType::MARIO_COSTUME_ICONS + 8,
        .group_id = 9,
        .sort_order = 9,
    },
    {
        // Classic (Mario Bros.) Mario
        .name_msg = "tot_cos1_10",
        .help_msg = "tot_cos1_10_h",
        .models = MARIO_MODEL_PATHS("F", true),
        .price = 10,
        .icon = IconType::MARIO_COSTUME_ICONS + 9,
        .group_id = 10,
        .sort_order = 10,
    },
    {
        // Classic (Super Mario Bros.) Luigi
        .name_msg = "tot_cos1_11",
        .help_msg = "tot_cos1_11_h",
        .models = MARIO_MODEL_PATHS("G", true),
        .price = 10,
        .icon = IconType::MARIO_COSTUME_ICONS + 10,
        .group_id = 11,
        .sort_order = 11,
    },
    {
        // SMB1 Mario
        .name_msg = "tot_cos1_12",
        .help_msg = "tot_cos1_12_h",
        .models = MARIO_MODEL_PATHS("H", true),
        .price = 10,
        .icon = IconType::MARIO_COSTUME_ICONS + 11,
        .group_id = 12,
        .sort_order = 12,
    },
    {
        // SMB Deluxe Luigi
        .name_msg = "tot_cos1_13",
        .help_msg = "tot_cos1_13_h",
        .models = MARIO_MODEL_PATHS("I", true),
        .price = 10,
        .icon = IconType::MARIO_COSTUME_ICONS + 12,
        .group_id = 13,
        .sort_order = 13,
    },
    {
        // SMB3
        .name_msg = "tot_cos1_14",
        .help_msg = "tot_cos1_14_h",
        .models = MARIO_MODEL_PATHS("J", true),
        .price = 10,
        .icon = IconType::MARIO_COSTUME_ICONS + 13,
        .group_id = 14,
        .sort_order = 14,
    },
    {
        // Super Mario World
        .name_msg = "tot_cos1_15",
        .help_msg = "tot_cos1_15_h",
        .models = MARIO_MODEL_PATHS("K", true),
        .price = 10,
        .icon = IconType::MARIO_COSTUME_ICONS + 14,
        .group_id = 15,
        .sort_order = 15,
    },
    {
        // Mario Golf 1
        .name_msg = "tot_cos1_16",
        .help_msg = "tot_cos1_16_h",
        .models = MARIO_MODEL_PATHS("M", true),
        .price = 10,
        .icon = IconType::MARIO_COSTUME_ICONS + 15,
        .group_id = 16,
        .sort_order = 16,
    },
    {
        // Mario Golf 2
        .name_msg = "tot_cos1_17",
        .help_msg = "tot_cos1_17_h",
        .models = MARIO_MODEL_PATHS("N", true),
        .price = 10,
        .icon = IconType::MARIO_COSTUME_ICONS + 16,
        .group_id = 17,
        .sort_order = 17,
    },
    {
        // Super Smash Bros. 1
        .name_msg = "tot_cos1_18",
        .help_msg = "tot_cos1_18_h",
        .models = MARIO_MODEL_PATHS("O", true),
        .price = 10,
        .icon = IconType::MARIO_COSTUME_ICONS + 17,
        .group_id = 18,
        .sort_order = 18,
    },
    {
        // Super Smash Bros. 2
        .name_msg = "tot_cos1_19",
        .help_msg = "tot_cos1_19_h",
        .models = MARIO_MODEL_PATHS("P", true),
        .price = 10,
        .icon = IconType::MARIO_COSTUME_ICONS + 18,
        .group_id = 19,
        .sort_order = 19,
    },
    {
        // Mario Maker
        .name_msg = "tot_cos1_20",
        .help_msg = "tot_cos1_20_h",
        .models = MARIO_MODEL_PATHS("Q", true),
        .price = 15,
        .icon = IconType::MARIO_COSTUME_ICONS + 19,
        .group_id = 20,
        .sort_order = 20,
    },
    {
        // Toadette
        .name_msg = "tot_cos1_21",
        .help_msg = "tot_cos1_21_h",
        .models = MARIO_MODEL_PATHS("S", true),
        .price = 15,
        .icon = IconType::MARIO_COSTUME_ICONS + 20,
        .group_id = 21,
        .sort_order = 24,
    },
    {
        // Secret
        .name_msg = "tot_cos1_22",
        .help_msg = "tot_cos1_22_h",
        .models = MARIO_MODEL_PATHS("T", true),
        .price = 15,
        .icon = IconType::MARIO_COSTUME_ICONS + 21,
        .secret = true,
        .group_id = 22,
        .sort_order = 28,
    },
    {
        // Shadowy
        .name_msg = "tot_cos1_23",
        .help_msg = "tot_cos1_23_h",
        .models = MARIO_MODEL_PATHS("U", true),
        .price = 25,
        .icon = IconType::MARIO_COSTUME_ICONS + 22,
        .secret = true,
        .group_id = 23,
        .sort_order = 27,
    },
    {
        // Silver
        .name_msg = "tot_cos1_24",
        .help_msg = "tot_cos1_24_h",
        .models = MARIO_MODEL_PATHS("X", true),
        .price = 25,
        .icon = IconType::MARIO_COSTUME_ICONS + 23,
        .group_id = 24,
        .sort_order = 29,
    },
    {
        // Gold
        .name_msg = "tot_cos1_25",
        .help_msg = "tot_cos1_25_h",
        .models = MARIO_MODEL_PATHS("Y", true),
        .price = 25,
        .icon = IconType::MARIO_COSTUME_ICONS + 24,
        .group_id = 25,
        .sort_order = 30,
    },
    {
        // Platinum
        .name_msg = "tot_cos1_26",
        .help_msg = "tot_cos1_26_h",
        .models = MARIO_MODEL_PATHS("Z", true),
        .price = 25,
        .icon = IconType::MARIO_COSTUME_ICONS + 25,
        .secret = true,
        .group_id = 26,
        .sort_order = 31,
    },
    {
        // Game Boy
        .name_msg = "tot_cos1_27",
        .help_msg = "tot_cos1_27_h",
        .models = MARIO_MODEL_PATHS("6", true),
        .price = 20,
        .icon = IconType::MARIO_COSTUME_26,
        .group_id = 27,
        .sort_order = 25,
    },
    {
        // 3D World unused Super Guide
        .name_msg = "tot_cos1_28",
        .help_msg = "tot_cos1_28_h",
        .models = MARIO_MODEL_PATHS("7", true),
        .price = 20,
        .icon = IconType::MARIO_COSTUME_27,
        .group_id = 28,
        .sort_order = 22,
    },
    // Wario's Woods: sort order 21
    // Foreman Spike: sort order 23
    // Virtual Boy:   sort order 26
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
    // Unlockables.
    { IconType::YOSHI_BLACK,                "tot_cos2_06" },
    { IconType::YOSHI_WHITE,                "tot_cos2_07" },
    { IconType::YOSHI_EXTRA_ICONS +  0,     "tot_cos2_08" },
    { IconType::YOSHI_EXTRA_ICONS +  1,     "tot_cos2_09" },
    { IconType::YOSHI_EXTRA_ICONS +  2,     "tot_cos2_10" },
    { IconType::YOSHI_EXTRA_ICONS +  3,     "tot_cos2_11" },
    { IconType::YOSHI_EXTRA_ICONS +  4,     "tot_cos2_12" },
    { IconType::YOSHI_EXTRA_ICONS +  5,     "tot_cos2_13" },
    { IconType::YOSHI_EXTRA_ICONS +  6,     "tot_cos2_14" },
    { IconType::YOSHI_EXTRA_ICONS +  7,     "tot_cos2_15" },
    { IconType::YOSHI_EXTRA_ICONS +  8,     "tot_cos2_16" },
    { IconType::YOSHI_EXTRA_ICONS +  9,     "tot_cos2_17" },
    { IconType::YOSHI_EXTRA_ICONS + 10,     "tot_cos2_18" },
    { IconType::YOSHI_EXTRA_ICONS + 11,     "tot_cos2_19" },
    { IconType::YOSHI_EXTRA_ICONS + 12,     "tot_cos2_20" },
    { IconType::YOSHI_EXTRA_ICONS + 13,     "tot_cos2_21" },
    { IconType::YOSHI_COSTUME_21,           "tot_cos2_22" },
    { IconType::YOSHI_COSTUME_22,           "tot_cos2_23" },
    { IconType::YOSHI_COSTUME_23,           "tot_cos2_24" },
    { IconType::YOSHI_COSTUME_24,           "tot_cos2_25" },
};

const YoshiCostumeData g_YoshiCostumeData[] = {
    {
        // Green (always unlocked).
        .name_msg = "tot_cos2_01",
        .help_msg = "tot_cos2_01_h",
        .models = { "a/c_babyyoshi", "a/EFF_m_yoshi", "a/c_tamago" },
        .icon = IconType::YOSHI_GREEN,
        .icon_hud = IconType::HUD_YOSHI_GREEN,
        .group_id = -1,
        .sort_order = 1,
    },
    {
        // Red
        .name_msg = "tot_cos2_02",
        .help_msg = "tot_cos2_02_h",
        .models = YOSHI_MODEL_PATHS("2"),
        .icon = IconType::YOSHI_RED,
        .icon_hud = IconType::HUD_YOSHI_RED,
        .group_id = 0,
        .sort_order = 2,
    },
    {
        // Blue
        .name_msg = "tot_cos2_03",
        .help_msg = "tot_cos2_03_h",
        .models = YOSHI_MODEL_PATHS("3"),
        .icon = IconType::YOSHI_BLUE,
        .icon_hud = IconType::HUD_YOSHI_BLUE,
        .group_id = 0,
        .sort_order = 3,
    },
    {
        // Orange
        .name_msg = "tot_cos2_04",
        .help_msg = "tot_cos2_04_h",
        .models = YOSHI_MODEL_PATHS("4"),
        .icon = IconType::YOSHI_ORANGE,
        .icon_hud = IconType::HUD_YOSHI_ORANGE,
        .group_id = 0,
        .sort_order = 4,
    },
    {
        // Pink
        .name_msg = "tot_cos2_05",
        .help_msg = "tot_cos2_05_h",
        .models = YOSHI_MODEL_PATHS("5"),
        .icon = IconType::YOSHI_PINK,
        .icon_hud = IconType::HUD_YOSHI_PINK,
        .group_id = 0,
        .sort_order = 5,
    },
    {
        // Black
        .name_msg = "tot_cos2_06",
        .help_msg = "tot_cos2_06_h",
        .models = YOSHI_MODEL_PATHS("6"),
        .price = 10,
        .icon = IconType::YOSHI_BLACK,
        .icon_hud = IconType::HUD_YOSHI_BLACK,
        .group_id = 6,
        .sort_order = 6,
    },
    {
        // White
        .name_msg = "tot_cos2_07",
        .help_msg = "tot_cos2_07_h",
        .models = YOSHI_MODEL_PATHS("7"),
        .price = 10,
        .icon = IconType::YOSHI_WHITE,
        .icon_hud = IconType::HUD_YOSHI_WHITE,
        .group_id = 7,
        .sort_order = 7,
    },
    {
        // Brown
        .name_msg = "tot_cos2_08",
        .help_msg = "tot_cos2_08_h",
        .models = YOSHI_MODEL_PATHS("H"),
        .price = 10,
        .icon = IconType::YOSHI_EXTRA_ICONS + 0,
        .icon_hud = IconType::YOSHI_EXTRA_HUD_ICONS + 0,
        .group_id = 8,
        .sort_order = 24,
    },
    {
        // Scarlet
        .name_msg = "tot_cos2_09",
        .help_msg = "tot_cos2_09_h",
        .models = YOSHI_MODEL_PATHS("I"),
        .price = 10,
        .icon = IconType::YOSHI_EXTRA_ICONS + 1,
        .icon_hud = IconType::YOSHI_EXTRA_HUD_ICONS + 1,
        .group_id = 9,
        .sort_order = 8,
    },
    {
        // Yellow
        .name_msg = "tot_cos2_10",
        .help_msg = "tot_cos2_10_h",
        .models = YOSHI_MODEL_PATHS("J"),
        .price = 10,
        .icon = IconType::YOSHI_EXTRA_ICONS + 2,
        .icon_hud = IconType::YOSHI_EXTRA_HUD_ICONS + 2,
        .group_id = 10,
        .sort_order = 12,
    },
    {
        // Lime
        .name_msg = "tot_cos2_11",
        .help_msg = "tot_cos2_11_h",
        .models = YOSHI_MODEL_PATHS("K"),
        .price = 10,
        .icon = IconType::YOSHI_EXTRA_ICONS + 3,
        .icon_hud = IconType::YOSHI_EXTRA_HUD_ICONS + 3,
        .group_id = 11,
        .sort_order = 13,
    },
    {
        // Teal
        .name_msg = "tot_cos2_12",
        .help_msg = "tot_cos2_12_h",
        .models = YOSHI_MODEL_PATHS("L"),
        .price = 10,
        .icon = IconType::YOSHI_EXTRA_ICONS + 4,
        .icon_hud = IconType::YOSHI_EXTRA_HUD_ICONS + 4,
        .group_id = 12,
        .sort_order = 15,
    },
    {
        // Indigo
        .name_msg = "tot_cos2_13",
        .help_msg = "tot_cos2_13_h",
        .models = YOSHI_MODEL_PATHS("M"),
        .price = 10,
        .icon = IconType::YOSHI_EXTRA_ICONS + 5,
        .icon_hud = IconType::YOSHI_EXTRA_HUD_ICONS + 5,
        .group_id = 13,
        .sort_order = 18,
    },
    {
        // Purple
        .name_msg = "tot_cos2_14",
        .help_msg = "tot_cos2_14_h",
        .models = YOSHI_MODEL_PATHS("N"),
        .price = 10,
        .icon = IconType::YOSHI_EXTRA_ICONS + 6,
        .icon_hud = IconType::YOSHI_EXTRA_HUD_ICONS + 6,
        .group_id = 14,
        .sort_order = 19,
    },
    {
        // Lilac
        .name_msg = "tot_cos2_15",
        .help_msg = "tot_cos2_15_h",
        .models = YOSHI_MODEL_PATHS("P"),
        .price = 10,
        .icon = IconType::YOSHI_EXTRA_ICONS + 7,
        .icon_hud = IconType::YOSHI_EXTRA_HUD_ICONS + 7,
        .group_id = 15,
        .sort_order = 20,
    },
    {
        // Fuchsia
        .name_msg = "tot_cos2_16",
        .help_msg = "tot_cos2_16_h",
        .models = YOSHI_MODEL_PATHS("Q"),
        .price = 10,
        .icon = IconType::YOSHI_EXTRA_ICONS + 8,
        .icon_hud = IconType::YOSHI_EXTRA_HUD_ICONS + 8,
        .group_id = 16,
        .sort_order = 22,
    },
    {
        // Grey
        .name_msg = "tot_cos2_17",
        .help_msg = "tot_cos2_17_h",
        .models = YOSHI_MODEL_PATHS("R"),
        .price = 10,
        .icon = IconType::YOSHI_EXTRA_ICONS + 9,
        .icon_hud = IconType::YOSHI_EXTRA_HUD_ICONS + 9,
        .group_id = 17,
        .sort_order = 26,
    },
    {
        // Secret
        .name_msg = "tot_cos2_18",
        .help_msg = "tot_cos2_18_h",
        .models = YOSHI_MODEL_PATHS("S"),
        .price = 15,
        .icon = IconType::YOSHI_EXTRA_ICONS + 10,
        .icon_hud = IconType::YOSHI_EXTRA_HUD_ICONS + 10,
        .secret = true,
        .group_id = 18,
        .sort_order = 27,
    },
    {
        // Silver
        .name_msg = "tot_cos2_19",
        .help_msg = "tot_cos2_19_h",
        .models = YOSHI_MODEL_PATHS("X"),
        .price = 25,
        .icon = IconType::YOSHI_EXTRA_ICONS + 11,
        .icon_hud = IconType::YOSHI_EXTRA_HUD_ICONS + 11,
        .group_id = 19,
        .sort_order = 28,
    },
    {
        // Gold
        .name_msg = "tot_cos2_20",
        .help_msg = "tot_cos2_20_h",
        .models = YOSHI_MODEL_PATHS("Y"),
        .price = 25,
        .icon = IconType::YOSHI_EXTRA_ICONS + 12,
        .icon_hud = IconType::YOSHI_EXTRA_HUD_ICONS + 12,
        .group_id = 20,
        .sort_order = 29,
    },
    {
        // Platinum
        .name_msg = "tot_cos2_21",
        .help_msg = "tot_cos2_21_h",
        .models = YOSHI_MODEL_PATHS("Z"),
        .price = 25,
        .icon = IconType::YOSHI_EXTRA_ICONS + 13,
        .icon_hud = IconType::YOSHI_EXTRA_HUD_ICONS + 13,
        .group_id = 21,
        .sort_order = 30,
    },
    {
        // Tangerine
        .name_msg = "tot_cos2_22",
        .help_msg = "tot_cos2_22_h",
        .models = YOSHI_MODEL_PATHS("A"),
        .price = 20,
        .icon = IconType::YOSHI_COSTUME_21,
        .icon_hud = IconType::YOSHI_COSTUME_21_HUD,
        .group_id = 22,
        .sort_order = 10,
    },
    {
        // Cerulean
        .name_msg = "tot_cos2_23",
        .help_msg = "tot_cos2_23_h",
        .models = YOSHI_MODEL_PATHS("B"),
        .price = 20,
        .icon = IconType::YOSHI_COSTUME_22,
        .icon_hud = IconType::YOSHI_COSTUME_22_HUD,
        .group_id = 23,
        .sort_order = 16,
    },
    {
        // Azure
        .name_msg = "tot_cos2_24",
        .help_msg = "tot_cos2_24_h",
        .models = YOSHI_MODEL_PATHS("C"),
        .price = 20,
        .icon = IconType::YOSHI_COSTUME_23,
        .icon_hud = IconType::YOSHI_COSTUME_23_HUD,
        .group_id = 24,
        .sort_order = 17,
    },
    {
        // Mustard
        .name_msg = "tot_cos2_25",
        .help_msg = "tot_cos2_25_h",
        .models = YOSHI_MODEL_PATHS("D"),
        .price = 20,
        .icon = IconType::YOSHI_COSTUME_24,
        .icon_hud = IconType::YOSHI_COSTUME_24_HUD,
        .group_id = 25,
        .sort_order = 11,
    },
    // Pea Green: sort order 14
    // Bubblegum: sort order 21
    // Maroon:    sort order 23
    // Chestnut:  sort order  9
    // Olive:     sort order 25
};
const int32_t g_NumYoshiCostumes =
    sizeof(g_YoshiCostumeData) / sizeof(YoshiCostumeData);


int32_t comp_MarioCostume(int8_t* lhs, int8_t* rhs) {
    return g_MarioCostumeData[*lhs].sort_order - g_MarioCostumeData[*rhs].sort_order;
}
int32_t comp_YoshiCostume(int8_t* lhs, int8_t* rhs) {
    return g_YoshiCostumeData[*lhs].sort_order - g_YoshiCostumeData[*rhs].sort_order;
}

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

bool CosmeticsManager::IsPurchaseable(int32_t type, int32_t id) {
    // Return false if the option is already unlocked.
    if (g_Mod->state_.GetOption(FLAGS_COSMETIC_PURCHASED, type * 32 + id))
        return false;

    switch (type) {
        case CosmeticType::ATTACK_FX: {
            auto* data = GetAttackFxData(id);
            if (!data || data->group_id < 0) return false;
            return AchievementsManager::CheckCosmeticGroupUnlocked(
                AchievementRewardType::ATTACK_FX, data->group_id);
        }
        case CosmeticType::MARIO_COSTUME: {
            auto* data = GetMarioCostumeData(id);
            if (!data || data->group_id < 0) return false;
            return AchievementsManager::CheckCosmeticGroupUnlocked(
                AchievementRewardType::MARIO_COSTUME, data->group_id);
        }
        case CosmeticType::YOSHI_COSTUME: {
            auto* data = GetYoshiCostumeData(id);
            if (!data || data->group_id < 0) return false;
            return AchievementsManager::CheckCosmeticGroupUnlocked(
                AchievementRewardType::YOSHI_COSTUME, data->group_id);
        }
    }
    return false;
}

bool CosmeticsManager::IsAvailable(int32_t type, int32_t id) {
    switch (type) {
        case CosmeticType::ATTACK_FX: {
            auto* data = GetAttackFxData(id);
            if (!data) return false;
            if (data->group_id < 0) return true;
            if (AchievementsManager::CheckCosmeticGroupUnlocked(
                AchievementRewardType::ATTACK_FX, data->group_id)) {
                return g_Mod->state_.GetOption(FLAGS_COSMETIC_PURCHASED, id);
            }
            break;
        }
        case CosmeticType::MARIO_COSTUME: {
            auto* data = GetMarioCostumeData(id);
            if (!data) return false;
            if (data->group_id < 0) return true;
            if (AchievementsManager::CheckCosmeticGroupUnlocked(
                AchievementRewardType::MARIO_COSTUME, data->group_id)) {
                return g_Mod->state_.GetOption(FLAGS_COSMETIC_PURCHASED, id + 32);
            }
            break;
        }
        case CosmeticType::YOSHI_COSTUME: {
            auto* data = GetYoshiCostumeData(id);
            if (!data) return false;
            if (data->group_id < 0) return true;
            if (AchievementsManager::CheckCosmeticGroupUnlocked(
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

int32_t CosmeticsManager::GetActiveCosmetics(int32_t type, int8_t* arr) {
    int32_t num_unlocked = 0;
    for (int32_t i = 0; i < 32; ++i) {
        if (CosmeticsManager::IsAvailable(type, i)) {
            arr[num_unlocked++] = i;
        }
    }
    // Attack FX are already sorted in menu order by default.
    switch (type) {
        case CosmeticType::MARIO_COSTUME:
            ttyd::system::qqsort(
                arr, num_unlocked, sizeof(int8_t), (void*)comp_MarioCostume);
            break;
        case CosmeticType::YOSHI_COSTUME:
            ttyd::system::qqsort(
                arr, num_unlocked, sizeof(int8_t), (void*)comp_YoshiCostume);
            break;
    }
    return num_unlocked;
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
    int32_t colors[32] = { 0 };
    int32_t num_colors = 0;
    int32_t selected_color = 0;
    
    for (int32_t i = 0; i < 32; ++i) {
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
    int32_t sounds[32] = { 0 };
    int32_t num_sounds = 0;
    
    for (int32_t i = 1; i < 32; ++i) {
        if (GetSWF(GSWF_AttackFxFlags + i)) {
            sounds[num_sounds++] = i;
        }
    }
    if (num_sounds) {
        return sounds[ttyd::system::irand(num_sounds)];
    }
    return 0;
}

const char* CosmeticsManager::GetSoundFromFXGroup(int32_t id) {
    const auto* data = GetAttackFxData(id);
    if (!data) return nullptr;
    return data->sounds[ttyd::system::irand(data->num_sounds)];
}

}