#include "tot_manager_strings.h"

#include "common_functions.h"
#include "mod.h"
#include "tot_generate_enemy.h"
#include "tot_gsw.h"
#include "tot_manager_achievements.h"
#include "tot_manager_move.h"
#include "tot_manager_options.h"
#include "tot_state.h"

#include <ttyd/battle.h>
#include <ttyd/battle_unit.h>
#include <ttyd/fontmgr.h>
#include <ttyd/msgdrv.h>
#include <ttyd/swdrv.h>
#include <ttyd/win_main.h>
#include <ttyd/win_root.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::tot {

// Enum representing indices of various strings in kKeyLookups.
namespace MsgKey {
    enum e {
        CUSTOM_TATTLE_BATTLE = 0,
        CUSTOM_TATTLE_KILLCOUNT,
        CUSTOM_TATTLE_MENU,
        MENU_MONOSIRI_DERUTOKORO,
        MSG_STAR_PIECE,
        SYS_NO_KEY,
        TOT_ACHD_89,
        TOT_FLOOR_SIGN,
        TOT_LOBBY_BACKSIGN,
        TOT_MOVELOG_DESC_DYN,
        TOT_UPG_SMOOCH,
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
    "msg_star_piece",
    "sys_no_key",
    "tot_achd_89",
    "tot_floor_sign",
    "tot_lobby_backsign",
    "tot_movelog_desc_dyn",
    "tot_upg_smooch",
};

// Determines the info printed on lobby / floor signs dynamically.
// "floor" = On floor sign (otherwise, on lobby sign)
// "options" = Is the ability to set options unlocked?
const char* GetRunInfoSignString(bool floor, bool options) {
    static char buf[512], temp[64];

    const int32_t base_x_offset = 210;
    int32_t y_offset = options ? 0 : 27;

    char* ptr = buf;
    ptr += sprintf(ptr, "<kanban>\n");

    if (floor) {
        sprintf(temp, "Floor: %" PRId32, g_Mod->state_.floor_);
        int32_t width = ttyd::fontmgr::FontGetMessageWidth(temp);
        int32_t x_offset = base_x_offset - width * 0.5f;

        ptr += sprintf(ptr,
            "<pos %" PRId32 " %" PRId32 ">\n"
            "Floor: <col cc0000ff>\n"
            "%" PRId32 "\n</col>\n", x_offset, y_offset, g_Mod->state_.floor_);
    } else {
        const char* difficulty = "";
        switch (g_Mod->state_.GetOptionValue(OPT_DIFFICULTY)) {
            case OPTVAL_DIFFICULTY_HALF:
                difficulty = "Hooktail";       break;
            case OPTVAL_DIFFICULTY_FULL:
                difficulty = "Gloomtail";      break;
            case OPTVAL_DIFFICULTY_FULL_EX:
                difficulty = "EX Difficulty";  break;
        }
        sprintf(temp, "Tower Type: %s", difficulty);
        int32_t width = ttyd::fontmgr::FontGetMessageWidth(temp);
        int32_t x_offset = base_x_offset - width * 0.5f;

        ptr += sprintf(ptr,
            "<pos %" PRId32 " %" PRId32 ">\n"
            "Tower Type: <col cc0000ff>\n"
            "%s\n</col>\n", x_offset, y_offset, difficulty);
    }
    
    y_offset += 27;

    if (options) {
        const char* seed = g_Mod->state_.GetSeedAsString();
        sprintf(temp, "Seed: %s", seed);
        int32_t width = ttyd::fontmgr::FontGetMessageWidth(temp);
        int32_t x_offset = base_x_offset - width * 0.5f;

        // Change color of seed number/name to red if selected manually.
        bool seeded = !g_Mod->state_.GetOption(OPT_UNSEEDED_RUN);
        if (!g_Mod->state_.GetOption(OPT_RUN_STARTED)) {
            seeded = 
                g_Mod->state_.seed_ || g_Mod->state_.GetOption(OPT_USE_SEED_NAME);
        }
        const char* seed_color = seeded ? "cc0000ff" : "0000ccff";

        ptr += sprintf(ptr,
            "<pos %" PRId32 " %" PRId32 ">\n"
            "Seed: <col %s>\n"
            "%s\n</col>\n", x_offset, y_offset, seed_color, seed);
            
        y_offset += 27;

        const char* options = OptionsManager::GetEncodedOptions();
        sprintf(temp, "Options: %s", options);
        width = ttyd::fontmgr::FontGetMessageWidth(temp);
        x_offset = base_x_offset - width * 0.5f;

        ptr += sprintf(ptr,
            "<pos %" PRId32 " %" PRId32 ">\n"
            "Options: <col 0000ccff>\n"
            "%s\n</col>\n", x_offset, y_offset, options);
    }

    ptr += sprintf(ptr, "<k>");

    return buf;
}

}
    
const char* StringsManager::LookupReplacement(const char* msg_key) {
    // Handle journal Tattle entries.
    if (strstr(msg_key, "menu_enemy_")) {
        if (auto* menu = ttyd::win_main::winGetPtr(); menu) {
            int32_t id = menu->tattle_logs[menu->tattle_log_cursor_idx].id;
            msg_key = SetCustomMenuTattle(id);
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
            return GetCustomTattle();
        case MsgKey::CUSTOM_TATTLE_KILLCOUNT: {
            // Print the number of this enemy defeated.
            static char buf[24];
            buf[0] = '\0';
            if (ttyd::mariost::g_MarioSt->bInBattle) {
                // In battle, read from Goombella's unit work.
                const auto* partner_unit = 
                    ttyd::battle::BattleGetPartyPtr(ttyd::battle::g_BattleWork);
                if (partner_unit) {
                    const auto* tattle_work = 
                        (const ttyd::win_root::WinLogTattleMenuWork*)
                            partner_unit->unit_work[1];
                    if (tattle_work) {
                        int32_t idx = GetCustomTattleIndex(tattle_work->enemy_id);
                        if (idx >= 1 && idx <= 102) {
                            char* ptr = buf;
                            ptr += sprintf(buf, "Times Defeated: ");
                            ptr += IntegerToFmtString(
                                g_Mod->state_.GetOption(
                                    STAT_PERM_ENEMY_KILLS, idx), ptr);
                        }
                    }
                }
                return buf;
            } else if (auto* menu = ttyd::win_main::winGetPtr(); menu) {
                // Otherwise, read from currently selected Tattle log entry.
                int32_t idx = menu->tattle_logs[menu->tattle_log_cursor_idx].order;
                char* ptr = buf;
                ptr += sprintf(buf, "Times Defeated: ");
                ptr += IntegerToFmtString(
                    g_Mod->state_.GetOption(STAT_PERM_ENEMY_KILLS, idx), ptr);
                return buf;
            }
            break;
        }
        case MsgKey::MENU_MONOSIRI_DERUTOKORO:
            // Get rid of extra spacing before location in Tattle log.
            return "";
        case MsgKey::MSG_STAR_PIECE:
            // Different descriptions in hub and during a run.
            if (!g_Mod->state_.GetOption(OPT_RUN_STARTED)) {
                return ttyd::msgdrv::msgSearch("msg_star_piece_inhub");
            }
            break;
        case MsgKey::TOT_ACHD_89: {
            // TODO: Return a redacted string before beating B&K specifically.
            if (!g_Mod->state_.GetOption(
                FLAGS_ACHIEVEMENT, AchievementId::META_SECRET_BOSS)) {
                return "Defeat ????? & ?????\nsimultaneously.";
            }
            return nullptr;
        }
        case MsgKey::TOT_FLOOR_SIGN: {
            return GetRunInfoSignString(
                /* floor = */ true,
                /* options = */ GetSWByte(GSW_Tower_TutorialClears) >= 2);
        }
        case MsgKey::TOT_LOBBY_BACKSIGN: {
            return GetRunInfoSignString(
                /* floor = */ false, /* options = */ true);
        }
        case MsgKey::TOT_MOVELOG_DESC_DYN:
            if (auto* menu = ttyd::win_main::winGetPtr(); menu) {
                int32_t move = menu->move_log_cursor_idx;
                return MoveManager::GetLogDescription(move);
            }
            break;
        case MsgKey::SYS_NO_KEY:
            // Swap out "it's locked" message with more descriptive strings.
            if (GetSWByte(GSW_Tower_DisplayChestIcons)) {
                return ttyd::msgdrv::msgSearch("tot_lock_claimchest");
            } else {
                return ttyd::msgdrv::msgSearch("tot_lock_defeatenemies");
            }
            break;
        case MsgKey::TOT_UPG_SMOOCH:
            if (MoveManager::GetUnlockedLevel(MoveType::MOWZ_SMOOCH) >= 2) {
                return ttyd::msgdrv::msgSearch("tot_upg_smooch_lv3");
            }
            break;
    }
    // Should not be reached.
    return nullptr;
}

}