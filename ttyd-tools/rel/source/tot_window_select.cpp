#include "tot_window_select.h"

#include "common_functions.h"
#include "mod.h"
#include "tot_generate_item.h"
#include "tot_generate_reward.h"
#include "tot_manager_move.h"
#include "tot_manager_timer.h"
#include "tot_state.h"

#include <gc/types.h>
#include <gc/mtx.h>
#include <ttyd/dispdrv.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/fontmgr.h>
#include <ttyd/icondrv.h>
#include <ttyd/item_data.h>
#include <ttyd/mariost.h>
#include <ttyd/memory.h>
#include <ttyd/msgdrv.h>
#include <ttyd/pmario_sound.h>
#include <ttyd/sound.h>
#include <ttyd/statuswindow.h>
#include <ttyd/system.h>
#include <ttyd/win_main.h>
#include <ttyd/winmgr.h>
#include <ttyd/gx/GXPixel.h>
#include <ttyd/gx/GXTransform.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::tot::window_select {

namespace {

// For convenience.
using namespace ::ttyd::winmgr;

using ::ttyd::dispdrv::CameraId;
using ::ttyd::evtmgr::EvtEntry;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::item_data::itemDataTable;
using ::ttyd::mariost::g_MarioSt;
using ::ttyd::msgdrv::msgSearch;

namespace IconType = ::ttyd::icondrv::IconType;

struct OptionMenuData {
    int32_t     option;     // Either ToT state Options or WindowOptions.
    const char* name_msg;
    const char* help_msg;
    uint16_t    lookup_key;
    bool        in_run_options;
    bool        in_run_stats;
};

enum WindowOptions {
    // All window-specific options should have low negative values.
    WIN_STAT_RUN_AVG_TURNS = -100000,
    WIN_STAT_RUN_AVG_BATTLE_TIME,
    WIN_STAT_RUN_MOST_BATTLE_TIME,
    WIN_SEED_SELECT,
    WIN_SEED_RANDOM,
};

OptionMenuData g_OptionMenuData[] = {
    { STAT_RUN_TURNS_SPENT, "tot_optr_turnsspent", nullptr, 1, false, true },
    { WIN_STAT_RUN_AVG_TURNS, "tot_optr_turnsavg", nullptr, 2, false, true },
    { WIN_STAT_RUN_AVG_BATTLE_TIME, "tot_optr_battletimeavg", nullptr, 3, false, true },
    { STAT_RUN_MOST_TURNS_RECORD, "tot_optr_turnsmost", nullptr, 4, false, true },
    { WIN_STAT_RUN_MOST_BATTLE_TIME, "tot_optr_battletimemost", nullptr, 5, false, true },
    { STAT_RUN_TIMES_RAN_AWAY, "tot_optr_timesran", nullptr, 6, false, true },
    { STAT_RUN_ENEMY_DAMAGE, "tot_optr_enemydamage", nullptr, 7, false, true },
    { STAT_RUN_PLAYER_DAMAGE, "tot_optr_playerdamage", nullptr, 8, false, true },
    { STAT_RUN_ITEMS_USED, "tot_optr_itemsused", nullptr, 9, false, true },
    { STAT_RUN_SHINE_SPRITES, "tot_optr_shinesprites", nullptr, 10, false, true },
    { STAT_RUN_STAR_PIECES, "tot_optr_starpieces", nullptr, 11, false, true },
    { STAT_RUN_COINS_EARNED, "tot_optr_coinsearned", nullptr, 12, false, true },
    { STAT_RUN_COINS_SPENT, "tot_optr_coinsspent", nullptr, 13, false, true },
    { STAT_RUN_FP_SPENT, "tot_optr_fpspent", nullptr, 14, false, true },
    { STAT_RUN_SP_SPENT, "tot_optr_spspent", nullptr, 15, false, true },
    { STAT_RUN_SUPERGUARDS, "tot_optr_superguards", nullptr, 16, false, true },
    { STAT_RUN_CONDITIONS_MET, "tot_optr_conditionsmet", nullptr, 17, false, true },
    { WIN_SEED_SELECT, "tot_optr_seed", "tot_opth_seed", 90, true, false },
    { WIN_SEED_RANDOM, "tot_optr_seed_random", nullptr, 91, false, false },
    { OPT_PRESET, "tot_optr_preset", "tot_opth_preset", 100, true, false },
    { OPTVAL_PRESET_CUSTOM, "tot_optr_preset_custom", nullptr, 101, false, false },
    { OPTVAL_PRESET_DEFAULT, "tot_optr_preset_default", nullptr, 102, false, false },
    { OPT_DIFFICULTY, "tot_optr_difficulty", "tot_opth_difficulty", 118, true, false },
    { OPTVAL_DIFFICULTY_TUTORIAL, "tot_optr_diff_tutorial", nullptr, 119, false, false },
    { OPTVAL_DIFFICULTY_HALF, "tot_optr_diff_half", nullptr, 120, false, false },
    { OPTVAL_DIFFICULTY_FULL, "tot_optr_diff_full", nullptr, 121, false, false },
    { OPTVAL_DIFFICULTY_FULL_EX, "tot_optr_diff_ex", nullptr, 122, false, false },
    { OPT_TIMER_DISPLAY, "tot_optr_timertype", "tot_opth_timertype", 123, true, false },
    { OPTVAL_TIMER_NONE, "tot_optr_timer_none", nullptr, 124, false, false },
    { OPTVAL_TIMER_IGT, "tot_optr_timer_igt", nullptr, 125, false, false },
    { OPTVAL_TIMER_RTA, "tot_optr_timer_rta", nullptr, 126, false, false },
    { OPT_BATTLE_DROPS, "tot_optr_drops", "tot_opth_drops", 127, true, false },
    { OPTVAL_DROP_STANDARD, "tot_optr_drops_def", nullptr, 128, false, false },
    { OPTVAL_DROP_HELD_FROM_BONUS, "tot_optr_drops_gated", nullptr, 129, false, false },
    { OPTVAL_DROP_NO_HELD_W_BONUS, "tot_optr_drops_noheld", nullptr, 130, false, false },
    { OPTVAL_DROP_ALL_HELD, "tot_optr_drops_all", nullptr, 131, false, false },
    { OPT_STARTER_ITEMS, "tot_optr_startitems", "tot_opth_startitems", 132, true, false },
    { OPTVAL_STARTER_ITEMS_OFF, "tot_optr_startitems_off", nullptr, 133, false, false },
    { OPTVAL_STARTER_ITEMS_BASIC, "tot_optr_startitems_basic", nullptr, 134, false, false },
    { OPTVAL_STARTER_ITEMS_STRONG, "tot_optr_startitems_strong", nullptr, 135, false, false },
    { OPTVAL_STARTER_ITEMS_RANDOM, "tot_optr_startitems_random", nullptr, 136, false, false },
    { OPT_MAX_PARTNERS, "tot_optr_maxpartners", "tot_opth_maxpartners", 140, true, false },
    { OPTVAL_NO_PARTNERS, "tot_optr_nopartners", nullptr, 141, false, false },
    { OPT_PARTNER, "tot_optr_partner", "tot_opth_partner", 150, true, false },
    { OPTVAL_PARTNER_RANDOM, "tot_optr_partner_random", nullptr, 151, false, false },
    { OPTVAL_PARTNER_GOOMBELLA, "tot_optr_partner_1", nullptr, 152, false, false },
    { OPTVAL_PARTNER_KOOPS, "tot_optr_partner_2", nullptr, 153, false, false },
    { OPTVAL_PARTNER_FLURRIE, "tot_optr_partner_3", nullptr, 154, false, false },
    { OPTVAL_PARTNER_YOSHI, "tot_optr_partner_4", nullptr, 155, false, false },
    { OPTVAL_PARTNER_VIVIAN, "tot_optr_partner_5", nullptr, 156, false, false },
    { OPTVAL_PARTNER_BOBBERY, "tot_optr_partner_6", nullptr, 157, false, false },
    { OPTVAL_PARTNER_MOWZ, "tot_optr_partner_7", nullptr, 158, false, false },
    { OPT_REVIVE_PARTNERS, "tot_optr_revive", "tot_opth_revive", 160, true, false },
    { OPTVAL_REVIVE_PARTNERS_OFF, "tot_optr_off", nullptr, 161, false, false },
    { OPTVAL_REVIVE_PARTNERS_ON, "tot_optr_on", nullptr, 162, false, false },
    { OPT_MARIO_HP, "tot_optr_mhp", "tot_opth_mhp", 170, true, false },
    { OPT_MARIO_FP, "tot_optr_mfp", "tot_opth_mfp", 171, true, false },
    { OPT_MARIO_BP, "tot_optr_mbp", "tot_opth_mbp", 172, true, false },
    { OPTVAL_INFINITE_BP, "tot_optr_mbp_inf", nullptr, 173, false, false },
    { OPT_PARTNER_HP, "tot_optr_php", "tot_opth_php", 174, true, false },
    { OPT_INVENTORY_SACK_SIZE, "tot_optr_itemgain", "tot_opth_itemgain", 175, true, false },
    { OPTNUM_ENEMY_HP, "tot_optr_ehp", "tot_opth_ehp", 176, true, false },
    { OPTNUM_ENEMY_ATK, "tot_optr_eatk", "tot_opth_eatk", 177, true, false },
    { OPTNUM_SUPERGUARD_SP_COST, "tot_optr_supercost", "tot_opth_supercost", 178, true, false },
    { OPT_AC_DIFFICULTY, "tot_optr_ac", "tot_opth_ac", 180, true, false },
    { OPTVAL_AC_3_SIMP, "tot_optr_ac_0", nullptr, 181, false, false },
    { OPTVAL_AC_2_SIMP, "tot_optr_ac_1", nullptr, 182, false, false },
    { OPTVAL_AC_1_SIMP, "tot_optr_ac_2", nullptr, 183, false, false },
    { OPTVAL_AC_DEFAULT, "tot_optr_ac_3", nullptr, 184, false, false },
    { OPTVAL_AC_1_UNSIMP, "tot_optr_ac_4", nullptr, 185, false, false },
    { OPTVAL_AC_2_UNSIMP, "tot_optr_ac_5", nullptr, 186, false, false },
    { OPTVAL_AC_3_UNSIMP, "tot_optr_ac_6", nullptr, 187, false, false },
    { OPT_BANDIT_ESCAPE, "tot_optr_bandit", "tot_opth_bandit", 190, true, false },
    { OPTVAL_BANDIT_NO_REFIGHT, "tot_optr_bandit_flee", nullptr, 191, false, false },
    { OPTVAL_BANDIT_FORCE_REFIGHT, "tot_optr_bandit_fight", nullptr, 192, false, false },
    { OPT_CHARLIETON_STOCK, "tot_optr_charlie", "tot_opth_charlie", 210, true, false },
    { OPTVAL_CHARLIETON_NORMAL, "tot_optr_charlie_5", nullptr, 211, false, false },
    { OPTVAL_CHARLIETON_SMALLER, "tot_optr_charlie_3", nullptr, 212, false, false },
    { OPTVAL_CHARLIETON_LIMITED, "tot_optr_charlie_lim", nullptr, 213, false, false },
    { OPT_ENABLE_NPC_WONKY, "tot_optr_npc_wonky", "tot_opth_npc_wonky", 214, true, false },
    { OPTVAL_NPC_WONKY_OFF, "tot_optr_off", nullptr, 215, false, false },
    { OPTVAL_NPC_WONKY_ON, "tot_optr_on", nullptr, 216, false, false },
    { OPT_ENABLE_NPC_DAZZLE, "tot_optr_npc_dazzle", "tot_opth_npc_dazzle", 217, true, false },
    { OPTVAL_NPC_DAZZLE_OFF, "tot_optr_off", nullptr, 218, false, false },
    { OPTVAL_NPC_DAZZLE_ON, "tot_optr_on", nullptr, 219, false, false },
    { OPT_ENABLE_NPC_CHET_RIPPO, "tot_optr_npc_chet", "tot_opth_npc_chet", 220, true, false },
    { OPTVAL_NPC_CHET_RIPPO_OFF, "tot_optr_off", nullptr, 221, false, false },
    { OPTVAL_NPC_CHET_RIPPO_ON, "tot_optr_on", nullptr, 222, false, false },
    { OPT_ENABLE_NPC_LUMPY, "tot_optr_npc_lumpy", "tot_opth_npc_lumpy", 223, true, false },
    { OPTVAL_NPC_LUMPY_OFF, "tot_optr_off", nullptr, 224, false, false },
    { OPTVAL_NPC_LUMPY_ON, "tot_optr_on", nullptr, 225, false, false },
    { OPT_ENABLE_NPC_DOOPLISS, "tot_optr_npc_doopliss", "tot_opth_npc_doopliss", 226, true, false },
    { OPTVAL_NPC_DOOPLISS_OFF, "tot_optr_off", nullptr, 227, false, false },
    { OPTVAL_NPC_DOOPLISS_ON, "tot_optr_on", nullptr, 228, false, false },
    { OPT_ENABLE_NPC_GRUBBA, "tot_optr_npc_grubba", "tot_opth_npc_grubba", 229, true, false },
    { OPTVAL_NPC_GRUBBA_OFF, "tot_optr_off", nullptr, 230, false, false },
    { OPTVAL_NPC_GRUBBA_ON, "tot_optr_on", nullptr, 231, false, false },
    { OPT_ENABLE_NPC_MOVER, "tot_optr_npc_mover", "tot_opth_npc_mover", 232, true, false },
    { OPTVAL_NPC_MOVER_OFF, "tot_optr_off", nullptr, 233, false, false },
    { OPTVAL_NPC_MOVER_ON, "tot_optr_on", nullptr, 234, false, false },
    { OPT_ENABLE_NPC_ZESS_T, "tot_optr_npc_zess", "tot_opth_npc_zess", 235, true, false },
    { OPTVAL_NPC_ZESS_T_OFF, "tot_optr_off", nullptr, 236, false, false },
    { OPTVAL_NPC_ZESS_T_ON, "tot_optr_on", nullptr, 237, false, false },
};

uint32_t OptionLookup(uint16_t lookup_key) {
    for (const auto& data : g_OptionMenuData) {
        if (data.lookup_key == lookup_key) {
            return data.option;
        }
    }
    return -1;
}

const char* OptionName(uint16_t lookup_key) {
    for (const auto& data : g_OptionMenuData) {
        if (data.lookup_key == lookup_key) {
            return msgSearch(data.name_msg);
        }
    }
    // Option not found, return placeholder text.
    return "Option text missing!";
}

const char* OptionValue(uint16_t lookup_key) {
    static char buf[32];
    // Set to empty string every invocation, by default.
    buf[0] = 0;

    const auto& state = g_Mod->state_;
    
    int32_t option = 0;
    int32_t option_index = 0;
    for (const auto& data : g_OptionMenuData) {
        if (data.lookup_key == lookup_key) {
            option = data.option;
            break;
        }
        ++option_index;
    }

    // For run options, get the string representation instead, if applicable.
    if (option && g_OptionMenuData[option_index].in_run_options) {
        int32_t option_value = g_Mod->state_.GetOptionValue(option);
        if (option_value > 0) {
            constexpr const int32_t num_options =
                sizeof(g_OptionMenuData) / sizeof(OptionMenuData);
            for (int32_t i = option_index + 1; i < num_options; ++i) {
                if (option_value == g_OptionMenuData[i].option) {
                    return msgSearch(g_OptionMenuData[i].name_msg);
                }
            }
        }
    }

    switch (option) {
        case WIN_SEED_SELECT: {
            if (state.seed_ == 0) {
                return msgSearch(g_OptionMenuData[option_index + 1].name_msg);
            }
            sprintf(buf, "%09" PRId32, state.seed_);
            break;
        }
        case WIN_STAT_RUN_AVG_BATTLE_TIME: {
            int32_t total_time_centis = 0;
            int32_t battles = 0;
            for (int32_t i = 0; i <= 64; ++i) {
                const int32_t floor_time = state.splits_battle_igt_[i];
                if (floor_time > 0) {
                    total_time_centis += floor_time;
                    ++battles;
                }
            }
            if (battles == 0) battles = 1;
            total_time_centis /= battles;
            // Print only minutes and seconds, up to 1000 minutes.
            constexpr const int32_t kMaxLongestBattleTime = 1000 * 60 * 100;
            if (total_time_centis >= kMaxLongestBattleTime) { 
                total_time_centis = kMaxLongestBattleTime - 1;
            }
            const int32_t s = (total_time_centis / 100) % 60;
            const int32_t m = (total_time_centis / (60 * 100));
            sprintf(buf, "%" PRId32 ":%02" PRId32, m, s);
            break;
        }
        case WIN_STAT_RUN_MOST_BATTLE_TIME: {
            int32_t max_time_centis = 0;
            int32_t floor = 0;
            for (int32_t i = 0; i <= 64; ++i) {
                const int32_t floor_time = state.splits_battle_igt_[i];
                if (floor_time > max_time_centis) {
                    max_time_centis = floor_time;
                    floor = i;
                }
            }
            // Print only minutes and seconds, up to 1000 minutes.
            constexpr const int32_t kMaxLongestBattleTime = 1000 * 60 * 100;
            if (max_time_centis >= kMaxLongestBattleTime) { 
                max_time_centis = kMaxLongestBattleTime - 1;
            }
            const int32_t s = (max_time_centis / 100) % 60;
            const int32_t m = (max_time_centis / (60 * 100));
            sprintf(buf, 
                "%" PRId32 ":%02" PRId32 " (Fl. %" PRId32 ")", 
                m, s, floor);
            break;
        }
        case WIN_STAT_RUN_AVG_TURNS: {
            int32_t turns = state.GetOption(STAT_RUN_TURNS_SPENT);
            int32_t battles = 0;
            for (int32_t i = 0; i <= 64; ++i) {
                if (state.splits_battle_igt_[i] > 0) ++battles;
            }
            if (battles == 0) battles = 1;
            sprintf(buf, "%.02f", static_cast<float>(turns) / battles);
            break;
        }
        case STAT_RUN_MOST_TURNS_RECORD: {
            int32_t most = state.GetOption(STAT_RUN_MOST_TURNS_RECORD);
            int32_t floor = state.GetOption(STAT_RUN_MOST_TURNS_FLOOR);
            sprintf(buf, "%" PRId32 " (Fl. %" PRId32 ")", most, floor);
            break;
        }
        case STAT_RUN_CONDITIONS_MET: {
            int32_t met = state.GetOption(STAT_RUN_CONDITIONS_MET);
            int32_t total = state.GetOption(STAT_RUN_CONDITIONS_TOTAL);
            sprintf(buf, "%" PRId32 " / %" PRId32, met, total);
            break;
        }
        case OPTNUM_SUPERGUARD_SP_COST: {
            int32_t value = state.GetOption(OPTNUM_SUPERGUARD_SP_COST);
            sprintf(buf, "%.02f", value * 0.01f);
            break;
        }
        default: {
            int32_t val = state.GetOption(option);
            IntegerToFmtString(val, buf);
            break;
        }
        case 0:
            break;
    }

    return buf;
}

void SelectMainOptionsWrapper(WinMgrEntry* entry) {
    auto* sel_entry = (WinMgrSelectEntry*)entry->param;
    auto& state = g_Mod->state_;

    // Handle special inputs for run options window.
    if (sel_entry->type == MenuType::RUN_OPTIONS && 
        sel_entry->state == WinMgrSelectEntry_State::IN_SELECTION) {
        // Incredibly hacky: Replace A presses with B presses.
        if (g_MarioSt->gamepad_buttons_pressed[0] & ButtonId::A) {
            g_MarioSt->gamepad_buttons_pressed[0] &= ~ButtonId::A;
            g_MarioSt->gamepad_buttons_pressed[0] |= ButtonId::B;
        }
        // Handle left/right presses (changing option).
        uint16_t dir_rep = ttyd::system::keyGetDirRep(0);
        int32_t change = 0;
        if (dir_rep & DirectionInputId::ANALOG_LEFT) {
            --change;
        } else if (dir_rep & DirectionInputId::ANALOG_RIGHT) {
            ++change;
        }

        int32_t value = sel_entry->row_data[sel_entry->cursor_index].value;
        uint32_t option = OptionLookup(value);

        if ((int32_t)option == WIN_SEED_SELECT) {
            // Modify the selected seed 
            uint32_t buttons = g_MarioSt->gamepad_buttons_pressed[0];
            if (buttons & ButtonId::X) {
                state.PickRandomSeed();
                // Play menu back-out sound to indicate randomization action.
                ttyd::pmario_sound::psndSFXOn((const char*)0x2002b);
            } else if (buttons & ButtonId::L) {
                state.seed_ = (state.seed_ % 100'000'000) * 10;
                ttyd::pmario_sound::psndSFXOn((const char*)0x20005);
            } else if (buttons & ButtonId::R) {
                state.seed_ /= 10;
                ttyd::pmario_sound::psndSFXOn((const char*)0x20005);
            } else if (change) {
                if (change == 1 && state.seed_ == 999'999'999) {
                    state.seed_ = 0;
                } else if (change == -1 && state.seed_ == 0) {
                    state.seed_ = 999'999'999;
                } else {
                    state.seed_ += change;
                }
                ttyd::pmario_sound::psndSFXOn((const char*)0x20005);
            }
        } else if (change) {
            // Only allow changing preset, difficulty and timer options if
            // a non-custom preset is selected.
            if (!state.CheckOptionValue(OPTVAL_PRESET_CUSTOM) &&
                option != OPT_PRESET && option != OPT_DIFFICULTY &&
                option != OPT_TIMER_DISPLAY) {
                // Play "failure" sound.
                ttyd::sound::SoundEfxPlayEx(0x266, 0, 0x64, 0x40);
            } else {
                // Change the option, wrapping around if necessary.
                state.NextOption(option, change);
                // Play selection sound.
                ttyd::pmario_sound::psndSFXOn((const char*)0x20005);
                // Re-enforce current preset's settings, if not custom.
                if (!state.CheckOptionValue(OPTVAL_PRESET_CUSTOM)) {
                    state.ApplyPresetOptions();
                }
            }
        }
    }

    // Run vanilla selection main function.
    select_main(entry);
}

void DispTimerSplits(WinMgrEntry* entry) {
    uint32_t kBlack = 0x0000'00FFU;
    uint32_t kReds[] = { 0xFF00'00FFU, 0xAA00'00FFU, 0xFF55'AAFFU, 0xCC2080FFU };
    uint32_t kBlues[] = { 0x0000'FFFFU, 0x0000'A0FFU, 0x1094'FFFFU, 0x0060'C0FFU };

    // Approximate max bounds of actual graph.
    float x_min = entry->x + 70.f;
    float x_max = entry->x + entry->width - 10.f;
    float y_min = entry->y - entry->height + 30.f;
    float y_max = entry->y - 25.f;

    uint32_t* battle_splits = g_Mod->state_.splits_battle_igt_;
    uint32_t* main_splits = g_Mod->state_.splits_igt_;
    if (g_Mod->state_.CheckOptionValue(OPTVAL_TIMER_RTA)) {
        main_splits = g_Mod->state_.splits_rta_;
    }

    // Get the maximum time, rounded up to the nearest half-minute.
    uint32_t max_time = 0;
    for (int32_t i = 0; i <= 64; ++i) {
        max_time = Max(max_time, main_splits[i]);
    }
    max_time = (max_time + 2999) / 3000;
    // Maximum of 10 minutes.
    if (max_time > 20) max_time = 20;

    ttyd::win_main::winFontInit();

    char buf[8] = { 0 };
    // Draw reference strings for 25%, 50%, 75%, 100% of time scale.
    for (int32_t i = 0; i <= 4; ++i) {
        int32_t mins = max_time * i / 8;
        int32_t secs = (max_time * i % 8) * 7.5f;
        sprintf(buf, "%" PRId32 "\"%02" PRId32, mins, secs);

        gc::vec3 pos = {
            entry->x + 15.f,
            y_min + i * (y_max - y_min) / 4 + 12.f,
            0.0f
        };
        gc::vec3 scale = { 1.0f, 0.8f, 1.0f };
        ttyd::win_main::winFontSetWidth(&pos, &scale, &kBlack, 40.0f, buf);
    }

    // Write strings for legend.
    gc::vec3 text_pos = { x_min + 33.f, y_min - 5.f, 0.0f };
    gc::vec3 text_scale = { 0.6f, 0.6f, 0.6f };
    ttyd::win_main::winFontSet(&text_pos, &text_scale, &kReds[0], "In-Battle Time");
    text_pos.x += 160.f;
    ttyd::win_main::winFontSet(&text_pos, &text_scale, &kBlues[0], "Out-of-Battle Time");

    ttyd::win_main::winIconInit();

    // Draw colored square icons for legend.
    gc::mtx34 mtx, mtx_s, mtx_t;
    gc::mtx::PSMTXScale(&mtx_s, 0.5f, 0.5f, 0.5f);

    gc::vec3 icon_pos = { x_min + 15.f, y_min - 20.f, 0.0f };
    gc::mtx::PSMTXTrans(&mtx_t, icon_pos.x, icon_pos.y, icon_pos.z);
    gc::mtx::PSMTXConcat(&mtx_t, &mtx_s, &mtx);
    ttyd::icondrv::iconDispGxCol(&mtx, 0x10, IconType::TOT_BLANK, &kReds[0]);

    icon_pos.x += 160.f;
    gc::mtx::PSMTXTrans(&mtx_t, icon_pos.x, icon_pos.y, icon_pos.z);
    gc::mtx::PSMTXConcat(&mtx_t, &mtx_s, &mtx);
    ttyd::icondrv::iconDispGxCol(&mtx, 0x10, IconType::TOT_BLANK, &kBlues[0]);
    
    // Draw actual graph based on splits / max time.
    const int32_t kIconSize = 32.0f;
    const int32_t max_floor = g_Mod->state_.floor_;
    for (int32_t i = 0; i <= max_floor; ++i) {
        // Choose color index; odd numbers are lighter shades, 
        // odd floor groups are lighter base colors.
        int32_t c = (i-1) / 8 % 2 == 0 ? 0 : 2;
        c += (i % 2 == 1) ? 0 : 1;

        gc::mtx34 mtx, mtx_s, mtx_t;
        gc::mtx::PSMTXTrans(
            &mtx_t, x_min + (x_max - x_min) * i / (max_floor + 1), y_min, 0.0f);

        float scale_x = (x_max - x_min) / (max_floor + 1) / kIconSize;
        float scale_y;

        scale_y =
            Clamp(main_splits[i] / (max_time * 3000.0f), 0.0f, 1.0f) * 
            (y_max - y_min) / kIconSize;

        gc::mtx::PSMTXScale(&mtx_s, scale_x, scale_y, 1.0f);
        gc::mtx::PSMTXConcat(&mtx_t, &mtx_s, &mtx);
        ttyd::icondrv::iconDispGxCol(&mtx, 0x10, IconType::TOT_BLANK, &kBlues[c]);

        scale_y =
            Clamp(battle_splits[i] / (max_time * 3000.0f), 0.0f, 1.0f) * 
            (y_max - y_min) / kIconSize;

        gc::mtx::PSMTXScale(&mtx_s, scale_x, scale_y, 1.0f);
        gc::mtx::PSMTXConcat(&mtx_t, &mtx_s, &mtx);
        ttyd::icondrv::iconDispGxCol(&mtx, 0x10, IconType::TOT_BLANK, &kReds[c]);
    }
}

void DispMainWindow(WinMgrEntry* entry) {
    auto* sel_entry = (WinMgrSelectEntry*)entry->param;
    if ((winmgr_work->entries[sel_entry->entry_indices[0]].flags & 
        WinMgrEntry_Flags::IN_FADE) != 0) return;
    
    uint32_t kWhite = 0xFFFF'FFFFU;
    uint32_t kOffWhite = 0xF0F0'F0FFU;
    uint32_t kMedGrey = 0xA0A0'A0FFU;
    uint32_t kBlack = 0x0000'00FFU;
    
    ttyd::gx::GXPixel::GXSetFog(/* GX_FOG_NONE */ 0, 0, 0, 0, 0, &kWhite);
    // Save previous scissor bounds.
    int32_t scissor_0, scissor_1, scissor_2, scissor_3;
    ttyd::gx::GXTransform::GXGetScissor(
        &scissor_0, &scissor_1, &scissor_2, &scissor_3);
    ttyd::gx::GXTransform::GXSetScissor(
        entry->x + 304, -entry->y + 34 + 240, entry->width, entry->height - 50);
    
    int32_t offset = 0;
    for (int32_t i = 0; i < sel_entry->num_rows; ++i) {
        auto& row = sel_entry->row_data[i];
        float y_trans = sel_entry->list_y_offset + entry->y - 44 - offset;
        
        // Only draw info for rows that are visible.
        if (y_trans - 32 <= entry->y && 
            entry->y - entry->height <= y_trans + 32) {

            // For RUN_OPTIONS, set greyed-out color based on preset / option.
            if (sel_entry->type == MenuType::RUN_OPTIONS) {
                uint32_t option = OptionLookup(row.value);
                if (g_Mod->state_.CheckOptionValue(OPTVAL_PRESET_CUSTOM) ||
                    option == OPT_PRESET || option == OPT_DIFFICULTY ||
                    option == OPT_TIMER_DISPLAY || 
                    (int32_t)option == WIN_SEED_SELECT) {
                    row.flags &= ~WinMgrSelectEntryRow_Flags::GREYED_OUT;
                } else {
                    row.flags |= WinMgrSelectEntryRow_Flags::GREYED_OUT;
                }
                // If no partner mode, grey out partner-related options.
                if (g_Mod->state_.CheckOptionValue(OPTVAL_NO_PARTNERS) &&
                    (option == OPT_PARTNER || option == OPT_REVIVE_PARTNERS ||
                     option == OPT_PARTNER_HP)) {
                    row.flags |= WinMgrSelectEntryRow_Flags::GREYED_OUT;
                }
            }

            uint32_t* text_color;
            if (row.flags & WinMgrSelectEntryRow_Flags::GREYED_OUT) {
                ttyd::win_main::winIconGrayInit();
                text_color = &kMedGrey;
            } else {
                ttyd::win_main::winIconInit();
                text_color = &kBlack;
            }
            
            int32_t entry_icon = -1;
            const char* entry_text = "";
            float max_width = 185.0f;

            switch (sel_entry->type) {
                case MenuType::CUSTOM_START:
                case MenuType::TOT_CHARLIETON_SHOP: {
                    entry_icon = itemDataTable[row.value].icon_id;
                    entry_text = msgSearch(itemDataTable[row.value].name);
                    break;
                }
                case MenuType::TOT_CHET_RIPPO_TRADE: {
                    switch (row.value) {
                        case 1:
                            entry_icon = IconType::HP_ICON;
                            entry_text = "HP";
                            break;
                        case 2:
                            entry_icon = IconType::HP_ICON;
                            entry_text = "Partner HP";
                            break;
                        case 3:
                            entry_icon = IconType::FP_ICON;
                            entry_text = "FP";
                            break;
                        case 4:
                            entry_icon = IconType::BP_ICON;
                            entry_text = "BP";
                            break;
                    }
                    max_width = 120.0f;
                    break;
                }
                case MenuType::MOVE_UNLOCK:
                case MenuType::MOVE_UPGRADE: {
                    auto* move_data = MoveManager::GetMoveData(row.value);
                    entry_icon = move_data->icon_id;
                    entry_text = msgSearch(move_data->name_msg);
                    if (sel_entry->type == MenuType::MOVE_UPGRADE) 
                        max_width -= 13.f;
                    break;
                }
                case MenuType::RUN_OPTIONS: {
                    entry_text = OptionName(row.value);
                    max_width = 200.f;
                    break;
                }
                case MenuType::RUN_RESULTS_STATS: {
                    entry_text = OptionName(row.value);
                    max_width = 300.f;
                    break;
                }
            }
            
            float x_pos = entry->x + 30.0f;
            if (entry_icon >= 0) {
                x_pos += 5.0f;
                gc::vec3 pos = { x_pos, y_trans, 0.0f };
                gc::vec3 scale = { 0.5f, 0.5f, 0.5f };
                ttyd::win_main::winIconSet(entry_icon, &pos, &scale, &kWhite);
                x_pos += 25.0f;
            }
            
            ttyd::win_main::winFontInit();
            gc::vec3 text_pos = { x_pos, y_trans + 12.0f, 0.0f };
            gc::vec3 text_scale = { 1.0f, 1.0f, 1.0f };
            ttyd::win_main::winFontSetWidth(
                &text_pos, &text_scale, text_color, max_width, entry_text);
        }
        
        switch (sel_entry->type) {
            case MenuType::CUSTOM_START:
            case MenuType::TOT_CHARLIETON_SHOP: {
                // Draw "buy prices".
                int32_t value = itemDataTable[row.value].buy_price;
                // For Charlieton, scale based on tower progress.
                if (sel_entry->type == MenuType::TOT_CHARLIETON_SHOP) {
                    value = value * GetBuyPriceScale() / 100;
                }
                
                if (value > 0) {
                    char buf[8] = { 0 };
                    sprintf(buf, "%" PRId32 "", value);
                    int32_t length = ttyd::fontmgr::FontGetMessageWidth(buf);
                    if (length > 30) length = 30;
                    ttyd::win_main::winFontInit();
                    
                    uint32_t* text_color;
                    if (row.flags & WinMgrSelectEntryRow_Flags::GREYED_OUT) {
                        text_color = &kMedGrey;
                    } else {
                        text_color = &kBlack;
                    }
                    gc::vec3 text_pos = {
                        entry->x + entry->width - 10.0f - length,
                        y_trans + 12.0f,
                        0.0f
                    };
                    gc::vec3 text_scale = { 1.0f, 1.0f, 1.0f };
                    ttyd::win_main::winFontSetWidth(
                        &text_pos, &text_scale, text_color, 30.0, buf);
                }
                break;
            }
            case MenuType::TOT_CHET_RIPPO_TRADE: {
                // Draw current and destination stat level.

                char buf[8] = { 0 };
                int current_level = 0;
                switch (row.value) {
                    case 1:
                        current_level = g_Mod->state_.hp_level_;
                        break;
                    case 2:
                        current_level = g_Mod->state_.hp_p_level_;
                        break;
                    case 3:
                        current_level = g_Mod->state_.fp_level_;
                        break;
                    case 4:
                        current_level = g_Mod->state_.bp_level_;
                        break;
                }

                ttyd::win_main::winFontInit();
                
                sprintf(buf, "%" PRId32 "", current_level);
                int32_t length = ttyd::fontmgr::FontGetMessageWidth(buf);
                if (length > 30.0f) length = 30.f;

                gc::vec3 text_pos = {
                    entry->x + entry->width - 80.0f - length,
                    y_trans + 12.0f,
                    0.0f
                };
                gc::vec3 text_scale = { 1.0f, 1.0f, 1.0f };
                uint32_t* text_color;
                if (row.flags & WinMgrSelectEntryRow_Flags::GREYED_OUT) {
                    text_color = &kMedGrey;
                } else {
                    text_color = &kBlack;
                }
                
                ttyd::win_main::winFontSetWidth(
                    &text_pos, &text_scale, text_color, 30.0, buf);

                if (current_level > 0) {
                    sprintf(buf, "%" PRId32 "", current_level - 1);
                    int32_t length = ttyd::fontmgr::FontGetMessageWidth(buf);
                    if (length > 30.0f) length = 30.f;
                    
                    text_pos.x = entry->x + entry->width - 20.0f - length;
                    ttyd::win_main::winFontSetWidth(
                        &text_pos, &text_scale, text_color, 30.0, buf);

                    ttyd::win_main::winIconInit();
                    gc::vec3 pos = { 
                        entry->x + entry->width - 65.f,
                        y_trans,
                        0.0f 
                    };
                    gc::vec3 scale = { -0.5f, 0.5f, 0.5f };
                    ttyd::win_main::winIconSet(
                        IconType::RUN_AWAY, &pos, &scale, &kWhite);
                }
                
                break;
            }
            case MenuType::MOVE_UPGRADE: {
                // Draw the level of the upgraded move.
                const char* lvl_string = 
                    MoveManager::GetUnlockedLevel(row.value) == 1 ? "2" : "3";
                
                uint32_t* text_color;
                if (row.flags & WinMgrSelectEntryRow_Flags::GREYED_OUT) {
                    text_color = &kMedGrey;
                } else {
                    text_color = &kBlack;
                }
                gc::vec3 text_pos = {
                    entry->x + entry->width - 25.0f,
                    y_trans + 12.0f,
                    0.0f
                };
                gc::vec3 text_scale = { 1.0f, 1.0f, 1.0f };
                ttyd::win_main::winFontInit();
                ttyd::win_main::winFontSetWidth(
                    &text_pos, &text_scale, text_color, 15.0, lvl_string);
                
                text_pos.x -= 27.0f;
                text_pos.y -= 10.0f;
                gc::vec3 lvl_text_scale = { 0.5f, 0.5f, 0.5f };
                ttyd::win_main::winFontInit();
                ttyd::win_main::winFontSetWidth(
                    &text_pos, &lvl_text_scale, text_color, 27.0, "Lvl.");
                
                break;
            }
            case MenuType::RUN_OPTIONS:
            case MenuType::RUN_RESULTS_STATS: {
                // Draw the value / string representation of the stat.
                const char* stat_text = OptionValue(row.value);

                // Options has longer option strings, is centered.
                const float length_factor =
                    sel_entry->type == MenuType::RUN_OPTIONS ? 0.5f : 1.0f;
                const int32_t max_length =
                    sel_entry->type == MenuType::RUN_OPTIONS ? 220 : 140;
                const int32_t offset =
                    sel_entry->type == MenuType::RUN_OPTIONS ? 130 : 20;

                if (stat_text) {
                    ttyd::win_main::winFontInit();

                    int32_t length = 
                        ttyd::fontmgr::FontGetMessageWidth(stat_text);
                    float x_scale = 1.0f;
                    if (length > max_length) {
                        x_scale = static_cast<float>(max_length) / length;
                        length = max_length;
                    }
                    
                    uint32_t* text_color;
                    if (row.flags & WinMgrSelectEntryRow_Flags::GREYED_OUT) {
                        text_color = &kMedGrey;
                    } else {
                        text_color = &kBlack;
                    }
                    gc::vec3 text_pos = {
                        entry->x + entry->width - offset - length * length_factor,
                        y_trans + 12.0f,
                        0.0f
                    };
                    gc::vec3 text_scale = { x_scale, 1.0f, 1.0f };
                    ttyd::win_main::winFontSet(
                        &text_pos, &text_scale, text_color, stat_text);
                }
                
                break;
            }
        }
        
        offset += 24;
    }
    
    // Restore previous scissor boundaries.
    ttyd::gx::GXTransform::GXSetScissor(
        scissor_0, scissor_1, scissor_2, scissor_3);

    // Draw timer splits.
    if (sel_entry->type == MenuType::RUN_RESULTS_SPLITS) {
        DispTimerSplits(entry);
    }
    
    const char* title = nullptr;
    switch (sel_entry->type) {
        case MenuType::CUSTOM_START:
            title = msgSearch("in_konran_hammer");
            break;
        case MenuType::TOT_CHARLIETON_SHOP:
            title = msgSearch("msg_window_title_1");  // "Items"
            break;
        case MenuType::TOT_CHET_RIPPO_TRADE:
            title = msgSearch("tot_winsel_titlestat");
            break;
        case MenuType::MOVE_UNLOCK:
        case MenuType::MOVE_UPGRADE:
            title = msgSearch("tot_winsel_titlemove");
            break;
        case MenuType::RUN_OPTIONS:
            title = msgSearch("tot_winsel_runoptions_header");
            break;
        case MenuType::RUN_RESULTS_STATS:
            title = msgSearch("tot_winsel_runresults_header");
            break;
        case MenuType::RUN_RESULTS_SPLITS:
            title = msgSearch("tot_winsel_runsplits_header");
            break;
    }
    if (title) {
        int32_t length = ttyd::fontmgr::FontGetMessageWidth(title);
        if (length > 120) length = 120;
        ttyd::win_main::winFontInit();
        
        gc::vec3 text_pos = {
            entry->x + (entry->width - length) * 0.5f,
            entry->y + 14.0f,
            0.0f
        };
        gc::vec3 text_scale = { 1.0f, 1.0f, 1.0f };
        ttyd::win_main::winFontSetEdgeWidth(
            &text_pos, &text_scale, &kWhite, 120.0, title);
    }

    // Draw white circle + currency icon in upper-right corner.
    switch (sel_entry->type) {
        case MenuType::CUSTOM_START:
        case MenuType::TOT_CHARLIETON_SHOP:
            gc::mtx34 mtx, mtx2;
            gc::mtx::PSMTXScale(&mtx2, 0.6f, 0.6f, 0.6f);
            gc::mtx::PSMTXTrans(
                &mtx, entry->x + 268.0f, entry->y - 18.0f, 0.0f);
            gc::mtx::PSMTXConcat(&mtx, &mtx2, &mtx);
            ttyd::icondrv::iconDispGxCol(
                &mtx, 0x10, IconType::BLACK_WITH_WHITE_CIRCLE, &kOffWhite);

            gc::vec3 icon_pos = { entry->x + 268.0f, entry->y - 12.0f, 0.0f };
            ttyd::icondrv::iconDispGx(0.6f, &icon_pos, 0x10, IconType::COIN);
            break;
    }
    
    // Draw cursor and scrolling arrows, if necessary.

    if (sel_entry->type != MenuType::RUN_RESULTS_SPLITS) {
        gc::vec3 cursor_pos = { 
            sel_entry->cursor_x, sel_entry->cursor_y, 1.0f 
        };
        ttyd::icondrv::iconDispGx(
            1.0f, &cursor_pos, 0x14, IconType::GLOVE_POINTER_H);
    }
    
    if (sel_entry->num_rows > 8 &&
        (g_MarioSt->currentRetraceCount & 0x1f) < 20) {
        if (sel_entry->list_row_offset != 0) {
            gc::vec3 pos = {
                entry->width * 0.5f +  entry->x,
                entry->y - 36.0f,
                1.0f
            };
            ttyd::icondrv::iconDispGx(
                0.6f, &pos, 0x10, IconType::MENU_UP_POINTER);
        }
        if (sel_entry->list_row_offset != sel_entry->num_rows - 8) {
            gc::vec3 pos = {
                entry->width * 0.5f +  entry->x,
                entry->y - entry->height - 12.0f,
                1.0f
            };
            ttyd::icondrv::iconDispGx(
                0.6f, &pos, 0x10, IconType::MENU_DOWN_POINTER);
        }
    }
}

void DispWindow2(WinMgrEntry* entry) {
    auto* sel_entry = (WinMgrSelectEntry*)entry->param;
    if ((winmgr_work->entries[sel_entry->entry_indices[1]].flags & 
        WinMgrEntry_Flags::IN_FADE) != 0) return;
    
    const char* msg = "";
    switch (sel_entry->type) {
        case MenuType::CUSTOM_START:
            msg = msgSearch("in_konran_hammer");
            break;
        case MenuType::TOT_CHARLIETON_SHOP:
            msg = msgSearch("msg_window_select_6");     // "Buy which one?"
            break;
        case MenuType::TOT_CHET_RIPPO_TRADE:
            msg = msgSearch("tot_winsel_tradewhichstat");
            break;
        case MenuType::MOVE_UNLOCK:
        case MenuType::MOVE_UPGRADE:
            msg = msgSearch("tot_winsel_whichunlock");
            break;
    }
    
    uint16_t lines;
    int32_t length = ttyd::fontmgr::FontGetMessageWidthLine(msg, &lines);
    
    gc::mtx34 mtx, mtx2;
    if (length <= entry->width - 30) {
        gc::mtx::PSMTXScale(&mtx2, 1.0, 1.0, 1.0);
    } else {
        gc::mtx::PSMTXScale(&mtx2, (entry->width - 30.0) / length, 1.0, 1.0);
        length = entry->width - 30;
    }
    gc::mtx::PSMTXTrans(
        &mtx,
        entry->x + (entry->width - length) * 0.5,
        entry->y - (entry->height - (lines + 1) * 24) / 2,
        0.0);
    gc::mtx::PSMTXConcat(&mtx, &mtx2, &mtx);
    ttyd::fontmgr::FontDrawStart();
    ttyd::fontmgr::FontDrawMessageMtx(&mtx, msg);
    
    auto& lookup_entry = winmgr_work->entries[sel_entry->entry_indices[1]];
    if (lookup_entry.flags & WinMgrEntry_Flags::IN_USE) {
        lookup_entry.y = entry->desc->y + lines * 11;
        lookup_entry.height = entry->desc->height + lines * 11;
    }
}

void DispSelectionHelp(WinMgrEntry* entry) {
    auto* sel_entry = (WinMgrSelectEntry*)entry->param;
    if ((winmgr_work->entries[sel_entry->entry_indices[2]].flags &
        WinMgrEntry_Flags::IN_FADE) == 0) {
            
        int32_t value = sel_entry->row_data[sel_entry->cursor_index].value;
        
        const char* help_msg = "";
        switch (sel_entry->type) {
            case MenuType::CUSTOM_START:
                help_msg = msgSearch("in_konran_hammer");
                break;
            case MenuType::TOT_CHARLIETON_SHOP:
                help_msg = msgSearch(itemDataTable[value].description);
                break;
            case MenuType::TOT_CHET_RIPPO_TRADE:
                help_msg = msgSearch("tot_desc_chet_adjustnone");
                switch (value) {
                    case 1:
                        if (g_Mod->state_.hp_level_ > 0) {
                            help_msg = msgSearch("tot_desc_chet_adjusthp");
                        }
                        break;
                    case 2:
                        if (g_Mod->state_.hp_p_level_ > 0) {
                            help_msg = msgSearch("tot_desc_chet_adjustphp");
                        }
                        break;
                    case 3:
                        if (g_Mod->state_.fp_level_ > 0) {
                            help_msg = msgSearch("tot_desc_chet_adjustfp");
                        }
                        break;
                    case 4:
                        if (g_Mod->state_.bp_level_ > 0) {
                            help_msg = msgSearch("tot_desc_chet_adjustbp");
                        }
                        break;
                }
                break;
            case MenuType::MOVE_UNLOCK:
                help_msg = msgSearch(
                    MoveManager::GetMoveData(value)->desc_msg);
                break;
            case MenuType::MOVE_UPGRADE:
                help_msg = msgSearch(
                    MoveManager::GetMoveData(value)->upgrade_msg);
                break;
            case MenuType::RUN_OPTIONS: {
                help_msg = "No option selected.";
                for (const auto& data : g_OptionMenuData) {
                    if (data.lookup_key == value) {
                        help_msg = msgSearch(data.help_msg);
                        break;
                    }
                }
                break;
            }
        }
        entry->help_msg = help_msg;
        winMgrHelpDraw(entry);
    }
}

void DispOptionsWindowTopBar(WinMgrEntry* entry) {
    auto* sel_entry = (WinMgrSelectEntry*)entry->param;
    if ((winmgr_work->entries[sel_entry->entry_indices[1]].flags & 
        WinMgrEntry_Flags::IN_FADE) != 0) return;

    uint32_t kBlack = 0x0000'00FFU;
    uint32_t kBlue  = 0x0000'C0FFU;
    const float min_x = entry->x + 12.f;
    const float max_x = entry->x + entry->width - 12.f;
    const float top_y = entry->y - 10.f;
    const float space_y = -24.0f;
    
    char text[24];
    gc::vec3 text_pos = { min_x, top_y, 0.0f };
    gc::vec3 text_scale = { 0.9f, 1.0f, 1.0f };
    
    ttyd::fontmgr::FontDrawStart();
    
    sprintf(text, "Seed: ");
    ttyd::win_main::winFontSet(&text_pos, &text_scale, &kBlack, text);
    text_pos.x += ttyd::fontmgr::FontGetMessageWidth(text) * text_scale.x;
    sprintf(text, "%09" PRId32, g_Mod->state_.seed_);
    ttyd::win_main::winFontSet(&text_pos, &text_scale, &kBlue, text);

    text_pos.x = min_x;
    text_pos.y += space_y;
    sprintf(text, "Difficulty: ");
    ttyd::win_main::winFontSet(&text_pos, &text_scale, &kBlack, text);
    text_pos.x += ttyd::fontmgr::FontGetMessageWidth(text) * text_scale.x;
    switch (g_Mod->state_.GetOptionValue(OPT_DIFFICULTY)) {
        case OPTVAL_DIFFICULTY_HALF:
            sprintf(text, "32F");
            break;
        case OPTVAL_DIFFICULTY_FULL:
            sprintf(text, "64F");
            break;
        case OPTVAL_DIFFICULTY_FULL_EX:
            sprintf(text, "64F (EX)");
            break;
    }
    ttyd::win_main::winFontSet(&text_pos, &text_scale, &kBlue, text);

    text_pos.x = min_x;
    text_pos.y += space_y;
    sprintf(text, "Options: ");
    ttyd::win_main::winFontSet(&text_pos, &text_scale, &kBlack, text);
    text_pos.x += ttyd::fontmgr::FontGetMessageWidth(text) * text_scale.x;
    sprintf(text, g_Mod->state_.GetEncodedOptions());
    ttyd::win_main::winFontSet(&text_pos, &text_scale, &kBlue, text);

    text_pos.x = max_x;
    text_pos.y = top_y;
    DurationCentisToFmtString(
        TimerManager::GetCurrentRunTotalTimeCentis(), text);
    text_pos.x -= ttyd::fontmgr::FontGetMessageWidth(text) * text_scale.x;
    ttyd::win_main::winFontSet(&text_pos, &text_scale, &kBlue, text);
    switch (g_Mod->state_.GetOptionValue(OPT_TIMER_DISPLAY)) {
        case OPTVAL_TIMER_RTA:
            sprintf(text, "RTA Time: ");
            break;
        default:
            sprintf(text, "Total Time: ");
            break;
    }
    text_pos.x -= ttyd::fontmgr::FontGetMessageWidth(text) * text_scale.x;
    ttyd::win_main::winFontSet(&text_pos, &text_scale, &kBlack, text);

    text_pos.x = max_x;
    text_pos.y += space_y;
    DurationCentisToFmtString(
        TimerManager::GetCurrentRunTotalBattleTimeCentis(), text);
    text_pos.x -= ttyd::fontmgr::FontGetMessageWidth(text) * text_scale.x;
    ttyd::win_main::winFontSet(&text_pos, &text_scale, &kBlue, text);
    sprintf(text, "In-Battle: ");
    text_pos.x -= ttyd::fontmgr::FontGetMessageWidth(text) * text_scale.x;
    ttyd::win_main::winFontSet(&text_pos, &text_scale, &kBlack, text);

    text_pos.x = max_x;
    text_pos.y += space_y;
    sprintf(text, "%" PRId32, g_Mod->state_.GetOption(STAT_RUN_CONTINUES));
    text_pos.x -= ttyd::fontmgr::FontGetMessageWidth(text) * text_scale.x;
    ttyd::win_main::winFontSet(&text_pos, &text_scale, &kBlue, text);
    sprintf(text, "Continues: ");
    text_pos.x -= ttyd::fontmgr::FontGetMessageWidth(text) * text_scale.x;
    ttyd::win_main::winFontSet(&text_pos, &text_scale, &kBlack, text);
}

void DispOptionsWindowBottomBar(WinMgrEntry* entry) {
    auto* sel_entry = (WinMgrSelectEntry*)entry->param;
    if ((winmgr_work->entries[sel_entry->entry_indices[1]].flags & 
        WinMgrEntry_Flags::IN_FADE) != 0) return;

    bool kIsOptions = sel_entry->type == MenuType::RUN_OPTIONS;
    uint32_t kBlack = 0x0000'00FFU;
    uint32_t kWhite = 0xFFFF'FFFFU;
    const float kContinueOffset = 0.17f;
    const float kBackOffset = 0.64f;

    ttyd::fontmgr::FontDrawStart();

    const char* msg = kIsOptions ? "Selection" : "Continue";
    int32_t length = ttyd::fontmgr::FontGetMessageWidth(msg);
    gc::vec3 text_pos = {
        entry->x + entry->width * (kContinueOffset + 0.22f) - length * 0.5f,
        entry->y - 9.f,
        0.0f
    };
    gc::vec3 text_scale = { 1.0f, 1.0f, 1.0f };
    ttyd::win_main::winFontSet(&text_pos, &text_scale, &kBlack, msg);

    msg = "Back";
    length = ttyd::fontmgr::FontGetMessageWidth(msg);
    text_pos.x = entry->x + entry->width * (kBackOffset + 0.15f) - length * 0.5f;    
    ttyd::win_main::winFontSet(&text_pos, &text_scale, &kBlack, msg);

    // Draw A and B button icons.
    ttyd::win_main::winIconInit();
    gc::vec3 pos = { 
        entry->x + entry->width * kContinueOffset, 
        entry->y - 20.0f,
        0.0f
    };
    gc::vec3 scale = { 0.75f, 0.75f, 0.75f };
    ttyd::win_main::winIconSet(
        kIsOptions ? IconType::CONTROL_STICK_CENTER : IconType::A_BUTTON, 
        &pos, &scale, &kWhite);

    pos.x = entry->x + entry->width * kBackOffset;
    ttyd::win_main::winIconSet(IconType::B_BUTTON, &pos, &scale, &kWhite);
}

WinMgrSelectDescList g_SelectDescList[MenuType::MAX_MENU_TYPE];

WinMgrDesc g_CustomDescs[] = {
    // Descs 0-2: "shop"-like windows
    {
        .fade_mode = WinMgrDesc_FadeMode::SCALE_AND_ROTATE,
        .heading_type = WinMgrDesc_HeadingType::SINGLE_CENTERED,
        .camera_id = (int32_t)CameraId::k2d,
        .x = -24,
        .y = 118,
        .width = 300,
        .height = 240,
        .color = 0xFFFFFFFFU,
        .main_func = (void*)select_main,
        .disp_func = (void*)DispMainWindow,
    },
    {
        .fade_mode = WinMgrDesc_FadeMode::INSTANT,
        .heading_type = WinMgrDesc_HeadingType::NONE,
        .camera_id = (int32_t)CameraId::k2d,
        .x = -280,
        .y = 118,
        .width = 240,
        .height = 45,
        .color = 0xFFFFFFFFU,
        .main_func = nullptr,
        .disp_func = (void*)DispWindow2,
    },
    {
        .fade_mode = WinMgrDesc_FadeMode::SCALE_AND_ROTATE,
        .heading_type = WinMgrDesc_HeadingType::NONE,
        .camera_id = (int32_t)CameraId::k2d,
        .x = -240,
        .y = -130,
        .width = 500,
        .height = 80,
        .color = 0xFFFFFFFFU,
        .main_func = (void*)select_main3,
        .disp_func = (void*)DispSelectionHelp,
    },
    // Descs 3-5: Run options windows.
    {
        .fade_mode = WinMgrDesc_FadeMode::SCALE_AND_ROTATE,
        .heading_type = WinMgrDesc_HeadingType::SINGLE_CENTERED,
        .camera_id = (int32_t)CameraId::k2d,
        .x = -250,
        .y = 135,
        .width = 500,
        .height = 240,
        .color = 0xFFFFFFFFU,
        .main_func = (void*)SelectMainOptionsWrapper,
        .disp_func = (void*)DispMainWindow,
    },
    {
        .fade_mode = WinMgrDesc_FadeMode::INSTANT,
        .heading_type = WinMgrDesc_HeadingType::NONE,
        .camera_id = (int32_t)CameraId::k2d,
        .x = -200,
        .y = 200,
        .width = 400,
        .height = 45,
        .color = 0xC4ECF2FFU,
        .main_func = nullptr,
        .disp_func = (void*)DispOptionsWindowBottomBar,
    },
    {
        .fade_mode = WinMgrDesc_FadeMode::INSTANT,
        .heading_type = WinMgrDesc_HeadingType::NONE,
        .camera_id = (int32_t)CameraId::k2d,
        .x = -240,
        .y = -130,
        .width = 500,
        .height = 80,
        .color = 0xFFFFFFFFU,
        .main_func = (void*)select_main3,
        .disp_func = (void*)DispSelectionHelp,
    },
    // Descs 6-8: Run stats windows.
    {
        .fade_mode = WinMgrDesc_FadeMode::SCALE_AND_ROTATE,
        .heading_type = WinMgrDesc_HeadingType::SINGLE_CENTERED,
        .camera_id = (int32_t)CameraId::k2d,
        .x = -250,
        .y = 96,
        .width = 500,
        .height = 240,
        .color = 0xFFFFFFFFU,
        .main_func = (void*)SelectMainOptionsWrapper,
        .disp_func = (void*)DispMainWindow,
    },
    {
        .fade_mode = WinMgrDesc_FadeMode::INSTANT,
        .heading_type = WinMgrDesc_HeadingType::NONE,
        .camera_id = (int32_t)CameraId::k2d,
        .x = -275,
        .y = 209,
        .width = 550,
        .height = 93,
        .color = 0xC4ECF2FFU,
        .main_func = nullptr,
        .disp_func = (void*)DispOptionsWindowTopBar,
    },
    {
        .fade_mode = WinMgrDesc_FadeMode::INSTANT,
        .heading_type = WinMgrDesc_HeadingType::NONE,
        .camera_id = (int32_t)CameraId::k2d,
        .x = -200,
        .y = -164,
        .width = 400,
        .height = 45,
        .color = 0xC4ECF2FFU,
        .main_func = nullptr,
        .disp_func = (void*)DispOptionsWindowBottomBar,
    },
};

}  // namespace

void* InitNewSelectDescTable() {
    // Copy descriptions from vanilla selection menus.
    memcpy(
        g_SelectDescList, ttyd::winmgr::select_desc_tbl,
        sizeof(ttyd::winmgr::select_desc_tbl));
    // Initialize custom menu parameters.
    g_SelectDescList[MenuType::CUSTOM_START] = WinMgrSelectDescList{ 
        .num_descs = 3, .descs = &g_CustomDescs[0]
    };
    g_SelectDescList[MenuType::MOVE_UNLOCK] = WinMgrSelectDescList{ 
        .num_descs = 3, .descs = &g_CustomDescs[0]
    };
    g_SelectDescList[MenuType::MOVE_UPGRADE] = WinMgrSelectDescList{ 
        .num_descs = 3, .descs = &g_CustomDescs[0]
    };
    g_SelectDescList[MenuType::TOT_CHARLIETON_SHOP] = WinMgrSelectDescList{ 
        .num_descs = 3, .descs = &g_CustomDescs[0]
    };
    g_SelectDescList[MenuType::TOT_CHET_RIPPO_TRADE] = WinMgrSelectDescList{
        .num_descs = 3, .descs = &g_CustomDescs[0]
    };
    g_SelectDescList[MenuType::RUN_OPTIONS] = WinMgrSelectDescList{
        .num_descs = 3, .descs = &g_CustomDescs[3]
    };
    g_SelectDescList[MenuType::RUN_RESULTS_STATS] = WinMgrSelectDescList{
        .num_descs = 3, .descs = &g_CustomDescs[6]
    };
    g_SelectDescList[MenuType::RUN_RESULTS_SPLITS] = WinMgrSelectDescList{
        .num_descs = 3, .descs = &g_CustomDescs[6]
    };
    return g_SelectDescList;
}

WinMgrSelectEntry* HandleSelectWindowEntry(int32_t type, int32_t new_item) {
    auto* sel_entry = 
        (WinMgrSelectEntry*)ttyd::memory::__memAlloc(0, sizeof(WinMgrSelectEntry));
    memset(sel_entry, 0, sizeof(WinMgrSelectEntry));
        
    sel_entry->type = type;
    sel_entry->cursor_index = 0;
    sel_entry->list_row_offset = 0;
    sel_entry->cursor_x = 0;
    sel_entry->cursor_y = 0;
    sel_entry->list_y_offset = 0;
    sel_entry->new_item = new_item;
    
    // Determine whether window should be cancellable based on type.
    switch (type) {
        case MenuType::MOVE_UNLOCK:
        case MenuType::MOVE_UPGRADE:
            // Not cancellable.
            break;
        case MenuType::CUSTOM_START:
        case MenuType::TOT_CHARLIETON_SHOP:
        case MenuType::TOT_CHET_RIPPO_TRADE:
        case MenuType::RUN_OPTIONS:
        case MenuType::RUN_RESULTS_STATS:
        case MenuType::RUN_RESULTS_SPLITS:
        default:
            sel_entry->flags |= WinMgrSelectEntry_Flags::CANCELLABLE;
            break;
    }
    
    sel_entry->num_entries = g_SelectDescList[type].num_descs;
    
    for (int32_t i = 0; i < sel_entry->num_entries; ++i) {
        int32_t entry_idx = 0;
        for (; entry_idx < winmgr_work->num_entries; ++entry_idx) {
            if (!(winmgr_work->entries[entry_idx].flags & 
                WinMgrEntry_Flags::IN_USE))
                break;
        }
        auto& entry = winmgr_work->entries[entry_idx];
        entry.flags = WinMgrEntry_Flags::IN_USE;
        entry.fade_state = WinMgrEntry_FadeState::IDLE;
        entry.fade_frame_counter = 0;
        entry.desc = g_SelectDescList[type].descs + i;
        entry.x = entry.desc->x;
        entry.y = entry.desc->y;
        entry.width = entry.desc->width;
        entry.height = entry.desc->height;
        entry.priority = 0;
        entry.help_line_cursor_index = 0;
        entry.help_line_count = 0;
        
        sel_entry->entry_indices[i] = entry_idx;
        entry.param = sel_entry;
    }
    
    auto& main_desc = g_SelectDescList[type].descs[0];
    sel_entry->cursor_x = main_desc.x;
    sel_entry->cursor_y = main_desc.y - 54;

    switch (type) {
        case MenuType::CUSTOM_START: {
            // Dummy selection dialog for testing.
            sel_entry->num_rows = 10;
            sel_entry->row_data = 
                (WinMgrSelectEntryRow*)ttyd::memory::__memAlloc(
                    0, sel_entry->num_rows * sizeof(WinMgrSelectEntryRow));
            memset(
                sel_entry->row_data, 0, 
                sel_entry->num_rows * sizeof(WinMgrSelectEntryRow));
            sel_entry->row_data[0].value = 0xf0;
            sel_entry->row_data[1].value = 0xfb;
            sel_entry->row_data[2].value = 0xf0;
            sel_entry->row_data[3].value = 0xfb;
            sel_entry->row_data[4].value = 0xf0;
            sel_entry->row_data[5].value = 0xfb;
            sel_entry->row_data[6].value = 0xfb;
            sel_entry->row_data[7].value = 0xfb;
            sel_entry->row_data[8].flags = 3;
            sel_entry->row_data[8].value = 0xf7;
            sel_entry->row_data[9].value = 0xfb;
            break;
        }
        case MenuType::TOT_CHARLIETON_SHOP: {
            // Read inventory from tot_generate_item.
            int16_t* inventory = GetCharlietonInventoryPtr();
            int32_t num_items = 0;
            for (int16_t* ptr = inventory; *ptr != -1; ++ptr) ++num_items;
            sel_entry->num_rows = num_items;
            sel_entry->row_data = 
                (WinMgrSelectEntryRow*)ttyd::memory::__memAlloc(
                    0, sel_entry->num_rows * sizeof(WinMgrSelectEntryRow));
            memset(
                sel_entry->row_data, 0,
                sel_entry->num_rows * sizeof(WinMgrSelectEntryRow));
            for (int32_t i = 0; i < num_items; ++i) {
                sel_entry->row_data[i].value = inventory[i];
            }
            break;
        }
        case MenuType::TOT_CHET_RIPPO_TRADE: {
            auto& state = g_Mod->state_;

            // Determine which options should be visible.
            int32_t ids[5] = { 0, 0, 0, 0, 0 };
            int32_t num_options = 0;
            // HP and FP should always be visible.
            ids[num_options++] = 1;
            ids[num_options++] = 3;
            // Add BP unless it's set to infinite.
            if (!state.CheckOptionValue(OPTVAL_INFINITE_BP))
                ids[num_options++] = 4;
            // Add Partner HP only if partners are enabled.
            if (!state.CheckOptionValue(OPTVAL_NO_PARTNERS))
                ids[num_options++] = 2;

            sel_entry->num_rows = num_options;
            sel_entry->row_data =
                (WinMgrSelectEntryRow*)ttyd::memory::__memAlloc(
                    0, sel_entry->num_rows * sizeof(WinMgrSelectEntryRow));
            memset(
                sel_entry->row_data, 0,
                sel_entry->num_rows * sizeof(WinMgrSelectEntryRow));

            for (int32_t i = 0; i < num_options; ++i) {
                int32_t id = ids[i];
                sel_entry->row_data[i].value = id;

                bool disable = false;
                switch (id) {
                    case 1:
                        if (state.hp_level_ <= 0 ||
                            state.GetOption(OPT_MARIO_HP) == 0) {
                            disable = true;
                        }
                        break;
                    case 2:
                        if (state.hp_p_level_ <= 0 ||
                            state.GetOption(OPT_PARTNER_HP) == 0) {
                            disable = true;
                        }
                        break;
                    case 3:
                        if (state.fp_level_ <= 0 ||
                            state.GetOption(OPT_MARIO_FP) == 0) {
                            disable = true;
                        }
                        break;
                    case 4:
                        if (state.bp_level_ <= 0 ||
                            state.GetOption(OPT_MARIO_BP) == 0 ||
                            state.CheckOptionValue(OPTVAL_INFINITE_BP)) {
                            disable = true;
                        }
                        break;
                }
                if (disable) {
                    sel_entry->row_data[i].flags |=
                        WinMgrSelectEntryRow_Flags::GREYED_OUT;
                    sel_entry->row_data[i].flags |=
                        WinMgrSelectEntryRow_Flags::UNSELECTABLE;
                }
            }
            break;
        }
        case MenuType::MOVE_UNLOCK:
        case MenuType::MOVE_UPGRADE: {
            // Read moves from tot_generate_reward.
            int32_t num_moves;
            int32_t* moves = RewardManager::GetSelectedMoves(&num_moves);
            sel_entry->num_rows = num_moves;
            sel_entry->row_data =
                (WinMgrSelectEntryRow*)ttyd::memory::__memAlloc(
                    0, sel_entry->num_rows * sizeof(WinMgrSelectEntryRow));
            memset(
                sel_entry->row_data, 0,
                sel_entry->num_rows * sizeof(WinMgrSelectEntryRow));
            for (int32_t i = 0; i < num_moves; ++i) {
                sel_entry->row_data[i].value = moves[i];
            }
            break;
        }
        case MenuType::RUN_OPTIONS:
        case MenuType::RUN_RESULTS_STATS: {
            // Assign options from g_OptionMenuData.
            int32_t num_options = 0;
            for (const auto& data : g_OptionMenuData) {
                if (sel_entry->type == MenuType::RUN_OPTIONS) {
                    if (data.in_run_options) ++num_options;
                } else {
                    if (data.in_run_stats) ++num_options;
                }
            }
            sel_entry->num_rows = num_options;
            sel_entry->row_data =
                (WinMgrSelectEntryRow*)ttyd::memory::__memAlloc(
                    0, sel_entry->num_rows * sizeof(WinMgrSelectEntryRow));
            memset(
                sel_entry->row_data, 0,
                sel_entry->num_rows * sizeof(WinMgrSelectEntryRow));
            int32_t i = 0;
            for (const auto& data : g_OptionMenuData) {
                if (sel_entry->type == MenuType::RUN_OPTIONS) {
                    if (!data.in_run_options) continue;
                } else {
                    if (!data.in_run_stats) continue;
                }
                sel_entry->row_data[i].value = data.lookup_key;
                ++i;
            }
            break;
        }
        case MenuType::RUN_RESULTS_SPLITS: {
            // Add a single dummy entry.
            sel_entry->num_rows = 1;
            sel_entry->row_data =
                (WinMgrSelectEntryRow*)ttyd::memory::__memAlloc(
                    0, sizeof(WinMgrSelectEntryRow));
            memset(sel_entry->row_data, 0, sizeof(WinMgrSelectEntryRow));
            break;
        }
    }
    
    // Shrink the window if it's fewer than eight entries.
    if (type != MenuType::RUN_RESULTS_SPLITS && sel_entry->num_rows < 8) {
        int32_t height_reduction = (8 - sel_entry->num_rows) * 24;
        auto& entry = winmgr_work->entries[sel_entry->entry_indices[0]];
        entry.height -= height_reduction;
    }
    
    return sel_entry;
}

int32_t HandleSelectWindowOther(WinMgrSelectEntry* sel_entry, EvtEntry* evt) {
    if (!(sel_entry->flags & WinMgrSelectEntry_Flags::FINISHED_SELECTION))
        return 0;
    if (sel_entry->flags & WinMgrSelectEntry_Flags::CANCELLED)
        return -1;
    
    int32_t value = sel_entry->row_data[sel_entry->cursor_index].value;

    switch(sel_entry->type) {
        case MenuType::CUSTOM_START:
            evt->lwData[1] = value;
            break;
        case MenuType::TOT_CHARLIETON_SHOP:
            evt->lwData[1] = value;
            evt->lwData[2] = PTR(msgSearch(itemDataTable[value].name));
            evt->lwData[3] = 
                itemDataTable[value].buy_price * GetBuyPriceScale() / 100;
            evt->lwData[4] = itemDataTable[value].bp_cost;
            break;
        case MenuType::TOT_CHET_RIPPO_TRADE:
            evt->lwData[1] = value;
            switch (value) {
                case 1:
                    evt->lwData[2] = PTR("HP");
                    evt->lwData[3] = g_Mod->state_.hp_level_;
                    evt->lwData[4] = g_Mod->state_.hp_level_ - 1;
                    break;
                case 2:
                    evt->lwData[2] = PTR("partner's HP");
                    evt->lwData[3] = g_Mod->state_.hp_p_level_;
                    evt->lwData[4] = g_Mod->state_.hp_p_level_ - 1;
                    break;
                case 3:
                    evt->lwData[2] = PTR("FP");
                    evt->lwData[3] = g_Mod->state_.fp_level_;
                    evt->lwData[4] = g_Mod->state_.fp_level_ - 1;
                    break;
                case 4:
                    evt->lwData[2] = PTR("BP");
                    evt->lwData[3] = g_Mod->state_.bp_level_;
                    evt->lwData[4] = g_Mod->state_.bp_level_ - 1;
                    break;
            }
            break;
        case MenuType::MOVE_UNLOCK:
        case MenuType::MOVE_UPGRADE:
            evt->lwData[1] = value;
            evt->lwData[2] = 
                PTR(msgSearch(MoveManager::GetMoveData(value)->name_msg));
            break;
    }
    
    return 1;
}

}  // namespace mod::tot::window_select