#include "tot_manager_debug.h"

#include "common_functions.h"
#include "common_types.h"
#include "common_ui.h"
#include "mod.h"
#include "patches_field.h"
#include "tot_generate_enemy.h"
#include "tot_gon_tower.h"
#include "tot_gsw.h"
#include "tot_manager_achievements.h"
#include "tot_manager_move.h"
#include "tot_manager_options.h"
#include "tot_state.h"

#include <gc/OSTime.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_monosiri.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/msgdrv.h>
#include <ttyd/sound.h>
#include <ttyd/swdrv.h>
#include <ttyd/system.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::tot {
    
enum DebugManagerMode {
    DEBUG_OFF       = -1,
    DEBUG_MAIN      = 0,
    
    DEBUG_MIN       = 100,
    DEBUG_ENEMIES,              // Tower only
    DEBUG_FLOOR,                // Tower only
    DEBUG_UNLOCK_ALL_MOVES,     // Tower only
    DEBUG_UNLOCK_ALL_PARTNERS,  // Tower only
    DEBUG_MAX_STATS,            // Tower only
    DEBUG_PURCHASE_ALL_ITEMS,
    DEBUG_PURCHASE_ALL_COSMETICS,
    DEBUG_UNLOCK_ALL_KEY_ITEMS,
    DEBUG_UNLOCK_ALL_BADGES,
    DEBUG_COMPLETE_TUTORIAL,
    DEBUG_COMPLETE_LOGS,
    DEBUG_COMPLETE_ACHIEVEMENTS,
    DEBUG_ADD_CURRENCY,
    DEBUG_EXIT,
    DEBUG_MAX,

    // Not accessible but supported.
    DEBUG_SOUND_TEST,
};
    
namespace {
    
using ::ttyd::mario_pouch::PouchData;
    
namespace BattleUnitType = ::ttyd::battle_database_common::BattleUnitType;
namespace ItemType = ::ttyd::item_data::ItemType;

// Globals.
int32_t g_DebugMode = DEBUG_OFF;
int32_t g_CursorPos = 0;
int32_t g_DebugEnemies[5] = { -1, -1, -1, -1, -1 };
int32_t g_LastSoundPlayed = -1;

// Gets all buttons pressed the current frame, converting stick inputs into
// the respective D-Pad directions.
uint32_t GetPressedButtons() {
    const uint32_t dir_trg = ttyd::system::keyGetDirTrg(0);
    uint32_t button_trg = ttyd::system::keyGetButtonTrg(0);
    switch (dir_trg) {
        case DirectionInputId::CSTICK_UP: 
        case DirectionInputId::ANALOG_UP: {
            button_trg |= ButtonId::DPAD_UP;
            break;
        }
        case DirectionInputId::CSTICK_DOWN: 
        case DirectionInputId::ANALOG_DOWN: {
            button_trg |= ButtonId::DPAD_DOWN;
            break;
        }
        case DirectionInputId::CSTICK_LEFT: 
        case DirectionInputId::ANALOG_LEFT: {
            button_trg |= ButtonId::DPAD_LEFT;
            break;
        }
        case DirectionInputId::CSTICK_RIGHT: 
        case DirectionInputId::ANALOG_RIGHT: {
            button_trg |= ButtonId::DPAD_RIGHT;
            break;
        }
    }
    return button_trg;
}

// Handle looping around and skipping invalid options for the given game state.
void UpdateMainMenuPos(int32_t change) {
    g_CursorPos += change;
    if (g_CursorPos == DEBUG_MAX) g_CursorPos = DEBUG_MIN + 1;
    if (g_CursorPos == DEBUG_MIN) g_CursorPos = DEBUG_MAX - 1; 
    
    // If not currently in a run, hide options that only pertain to tower.
    if (!g_Mod->state_.GetOption(OPT_RUN_STARTED)) {
        switch (g_CursorPos) {
            case DEBUG_ENEMIES:
            case DEBUG_FLOOR:
            case DEBUG_UNLOCK_ALL_MOVES:
            case DEBUG_UNLOCK_ALL_PARTNERS:
            case DEBUG_MAX_STATS:
                g_CursorPos = change > 0 ? DEBUG_PURCHASE_ALL_ITEMS : DEBUG_EXIT;
                break;
            default:
                break;
        }
    }
}

}

void DebugManager::Update() {
    if (g_DebugMode == DEBUG_OFF) return;
    const uint32_t buttons = ttyd::system::keyGetButton(0);
    const uint32_t button_trg = GetPressedButtons();
    
    if (g_DebugMode == DEBUG_MAIN) {
        UpdateMainMenuPos(0);
        if (button_trg & (ButtonId::DPAD_UP | ButtonId::DPAD_RIGHT)) {
            UpdateMainMenuPos(1);
        } else if (button_trg & (ButtonId::DPAD_DOWN | ButtonId::DPAD_LEFT)) {
            UpdateMainMenuPos(-1);
        } else if (button_trg & ButtonId::Y) {
            switch (g_CursorPos) {
                case DEBUG_ENEMIES: {
                    // Go to submenu on next frame.
                    g_DebugMode = g_CursorPos;
                    g_CursorPos = 0;
                    return;
                }
                case DEBUG_SOUND_TEST: {
                    // Go to submenu on next frame.
                    g_DebugMode = g_CursorPos;
                    g_CursorPos = 0;
                    return;
                }
                case DEBUG_FLOOR: {
                    // Go to submenu on next frame.
                    g_DebugMode = g_CursorPos;
                    g_CursorPos = g_Mod->state_.floor_;
                    return;
                }
                case DEBUG_MAX_STATS: {
                    auto& state = g_Mod->state_;
                    state.hp_level_ = 99;
                    state.fp_level_ = 99;
                    state.bp_level_ = 99;
                    state.hp_p_level_ = 99;
                    state.max_inventory_ = 99;
                    tot::OptionsManager::UpdateLevelupStats();
                    break;
                }
                case DEBUG_ADD_CURRENCY: {
                    ttyd::mario_pouch::pouchAddCoin(500);
                    ttyd::mario_pouch::pouchAddStarPiece(50);
                    if (!g_Mod->state_.GetOption(OPT_RUN_STARTED)) {
                        g_Mod->state_.ChangeOption(STAT_PERM_CURRENT_COINS, 500);
                        g_Mod->state_.ChangeOption(STAT_PERM_CURRENT_SP, 50);
                    } else {
                        g_Mod->state_.ChangeOption(STAT_RUN_STAR_PIECES, 50);
                        g_Mod->state_.ChangeOption(STAT_PERM_STAR_PIECES, 50);
                    }
                    break;
                }
                case DEBUG_UNLOCK_ALL_MOVES: {
                    // Unlock all moves and move levels.
                    for (int32_t i = 0; i < tot::MoveType::MOVE_TYPE_MAX; ++i) {
                        tot::MoveManager::UpgradeMove(i);
                        tot::MoveManager::UpgradeMove(i);
                        tot::MoveManager::UpgradeMove(i);
                    }
                    break;
                }
                case DEBUG_UNLOCK_ALL_PARTNERS: {
                    // Unlock all partners, if you have at least one unlocked.
                    if (GetNumActivePartners() > 0) {
                        auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
                        for (int32_t i = 1; i <= 7; ++i) {
                            pouch.party_data[i].flags |= 1;
                        }
                        g_Mod->state_.SetOption(STAT_PERM_PARTNERS_OBTAINED, 0xfe);
                    }
                    break;
                }
                case DEBUG_PURCHASE_ALL_ITEMS: {
                    for (int32_t i = 0; i < 256; ++i) {
                        g_Mod->state_.SetOption(FLAGS_ITEM_ENCOUNTERED, i);
                        g_Mod->state_.SetOption(FLAGS_ITEM_PURCHASED, i);
                    }
                    break;
                }
                case DEBUG_PURCHASE_ALL_COSMETICS: {
                    for (int32_t i = 0; i < 96; ++i) {
                        if (i % 32 == 0) continue;
                        g_Mod->state_.SetOption(FLAGS_COSMETIC_PURCHASED, i);
                    }
                    break;
                }
                case DEBUG_UNLOCK_ALL_KEY_ITEMS: {
                    for (int32_t i = ItemType::TOT_KEY_PEEKABOO;
                         i < ItemType::TOT_KEY_ITEM_MAX; ++i) {
                        if (!ttyd::mario_pouch::pouchCheckItem(i)) {
                            ttyd::mario_pouch::pouchGetItem(i);
                        }
                        SetSWF(GSWF_PeekabooEnabled);
                        SetSWF(GSWF_SuperPeekabooEnabled);
                        SetSWF(GSWF_TimingTutorEnabled);
                        SetSWF(GSWF_BgmEnabled);
                    }
                    break;
                }
                case DEBUG_UNLOCK_ALL_BADGES: {
                    auto* item_data = ttyd::item_data::itemDataTable;
                    auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
                    
                    for (int32_t i = 0; i < 200; ++i) {
                        pouch.badges[i] = 0;
                        pouch.equipped_badges[i] = 0;
                    }
                    pouch.unallocated_bp = pouch.total_bp;
                    
                    for (int32_t i = ItemType::POWER_JUMP;
                         i <= ItemType::MAX_ITEM_TYPE; ++i) {
                        if (item_data[i].type_sort_order >= 0) {
                            ttyd::mario_pouch::pouchGetItem(i);
                        }
                    }
                    ttyd::system::qqsort(
                        &pouch.badges[0],
                        ttyd::mario_pouch::pouchGetHaveBadgeCnt(),
                        sizeof(int16_t),
                        (void*)ttyd::mario_pouch::comp_kind);
                    break;
                }
                case DEBUG_COMPLETE_TUTORIAL: {
                    SetSWByte(GSW_Tower_TutorialClearAttempts, 3);
                    SetSWByte(GSW_Tower_TutorialClears, 2);
                    SetSWF(GSWF_CosmeticShopTutorial);
                    SetSWF(GSWF_HubShopTutorial);
                    SetSWF(GSWF_RunOptionsTutorial);
                    SetSWF(GSWF_NpcF_SeedUnlocked);
                    break;
                }
                case DEBUG_COMPLETE_LOGS: {
                    for (int32_t i = 0; i <= BattleUnitType::BONETAIL; ++i) {
                        // Set Tattle flags for only enemies in Infinite Pit.
                        if (tot::GetCustomTattleIndex(i) > 0) {
                            ttyd::swdrv::swSet(0x117a + i);
                        }
                    }
                    for (int32_t i = 0; i < 256; ++i) {
                        // Set all "item obtained" flags in state manager.
                        g_Mod->state_.SetOption(FLAGS_ITEM_ENCOUNTERED, i);
                    }
                    for (int32_t i = 0; i < MoveType::MOVE_TYPE_MAX; ++i) {
                        // Set all "move unlocked / used / Stylish" flags.
                        g_Mod->state_.SetOption(STAT_PERM_MOVE_LOG, 0xff, i);
                    }
                    // Set all partners as having been obtained once.
                    g_Mod->state_.SetOption(STAT_PERM_PARTNERS_OBTAINED, 0xfe);
                    break;
                }
                case DEBUG_COMPLETE_ACHIEVEMENTS: {
                    int32_t max_achievement = AchievementId::META_ALL_ACHIEVEMENTS;
                    if (buttons & ButtonId::L) {
                        // Also mark off secret achievements if L is held.
                        max_achievement = AchievementId::MAX_ACHIEVEMENT - 1;
                    }
                    for (int32_t i = 0; i <= max_achievement; ++i) {
                        AchievementsManager::MarkCompleted(i);
                    }
                    break;
                }
            }
            g_DebugMode = DEBUG_OFF;
            return;
        }
    } else if (g_DebugMode == DEBUG_ENEMIES) {
        int32_t dir = 0;
        
        if (button_trg & ButtonId::DPAD_UP) {
            g_CursorPos = (g_CursorPos + 4) % 5;
        } else if (button_trg & ButtonId::DPAD_DOWN) {
            g_CursorPos = (g_CursorPos + 1) % 5;
        } else if (button_trg & ButtonId::DPAD_LEFT) {
            dir = -1;
        } else if (button_trg & ButtonId::DPAD_RIGHT) {
            dir = 1;
        } else if (button_trg & ButtonId::Y) {
            if (buttons & ButtonId::L) {
                g_DebugMode = DEBUG_OFF;
                return;
            } else {
                // Clear all enemies after this point.
                for (int32_t i = g_CursorPos; i < 5; ++i)
                    g_DebugEnemies[i] = -1;
            }
        }
        if (dir == 0) return;

        // Don't allow changing enemies after a blank spot.
        if (g_CursorPos > 0 && g_DebugEnemies[g_CursorPos-1] == -1) return;
        
        const int32_t max_enemy = BattleUnitType::BONETAIL;
        const int32_t first_move = (buttons & ButtonId::L) ? 0x10 : 1;
        int32_t enemy_type = g_DebugEnemies[g_CursorPos];
        
        // Change selected enemy by +/-1; if L is held, add/subtract 0x10 first.
        if (dir < 0) {
            if (enemy_type < first_move) {
                enemy_type = -1;
            } else {
                enemy_type -= first_move;
            }
        } else {
            if (enemy_type + first_move > max_enemy) {
                enemy_type = -1;
            } else {
                enemy_type += first_move;
            }
        }
        
        do {
            if (g_CursorPos == 0) {
                // If valid for the front, fill the first 3 slots with the type.
                // (Bosses only fill the first slot and wipe all others).
                if (tot::IsEligibleFrontEnemy(enemy_type)) {
                    int32_t num_enemies = 3;
                    switch (enemy_type) {
                        case BattleUnitType::GOLD_FUZZY:
                        case BattleUnitType::ATOMIC_BOO:
                        case BattleUnitType::TOT_COSMIC_BOO:
                        case BattleUnitType::HOOKTAIL:
                        case BattleUnitType::GLOOMTAIL:
                        case BattleUnitType::BONETAIL:
                            num_enemies = 1;
                            break;
                    }
                    for (int32_t i = 0; i < 5; ++i)
                        g_DebugEnemies[i] = i < num_enemies ? enemy_type : -1;
                    break;
                }
            } else {
                // If valid, change the currently selected slot only.
                if (tot::IsEligibleLoadoutEnemy(enemy_type) || enemy_type == -1) {
                    bool valid_back_enemy = true;
                    switch (enemy_type) {
                        case BattleUnitType::GOLD_FUZZY:
                        case BattleUnitType::FUZZY_HORDE:
                        case BattleUnitType::ATOMIC_BOO:
                        case BattleUnitType::TOT_COSMIC_BOO:
                        case BattleUnitType::HOOKTAIL:
                        case BattleUnitType::GLOOMTAIL:
                        case BattleUnitType::BONETAIL:
                            valid_back_enemy = false;
                            break;
                    }
                    switch (g_DebugEnemies[0]) {
                        case BattleUnitType::GOLD_FUZZY:
                        case BattleUnitType::ATOMIC_BOO:
                        case BattleUnitType::TOT_COSMIC_BOO:
                        case BattleUnitType::HOOKTAIL:
                        case BattleUnitType::GLOOMTAIL:
                        case BattleUnitType::BONETAIL:
                        case -1:
                            valid_back_enemy = false;
                            break;
                    }
                    if (valid_back_enemy) {
                        g_DebugEnemies[g_CursorPos] = enemy_type;
                        // Exception: if type == "None", clear all later slots.
                        if (enemy_type == -1) {
                            for (int32_t i = g_CursorPos + 1; i < 5; ++i)
                                g_DebugEnemies[i] = -1;
                        }
                        break;
                    }
                }
            }

            // Not a valid enemy, keep looking for the next valid one.
            if (dir < 0 && enemy_type == -1) {
                enemy_type = max_enemy;
            } else if (dir > 0 && enemy_type == max_enemy) {
                enemy_type = -1;
            } else {
                enemy_type += dir;
            }
        } while (true);
    } else if (g_DebugMode == DEBUG_SOUND_TEST) {
        int32_t dir = 0;
        if (button_trg & (ButtonId::DPAD_UP | ButtonId::DPAD_RIGHT)) {
            dir = 1;
        } else if (button_trg & (ButtonId::DPAD_DOWN | ButtonId::DPAD_LEFT)) {
            dir = -1;
        } else if (button_trg & ButtonId::Y) {
            // Stop previous sound and play currently selected one.
            if (g_LastSoundPlayed >= 0) {
                ttyd::sound::SoundEfxStop(g_LastSoundPlayed);
            }
            if (buttons & ButtonId::L) {
                g_DebugMode = DEBUG_OFF;
                return;
            }
            g_LastSoundPlayed = 
                ttyd::sound::SoundEfxPlayEx(g_CursorPos, 0, 0x64, 0x40);
        }
        if (buttons & ButtonId::L) {
            dir *= 16;
        }
        g_CursorPos += dir;
        if (g_CursorPos < 0) g_CursorPos = 0;
    } else if (g_DebugMode == DEBUG_FLOOR) {
        int32_t dir = 0;
        if (button_trg & (ButtonId::DPAD_UP | ButtonId::DPAD_RIGHT)) {
            dir = 1;
        } else if (button_trg & (ButtonId::DPAD_DOWN | ButtonId::DPAD_LEFT)) {
            dir = -1;
        } else if (button_trg & ButtonId::Y) {
            g_Mod->state_.floor_ = g_CursorPos;
            tot::gon::UpdateDestinationMap();
            g_DebugMode = DEBUG_OFF;
            return;
        }
        
        if (dir == 0) return;
        if (buttons & ButtonId::L) dir *= 8;

        int32_t max_floor = 64;
        switch (g_Mod->state_.GetOptionValue(tot::OPT_DIFFICULTY)) {
            case tot::OPTVAL_DIFFICULTY_TUTORIAL:
                max_floor = 8;
                break;
            case tot::OPTVAL_DIFFICULTY_HALF:
                max_floor = 32;
                break;
        }
        g_CursorPos = Clamp(g_CursorPos + dir, 0, max_floor);
        if (g_CursorPos < 0) g_CursorPos = 0;
    }
}

void DebugManager::Draw() {
    if (!InMainGameModes()) return;
    
    char buf[32];
    const uint32_t black_alpha = 0x000000E5u;
    const uint32_t red_alpha  = 0xC00000E5u;
    if (g_DebugMode == DEBUG_MAIN) {
        switch (g_CursorPos) {
            case DEBUG_ENEMIES: {
                strcpy(buf, "Select Enemy Loadout");        break;
            }
            case DEBUG_FLOOR: {
                strcpy(buf, "Set Current Floor");           break;
            }
            case DEBUG_MAX_STATS: {
                strcpy(buf, "Max Stats");                   break;
            }
            case DEBUG_ADD_CURRENCY: {
                strcpy(buf, "Add 500 Coins + 50 Star Pieces"); break;
            }
            case DEBUG_UNLOCK_ALL_MOVES: {
                strcpy(buf, "Unlock All Moves");            break;
            }
            case DEBUG_UNLOCK_ALL_PARTNERS: {
                strcpy(buf, "Unlock All Partners");         break;
            }
            case DEBUG_PURCHASE_ALL_ITEMS: {
                strcpy(buf, "Purchase All Items/Badges");   break;
            }
            case DEBUG_PURCHASE_ALL_COSMETICS: {
                strcpy(buf, "Purchase All Cosmetics");      break;
            }
            case DEBUG_UNLOCK_ALL_KEY_ITEMS: {
                strcpy(buf, "Unlock All Key Items");        break;
            }
            case DEBUG_UNLOCK_ALL_BADGES: {
                strcpy(buf, "Unlock 1 of Each Badge");      break;
            }
            case DEBUG_COMPLETE_TUTORIAL: {
                strcpy(buf, "Skip All Tutorials");          break;
            }
            case DEBUG_COMPLETE_LOGS: {
                strcpy(buf, "Unlock All Journal Logs");     break;
            }
            case DEBUG_COMPLETE_ACHIEVEMENTS: {
                strcpy(buf, "Complete All Achievements");   break;
            }
            case DEBUG_SOUND_TEST: {
                strcpy(buf, "Test Sound Effects");          break;
            }
            case DEBUG_EXIT: {
                strcpy(buf, "Exit Debug Mode");             break;
            }
        }
        DrawCenteredTextWindow(
            buf, 0, -20, 0xFFu, true, 0xFFFFFFFFu, 0.7f, red_alpha, 10, 7);
    } else if (g_DebugMode == DEBUG_ENEMIES) {
        for (int32_t i = 0; i < 5; ++i) {
            int32_t bg_color = i == g_CursorPos ? red_alpha : black_alpha;
            if (g_DebugEnemies[i] >= 1) {
                sprintf(buf, "%s (0x%02" PRIx32 ")",
                    ttyd::msgdrv::msgSearch(
                        ttyd::battle_monosiri::battleGetUnitMonosiriPtr(
                            g_DebugEnemies[i])->unit_name),
                    g_DebugEnemies[i]);
            } else {
                sprintf(buf, "None");
            }
            DrawCenteredTextWindow(
                buf, 0, -20 - (30 * i), 0xFFu, true, 0xFFFFFFFFu, 0.7f, 
                bg_color, 10, 7);
        }
    } else if (g_DebugMode == DEBUG_FLOOR) {
        // Print current floor number to buffer.
        sprintf(buf, "%" PRId32, g_CursorPos);
        DrawCenteredTextWindow(
            buf, 0, -60, 0xFFu, true, 0xFFFFFFFFu, 0.7f, red_alpha, 10, 7);
        // Draw main menu text to make it look like a contextual menu.
        DrawCenteredTextWindow(
            "Set Current Floor",
            0, -20, 0xFFu, true, 0xFFFFFFFFu, 0.7f, black_alpha, 10, 7);
        DrawText(
            "Warning: don't change floor during a warp transition!",
            0, -90, 0xFFu, true, ~0U, 0.6f, /* top-middle */ 1);
    } else if (g_DebugMode == DEBUG_SOUND_TEST) {
        sprintf(buf, "0x%" PRIx32, g_CursorPos);
        DrawCenteredTextWindow(
            buf, 0, -60, 0xFFu, true, 0xFFFFFFFFu, 0.7f, red_alpha, 10, 7);
        // Draw main menu text to make it look like a contextual menu.
        DrawCenteredTextWindow(
            "Test Sound Effects",
            0, -20, 0xFFu, true, 0xFFFFFFFFu, 0.7f, black_alpha, 10, 7);
    }
    
    // Display a string confirming that debug enemies are queued.
    if (g_DebugMode != DEBUG_ENEMIES && g_DebugEnemies[0] != -1) {
        DrawText(
            "Debug enemies selected.",
            -260, -176, 0xFF, true, 0xff0000ffU, 0.75f, /* center-left */ 3);
    }
}
    
void DebugManager::ChangeMode() {
    if (g_DebugMode == DEBUG_OFF) {
        g_DebugMode = DEBUG_MAIN;
        g_CursorPos = DEBUG_EXIT;
    }
}

int32_t* DebugManager::GetEnemies() {
    if (g_DebugMode && g_DebugEnemies[0] >= 0) return g_DebugEnemies;
    return nullptr;
}

}