#include "custom_strings.h"

#include "common_functions.h"
#include "mod.h"
#include "mod_state.h"
#include "custom_enemy.h"

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
        MSG_CRYSTAL_STAR,
        MSG_DIAMOND_STAR,
        MSG_EMERALD_STAR,
        MSG_GARNET_STAR,
        MSG_GOLD_STAR,
        MSG_JON_KANBAN_1,
        MSG_JON_KANBAN_2,
        MSG_JON_KANBAN_3,
        MSG_JON_MOVER_SELECT,
        MSG_KIKEN_DE_POWER,
        MSG_KIKEN_DE_POWER_P,
        MSG_PINCH_DE_GANBARU,
        MSG_PINCH_DE_GANBARU_P,
        MSG_RUBY_STAR,
        MSG_SAPPHIRE_STAR,
        MSG_TREASURE_MAP,
        RIPPO_TOP_MENU,
        TIK_06_02,
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
    "msg_crystal_star",
    "msg_diamond_star",
    "msg_emerald_star",
    "msg_garnet_star",
    "msg_gold_star",
    "msg_jon_kanban_1",
    "msg_jon_kanban_2",
    "msg_jon_kanban_3",
    "msg_jon_mover_select",
    "msg_kiken_de_power",
    "msg_kiken_de_power_p",
    "msg_pinch_de_ganbaru",
    "msg_pinch_de_ganbaru_p",
    "msg_ruby_star",
    "msg_sapphire_star",
    "msg_treasure_map",
    "rippo_top_menu",
    "tik_06_02",
};

const char* GetYoshiTextColor() {
    const char* kYoshiColorStrings[] = {
        "00c100", "e50000", "0000e5", "d07000",
        "e080d0", "404040", "90b0c0", "000000",
    };
    if (!g_Mod->state_.GetOptionNumericValue(OPT_YOSHI_COLOR_SELECT)) {
        return kYoshiColorStrings[7];
    } else {
        return kYoshiColorStrings[ttyd::mario_pouch::pouchGetPartyColor(4)];
    }
}

const char* GetStarPowerItemDescription(char* buf, int32_t index) {
    int32_t level = g_Mod->state_.GetStarPowerLevel(index);
    if (!InPauseMenu()) ++level;
    const char* name_msg = ttyd::battle_mario::superActionTable[index]->name;
    sprintf(buf,
        "Allows Mario to use level %" PRId32 "\n"
        "of the move %s.", level, ttyd::msgdrv::msgSearch(name_msg));
    return buf;
}

const char* GetMoverOptionsString(char* buf) {
    char* buf_start = buf;
    int32_t floor = g_Mod->state_.floor_ + 1;
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
        msg_key = SetCustomMenuTattle(msg_key);
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
            return GetCustomTattle();
        case MsgKey::MSG_JON_KANBAN_1: {
            sprintf(buf, "<kanban>\n<pos 150 25>\nFloor %" PRId32 "\n<k>", 
                    g_Mod->state_.floor_ + 1);
            return buf;
        }
        case MsgKey::MSG_JON_KANBAN_2: {
            if (g_Mod->state_.GetPlayStatsString(buf)) return buf;
            return "<kanban>\n"
                   "Start a new file to see some\n"
                   "of your play stats here!\n<k>";
        }
        case MsgKey::MSG_JON_KANBAN_3: {
            sprintf(buf, "<kanban>\nYour seed: <col %sff>%s\n</col>"
                "Currently selected options:\n<col 0000ffff>%s\n</col><k>",
                GetYoshiTextColor(),
                ttyd::mariost::g_MarioSt->saveFileName, 
                g_Mod->state_.GetEncodedOptions());
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
            if (g_Mod->state_.GetOptionNumericValue(OPT_WEAKER_RUSH_BADGES)) {
                return "Increase Attack power by 2\n"
                       "when Mario is in Peril.";
            }
            return nullptr;
        case MsgKey::MSG_KIKEN_DE_POWER_P:
        case MsgKey::MENU_KIKEN_DE_POWER_P:
            if (g_Mod->state_.GetOptionNumericValue(OPT_WEAKER_RUSH_BADGES)) {
                return "Increase Attack power by 2\n"
                       "when your partner is in Peril.";
            }
            return nullptr;
        case MsgKey::MSG_PINCH_DE_GANBARU:
        case MsgKey::MENU_PINCH_DE_GANBARU:
            if (g_Mod->state_.GetOptionNumericValue(OPT_WEAKER_RUSH_BADGES)) {
                return "Increase Attack power by 1\n"
                       "when Mario is in Danger.";
            }
            return nullptr;
        case MsgKey::MSG_PINCH_DE_GANBARU_P:
        case MsgKey::MENU_PINCH_DE_GANBARU_P:
            if (g_Mod->state_.GetOptionNumericValue(OPT_WEAKER_RUSH_BADGES)) {
                return "Increase Attack power by 1\n"
                       "when your ally is in Danger.";
            }
            return nullptr;
        case MsgKey::MSG_TREASURE_MAP:
            return GetStarPowerItemDescription(buf, 0);
        case MsgKey::MSG_DIAMOND_STAR:
            return GetStarPowerItemDescription(buf, 1);
        case MsgKey::MSG_EMERALD_STAR:
            return GetStarPowerItemDescription(buf, 2);
        case MsgKey::MSG_GOLD_STAR:
            return GetStarPowerItemDescription(buf, 3);
        case MsgKey::MSG_RUBY_STAR:
            return GetStarPowerItemDescription(buf, 4);
        case MsgKey::MSG_SAPPHIRE_STAR:
            return GetStarPowerItemDescription(buf, 5);
        case MsgKey::MSG_GARNET_STAR:
            return GetStarPowerItemDescription(buf, 6);
        case MsgKey::MSG_CRYSTAL_STAR:
            return GetStarPowerItemDescription(buf, 7);
        case MsgKey::MSG_JON_MOVER_SELECT:
            return GetMoverOptionsString(buf);
        case MsgKey::RIPPO_TOP_MENU: {
            if (g_Mod->state_.GetOptionNumericValue(OPT_NO_EXP_MODE)) {
                return "<select 0 3 0 40>\nItems\nBadges";
            }
            return "<select 0 3 0 40>\nItems\nBadges\nLevel-ups";
        }
    }
    // Should not be reached.
    return nullptr;
}

}