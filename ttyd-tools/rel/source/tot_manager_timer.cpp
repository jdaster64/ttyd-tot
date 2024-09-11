#include "tot_manager_timer.h"

#include "common_functions.h"
#include "evt_cmd.h"
#include "mod.h"
#include "tot_generate_item.h"
#include "tot_gsw.h"
#include "tot_manager_achievements.h"
#include "tot_manager_options.h"
#include "tot_state.h"

#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/icondrv.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/mariost.h>

namespace mod::tot {
    
namespace {

using ::ttyd::evtmgr_cmd::evtGetValue;

namespace IconType = ::ttyd::icondrv::IconType;

uint64_t GetRTATime() {
    // Freeze RTA timer after ending floor.
    auto& state = g_Mod->state_;
    if (state.IsFinalBossFloor() && state.splits_rta_[state.floor_]) {
        return state.last_floor_rta_;
    }
    return ttyd::mariost::g_MarioSt->lastFrameRetraceTime;
}

}  // namespace

void TimerManager::Update() {
    g_Mod->state_.TimerTick();
}

void TimerManager::Draw() {
    auto& state = g_Mod->state_;
    if (!state.GetOption(OPT_RUN_STARTED)) return;
    // If splits have been saved for final floor, don't show timer.
    if (state.IsFinalBossFloor() && state.splits_rta_[state.floor_]) return;
    uint32_t color = ~0U;
    
    uint64_t time_ticks;
    switch (state.GetOptionValue(OPT_TIMER_DISPLAY)) {
        case OPTVAL_TIMER_IGT: {
            if (!state.igt_active_) return;
            time_ticks = state.current_total_igt_;
            break;
        }
        case OPTVAL_TIMER_RTA: {
            time_ticks = GetRTATime() - state.run_start_time_rta_;
            color = 0xd0d0ffffU;
            break;
        }
        default: {
            return;
        }
    }
    
    int32_t parts[4];
    DurationTicksToParts(time_ticks, &parts[0], &parts[1], &parts[2], &parts[3]);
    
    gc::mtx34 mtx = { { 0 } };
    mtx.m[0][0] = 1.0f;
    mtx.m[1][1] = 1.0f;
    mtx.m[2][2] = 1.0f;
    mtx.m[0][3] = 120.0f;
    mtx.m[1][3] = -228.0f;
    
    for (int32_t i = 0; i < 4; ++i) {
        float offset = i < 3 ? 20.0f : 16.0f;
        int32_t icon_base = i < 3 ? IconType::NUMBER_0 : IconType::NUMBER_0_SMALL;
        ttyd::icondrv::iconDispGxCol(
            &mtx, 0x10, icon_base + 2 * (parts[i] / 10), &color);
        mtx.m[0][3] += offset;
        ttyd::icondrv::iconDispGxCol(
            &mtx, 0x10, icon_base + 2 * (parts[i] % 10), &color);
        mtx.m[0][3] += offset;
        if (i < 2) {
            mtx.m[0][3] -= 4.0f;
            ttyd::icondrv::iconDispGxCol(
                &mtx, 0x10, IconType::TOT_TIMER_COLON, &color);
            mtx.m[0][3] += 20.0f - 4.0f;
        }
    }
}

int32_t TimerManager::GetCurrentRunTotalTimeCentis() {
    auto& state = g_Mod->state_;
    if (state.CheckOptionValue(OPTVAL_TIMER_RTA)) {
        return DurationTicksToCentiseconds(
            GetRTATime() - state.run_start_time_rta_);
    }
    return DurationTicksToCentiseconds(state.current_total_igt_);
}

int32_t TimerManager::GetCurrentRunTotalBattleTimeCentis() {
    int32_t total = 0;
    for (int32_t i = 0; i <= 64; ++i) {
        total += g_Mod->state_.splits_battle_igt_[i];
    }
    return total;
}

int32_t TimerManager::GetNumberOfBattles() {
    int32_t battles = 0;
    for (int32_t i = 0; i <= 64; ++i) {
        battles += g_Mod->state_.splits_battle_igt_[i] ? 1 : 0;
    }
    return battles;
}

EVT_DEFINE_USER_FUNC(evtTot_ToggleIGT) {
    bool toggle = evtGetValue(evt, evt->evtArguments[0]);
    g_Mod->state_.ToggleIGT(toggle);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_TrackCompletedRun) {
    auto& state = g_Mod->state_;
    int32_t igt_centis = DurationTicksToCentiseconds(state.current_total_igt_);

    switch (state.GetOptionValue(OPT_DIFFICULTY)) {
        case OPTVAL_DIFFICULTY_HALF:
            state.ChangeOption(STAT_PERM_HALF_FINISHES);
            if (igt_centis < state.GetOption(STAT_PERM_HALF_BEST_TIME))
                state.SetOption(STAT_PERM_HALF_BEST_TIME, igt_centis);
            AchievementsManager::MarkCompleted(AchievementId::RUN_HALF_FIRST);
            if (state.CheckOptionValue(OPTVAL_PRESET_DEFAULT)) {
                if (igt_centis < 60 * 6000) {
                    AchievementsManager::MarkCompleted(AchievementId::RUN_HALF_SPEED1);
                }
                if (igt_centis < 40 * 6000) {
                    AchievementsManager::MarkCompleted(AchievementId::RUN_HALF_SPEED2);
                }
            }
            break;
        case OPTVAL_DIFFICULTY_FULL:
            state.ChangeOption(STAT_PERM_FULL_FINISHES);
            if (igt_centis < state.GetOption(STAT_PERM_FULL_BEST_TIME))
                state.SetOption(STAT_PERM_FULL_BEST_TIME, igt_centis);
            AchievementsManager::MarkCompleted(AchievementId::RUN_FULL_FIRST);
            if (state.CheckOptionValue(OPTVAL_PRESET_DEFAULT)) {
                if (igt_centis < 120 * 6000) {
                    AchievementsManager::MarkCompleted(AchievementId::RUN_FULL_SPEED1);
                }
                if (igt_centis < 90 * 6000) {
                    AchievementsManager::MarkCompleted(AchievementId::RUN_FULL_SPEED2);
                }
            }
            break;
        case OPTVAL_DIFFICULTY_FULL_EX:
            state.ChangeOption(STAT_PERM_EX_FINISHES);
            if (igt_centis < state.GetOption(STAT_PERM_EX_BEST_TIME))
                state.SetOption(STAT_PERM_EX_BEST_TIME, igt_centis);
            if (state.CheckOptionValue(OPTVAL_PRESET_DEFAULT)) {
                AchievementsManager::MarkCompleted(AchievementId::RUN_EX_FIRST);
                if (igt_centis < 180 * 6000) {
                    AchievementsManager::MarkCompleted(AchievementId::RUN_EX_SPEED1);
                }
                if (igt_centis < 140 * 6000) {
                    AchievementsManager::MarkCompleted(AchievementId::RUN_EX_SPEED2);
                }
            }
            break;
        default:
            break;
    }

    // Check for other run completion achievements.
    int32_t runs_completed = 0;
    runs_completed += state.GetOption(STAT_PERM_HALF_FINISHES);
    runs_completed += state.GetOption(STAT_PERM_FULL_FINISHES);
    runs_completed += state.GetOption(STAT_PERM_EX_FINISHES);
    if (runs_completed >= 10) {
        AchievementsManager::MarkCompleted(AchievementId::META_RUNS_10);
    }
    if (runs_completed >= 25) {
        AchievementsManager::MarkCompleted(AchievementId::META_RUNS_25);
    }

    int32_t num_npc_deals = 0;
    for (int32_t i = 0; i < 8; ++i) {
        if (state.GetOption(STAT_RUN_NPCS_DEALT_WITH, i)) ++num_npc_deals;
    }
    if (num_npc_deals >= 7) {
        AchievementsManager::MarkCompleted(AchievementId::RUN_NPC_DEALS_7);
    }

    if (state.CheckOptionValue(OPTVAL_AC_3_UNSIMP)) {
        AchievementsManager::MarkCompleted(AchievementId::RUN_MAX_AC_DIFFICULTY);
    }

    int32_t num_partners = GetNumActivePartners();
    if (num_partners == 0) {
        AchievementsManager::MarkCompleted(AchievementId::RUN_NO_PARTNERS);
    }
    if (num_partners == 7) {
        AchievementsManager::MarkCompleted(AchievementId::RUN_ALL_PARTNERS);
    }

    int32_t conditions_met = state.GetOption(STAT_RUN_CONDITIONS_MET);
    int32_t conditions_total = state.GetOption(STAT_RUN_CONDITIONS_TOTAL);
    if (conditions_met > 0 && conditions_met == conditions_total) {
        AchievementsManager::MarkCompleted(AchievementId::RUN_ALL_CONDITIONS_MET);
    }

    bool all_moves_maxed = true;
    for (int32_t i = 0; i < MoveType::MOVE_TYPE_MAX; ++i) {
        // Skip base moves for non-unlocked partners.
        if (i == MoveType::GOOMBELLA_BASE   && !IsPartnerActive(1)) continue;
        if (i == MoveType::KOOPS_BASE       && !IsPartnerActive(2)) continue;
        if (i == MoveType::FLURRIE_BASE     && !IsPartnerActive(3)) continue;
        if (i == MoveType::YOSHI_BASE       && !IsPartnerActive(4)) continue;
        if (i == MoveType::VIVIAN_BASE      && !IsPartnerActive(5)) continue;
        if (i == MoveType::BOBBERY_BASE     && !IsPartnerActive(6)) continue;
        if (i == MoveType::MOWZ_BASE        && !IsPartnerActive(7)) continue;

        int32_t unlocked_level = MoveManager::GetUnlockedLevel(i);
        if (unlocked_level && 
            unlocked_level != MoveManager::GetMoveData(i)->max_level) {
            all_moves_maxed = false;
            break;
        }
    }
    if (all_moves_maxed) {
        AchievementsManager::MarkCompleted(AchievementId::RUN_ALL_MOVES_MAXED);
    }

    if (state.GetOption(STAT_RUN_COINS_SPENT) == 417) {
        AchievementsManager::MarkCompleted(AchievementId::SECRET_COINS);
    }

    if (state.GetOption(STAT_RUN_PLAYER_DAMAGE) == 654) {
        AchievementsManager::MarkCompleted(AchievementId::SECRET_DAMAGE);
    }

    if (state.CheckOptionValue(OPTVAL_PRESET_DEFAULT)) {
        if (state.GetOption(STAT_RUN_ITEMS_USED) == 0) {
            AchievementsManager::MarkCompleted(AchievementId::RUN_NO_ITEMS);
        }
        if (state.GetOption(STAT_RUN_BADGES_EQUIPPED) == 0) {
            AchievementsManager::MarkCompleted(AchievementId::RUN_NO_BADGES);
        }
        if (state.GetOption(STAT_RUN_JUMPS_HAMMERS_USED) == 0) {
            AchievementsManager::MarkCompleted(AchievementId::RUN_NO_JUMP_HAMMER);
        }
        if (state.GetOption(STAT_RUN_PLAYER_DAMAGE) == 0) {
            AchievementsManager::MarkCompleted(AchievementId::RUN_NO_DAMAGE);
        }
        if (state.GetOption(STAT_RUN_MOST_TURNS_RECORD) <= 3) {
            AchievementsManager::MarkCompleted(AchievementId::RUN_ALL_FLOORS_3_TURN);
        }
        if (state.GetOption(STAT_RUN_INFATUATE_DAMAGE) >= 1000) {
            AchievementsManager::MarkCompleted(AchievementId::SECRET_INFATUATE);
        }
    }

    if (OptionsManager::AllDefaultExceptZeroStatLevels()) {
        int32_t mario_hp_scale = state.GetOption(OPT_MARIO_HP);
        int32_t mario_fp_scale = state.GetOption(OPT_MARIO_FP);
        int32_t mario_bp_scale = state.GetOption(OPT_MARIO_BP);
        int32_t mario_hp_level = state.hp_level_;
        int32_t mario_fp_level = state.fp_level_;
        int32_t mario_bp_level = state.bp_level_;

        int32_t zero = 0;
        if (mario_hp_scale == 0 || mario_hp_level == 0) ++zero;
        if (mario_fp_scale == 0 || mario_fp_level == 0) ++zero;
        if (mario_bp_scale == 0 || mario_bp_level == 0) ++zero;

        if (zero >= 1) {
            AchievementsManager::MarkCompleted(AchievementId::RUN_ZERO_STAT_1);
        }
        if (zero >= 2) {
            AchievementsManager::MarkCompleted(AchievementId::RUN_ZERO_STAT_2);
        }
        if (zero >= 3) {
            AchievementsManager::MarkCompleted(AchievementId::SECRET_ZERO_STATS_3);
        }
    }

    // Update currency-based aggregate stats.
    int32_t coins_earned = state.GetOption(STAT_RUN_COINS_EARNED);
    int32_t coins_spent = state.GetOption(STAT_RUN_COINS_SPENT);
    int32_t sp_earned = state.GetOption(STAT_RUN_STAR_PIECES);
    int32_t shines_earned = state.GetOption(STAT_RUN_SHINE_SPRITES);

    state.ChangeOption(STAT_PERM_COINS_EARNED, coins_earned);
    state.ChangeOption(STAT_PERM_COINS_SPENT, coins_spent);
    state.ChangeOption(STAT_PERM_STAR_PIECES, sp_earned);
    state.ChangeOption(STAT_PERM_SHINE_SPRITES, shines_earned);

    // Apply intensity multiplier to currency earned.
    int32_t intensity = state.GetOption(STAT_RUN_INTENSITY);
    int32_t meta_coins_earned = (coins_earned * intensity + 99) / 100;
    int32_t meta_sp_earned =
        ((sp_earned + shines_earned * 3) * intensity + 99) / 100;

    state.ChangeOption(STAT_PERM_META_COINS_EARNED, meta_coins_earned);
    state.ChangeOption(STAT_PERM_META_SP_EARNED, meta_sp_earned);

    if (state.GetOption(STAT_PERM_META_COINS_EARNED) >= 10000) {
        AchievementsManager::MarkCompleted(AchievementId::AGG_COINS_10000);
    }
    if (intensity >= 200) {
        AchievementsManager::MarkCompleted(AchievementId::RUN_HIGH_INTENSITY);
    }

    // Update amount of meta-currency available in the hub.
    state.ChangeOption(STAT_PERM_CURRENT_COINS, meta_coins_earned);
    state.ChangeOption(STAT_PERM_CURRENT_SP, meta_sp_earned);

    // Update highest intensity cleared.
    if (intensity > state.GetOption(STAT_PERM_MAX_INTENSITY)) {
        state.SetOption(STAT_PERM_MAX_INTENSITY, intensity);
    }

    // Update number of 'tutorial' tower run clears.
    int32_t tut_clears = GetSWByte(GSW_Tower_TutorialClears);
    if (tut_clears < 2) {
        SetSWByte(GSW_Tower_TutorialClears, ++tut_clears);
    }

    // Reshuffle the hub item shop.
    state.SetOption(OPT_SHOP_ITEMS_CHOSEN, 0);
    GenerateHubShopItems();
    
    return 2;
}

}  // namespace mod::tot