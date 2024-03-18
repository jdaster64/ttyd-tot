#include "custom_strings.h"

#include "common_functions.h"
#include "mod.h"
#include "mod_state.h"
#include "tot_generate_enemy.h"

#include <ttyd/battle_mario.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/mariost.h>
#include <ttyd/msgdrv.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::infinite_pit {

// Enum representing indices of various strings in kKeyLookups.
namespace MsgKey {
    enum e {
        CUSTOM_TATTLE_BATTLE = 0,
        CUSTOM_TATTLE_MENU,
        MENU_KIKEN_DE_POWER,
        MENU_KIKEN_DE_POWER_P,
        MENU_PINCH_DE_GANBARU,
        MENU_PINCH_DE_GANBARU_P,
        MSG_JON_KANBAN_2,
        MSG_JON_KANBAN_3,
        MSG_JON_MOVER_SELECT,
        MSG_KIKEN_DE_POWER,
        MSG_KIKEN_DE_POWER_P,
        MSG_PINCH_DE_GANBARU,
        MSG_PINCH_DE_GANBARU_P,
        RIPPO_TOP_MENU,
        TIK_06_02,
        ZZ_TEST_WIN_SELECT,
    };
}
    
namespace {

// Keys for strings to be replaced / added to msgSearch. Should be kept
// in sync with the above enum, and always maintain alphabetical order.
constexpr const char* kKeyLookups[] = {
    "custom_tattle_battle",
    "custom_tattle_menu",
    "menu_kiken_de_power",
    "menu_kiken_de_power_p",
    "menu_pinch_de_ganbaru",
    "menu_pinch_de_ganbaru_p",
    "msg_jon_kanban_2",
    "msg_jon_kanban_3",
    "msg_jon_mover_select",
    "msg_kiken_de_power",
    "msg_kiken_de_power_p",
    "msg_pinch_de_ganbaru",
    "msg_pinch_de_ganbaru_p",
    "rippo_top_menu",
    "tik_06_02",
    "zz_test_win_select",
};

const char* GetYoshiTextColor() {
    const char* kYoshiColorStrings[] = {
        "00c100", "e50000", "0000e5", "d07000",
        "e080d0", "404040", "90b0c0", "000000",
    };
    if (!g_Mod->inf_state_.GetOptionNumericValue(OPT_YOSHI_COLOR_SELECT)) {
        return kYoshiColorStrings[7];
    } else {
        return kYoshiColorStrings[ttyd::mario_pouch::pouchGetPartyColor(4)];
    }
}

const char* GetMoverOptionsString(char* buf) {
    char* buf_start = buf;
    int32_t floor = g_Mod->inf_state_.floor_ + 1;
    int32_t cost = (floor > 90 ? 90 : floor) / 10 + 1;
    buf += sprintf(buf, 
        "<select 0 3 0 40>\n"
        "Down 3 floors:       %" PRId32 " coins", cost * 5);
    // Don't provide options that allow warping past a Bonetail floor.
    if (floor % 100 <= 95) {
        buf += sprintf(buf, 
            "\nDown 5 floors:       %" PRId32 " coins", cost * 10);
        if (floor % 100 <= 90) {
            buf += sprintf(buf, 
                "\nDown 10 floors:      %" PRId32 " coins", cost * 20);
        }
    }
    return buf_start;
}

}
    
const char* StringsManager::LookupReplacement(const char* msg_key) {
    // Do not use for more than one custom message at a time!
    static char buf[1024];
    
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
    
    // TODO: Order of case statements shouldn't matter, but consider either
    // ordering them alphabetically or putting logically similar ones together?
    switch (idx) {
        case MsgKey::CUSTOM_TATTLE_BATTLE:
        case MsgKey::CUSTOM_TATTLE_MENU:
            return tot::GetCustomTattle();
        case MsgKey::MSG_JON_KANBAN_2: {
            if (g_Mod->inf_state_.GetPlayStatsString(buf)) return buf;
            return "<kanban>\n"
                   "Start a new file to see some\n"
                   "of your play stats here!\n<k>";
        }
        case MsgKey::MSG_JON_KANBAN_3: {
            sprintf(buf, "<kanban>\nYour seed: <col %sff>%s\n</col>"
                "Currently selected options:\n<col 0000ffff>%s\n</col><k>",
                GetYoshiTextColor(),
                ttyd::mariost::g_MarioSt->saveFileName, 
                g_Mod->inf_state_.GetEncodedOptions());
            return buf;
        }
        case MsgKey::TIK_06_02: {
            sprintf(buf, "<kanban>\n"
                "Thanks for playing the PM:TTYD\n"
                "Infinite Pit mod! Check the \n"
                "sign in back for your seed,\n<k><p>\n"
                "and currently selected options.\n"
                "If you want a random seed,\n"
                "name your file \"random\" or \"\xde\".\n<k>");
            return buf;
        }
        case MsgKey::MSG_KIKEN_DE_POWER:
        case MsgKey::MENU_KIKEN_DE_POWER:
            if (g_Mod->inf_state_.GetOptionNumericValue(OPT_WEAKER_RUSH_BADGES)) {
                return "Increase Attack power by 2\n"
                       "when Mario is in Peril.";
            }
            return nullptr;
        case MsgKey::MSG_KIKEN_DE_POWER_P:
        case MsgKey::MENU_KIKEN_DE_POWER_P:
            if (g_Mod->inf_state_.GetOptionNumericValue(OPT_WEAKER_RUSH_BADGES)) {
                return "Increase Attack power by 2\n"
                       "when your partner is in Peril.";
            }
            return nullptr;
        case MsgKey::MSG_PINCH_DE_GANBARU:
        case MsgKey::MENU_PINCH_DE_GANBARU:
            if (g_Mod->inf_state_.GetOptionNumericValue(OPT_WEAKER_RUSH_BADGES)) {
                return "Increase Attack power by 1\n"
                       "when Mario is in Danger.";
            }
            return nullptr;
        case MsgKey::MSG_PINCH_DE_GANBARU_P:
        case MsgKey::MENU_PINCH_DE_GANBARU_P:
            if (g_Mod->inf_state_.GetOptionNumericValue(OPT_WEAKER_RUSH_BADGES)) {
                return "Increase Attack power by 1\n"
                       "when your ally is in Danger.";
            }
            return nullptr;
        case MsgKey::MSG_JON_MOVER_SELECT:
            return GetMoverOptionsString(buf);
        case MsgKey::RIPPO_TOP_MENU: {
            if (g_Mod->inf_state_.GetOptionNumericValue(OPT_NO_EXP_MODE)) {
                return "<select 0 3 0 40>\nItems\nBadges";
            }
            return "<select 0 3 0 40>\nItems\nBadges\nLevel-ups";
        }
        case MsgKey::ZZ_TEST_WIN_SELECT:
            return "Selection: %d %d %d %d<k>";
    }
    // Should not be reached.
    return nullptr;
}

}