#include "tot_manager_move.h"

#include "evt_cmd.h"
#include "mod.h"
#include "tot_gsw.h"
#include "tot_manager_achievements.h"

#include <ttyd/battle.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_seq_command.h>
#include <ttyd/battle_unit.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/icondrv.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/msgdrv.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::tot {
    
namespace {

using ::mod::g_Mod;
using ::ttyd::battle::g_BattleWork;
using ::ttyd::battle_database_common::BattleWeapon;
using ::ttyd::battle_unit::BattleWorkUnit;
using ::ttyd::battle_unit::BattleWorkUnitPart;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::msgdrv::msgSearch;

namespace IconType = ::ttyd::icondrv::IconType;
namespace ItemType = ::ttyd::item_data::ItemType;

// Globals for badge move levels.
int8_t g_MaxBadgeMoveLevels[MoveType::BADGE_MOVE_MAX - MoveType::BADGE_MOVE_BASE];
int8_t g_CurBadgeMoveLevels[MoveType::BADGE_MOVE_MAX - MoveType::BADGE_MOVE_BASE];
    
}

const MoveData g_MoveData[] = {
    { { 0, 2, 4 }, 3, 0, 0, IconType::BOOTS, "btl_cmd_act_jump", nullptr, "msg_normal_jump", "msg_ac_jump", "tot_upg_damage", },
    { { 2, 4, 6 }, 3, 1, 0, IconType::SUPER_BOOTS, "btl_wn_mario_kururin_jump", nullptr, "msg_kururin_jump", "msg_ac_k_jump", "tot_upg_damage", },
    { { 4, 6, 8 }, 3, 4, 0, IconType::ULTRA_BOOTS, "btl_wn_mario_jyabara_jump", "tot_mj_spring_abb", "msg_jyabara_jump", "msg_ac_j_jump", "tot_upg_damage", },
    { { 2, 4, 6 }, 3, 1, 0, IconType::POWER_JUMP, "in_gatsun_jump", "tot_mj_pjump_abb", "msg_gatsun_jump", "msg_ac_jump", "tot_upg_damage", },
    { { 3, 4, 5 }, 3, 1, 0, IconType::MULTIBOUNCE, "in_tugitugi_jump", "tot_mj_multi_abb", "msg_tugitugi_jump", "msg_ac_jump", "tot_upg_damage", },
    { { 3, 5, 7 }, 3, 1, 0, IconType::POWER_BOUNCE, "in_renzoku_jump", "tot_mj_pbounce_abb", "msg_renzoku_jump", "msg_ac_jump", "tot_upg_damage", },
    { { 2, 4, 6 }, 3, 1, 0, IconType::SLEEPY_STOMP, "in_nemurase_fumi", "tot_mj_sleep_abb", "msg_nemurase_fumi", "msg_ac_jump", "tot_upg_status", },
    { { 3, 4, 6 }, 3, 1, 0, IconType::TORNADO_JUMP, "in_tamatsuki_jump", "tot_mj_tjump_abb", "msg_tamatsuki_jump", "msg_ac_tatsumaki_jump", "tot_upg_damage", },
    { { 0, 2, 4 }, 3, 0, 0, IconType::HAMMER, "btl_cmd_act_hammer", nullptr, "msg_normal_hammer", "msg_ac_hammer", "tot_upg_damage", },
    { { 2, 3, 5 }, 3, 1, 0, IconType::SUPER_HAMMER, "btl_wn_mario_kaiten_hammer", "tot_mh_super_abb", "msg_kaiten_hammer", "msg_ac_kaiten_hammer", "tot_upg_damage", },
    { { 4, 6, 8 }, 3, 4, 0, IconType::ULTRA_HAMMER, "btl_wn_mario_ultra_hammer", "tot_mh_ultra_abb", "msg_ultra_hammer", "msg_ac_ultra_hammer", "tot_upg_damage", },
    { { 2, 4, 6 }, 3, 1, 0, IconType::POWER_SMASH, "in_gatsun_naguri", "tot_mh_power_abb", "msg_gatsun_naguri", "msg_ac_hammer", "tot_upg_damage", },
    { { 2, 4, 6 }, 3, 1, 0, IconType::HEAD_RATTLE, "in_konran_hammer", "tot_mh_shrink_abb", "msg_konran_hammer", "msg_ac_hammer", "tot_upg_status", },
    { { 3, 5, 7 }, 3, 1, 0, IconType::ICE_SMASH, "in_ice_naguri", nullptr, "msg_ice_naguri", "msg_ac_hammer", "tot_upg_status", },
    { { 3, 5, 7 }, 3, 1, 0, IconType::QUAKE_HAMMER, "in_jishin_attack", "tot_mh_quake_abb", "msg_jishin_attack", "msg_ac_hammer", "tot_upg_damage", },
    { { 3, 5, 7 }, 3, 1, 0, IconType::FIRE_DRIVE, "in_fire_naguri", nullptr, "msg_fire_naguri", "msg_ac_fire_naguri", "tot_upg_damage", },
    { { 1, 2, 3 }, 3, 0, 0, IconType::MAGICAL_MAP, "btl_wn_sac_genki0", "tot_ms_1_abb", "msg_genkigenki", "msg_sac_1", "tot_upg_sac1", },
    { { 1, 2, 3 }, 3, 0, 0, IconType::DIAMOND_STAR, "btl_wn_sac_dekkaku_dokkan", "tot_ms_2_abb", "msg_dekkaku_dokkan", "msg_sac_2", "tot_upg_damage", },
    { { 2, 3, 4 }, 3, 1, 0, IconType::EMERALD_STAR, "btl_wn_sac_bakugame", nullptr, "msg_bakugame", "msg_sac_3", "tot_upg_status", },
    { { 2, 3, 4 }, 3, 1, 0, IconType::GOLD_STAR, "btl_wn_sac_mukimuki_body", nullptr, "msg_mukimuki_body", "msg_sac_4", "tot_upg_sac1", },
    { { 3, 5, 6 }, 3, 2, 0, IconType::RUBY_STAR, "btl_wn_sac_scissor", nullptr, "msg_scissor", "msg_sac_5", "tot_upg_damage", },
    { { 2, 3, 5 }, 3, 1, 0, IconType::SAPPHIRE_STAR, "btl_wn_sac_genki1", "tot_ms_6_abb", "msg_super_genki", "msg_sac_6", "tot_upg_sac1", },
    { { 3, 4, 5 }, 3, 2, 0, IconType::GARNET_STAR, "btl_wn_sac_sukkari_sukkiri", "tot_ms_7_abb", "msg_sukkari_sukkiri", "msg_sac_7", "tot_upg_statuschance", },
    { { 4, 6, 7 }, 3, 3, 0, IconType::CRYSTAL_STAR, "btl_wn_sac_zubastar", nullptr, "msg_zubastar", "msg_sac_8", "tot_upg_damage", },
    { { 0, 2, 4 }, 3, 0, 1, IconType::PARTNER_MOVE_0, "btl_wn_pkr_normal", nullptr, "msg_pkr_normal_jump", "msg_ac_zutsuki", "tot_upg_damage", },
    { { 0, 0, 0 }, 1, 0, 1, IconType::PARTNER_MOVE_0, "btl_wn_pkr_lv1", nullptr, "msg_pkr_monosiri", "msg_ac_monoshiri", "tot_upg_none", },
    { { 3, 5, 7 }, 3, 1, 1, IconType::PARTNER_MOVE_1, "btl_wn_pkr_lv2", nullptr, "msg_pkr_renzoku_zutsuki", "msg_ac_zutsuki", "tot_upg_damage", },
    { { 2, 5, 5 }, 2, 1, 1, IconType::PARTNER_MOVE_1, "tot_ptr1_scope_out", nullptr, "tot_ptr1_scope_out_desc", "msg_ac_monoshiri", "tot_upg_scope_out", },
    { { 3, 4, 5 }, 3, 2, 1, IconType::PARTNER_MOVE_2, "tot_ptr1_ironbonk", nullptr, "tot_ptr1_ironbonk_desc", "msg_ac_zutsuki", "tot_upg_damage", },
    { { 4, 4, 4 }, 1, 3, 1, IconType::PARTNER_MOVE_3, "btl_wn_pkr_lv3", nullptr, "msg_pkr_nage_kiss", "msg_ac_kiss", "tot_upg_none", },
    { { 0, 2, 4 }, 3, 0, 2, IconType::PARTNER_MOVE_0, "btl_wn_pnk_normal", nullptr, "msg_pnk_normal_attack", "msg_ac_hammer", "tot_upg_damage", },
    { { 2, 3, 4 }, 3, 1, 2, IconType::PARTNER_MOVE_1, "btl_wn_pnk_lv1", nullptr, "msg_pnk_syubibin_koura", "msg_ac_koura_shubibin", "tot_upg_damage", },
    { { 2, 2, 2 }, 1, 1, 2, IconType::PARTNER_MOVE_1, "tot_ptr2_withdraw", nullptr, "tot_ptr2_withdraw_desc", "msg_ac_koura_no_mamori", "tot_upg_none", },
    { { 4, 6, 8 }, 3, 2, 2, IconType::PARTNER_MOVE_2, "btl_wn_pnk_lv2", nullptr, "msg_pnk_koura_no_mamori", "msg_ac_koura_no_mamori", "tot_upg_shellsh", },
    { { 3, 6, 9 }, 3, 2, 2, IconType::PARTNER_MOVE_2, "tot_ptr2_bulkup", nullptr, "tot_ptr2_bulkup_desc", "msg_ac_bom_counter", "tot_upg_statuseffect", },
    { { 5, 7, 9 }, 3, 3, 2, IconType::PARTNER_MOVE_3, "btl_wn_pnk_lv3_tsuranuki", nullptr, "msg_pnk_koura_tsuranuki", "msg_ac_tsuranuki_koura", "tot_upg_damage", },
    { { 0, 2, 4 }, 3, 0, 5, IconType::PARTNER_MOVE_0, "btl_wn_pwd_normal", nullptr, "msg_pwd_body_press", "msg_ac_body_press", "tot_upg_damage", },
    { { 5, 5, 5 }, 1, 1, 5, IconType::PARTNER_MOVE_1, "btl_wn_pwd_lv1", nullptr, "msg_pwd_breath", "msg_ac_breath", "tot_upg_none", },
    { { 3, 4, 5 }, 3, 1, 5, IconType::PARTNER_MOVE_1, "btl_wn_pwd_lv2", nullptr, "msg_pwd_sexy_kiss", "msg_ac_sexy_kiss", "tot_upg_damage", },
    { { 4, 4, 4 }, 1, 1, 5, IconType::PARTNER_MOVE_1, "btl_wn_pwd_lv3", nullptr, "msg_pwd_kumogakure", "msg_ac_kumogakure", "tot_upg_none", },
    { { 5, 7, 9 }, 3, 2, 5, IconType::PARTNER_MOVE_2, "tot_ptr3_blizzard", nullptr, "tot_ptr3_blizzard_desc", "msg_ac_breath", "tot_upg_blizzard", },
    { { 4, 9, 9 }, 2, 3, 5, IconType::PARTNER_MOVE_3, "tot_ptr3_thunder", "tot_ptr3_thund_abb", "tot_ptr3_thunder_desc", "tot_ptr3_thunder_ac", "tot_upg_multi", },
    { { 0, 2, 4 }, 3, 0, 4, IconType::PARTNER_MOVE_0, "btl_wn_pys_normal", "tot_ptr4_norm_abb", "msg_pys_hip_drop", "msg_ac_hip_drop", "tot_upg_hits", },
    { { 2, 4, 6 }, 3, 1, 4, IconType::PARTNER_MOVE_1, "tot_ptr4_spit", "tot_ptr4_spit_abb", "tot_ptr4_spit_desc", "msg_ac_nomikomi", "tot_upg_damage", },
    { { 3, 5, 7 }, 3, 1, 4, IconType::PARTNER_MOVE_1, "tot_ptr4_egg", "tot_ptr4_egg_abb", "tot_ptr4_egg_desc", "msg_ac_wonder_egg", "tot_upg_hits", },
    { { 5, 5, 5 }, 1, 2, 4, IconType::PARTNER_MOVE_2, "tot_ptr4_gulp", nullptr, "tot_ptr4_gulp_desc", "msg_ac_taigun_yoshi", "tot_upg_none", },
    { { 3, 4, 5 }, 3, 2, 4, IconType::PARTNER_MOVE_2, "btl_wn_pys_lv2", nullptr, "msg_pys_wonder_egg", "msg_ac_wonder_egg", "tot_upg_hits", },
    { { 5, 6, 7 }, 3, 3, 4, IconType::PARTNER_MOVE_3, "btl_wn_pys_lv3", nullptr, "msg_pys_taigun_yoshi", "msg_ac_taigun_yoshi", "tot_upg_hits", },
    { { 0, 2, 4 }, 3, 0, 6, IconType::PARTNER_MOVE_0, "btl_wn_ptr_normal", nullptr, "msg_ptr_kagenuke", "msg_ac_kagenuke", "tot_upg_damage", },
    { { 2, 5, 5 }, 2, 1, 6, IconType::PARTNER_MOVE_1, "tot_ptr5_curse", nullptr, "tot_ptr5_curse_desc", "msg_ac_meromero_kiss", "tot_upg_multi", },
    { { 2, 5, 5 }, 2, 1, 6, IconType::PARTNER_MOVE_1, "tot_ptr5_neutralize", nullptr, "tot_ptr5_neutralize_desc", "msg_ac_mahou_no_kona", "tot_upg_multi", },
    { { 1, 6, 6 }, 2, 2, 6, IconType::PARTNER_MOVE_2, "btl_wn_ptr_lv1", nullptr, "msg_ptr_kagegakure", "msg_ac_kagegakure", "tot_upg_veil", },
    { { 4, 6, 8 }, 3, 2, 6, IconType::PARTNER_MOVE_2, "btl_wn_ptr_lv2", nullptr, "msg_ptr_mahou_no_kona", "msg_ac_mahou_no_kona", "tot_upg_damage", },
    { { 5, 5, 5 }, 1, 3, 6, IconType::PARTNER_MOVE_3, "btl_wn_ptr_lv3", nullptr, "msg_ptr_meromero_kiss", "msg_ac_meromero_kiss", "tot_upg_none", },
    { { 0, 2, 4 }, 3, 0, 3, IconType::PARTNER_MOVE_0, "btl_wn_pbm_normal", nullptr, "msg_pbm_bakuhatsu", "msg_ac_bakuhatsu", "tot_upg_damage", },
    { { 2, 4, 6 }, 3, 1, 3, IconType::PARTNER_MOVE_1, "btl_wn_pbm_lv1", "tot_ptr6_bsquad_abb", "msg_pbm_jigen_bakudan", "msg_ac_jigen_bakudan", "tot_upg_bombs", },
    { { 4, 4, 4 }, 1, 1, 3, IconType::PARTNER_MOVE_1, "btl_wn_pbm_lv2", nullptr, "msg_pbm_counter", "msg_ac_bom_counter", "tot_upg_none", },
    { { 2, 4, 6 }, 3, 2, 3, IconType::PARTNER_MOVE_2, "tot_ptr6_poisonbomb", "tot_ptr6_poison_abb", "tot_ptr6_poisonbomb_desc", "msg_ac_jigen_bakudan", "tot_upg_bombs", },
    { { 6, 8, 10 }, 3, 2, 3, IconType::PARTNER_MOVE_2, "btl_wn_pbm_lv3", nullptr, "msg_pbm_sungoi_bakuhatsu", "msg_ac_super_bakuhatsu", "tot_upg_damage", },
    { { 10, 10, 10 }, 1, 3, 3, IconType::PARTNER_MOVE_3, "tot_ptr6_megatonbomb", nullptr, "tot_ptr6_megatonbomb_desc", "msg_ac_heart_catch", "tot_upg_none", },
    { { 0, 2, 4 }, 3, 0, 7, IconType::PARTNER_MOVE_0, "btl_wn_pcr_normal", nullptr, "msg_pch_binta", "msg_ac_binta", "tot_upg_hits", },
    { { 5, 5, 5 }, 1, 1, 7, IconType::PARTNER_MOVE_1, "btl_wn_pcr_steal", nullptr, "msg_pch_heart_catch", "msg_ac_heart_catch", "tot_upg_none", },
    { { 2, 4, 6 }, 3, 1, 7, IconType::PARTNER_MOVE_1, "btl_wn_pcr_madowase", nullptr, "msg_pch_madowaseru", "msg_ac_madowaseru", "tot_upg_tease", },
    { { 2, 2, 2 }, 1, 1, 7, IconType::PARTNER_MOVE_1, "tot_ptr7_embargo", nullptr, "tot_ptr7_embargo_desc", "msg_ac_madowaseru", "tot_upg_none", },
    { { 4, 5, 6 }, 3, 2, 7, IconType::PARTNER_MOVE_2, "tot_ptr7_smokebomb", "tot_ptr7_smokeb_abb", "tot_ptr7_smokebomb_desc", "msg_ac_madowaseru", "tot_upg_smokebomb", },
    { { 4, 7, 10 }, 3, 3, 7, IconType::PARTNER_MOVE_3, "btl_wn_pcr_kiss", nullptr, "msg_pch_kiss", "msg_ac_chuchurina_kiss", "tot_upg_smooch", },
    { { 1, 1, 1 }, 1, 1, -1, IconType::CHARGE, "in_charge", nullptr, "tot_msg_dummy", "tot_msg_dummy", "tot_msg_dummy", },
    { { 1, 1, 1 }, 1, 1, -1, IconType::CHARGE, "in_charge", nullptr, "tot_msg_dummy", "tot_msg_dummy", "tot_msg_dummy", },
    { { 1, 1, 1 }, 1, 1, -1, IconType::DEFEND_BADGE, "in_toughen_up", "tot_toughen_up_abb", "tot_msg_dummy", "tot_msg_dummy", "tot_msg_dummy", },
    { { 1, 1, 1 }, 1, 1, -1, IconType::DEFEND_BADGE, "in_toughen_up", "tot_toughen_up_abb", "tot_msg_dummy", "tot_msg_dummy", "tot_msg_dummy", },
}; 

const char* GetLogMoveLevelDesc(int32_t move_type, int32_t level) {
    char buf[20];
    int32_t move_class = move_type < MoveType::GOOMBELLA_BASE
        ? move_type / 8 : move_type / 6 - 1;
    int32_t move_class_idx = move_type < MoveType::GOOMBELLA_BASE
        ? move_type + 1 - move_class * 8 : move_type + 1 - (move_class + 1) * 6;
    if (level > g_MoveData[move_type].max_level) {
        // Level does not exist; return empty string.
        return "";
    } else if (
        // Never unlocked; return 00X placeholder that always says "Lv. X: ???".
        (level == 2 && !(g_Mod->state_.GetOption(STAT_PERM_MOVE_LOG, move_type) 
            & MoveLogFlags::UNLOCKED_LV_2)) ||
        (level == 3 && !(g_Mod->state_.GetOption(STAT_PERM_MOVE_LOG, move_type)
            & MoveLogFlags::UNLOCKED_LV_3))) {
        move_class = 0;
        move_class_idx = 0;
    }

    sprintf(
        buf, "tot_movelog_d%" PRId32 "%" PRId32 "%" PRId32, 
        move_class, move_class_idx, level);
    return msgSearch(buf);
}

void MoveManager::Init() {
    // Set unlocked levels of moves.
    for (int32_t i = 0; i < MoveType::MOVE_TYPE_MAX; ++i) {
        int32_t default_level = 0;
        switch (i) {
            case MoveType::JUMP_BASE:
            case MoveType::HAMMER_BASE:
            case MoveType::GOOMBELLA_BASE:
            case MoveType::KOOPS_BASE:
            case MoveType::FLURRIE_BASE:
            case MoveType::YOSHI_BASE:
            case MoveType::VIVIAN_BASE:
            case MoveType::BOBBERY_BASE:
            case MoveType::MOWZ_BASE:
                // Base attacks start with 2 levels unlocked.
                default_level = 2;
                LogMoveUnlock(i, 1);
                LogMoveUnlock(i, 2);
                break;
            case MoveType::GOOMBELLA_TATTLE:
            case MoveType::SP_SWEET_TREAT:
            case MoveType::SP_EARTH_TREMOR:
                // Star Powers start with only first level unlocked.
                default_level = 1;
                LogMoveUnlock(i, 1);
                break;
        }
        g_Mod->state_.SetOption(STAT_RUN_MOVE_LV_UNLOCKED, default_level, i);
        g_Mod->state_.SetOption(STAT_RUN_MOVE_LV_SELECTED, default_level, i);
    }
    // Set Mario pouch stuff.
    auto& pouch = *::ttyd::mario_pouch::pouchGetPtr();
    pouch.jump_level = 1;
    pouch.hammer_level = 1;
}
    
const MoveData* MoveManager::GetMoveData(int32_t starting_move) {
    return g_MoveData + starting_move;
}

int32_t MoveManager::GetBaseMoveType(int32_t move_type) {
    if (move_type < MoveType::GOOMBELLA_BASE) {
        return move_type / 8 * 8;
    } else {
        return MoveType::GOOMBELLA_BASE + (move_type - MoveType::GOOMBELLA_BASE) / 6 * 6;
    }
}

int32_t MoveManager::GetMoveTypeFromBadge(int32_t badge_id) {
    switch (badge_id) {
        case ItemType::CHARGE:
            return MoveType::BADGE_MOVE_CHARGE;
        case ItemType::CHARGE_P:
            return MoveType::BADGE_MOVE_CHARGE_P;
        case ItemType::SUPER_CHARGE:
            return MoveType::BADGE_MOVE_SUPER_CHARGE;
        case ItemType::SUPER_CHARGE_P:
            return MoveType::BADGE_MOVE_SUPER_CHARGE_P;
    }
    // Should not be reached.
    return -1;
}

int32_t MoveManager::GetUnlockedLevel(int32_t move_type) {
    if (move_type >= MoveType::BADGE_MOVE_BASE) {
        return g_MaxBadgeMoveLevels[move_type - MoveType::BADGE_MOVE_BASE];
    }
    return g_Mod->state_.GetOption(STAT_RUN_MOVE_LV_UNLOCKED, move_type);
}

int32_t MoveManager::GetSelectedLevel(int32_t move_type) {
    if (move_type >= MoveType::BADGE_MOVE_BASE) {
        return g_CurBadgeMoveLevels[move_type - MoveType::BADGE_MOVE_BASE];
    }
    return g_Mod->state_.GetOption(STAT_RUN_MOVE_LV_SELECTED, move_type);
}

int32_t MoveManager::GetMoveCost(int32_t move_type) {
    return g_MoveData[move_type].move_cost[
        g_Mod->state_.GetOption(STAT_RUN_MOVE_LV_SELECTED, move_type) - 1];
}

bool MoveManager::UpgradeMove(int32_t move_type) {
    // Increment the move level, if it's not at its max.
    int32_t level = g_Mod->state_.GetOption(STAT_RUN_MOVE_LV_UNLOCKED, move_type);
    if (level >= g_MoveData[move_type].max_level) return false;
    g_Mod->state_.SetOption(STAT_RUN_MOVE_LV_UNLOCKED, ++level, move_type);
    
    // If newly unlocked...
    if (level == 1) {
        auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
        switch (move_type) {
            // Unlock field moves for Spin/Spring Jump + Super/Ultra Hammer.
            case MoveType::JUMP_SPIN:
                pouch.jump_level = 2;
                ttyd::mario_pouch::pouchGetItem(ItemType::SUPER_BOOTS);
                break;
            case MoveType::JUMP_SPRING:
                pouch.jump_level = 3;
                ttyd::mario_pouch::pouchGetItem(ItemType::ULTRA_BOOTS);
                break;
            case MoveType::HAMMER_SUPER:
                pouch.hammer_level = 2;
                ttyd::mario_pouch::pouchGetItem(ItemType::SUPER_HAMMER);
                break;
            case MoveType::HAMMER_ULTRA:
                pouch.hammer_level = 3;
                ttyd::mario_pouch::pouchGetItem(ItemType::ULTRA_HAMMER);
                break;
            // Unlock corresponding SP move + increase max SP by 1.00.
            case MoveType::SP_CLOCK_OUT:
            case MoveType::SP_POWER_LIFT:
            case MoveType::SP_ART_ATTACK:
            case MoveType::SP_SWEET_FEAST:
            case MoveType::SP_SHOWSTOPPER:
            case MoveType::SP_SUPERNOVA:
                pouch.star_powers_obtained |= 
                    (1 << (move_type - MoveType::SP_SWEET_TREAT));
                pouch.max_sp += 100;
                break;
        }
        g_Mod->state_.SetOption(STAT_RUN_MOVE_LV_SELECTED, 1, move_type);
    }
    LogMoveUnlock(move_type, level);
    return true;
}

bool MoveManager::ChangeSelectedLevel(int32_t move_type, int32_t change) {
    if (move_type >= MoveType::BADGE_MOVE_BASE) {
        move_type -= MoveType::BADGE_MOVE_BASE;

        int32_t old_level = g_CurBadgeMoveLevels[move_type];
        int32_t max_level = g_MaxBadgeMoveLevels[move_type];
        int32_t new_level = old_level + change;
        
        if (new_level < 1) new_level = 1;
        if (new_level > max_level) new_level = max_level;
        
        g_CurBadgeMoveLevels[move_type] = new_level;
        return new_level != old_level;
    }

    int32_t old_level =
        g_Mod->state_.GetOption(STAT_RUN_MOVE_LV_SELECTED, move_type);
    int32_t max_level =
        g_Mod->state_.GetOption(STAT_RUN_MOVE_LV_UNLOCKED, move_type);
    int32_t new_level = old_level + change;
    
    if (new_level < 1) new_level = 1;
    if (new_level > max_level) new_level = max_level;
    
    g_Mod->state_.SetOption(STAT_RUN_MOVE_LV_SELECTED, new_level, move_type);
    return new_level != old_level;
}

void MoveManager::ResetSelectedLevels() {
    // Set all unlocked moves' selected level to 1.
    for (int32_t i = 0; i < MoveType::MOVE_TYPE_MAX; ++i) {
        if (g_Mod->state_.GetOption(STAT_RUN_MOVE_LV_UNLOCKED, i))
            g_Mod->state_.SetOption(STAT_RUN_MOVE_LV_SELECTED, 1, i);
    }
    // Set all badge moves' selected level to 1.
    int32_t num_badge_moves =
        MoveType::BADGE_MOVE_MAX - MoveType::BADGE_MOVE_BASE;
    for (int32_t i = 0; i < num_badge_moves; ++i) {
        g_CurBadgeMoveLevels[i] = 1;
    }
}

bool MoveManager::IsUnlockable(int32_t move_type) {
    if (g_Mod->state_.GetOption(STAT_RUN_MOVE_LV_UNLOCKED, move_type) != 0)
        return false;
    // Spring Jump / Ultra Hammer require unlocking Spin/Super as precursor,
    // unless playing with "random unlock order" setting.
    if (g_MoveData[move_type].move_tier == 4 &&
        !g_Mod->state_.CheckOptionValue(OPTVAL_MOVES_RANDOM) &&
        g_Mod->state_.GetOption(STAT_RUN_MOVE_LV_UNLOCKED, move_type - 1) == 0)
        return false;
    return true;
}

bool MoveManager::IsUpgradable(int32_t move_type) {
    int32_t level = g_Mod->state_.GetOption(STAT_RUN_MOVE_LV_UNLOCKED, move_type);
    if (level == 0 || level >= g_MoveData[move_type].max_level) return false;
    if (g_MoveData[move_type].partner_id > 0 &&
        !(ttyd::mario_pouch::pouchGetPtr()->party_data[
            g_MoveData[move_type].partner_id
        ].flags & 1)) return false;
    return true;
}

bool MoveManager::GetCurrentSelectionString(int32_t move_type, char* out_buf) {
    if (GetUnlockedLevel(move_type) == 1) return false;
    // Shorten move's name, if there's a short name available.
    const char* move_name =
        g_MoveData[move_type].abbrev_msg
            ? msgSearch(g_MoveData[move_type].abbrev_msg)
            : msgSearch(g_MoveData[move_type].name_msg);
    sprintf(
        out_buf, "%s Lv. %" PRId8, move_name, GetSelectedLevel(move_type));
    return true;
}

void MoveManager::InitBadgeMoveLevels() {
    const int32_t badge_types[] = {
        ItemType::CHARGE, ItemType::CHARGE_P, ItemType::SUPER_CHARGE,
        ItemType::SUPER_CHARGE_P
    };

    int32_t num_badge_moves =
        MoveType::BADGE_MOVE_MAX - MoveType::BADGE_MOVE_BASE;
    for (int32_t i = 0; i < num_badge_moves; ++i) {
        int32_t badge_count = ttyd::mario_pouch::pouchEquipCheckBadge(
            badge_types[i]);
        g_MaxBadgeMoveLevels[i] = badge_count;
        g_CurBadgeMoveLevels[i] = 1;
    }
}

void MoveManager::LogMoveUnlock(int32_t move_type, int32_t level) {
    uint32_t value = g_Mod->state_.GetOption(STAT_PERM_MOVE_LOG, move_type);
    value |= (MoveLogFlags::UNLOCKED_LV_1 << (level - 1));
    g_Mod->state_.SetOption(STAT_PERM_MOVE_LOG, value, move_type);
}

void MoveManager::LogMoveUse(int32_t move_type) {
    uint32_t value = g_Mod->state_.GetOption(STAT_PERM_MOVE_LOG, move_type);
    int32_t level = GetSelectedLevel(move_type);
    value |= (MoveLogFlags::USED_LV_1 << (level - 1));
    g_Mod->state_.SetOption(STAT_PERM_MOVE_LOG, value, move_type);

    if (move_type < MoveType::SP_SWEET_TREAT) {
        g_Mod->state_.ChangeOption(STAT_RUN_JUMPS_HAMMERS_USED);
    }
    if (g_MoveData[move_type].move_tier > 0) {
        g_Mod->state_.ChangeOption(STAT_RUN_UNLOCK_MOVES_USED);
    }

    // Mark completion of "Use a 3+ SP move on turn 1".
    if (move_type >= MoveType::SP_SWEET_TREAT && 
        move_type <= MoveType::SP_SUPERNOVA &&
        g_MoveData[move_type].move_cost[level - 1] >= 3 &&
        ttyd::battle::g_BattleWork->turn_count == 1) {
        AchievementsManager::MarkCompleted(AchievementId::V2_MISC_SP_TURN1);
    }

    // Check for new completion of "Using 10 different Lvl. 3 moves".
    int32_t lv3_uses = 0;
    for (int32_t i = 0; i < MoveType::MOVE_TYPE_MAX; ++i) {
        uint32_t value = g_Mod->state_.GetOption(STAT_PERM_MOVE_LOG, i);
        if (value & MoveLogFlags::USED_LV_3) ++lv3_uses;
    }
    if (lv3_uses >= 10) {
        AchievementsManager::MarkCompleted(AchievementId::AGG_USE_LV_3_MOVES_10);
    }

    // Check for move log completion.
    AchievementsManager::CheckCompleted(AchievementId::V2_META_USE_50_MOVES);
    AchievementsManager::CheckCompleted(AchievementId::META_MOVE_LOG_ALL);
}

void MoveManager::LogMoveStylish(int32_t move_type, uint32_t stylish_flags) {
    uint32_t value = g_Mod->state_.GetOption(STAT_PERM_MOVE_LOG, move_type);
    value |= stylish_flags ? stylish_flags : MoveLogFlags::STYLISH_ALL;
    g_Mod->state_.SetOption(STAT_PERM_MOVE_LOG, value, move_type);

    // Check for new completion of "15 Stylishes".
    int32_t stylish_count = 0;
    for (int32_t i = 0; i < MoveType::MOVE_TYPE_MAX; ++i) {
        uint32_t value = g_Mod->state_.GetOption(STAT_PERM_MOVE_LOG, i);
        if ((value & MoveLogFlags::STYLISH_ALL) == MoveLogFlags::STYLISH_ALL) {
            ++stylish_count;
        }
    }
    if (stylish_count >= 15) {
        AchievementsManager::MarkCompleted(AchievementId::AGG_STYLISH_15);
    }

    // Check for move log completion.
    AchievementsManager::CheckCompleted(AchievementId::META_MOVE_LOG_ALL);
}

const char* MoveManager::GetLogDescription(int32_t move_type) {
    static char buf[1024];
    sprintf(
        buf, "%s%s%s%s",
        msgSearch(g_MoveData[move_type].desc_msg),
        GetLogMoveLevelDesc(move_type, 1),
        GetLogMoveLevelDesc(move_type, 2),
        GetLogMoveLevelDesc(move_type, 3));
    return buf;
}

bool MoveManager::PartnerNeverObtained(int32_t move_type) {
    if (move_type < MoveType::GOOMBELLA_BASE) return false;
    int32_t partner_flag = 1 << ((move_type - MoveType::GOOMBELLA_BASE) / 6 + 1);
    return !(g_Mod->state_.GetOption(STAT_PERM_PARTNERS_OBTAINED) & partner_flag);
}

bool MoveManager::HasReachedLimit(int32_t move_base) {
    int32_t limit = 5 - g_Mod->state_.GetOption(OPT_MOVE_LIMIT);
    if (limit == 5) return false;

    // Get number of moves, and make sure that untaken partners aren't skipped.
    int32_t num_moves = 8;
    if (move_base >= MoveType::GOOMBELLA_BASE) {
        num_moves = 6;

        int32_t id = g_MoveData[move_base].partner_id;
        if (!(ttyd::mario_pouch::pouchGetPtr()->party_data[id].flags & 1)) {
            return false;
        }
    }
    
    // Count number of non-default moves unlocked for the given type.
    int32_t unlocked = 0;
    for (int32_t move = move_base; move < move_base + num_moves; ++move) {
        if (g_MoveData[move].move_tier > 0 &&
            g_Mod->state_.GetOption(STAT_RUN_MOVE_LV_UNLOCKED, move)) {
            ++unlocked;
        }
    }
    return unlocked >= limit;
}

uint32_t GetWeaponPowerFromSelectedLevel(
    BattleWorkUnit* unit1, BattleWeapon* weapon, BattleWorkUnit* unit2,
    BattleWorkUnitPart* part) {
    const int32_t move = weapon->damage_function_params[7];

    const int32_t ac_success =
        ((g_BattleWork->ac_manager_work.ac_result == 2) || 
        GetSWByte(GSW_Battle_DooplissMove)) ? 1 : 0;

    int32_t level = MoveManager::GetSelectedLevel(move);
    // To prevent issues with Spin Jump first strike if Spring Jump taken first.
    if (level < 1) level = 1;
        
    int32_t power = weapon->damage_function_params[level * 2 - 2 + ac_success];
    if ((move & ~7) == MoveType::JUMP_BASE) {
        power += unit1->badges_equipped.jumpman;
    } else if ((move & ~7) == MoveType::HAMMER_BASE) {
        power += unit1->badges_equipped.hammerman;
    }
    return power;
}

uint32_t GetWeaponPowerFromMaxLevel(
    BattleWorkUnit* unit1, BattleWeapon* weapon, BattleWorkUnit* unit2,
    BattleWorkUnitPart* part) {
    const int32_t move = weapon->damage_function_params[7];
    const int32_t ac_success =
        g_BattleWork->ac_manager_work.ac_result == 2 ? 1 : 0;
    int32_t level = MoveManager::GetUnlockedLevel(move);
    // To prevent issues with Spin Jump first strike if Spring Jump taken first.
    if (level < 1) level = 1;
        
    int32_t power = weapon->damage_function_params[level * 2 - 2 + ac_success];
    if ((move & ~7) == MoveType::JUMP_BASE) {
        power += unit1->badges_equipped.jumpman;
    } else if ((move & ~7) == MoveType::HAMMER_BASE) {
        power += unit1->badges_equipped.hammerman;
    }
    return power;
}

uint32_t GetWeaponPowerFromUnitWorkVariable(
    ttyd::battle_unit::BattleWorkUnit* unit1,
    ttyd::battle_database_common::BattleWeapon* weapon,
    ttyd::battle_unit::BattleWorkUnit* unit2,
    ttyd::battle_unit::BattleWorkUnitPart* part) {
    return unit1->unit_work[weapon->damage_function_params[0]];
}

EVT_DEFINE_USER_FUNC(evtTot_GetMoveSelectedLevel) {
    int32_t move = evtGetValue(evt, evt->evtArguments[0]);
    evtSetValue(
        evt, evt->evtArguments[1], MoveManager::GetSelectedLevel(move));
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_UpgradeMove) {
    int32_t move = evtGetValue(evt, evt->evtArguments[0]);
    MoveManager::UpgradeMove(move);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_LogActiveMoveStylish) {
    // Get the move type for the currently selected command.
    auto* command_work = &g_BattleWork->command_work;
    ttyd::battle::BattleWorkCommandCursor* cursor;
    ttyd::battle_seq_command::_btlcmd_GetCursorPtr(
        command_work, command_work->current_cursor_type, &cursor);
    const int32_t move_type = 
        command_work->weapon_table[cursor->abs_position].index;

    if (static_cast<int32_t>(evtGetValue(evt, evt->evtArguments[0])) == -1) {
        // Special case, since this Stylish doesn't occur within the move.
        MoveManager::LogMoveStylish(
            MoveType::VIVIAN_VEIL, MoveLogFlags::STYLISH_ALL);
    } else {
        const uint32_t stylish_flags =
            evtGetValue(evt, evt->evtArguments[0]) * MoveLogFlags::STYLISH_1;
        MoveManager::LogMoveStylish(move_type, stylish_flags);
    }

    return 2;
}

}  // namespace mod::tot