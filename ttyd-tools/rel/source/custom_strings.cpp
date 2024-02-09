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
        BTL_HLP_CMD_OPERATION_SUPER_CHARGE = 0,
        CUSTOM_TATTLE_BATTLE,
        CUSTOM_TATTLE_MENU,
        IN_2BAI_DAMAGE,
        IN_ACH_1,
        IN_ACH_2,
        IN_ACH_3,
        IN_CAKE,
        IN_KONRAN_HAMMER,
        IN_SUITORU,
        IN_TOUGHEN_UP,
        IN_TOUGHEN_UP_P,
        LIST_ICE_CANDY,
        LIST_NANCY_FRAPPE,
        MENU_2BAI_DAMAGE,
        MENU_CHARGE,
        MENU_CHARGE_P,
        MENU_DAMAGE_FLOWER,
        MENU_DAMAGE_FLOWER_P,
        MENU_DAMAGE_GAESHI,
        MENU_FIRE_NAGURI,
        MENU_ICE_NAGURI,
        MENU_KIKEN_DE_POWER,
        MENU_KIKEN_DE_POWER_P,
        MENU_PINCH_DE_GANBARU,
        MENU_PINCH_DE_GANBARU_P,
        MENU_TAMATSUKI_JUMP,
        MENU_TSURANUKI_NAGURI,
        MSG_2BAI_DAMAGE,
        MSG_ACH_1,
        MSG_ACH_2,
        MSG_ACH_3,
        MSG_CAKE,
        MSG_CRYSTAL_STAR,
        MSG_CUSTOM_SUPER_BOOTS,
        MSG_CUSTOM_SUPER_HAMMER,
        MSG_CUSTOM_ULTRA_BOOTS,
        MSG_CUSTOM_ULTRA_HAMMER,
        MSG_DAMAGE_FLOWER,
        MSG_DAMAGE_FLOWER_P,
        MSG_DAMAGE_GAESHI,
        MSG_DIAMOND_STAR,
        MSG_EMERALD_STAR,
        MSG_GARNET_STAR,
        MSG_GATSUN_JUMP,
        MSG_GATSUN_NAGURI,
        MSG_GOLD_STAR,
        MSG_ICE_CANDY,
        MSG_ICE_NAGURI,
        MSG_JISHIN_ATTACK,
        MSG_JON_KANBAN_1,
        MSG_JON_KANBAN_2,
        MSG_JON_KANBAN_3,
        MSG_JON_MOVER_SELECT,
        MSG_KAITEN_HAMMER,
        MSG_KAME_NO_NOROI,
        MSG_KIKEN_DE_POWER,
        MSG_KIKEN_DE_POWER_P,
        MSG_KONRAN_HAMMER,
        MSG_KURI_MAP,
        MSG_NANCY_FRAPPE,
        MSG_NEMURASE_FUMI,
        MSG_PBM_JIGEN_BAKUDAN,
        MSG_PCH_KISS,
        MSG_PCH_MADOWASERU,
        MSG_PINCH_DE_GANBARU,
        MSG_PINCH_DE_GANBARU_P,
        MSG_PKR_MONOSIRI,
        MSG_PNK_NORMAL_ATTACK,
        MSG_PNK_SYUBIBIN_KOURA,
        MSG_PTR_MEROMERO_KISS,
        MSG_PWD_KUMOGAKURE,
        MSG_RUBY_STAR,
        MSG_SAPPHIRE_STAR,
        MSG_SHIKAESHI_NO_KONA,
        MSG_ST_CHG_ALLERGY,
        MSG_SUPER_COIN,
        MSG_SUPER_GENKI,
        MSG_TAMATSUKI_JUMP,
        MSG_TEKI_KYOUKA,
        MSG_TOUGHEN_UP,
        MSG_TOUGHEN_UP_MENU,
        MSG_TOUGHEN_UP_P,
        MSG_TOUGHEN_UP_P_MENU,
        MSG_TREASURE_MAP,
        MSG_ULTRA_HAMMER,
        PIT_CHARLIETON_FULL_INV,
        PIT_CHEST_UNCLAIMED,
        PIT_DISABLED_RETURN,
        PIT_MOVE_LEVEL,
        PIT_REWARD_PARTY_JOIN,
        RIPPO_CONFIRM_BP,
        RIPPO_CONFIRM_FP,
        RIPPO_CONFIRM_HP,
        RIPPO_EXIT,
        RIPPO_INTRO,
        RIPPO_ITEM_DIFFERENT,
        RIPPO_ITEM_OK,
        RIPPO_ITEM_THANKS_LAST,
        RIPPO_ITEM_THANKS_NEXT,
        RIPPO_NO_BADGES,
        RIPPO_NO_FREE_BP,
        RIPPO_NO_ITEMS,
        RIPPO_NO_STATS,
        RIPPO_STAT_DIFFERENT,
        RIPPO_STAT_MENU,
        RIPPO_STAT_THANKS_LAST,
        RIPPO_STAT_THANKS_NEXT,
        RIPPO_STAT_TOO_LOW,
        RIPPO_TOP_MENU,
        RIPPO_WHICH_ITEM,
        RIPPO_WHICH_STAT,
        RIPPO_YES_NO,
        TIK_06_02,
        TOT_PTR1_IRONBONK,
        TOT_PTR1_IRONBONK_DESC,
        TOT_PTR1_SCOPE_OUT,
        TOT_PTR1_SCOPE_OUT_DESC,
        TOT_PTR1_SCOPE_OUT_EFFECT_MSG,
        TOT_PTR2_BULKUP,
        TOT_PTR2_BULKUP_DESC,
        TOT_PTR2_WITHDRAW,
        TOT_PTR2_WITHDRAW_DESC,
        TOT_PTR3_BLIZZARD,
        TOT_PTR3_BLIZZARD_DESC,
        TOT_PTR3_THUNDER,
        TOT_PTR3_THUNDER_AC,
        TOT_PTR3_THUNDER_DESC,
        TOT_PTR4_EGG,
        TOT_PTR4_EGG_DESC,
        TOT_PTR4_GULP,
        TOT_PTR4_GULP_DESC,
        TOT_PTR4_SPIT,
        TOT_PTR4_SPIT_DESC,
        TOT_PTR5_CURSE,
        TOT_PTR5_CURSE_DESC,
        TOT_PTR5_INFATUATE_EFFECT_MSG,
        TOT_PTR5_NEUTRALIZE,
        TOT_PTR5_NEUTRALIZE_DESC,
        TOT_PTR6_MEGATONBOMB,
        TOT_PTR6_MEGATONBOMB_DESC,
        TOT_PTR6_POISONBOMB,
        TOT_PTR6_POISONBOMB_DESC,
        TOT_PTR7_EMBARGO,
        TOT_PTR7_EMBARGO_DESC,
        TOT_PTR7_SMOKEBOMB,
        TOT_PTR7_SMOKEBOMB_DESC,
        x83x70x83x8f_CHET_RIPPO_TATTLE,
    };
}
    
namespace {

// Keys for strings to be replaced / added to msgSearch. Should be kept
// in sync with the above enum, and always maintain alphabetical order.
constexpr const char* kKeyLookups[] = {
    "btl_hlp_cmd_operation_super_charge",
    "custom_tattle_battle",
    "custom_tattle_menu",
    "in_2bai_damage",
    "in_ach_1",
    "in_ach_2",
    "in_ach_3",
    "in_cake",
    "in_konran_hammer",
    "in_suitoru",
    "in_toughen_up",
    "in_toughen_up_p",
    "list_ice_candy",
    "list_nancy_frappe",
    "menu_2bai_damage",
    "menu_charge",
    "menu_charge_p",
    "menu_damage_flower",
    "menu_damage_flower_p",
    "menu_damage_gaeshi",
    "menu_fire_naguri",
    "menu_ice_naguri",
    "menu_kiken_de_power",
    "menu_kiken_de_power_p",
    "menu_pinch_de_ganbaru",
    "menu_pinch_de_ganbaru_p",
    "menu_tamatsuki_jump",
    "menu_tsuranuki_naguri",
    "msg_2bai_damage",
    "msg_ach_1",
    "msg_ach_2",
    "msg_ach_3",
    "msg_cake",
    "msg_crystal_star",
    "msg_custom_super_boots",
    "msg_custom_super_hammer",
    "msg_custom_ultra_boots",
    "msg_custom_ultra_hammer",
    "msg_damage_flower",
    "msg_damage_flower_p",
    "msg_damage_gaeshi",
    "msg_diamond_star",
    "msg_emerald_star",
    "msg_garnet_star",
    "msg_gatsun_jump",
    "msg_gatsun_naguri",
    "msg_gold_star",
    "msg_ice_candy",
    "msg_ice_naguri",
    "msg_jishin_attack",
    "msg_jon_kanban_1",
    "msg_jon_kanban_2",
    "msg_jon_kanban_3",
    "msg_jon_mover_select",
    "msg_kaiten_hammer",
    "msg_kame_no_noroi",
    "msg_kiken_de_power",
    "msg_kiken_de_power_p",
    "msg_konran_hammer",
    "msg_kuri_map",
    "msg_nancy_frappe",
    "msg_nemurase_fumi",
    "msg_pbm_jigen_bakudan",
    "msg_pch_kiss",
    "msg_pch_madowaseru",
    "msg_pinch_de_ganbaru",
    "msg_pinch_de_ganbaru_p",
    "msg_pkr_monosiri",
    "msg_pnk_normal_attack",
    "msg_pnk_syubibin_koura",
    "msg_ptr_meromero_kiss",
    "msg_pwd_kumogakure",
    "msg_ruby_star",
    "msg_sapphire_star",
    "msg_shikaeshi_no_kona",
    "msg_st_chg_allergy",
    "msg_super_coin",
    "msg_super_genki",
    "msg_tamatsuki_jump",
    "msg_teki_kyouka",
    "msg_toughen_up",
    "msg_toughen_up_menu",
    "msg_toughen_up_p",
    "msg_toughen_up_p_menu",
    "msg_treasure_map",
    "msg_ultra_hammer",
    "pit_charlieton_full_inv",
    "pit_chest_unclaimed",
    "pit_disabled_return",
    "pit_move_level",
    "pit_reward_party_join",
    "rippo_confirm_bp",
    "rippo_confirm_fp",
    "rippo_confirm_hp",
    "rippo_exit",
    "rippo_intro",
    "rippo_item_different",
    "rippo_item_ok",
    "rippo_item_thanks_last",
    "rippo_item_thanks_next",
    "rippo_no_badges",
    "rippo_no_free_bp",
    "rippo_no_items",
    "rippo_no_stats",
    "rippo_stat_different",
    "rippo_stat_menu",
    "rippo_stat_thanks_last",
    "rippo_stat_thanks_next",
    "rippo_stat_too_low",
    "rippo_top_menu",
    "rippo_which_item",
    "rippo_which_stat",
    "rippo_yes_no",
    "tik_06_02",
    "tot_ptr1_ironbonk",
    "tot_ptr1_ironbonk_desc",
    "tot_ptr1_scope_out",
    "tot_ptr1_scope_out_desc",
    "tot_ptr1_scope_out_effect_msg",
    "tot_ptr2_bulkup",
    "tot_ptr2_bulkup_desc",
    "tot_ptr2_withdraw",
    "tot_ptr2_withdraw_desc",
    "tot_ptr3_blizzard",
    "tot_ptr3_blizzard_desc",
    "tot_ptr3_thunder",
    "tot_ptr3_thunder_ac",
    "tot_ptr3_thunder_desc",
    "tot_ptr4_egg",
    "tot_ptr4_egg_desc",
    "tot_ptr4_gulp",
    "tot_ptr4_gulp_desc",
    "tot_ptr4_spit",
    "tot_ptr4_spit_desc",
    "tot_ptr5_curse",
    "tot_ptr5_curse_desc",
    "tot_ptr5_infatuate_effect_msg",
    "tot_ptr5_neutralize",
    "tot_ptr5_neutralize_desc",
    "tot_ptr6_megatonbomb",
    "tot_ptr6_megatonbomb_desc",
    "tot_ptr6_poisonbomb",
    "tot_ptr6_poisonbomb_desc",
    "tot_ptr7_embargo",
    "tot_ptr7_embargo_desc",
    "tot_ptr7_smokebomb",
    "tot_ptr7_smokebomb_desc",
    "\x83\x70\x83\x8f\x81\x5b\x83\x5f\x83\x45\x83\x93\x89\xae",  // Chet Rippo
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
        case MsgKey::PIT_CHARLIETON_FULL_INV:
            return "<p>\n"
                   "Ooh, you can't carry any more \n"
                   "stuff, my man. You sure you\n"
                   "want to buy this anyway?\n<o>";
        case MsgKey::PIT_REWARD_PARTY_JOIN:
            return "<system>\n<p>\nYou got a new party member!\n<k>";
        case MsgKey::PIT_DISABLED_RETURN:
            return "<system>\n<p>\nYou can't leave the Infinite Pit!\n<k>";
        case MsgKey::PIT_CHEST_UNCLAIMED:
            return "<system>\n<p>\nYou haven't claimed your\nreward!\n<k>";
        case MsgKey::PIT_MOVE_LEVEL:
            return "<system>\n<p>\n"
                   "You obtained a Lv. 2 Star Power!\n"
                   "Tap left or right to select which\n"
                   "power level you want to use.\n<k>";
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
        case MsgKey::IN_2BAI_DAMAGE:
            return "No Pain, No Gain";
        case MsgKey::IN_CAKE:
            return "Strawberry Cake";
        case MsgKey::IN_SUITORU:
            return "HP-Sucker";
        case MsgKey::IN_TOUGHEN_UP:
            return "Toughen Up";
        case MsgKey::IN_TOUGHEN_UP_P:
            return "Toughen Up P";
        case MsgKey::MSG_2BAI_DAMAGE:
        case MsgKey::MENU_2BAI_DAMAGE:
            return "Doubles the damage Mario \n"
                   "takes, but doubles coin drops.";
        case MsgKey::MSG_CAKE:
            return "Scrumptious strawberry cake.\n"
                   "Heals 5 to 30 HP and FP.";
        case MsgKey::MSG_SHIKAESHI_NO_KONA:
            return "Direct attackers take back\n"
                   "the same damage they deal.";
        case MsgKey::MSG_KAME_NO_NOROI:
            return "Has a chance of inducing Slow \n"
                   "status on all foes.";
        case MsgKey::MSG_TEKI_KYOUKA:
            return "Boosts foes' level by 5, but \n"
                   "temporarily gives them +3 ATK.";
        case MsgKey::MSG_ICE_CANDY:
            return "A dessert made by Zess T.\n"
                   "Gives 15 FP, but might freeze!";
        case MsgKey::LIST_ICE_CANDY:
            return "A dessert made by Zess T.\n"
                   "Gives 15 FP, but might freeze!\n"
                   "Made by mixing Honey Syrup \n"
                   "with an Ice Storm.";
        case MsgKey::MSG_NANCY_FRAPPE:
            return "A dessert made by Zess T.\n"
                   "Gives 20 FP, but might freeze!";
        case MsgKey::LIST_NANCY_FRAPPE:
            return "A dessert made by Zess T.\n"
                   "Gives 20 FP, but might freeze!\n"
                   "Made by mixing Maple Syrup \n"
                   "with an Ice Storm.";
        case MsgKey::MSG_TOUGHEN_UP:
            return "Wear this to add Toughen Up\n"
                   "to Mario's Tactics menu.";
        case MsgKey::MSG_TOUGHEN_UP_P:
            return "Wear this to add Toughen Up\n"
                   "to partners' Tactics menu.";
        case MsgKey::MSG_TOUGHEN_UP_MENU:
            return "Wear this to add Toughen Up\n"
                   "to Mario's Tactics menu.\n"
                   "This uses 1 FP to raise DEF\n"
                   "by 2 points for a turn.\n"
                   "Wearing more copies raises\n"
                   "the effect and FP cost.";
        case MsgKey::MSG_TOUGHEN_UP_P_MENU:
            return "Wear this to add Toughen Up\n"
                   "to partners' Tactics menu.\n"
                   "This uses 1 FP to raise DEF\n"
                   "by 2 points for a turn.\n"
                   "Wearing more copies raises\n"
                   "the effect and FP cost.";
        case MsgKey::BTL_HLP_CMD_OPERATION_SUPER_CHARGE:
            return "Briefly increases DEF by\n"
                   "more than Defending.";
        case MsgKey::MSG_SUPER_COIN:
            return "A powerful object that raises\n"
                   "max SP, or ranks up a partner.";
        case MsgKey::MSG_CUSTOM_SUPER_BOOTS:
            return "A stronger pair of boots.";
        case MsgKey::MSG_CUSTOM_ULTRA_BOOTS:
            return "An even stronger pair of boots.";
        case MsgKey::MSG_CUSTOM_SUPER_HAMMER:
            return "A more powerful hammer.";
        case MsgKey::MSG_CUSTOM_ULTRA_HAMMER:
            return "An even more powerful hammer.";
        case MsgKey::MENU_TAMATSUKI_JUMP:
            return "Wear this to use Tornado\n"
                   "Jump, a 2-FP move which can\n"
                   "damage and dizzy airborne\n"
                   "enemies if executed well.\n"
                   "Wearing more copies of the\n"
                   "badge increases its FP\n"
                   "cost and attack power.";
        case MsgKey::MENU_TSURANUKI_NAGURI:
            return "Wear this to use Piercing\n"
                   "Blow, a 2-FP move which\n"
                   "deals damage that pierces\n"
                   "enemy defenses.\n"
                   "Wearing more copies of the\n"
                   "badge increases its FP\n"
                   "cost and attack power.";
        case MsgKey::MENU_FIRE_NAGURI:
            return "Wear this to use Fire Drive,\n"
                   "a 3-FP move which deals \n"
                   "fire damage and Burn status\n"
                   "to all grounded enemies.\n"
                   "Wearing more copies of the\n"
                   "badge increases its FP\n"
                   "cost and attack power.";
        case MsgKey::MENU_ICE_NAGURI:
            return "Wear this to use Ice Smash,\n"
                   "a 2-FP move which deals\n"
                   "ice damage and may give its\n"
                   "target the Freeze status.\n"
                   "Wearing more copies of the\n"
                   "badge increases its FP\n"
                   "cost and status duration.";
        case MsgKey::MENU_CHARGE:
            return "Wear this to add Charge\n"
                   "to Mario's Tactics menu.\n"
                   "This uses 2 FP to increase\n"
                   "the next move's ATK by 2.\n"
                   "Wearing more copies raises\n"
                   "the effect and FP cost.";
        case MsgKey::MENU_CHARGE_P:
            return "Wear this to add Charge\n"
                   "to partners' Tactics menu.\n"
                   "This uses 2 FP to increase\n"
                   "the next move's ATK by 2.\n"
                   "Wearing more copies raises\n"
                   "the effect and FP cost.";
        case MsgKey::MSG_DAMAGE_GAESHI:
        case MsgKey::MENU_DAMAGE_GAESHI:
            return "Make direct-attackers take\n"
                   "the same damage they deal.";
        case MsgKey::MSG_DAMAGE_FLOWER:
        case MsgKey::MENU_DAMAGE_FLOWER:
            return "Recover 1 FP whenever\n"
                   "Mario receives damage.";
        case MsgKey::MSG_DAMAGE_FLOWER_P:
        case MsgKey::MENU_DAMAGE_FLOWER_P:
            return "Recover 1 FP whenever your\n"
                   "partner receives damage.";
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
        case MsgKey::MSG_SUPER_GENKI:
            return "Restores Mario and his ally's\n"
                   "HP and FP over 5 turns.";
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
        case MsgKey::IN_ACH_1:
            return "Chest Collector";
        case MsgKey::IN_ACH_2:
            return "Badge Buff";
        case MsgKey::IN_ACH_3:
            return "Tattletale";
        case MsgKey::MSG_ACH_1:
            return "Commemorates those that have\n"
                   "collected all unique rewards.";
        case MsgKey::MSG_ACH_2:
            return "Commemorates those that have\n"
                   "collected one of every badge.";
        case MsgKey::MSG_ACH_3:
            return "Commemorates those that have\n"
                   "Tattled every kind of enemy.";
        case MsgKey::RIPPO_CONFIRM_BP:
            return "<p>\nI'll give you 39 coins for 3 BP.\n"
                   "<wait 350>You won't be able to get it back,\n"
                   "so are you absolutely sure?\n<o>";
        case MsgKey::RIPPO_CONFIRM_FP:
            return "<p>\nI'll give you 39 coins for 5 FP.\n"
                   "<wait 350>You won't be able to get it back,\n"
                   "so are you absolutely sure?\n<o>";
        case MsgKey::RIPPO_CONFIRM_HP:
            return "<p>\nI'll give you 39 coins for 5 HP.\n"
                   "<wait 350>You won't be able to get it back,\n"
                   "so are you absolutely sure?\n<o>";
        case MsgKey::RIPPO_EXIT:
            return "<p>\nNow get outta here!\n<k>";
        case MsgKey::RIPPO_INTRO:
            return "<shake>\n"
                   "Hee hee hee!</shake><wait 300> I can help\n"
                   "lighten your load if you've\n"
                   "got junk weighing you down!\n<k>\n<p>\n"
                   "What're you selling?\n<o>";
        case MsgKey::RIPPO_ITEM_DIFFERENT:
            return "<p>\nHmph. Anything else tickle \n"
                   "your fancy, then?\n<k>";
        case MsgKey::RIPPO_STAT_DIFFERENT:
            return "<p>\nHmph. Anything else tickle \n"
                   "your fancy, then?\n<o>";
        case MsgKey::RIPPO_ITEM_OK:
            return "I'm afraid I can only spare\n"
                   "a mere <NUM> coin<S> for your\n"
                   "<ITEM>. OK?\n<o>";
        case MsgKey::RIPPO_ITEM_THANKS_LAST:
            return "<p>\nIf you don't have anything\n"
                   "else for me, then get\n"
                   "outta here!\n<k>";
        case MsgKey::RIPPO_ITEM_THANKS_NEXT:
            return "<p>\nYeah, yeah. <wait 500>You got any\n"
                   "other goods?\n<o>";
        case MsgKey::RIPPO_NO_BADGES:
        case MsgKey::RIPPO_NO_ITEMS:
            return "<p>\nI don't see any of those\n"
                   "in your stash. <wait 300>Quit wastin'\n"
                   "both our time!\n<k>";
        case MsgKey::RIPPO_NO_FREE_BP:
            return "<p>\nYou've got too many badges\n"
                   "equipped! <wait 300>Make some space\n"
                   "first, <wait 200>sheesh!\n<k>";
        case MsgKey::RIPPO_NO_STATS:
            return "<p>\n<dynamic 3>Ha,</dynamic> <wait 200>are you kidding?\n"
                   "<wait 300>Go level up some, <wait 150>then we\n"
                   "can talk business!\n<k>";
        case MsgKey::RIPPO_STAT_MENU:
            return "<select 0 3 0 40>\nHP\nFP\nBP";
        case MsgKey::RIPPO_STAT_THANKS_LAST:
            return "<p>\nI'm finished. <wait 500>So... \n"
                   "<wait 300>Unless you need anything\n"
                   "else, get outta here!\n<k>";
        case MsgKey::RIPPO_STAT_THANKS_NEXT:
            return "<p>\nDone. <wait 500>So... \n"
                   "<wait 300>Got any other stat points\n"
                   "you aren't usin'?\n<o>";
        case MsgKey::RIPPO_STAT_TOO_LOW:
            return "<p>\nYou're looking too low on\n"
                   "that already! <wait 250>Perhaps\n"
                   "something else?\n<o>";
        case MsgKey::RIPPO_TOP_MENU: {
            if (g_Mod->state_.GetOptionNumericValue(OPT_NO_EXP_MODE)) {
                return "<select 0 3 0 40>\nItems\nBadges";
            }
            return "<select 0 3 0 40>\nItems\nBadges\nLevel-ups";
        }
        case MsgKey::RIPPO_WHICH_ITEM:
            return "<p>\nWhaddya want to sell?\n<k>";
        case MsgKey::RIPPO_WHICH_STAT:
            return "<p>\nWhaddya want to sell?\n<o>";
        case MsgKey::RIPPO_YES_NO:
            return "<select 0 1 0 40>\nYes\nNo";
        case MsgKey::x83x70x83x8f_CHET_RIPPO_TATTLE:
            return "<keyxon>\nThat's Chet Rippo, the\n"
                   "adjuster.<dkey><wait 300></dkey> Seems he's found a\n"
                   "new niche here in the Pit.\n<k>\n<p>\n"
                   "He'll take your unwanted\n"
                   "items, badges, and level-ups\n"
                   "in exchange for a few coins.\n<k>\n<p>\n"
                   "Me, <wait 200>I wouldn't trust this guy\n"
                   "personally, Mario, but I guess\n"
                   "if you're desperate...\n<k>";
        case MsgKey::MSG_JON_MOVER_SELECT:
            return GetMoverOptionsString(buf);
        case MsgKey::MSG_KURI_MAP:
            return "<keyxon>\nWe're in the Infinite Pit,\n"
                   "an endless series of trials\n"
                   "and enemy encounters.\n<k>\n<p>\n"
                   "It may be never-ending, but that\n"
                   "doesn't mean we can't give it\n"
                   "our best effort, right, Mario?\n<k>";
        case MsgKey::TOT_PTR1_SCOPE_OUT_EFFECT_MSG:
            return "Scoped! Your next attack is\n"
                   "guaranteed to connect!";
        case MsgKey::TOT_PTR5_INFATUATE_EFFECT_MSG:
            return "Infatuated! The enemy will\n"
                   "fight for your side now!";
        case MsgKey::MSG_ST_CHG_ALLERGY:
            return "Allergic! Status effects\n"
                   "cannot be changed!";
        // TOT item name changes.
        case MsgKey::IN_KONRAN_HAMMER:
            return "Shrink Smash";
        case MsgKey::TOT_PTR1_IRONBONK:
            return "Ironbonk";
        case MsgKey::TOT_PTR1_SCOPE_OUT:
            return "Scope Out";
        case MsgKey::TOT_PTR2_WITHDRAW:
            return "Withdraw";
        case MsgKey::TOT_PTR2_BULKUP:
            return "Bulk Up";
        case MsgKey::TOT_PTR3_BLIZZARD:
            return "Blizzard";
        case MsgKey::TOT_PTR3_THUNDER:
            return "Thunder Storm";
        case MsgKey::TOT_PTR4_EGG:
            return "Egg Barrage";
        case MsgKey::TOT_PTR4_SPIT:
            return "Lickety Spit";
        case MsgKey::TOT_PTR4_GULP:
            return "Gulp";
        case MsgKey::TOT_PTR5_CURSE:
            return "Curse";
        case MsgKey::TOT_PTR5_NEUTRALIZE:
            return "Neutralize";
        case MsgKey::TOT_PTR6_POISONBOMB:
            return "Poison Bomb";
        case MsgKey::TOT_PTR6_MEGATONBOMB:
            return "Megaton Bomb";
        case MsgKey::TOT_PTR7_EMBARGO:
            return "Embargo";
        case MsgKey::TOT_PTR7_SMOKEBOMB:
            return "Smoke Bomb";
        // TOT item/move description changes.
        case MsgKey::MSG_GATSUN_JUMP:
            return "Attack an enemy with a single\n"
                   "powerful stomp, softening them.";
        case MsgKey::MSG_NEMURASE_FUMI:
            return "Attack an enemy with a stomp\n"
                   "that can cause drowziness.";
        case MsgKey::MSG_TAMATSUKI_JUMP:
            return "Hit all enemies with a gust of\n"
                   "wind, dizzying aerial foes.";
        case MsgKey::MSG_KAITEN_HAMMER:
        case MsgKey::MSG_ULTRA_HAMMER:
            return "Strike an enemy, knocking it\n"
                   "into the enemies behind it.";
        case MsgKey::MSG_GATSUN_NAGURI:
            return "Pound an enemy with a powerful,\n"
                   "defense-piercing strike.";
        case MsgKey::MSG_KONRAN_HAMMER:
            return "Strike an enemy with a blow\n"
                   "that may shrink them.";
        case MsgKey::MSG_ICE_NAGURI:
            return "Strike an enemy with a blow\n"
                   "that may freeze them solid.";
        case MsgKey::MSG_JISHIN_ATTACK:
            return "Attack all grounded enemies,\n"
                   "piercing their defense.";
        case MsgKey::MSG_PKR_MONOSIRI:
            return "View enemies' descriptions\n"
                   "and see their stats in battle.";
        case MsgKey::TOT_PTR1_IRONBONK_DESC:
            return "Attack with an invulnerable,\n"
                   "defense-piercing Headbonk.";
        case MsgKey::TOT_PTR1_SCOPE_OUT_DESC:
            return "Scope out an enemy, ensuring\n"
                   "your next attack lands.";
        case MsgKey::MSG_PNK_NORMAL_ATTACK:
            return "Attack the front enemy with\n"
                   "a spinning shell drive.";
        case MsgKey::MSG_PNK_SYUBIBIN_KOURA:
            return "Attack all grounded enemies\n"
                   "with a spinning shell.";
        case MsgKey::TOT_PTR2_WITHDRAW_DESC:
            return "Make Koops retreat into his\n"
                   "shell, impervious to attack.";
        case MsgKey::TOT_PTR2_BULKUP_DESC:
            return "Raise an ally's Attack and\n"
                   "Defense power for a while.";
        case MsgKey::MSG_PWD_KUMOGAKURE:
            return "Envelop the party in a fog,\n"
                   "causing foes to sometimes miss.";
        case MsgKey::TOT_PTR3_BLIZZARD_DESC:
            return "Attack all enemies with a\n"
                   "blast of icy wind.";
        case MsgKey::TOT_PTR3_THUNDER_DESC:
            return "Strike all enemies with\n"
                   "powerful bolts of lightning!";
        case MsgKey::TOT_PTR4_EGG_DESC:
            return "Throw a volley of eggs at\n"
                   "foes, dealing light damage.";
        case MsgKey::TOT_PTR4_SPIT_DESC:
            return "Spit the front enemy into all\n"
                   "ground-bound enemies behind it.";
        case MsgKey::TOT_PTR4_GULP_DESC:
            return "Swallow an enemy in one gulp,\n"
                   "stealing their held item.";
        case MsgKey::TOT_PTR5_CURSE_DESC:
            return "Cast a hex on the enemy to\n"
                   "slow their movements.";
        case MsgKey::TOT_PTR5_NEUTRALIZE_DESC:
            return "Make the enemy unable to\n"
                   "change their status.";
        case MsgKey::MSG_PTR_MEROMERO_KISS:
            return "Blow a kiss to an enemy to try\n"
                   "to win them to your side.";
        case MsgKey::MSG_PBM_JIGEN_BAKUDAN:
            return "Throw a volley of time bombs\n"
                   "that explode on the next turn.";
        case MsgKey::TOT_PTR6_POISONBOMB_DESC:
            return "Throw a volley of bombs that\n"
                   "explode in a noxious blast.";
        case MsgKey::TOT_PTR6_MEGATONBOMB_DESC:
            return "Arm a bomb that does massive\n"
                   "damage to all combatants.";
        case MsgKey::MSG_PCH_MADOWASERU:
            return "Tease enemies with some sly\n"
                   "moves, confusing them.";
        case MsgKey::TOT_PTR7_EMBARGO_DESC:
            return "Distract foes and scatter\n"
                   "their held items and badges.";
        case MsgKey::TOT_PTR7_SMOKEBOMB_DESC:
            return "Lightly damage and dizzy foes\n"
                   "with a burst of smoke.";
        case MsgKey::MSG_PCH_KISS:
            return "Replenish the party's HP\n"
                   "with a friendly kiss.";
        case MsgKey::TOT_PTR3_THUNDER_AC:
            return "Press <icon PAD_A 0.6 1 2 6>"
                   "<icon PAD_B 0.6 1 2 6>"
                   "<icon PAD_Y 0.6 1 2 6>"
                   "<icon PAD_X 0.6 1 2 6> repeatedly\n"
                   "in quick succession!";
    }
    // Should not be reached.
    return nullptr;
}

}