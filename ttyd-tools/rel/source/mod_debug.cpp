#include "mod_debug.h"

#include "common_functions.h"
#include "common_types.h"
#include "common_ui.h"
#include "custom_enemy.h"
#include "mod.h"
#include "mod_state.h"
#include "patches_field.h"

#include <ttyd/battle_database_common.h>
#include <ttyd/battle_monosiri.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/msgdrv.h>
#include <ttyd/swdrv.h>
#include <ttyd/system.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::infinite_pit {
    
enum DebugManagerMode {
    DEBUG_OFF       = -1,
    DEBUG_MAIN      = 0,
    
    DEBUG_MIN       = 100,
    DEBUG_ENEMIES,
    DEBUG_FLOOR,
    DEBUG_UNLOCK_SPECIAL_MOVES,
    DEBUG_UNLOCK_TATTLE_LOG,
    DEBUG_EXIT,
    DEBUG_MAX
};
    
namespace {
    
using ::ttyd::mario_pouch::PouchData;
    
namespace BattleUnitType = ::ttyd::battle_database_common::BattleUnitType;

// Globals.
int32_t g_DebugMode = DEBUG_OFF;
int32_t g_CursorPos = 0;
int32_t g_DebugEnemies[5] = { -1, -1, -1, -1, -1 };

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

}

void DebugManager::Update() {
    if (g_DebugMode == DEBUG_OFF) return;
    const uint32_t buttons = ttyd::system::keyGetButton(0);
    const uint32_t button_trg = GetPressedButtons();
    
    // Continuously set the "used debug mode" flag on if in use.
    g_Mod->state_.SetOption(OPT_DEBUG_MODE_USED, 1);
    
    if (g_DebugMode == DEBUG_MAIN) {
        if (button_trg & (ButtonId::DPAD_UP | ButtonId::DPAD_RIGHT)) {
            ++g_CursorPos;
            if (g_CursorPos == DEBUG_MAX) g_CursorPos = DEBUG_MIN + 1;
        } else if (button_trg & (ButtonId::DPAD_DOWN | ButtonId::DPAD_LEFT)) {
            --g_CursorPos;
            if (g_CursorPos == DEBUG_MIN) g_CursorPos = DEBUG_MAX - 1;          
        } else if (button_trg & ButtonId::Y) {
            switch (g_CursorPos) {
                case DEBUG_ENEMIES: {
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
                case DEBUG_UNLOCK_SPECIAL_MOVES: {
                    PouchData& pouch = *ttyd::mario_pouch::pouchGetPtr();
                    pouch.star_powers_obtained = 0xff;
                    pouch.max_sp = 1000;
                    pouch.current_sp = 1000;
                    g_Mod->state_.star_power_levels_ = 0xffff;
                    break;
                }
                case DEBUG_UNLOCK_TATTLE_LOG: {
                    for (int32_t i = 0; i <= BattleUnitType::BONETAIL; ++i) {
                        // Set Tattle flags for only enemies in Infinite Pit.
                        if (GetCustomTattleIndex(i) > 0) {
                            ttyd::swdrv::swSet(0x117a + i);
                        }
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
        
        const int32_t max_enemy = BattleUnitType::BONETAIL - 1;
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
                if (IsEligibleFrontEnemy(enemy_type)) {
                    const int32_t num_enemies = 
                        enemy_type == BattleUnitType::ATOMIC_BOO ? 1 : 3;
                    for (int32_t i = 0; i < 5; ++i)
                        g_DebugEnemies[i] = i < num_enemies ? enemy_type : -1;
                    break;
                }
            } else {
                // If valid, change the currently selected slot only.
                if (IsEligibleLoadoutEnemy(enemy_type) || enemy_type == -1) {
                    if (enemy_type != BattleUnitType::ATOMIC_BOO) {
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
    } else if (g_DebugMode == DEBUG_FLOOR) {
        int32_t dir = 0;
        if (button_trg & (ButtonId::DPAD_UP | ButtonId::DPAD_RIGHT)) {
            dir = 1;
        } else if (button_trg & (ButtonId::DPAD_DOWN | ButtonId::DPAD_LEFT)) {
            dir = -1;       
        } else if (button_trg & ButtonId::Y) {
            g_Mod->state_.floor_ = g_CursorPos;
            field::UpdateExitDestination();
            g_DebugMode = DEBUG_OFF;
            return;
        }
        
        if (dir == 0) return;
        if (buttons & ButtonId::L) dir *= 10;
        g_CursorPos += dir;
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
            case DEBUG_UNLOCK_SPECIAL_MOVES: {
                strcpy(buf, "Unlock All Special Moves");    break;
            }
            case DEBUG_UNLOCK_TATTLE_LOG: {
                strcpy(buf, "Unlock All Tattle Logs");      break;
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
        sprintf(buf, "%" PRId32, g_CursorPos + 1);
        DrawCenteredTextWindow(
            buf, 0, -60, 0xFFu, true, 0xFFFFFFFFu, 0.7f, red_alpha, 10, 7);
        // Draw main menu text to make it look like a contextual menu.
        DrawCenteredTextWindow(
            "Set Current Floor",
            0, -20, 0xFFu, true, 0xFFFFFFFFu, 0.7f, black_alpha, 10, 7);
        DrawText(
            "Warning: don't change floor during a warp transition!",
            0, -90, 0xFFu, true, ~0U, 0.6f, /* top-middle */ 1);
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