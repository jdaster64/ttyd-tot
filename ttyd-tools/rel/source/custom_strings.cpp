#include "custom_strings.h"

#include "common_functions.h"
#include "mod.h"
#include "mod_state.h"
#include "tot_generate_enemy.h"
#include "tot_gsw.h"
#include "tot_state.h"

#include <ttyd/msgdrv.h>
#include <ttyd/swdrv.h>
#include <ttyd/win_main.h>
#include <ttyd/win_root.h>

#include <cstdio>
#include <cstring>

namespace mod::infinite_pit {

// Enum representing indices of various strings in kKeyLookups.
namespace MsgKey {
    enum e {
        CUSTOM_TATTLE_BATTLE = 0,
        CUSTOM_TATTLE_KILLCOUNT,
        CUSTOM_TATTLE_MENU,
        MENU_MONOSIRI_DERUTOKORO,
        SYS_NO_KEY,
        TOT_MOVELOG_DESC_DYN,
    };
}
    
namespace {

// Keys for strings to be replaced / added to msgSearch. Should be kept
// in sync with the above enum, and always maintain alphabetical order.
constexpr const char* kKeyLookups[] = {
    "custom_tattle_battle",
    "custom_tattle_killcount",
    "custom_tattle_menu",
    "menu_monosiri_derutokoro",
    "sys_no_key",
    "tot_movelog_desc_dyn",
};

}
    
const char* StringsManager::LookupReplacement(const char* msg_key) {
    // Handle journal Tattle entries.
    if (strstr(msg_key, "menu_enemy_")) {
        if (auto* menu = ttyd::win_main::winGetPtr(); menu) {
            int32_t id = menu->tattle_logs[menu->tattle_log_cursor_idx].id;
            msg_key = tot::SetCustomMenuTattle(id);
        }
    }
    
    // Binary search on all possible message replacements.
    constexpr const int32_t kNumMsgKeys =
        sizeof(kKeyLookups) / sizeof(const char*);
    int32_t idx_min = 0;
    int32_t idx_max = kNumMsgKeys - 1;
    int32_t idx;
    int32_t strcmp_result;
    bool found = false;
    while (idx_min <= idx_max) {
        idx = (idx_min + idx_max) / 2;
        strcmp_result = strcmp(msg_key, kKeyLookups[idx]);
        if (strcmp_result < 0) {
            idx_max = idx - 1;
        } else if (strcmp_result > 0) {
            idx_min = idx + 1;
        } else {
            found = true;
            break;
        }
    }
    
    if (!found) return nullptr;
    
    // Order of case statements doesn't matter.
    switch (idx) {
        case MsgKey::CUSTOM_TATTLE_BATTLE:
        case MsgKey::CUSTOM_TATTLE_MENU:
            return tot::GetCustomTattle();
        case MsgKey::CUSTOM_TATTLE_KILLCOUNT: {
            // Print the number of this enemy defeated.
            static char buf[24];
            if (auto* menu = ttyd::win_main::winGetPtr(); menu) {
                int32_t idx = menu->tattle_logs[menu->tattle_log_cursor_idx].order;
                char* ptr = buf;
                ptr += sprintf(buf, "Times Defeated: ");
                ptr += IntegerToFmtString(
                    g_Mod->state_.GetOption(tot::STAT_PERM_ENEMY_KILLS, idx), ptr);
                return buf;
            }
            break;
        }
        case MsgKey::MENU_MONOSIRI_DERUTOKORO:
            // Get rid of extra spacing before location in Tattle log.
            return "";
        case MsgKey::TOT_MOVELOG_DESC_DYN:
            if (auto* menu = ttyd::win_main::winGetPtr(); menu) {
                int32_t move = menu->move_log_cursor_idx;
                return tot::MoveManager::GetLogDescription(move);
            }
            break;
        case MsgKey::SYS_NO_KEY:
            // Swap out "it's locked" message with more descriptive strings.
            if (GetSWByte(tot::GSW_Tower_DisplayChestIcons)) {
                return ttyd::msgdrv::msgSearch("tot_lock_claimchest");
            } else {
                return ttyd::msgdrv::msgSearch("tot_lock_defeatenemies");
            }
    }
    // Should not be reached.
    return nullptr;
}

}