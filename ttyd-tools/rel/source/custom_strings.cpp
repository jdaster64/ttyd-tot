#include "custom_strings.h"

#include "common_functions.h"
#include "mod.h"
#include "mod_state.h"
#include "tot_generate_enemy.h"
#include "tot_gsw.h"

#include <ttyd/msgdrv.h>
#include <ttyd/swdrv.h>

#include <cstring>

namespace mod::infinite_pit {

// Enum representing indices of various strings in kKeyLookups.
namespace MsgKey {
    enum e {
        CUSTOM_TATTLE_BATTLE = 0,
        CUSTOM_TATTLE_MENU,
        SYS_NO_KEY,
    };
}
    
namespace {

// Keys for strings to be replaced / added to msgSearch. Should be kept
// in sync with the above enum, and always maintain alphabetical order.
constexpr const char* kKeyLookups[] = {
    "custom_tattle_battle",
    "custom_tattle_menu",
    "sys_no_key",
};

}
    
const char* StringsManager::LookupReplacement(const char* msg_key) {
    // Handle journal Tattle entries.
    if (strstr(msg_key, "menu_enemy_")) {
        msg_key = tot::SetCustomMenuTattle(msg_key);
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
    
    // Order of case statements shouldn't matter, but consider either
    // ordering them alphabetically or putting logically similar ones together?
    switch (idx) {
        case MsgKey::CUSTOM_TATTLE_BATTLE:
        case MsgKey::CUSTOM_TATTLE_MENU:
            return tot::GetCustomTattle();
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