#! /usr/bin/python3.6

"""Patches msg files used by TTYD, adding/updating particular text strings."""
# Jonathan Aldrich 2024-02-23

import os
import subprocess
import sys
from pathlib import Path

import flags    # jdalib

FLAGS = flags.Flags()

FLAGS.DefineString("in_msg_dir", "")
FLAGS.DefineString("out_msg_dir", "")

g_StringMap = {
    'global': {
        # Item names.
        b"in_cake":             b"Strawberry Cake",
        b"in_suitoru":          b"HP-Sucker",
        b"in_2bai_damage":      b"No Pain, No Gain",
        b"in_toughen_up":       b"Toughen Up",
        b"in_toughen_up_p":     b"Toughen Up P",
        b"in_super_start":      b"Super Start",
        b"in_perfect_power":    b"Perfect Power",
        b"in_perfect_power_p":  b"Perfect Power P",
        
        # Item descriptions.
        b"msg_super_coin":
            b"A powerful object that raises\n"
            b"max SP, or ranks up a partner.",
            
        b"msg_custom_super_boots":
            b"A stronger pair of boots.",
            
        b"msg_custom_ultra_boots":
            b"An even stronger pair of boots.",
            
        b"msg_custom_super_hammer":
            b"A more powerful hammer.",
            
        b"msg_custom_ultra_hammer":
            b"An even more powerful hammer.",
        
        b"msg_cake":
            b"Scrumptious strawberry cake.\n"
            b"Heals 5 to 30 HP and FP.",
            
        b"msg_shikaeshi_no_kona":
            b"Direct attackers take back\n"
            b"the same damage they deal.",
            
        b"msg_kame_no_noroi":
            b"Has a chance of inducing Slow \n"
            b"status on all foes.",
            
        b"msg_teki_kyouka":
            b"Boosts foes' level by 5, but \n"
            b"temporarily gives them +3 ATK.",
            
        b"msg_ice_candy":
            b"A dessert made by Zess T.\n"
            b"Gives 15 FP, but might freeze!",
            
        b"list_ice_candy":
            b"A dessert made by Zess T.\n"
            b"Gives 15 FP, but might freeze!\n"
            b"Made by mixing Honey Syrup \n"
            b"with an Ice Storm.",
            
        b"msg_nancy_frappe":
            b"A dessert made by Zess T.\n"
            b"Gives 20 FP, but might freeze!",
            
        b"list_nancy_frappe":
            b"A dessert made by Zess T.\n"
            b"Gives 20 FP, but might freeze!\n"
            b"Made by mixing Maple Syrup \n"
            b"with an Ice Storm.",
            
        b"msg_kiken_de_power":
            b"Increase Attack power by 3\n"
            b"when Mario is in Peril.",
            
        b"menu_kiken_de_power":
            b"Increase Attack power by 3\n"
            b"when Mario is in Peril.",
            
        b"msg_kiken_de_power_p":
            b"Increase Attack power by 3\n"
            b"when your partner is in Peril.",

        b"menu_kiken_de_power_p":
            b"Increase Attack power by 3\n"
            b"when your partner is in Peril.",

        b"msg_pinch_de_ganbaru":
            b"Increase Attack power by 1\n"
            b"when Mario is in Danger.",
            
        b"menu_pinch_de_ganbaru":
            b"Increase Attack power by 1\n"
            b"when Mario is in Danger.",
            
        b"msg_pinch_de_ganbaru_p":
            b"Increase Attack power by 1\n"
            b"when your ally is in Danger.",
            
        b"menu_pinch_de_ganbaru_p":
            b"Increase Attack power by 1\n"
            b"when your ally is in Danger.",
        
        b"msg_2bai_damage":
            b"Doubles the damage Mario \n"
            b"takes, but doubles coin drops.",
            
        b"menu_2bai_damage":
            b"Doubles the damage Mario \n"
            b"takes, but doubles coin drops.",
            
        b"msg_damage_gaeshi":
            b"Make direct-attackers take\n"
            b"the same damage they deal.",
            
        b"menu_damage_gaeshi":
            b"Make direct-attackers take\n"
            b"the same damage they deal.",
            
        b"menu_happy_heart":
            b"Mario restores 1 HP at the\n"
            b"start of every other turn.",
            
        b"msg_happy_heart":
            b"Mario restores 1 HP at the\n"
            b"start of every other turn.",
            
        b"menu_happy_heart_p":
            b"Allies restore 1 HP at the\n"
            b"start of every other turn.",
            
        b"msg_happy_heart_p":
            b"Allies restore 1 HP at the\n"
            b"start of every other turn.",
            
        b"menu_happy_flower":
            b"Mario restores 1 FP at the\n"
            b"start of every other turn.",
            
        b"menu_happy_flower":
            b"Mario restores 1 FP at the\n"
            b"start of every other turn.",
            
        b"msg_damage_flower":
            b"Recover 1 FP whenever\n"
            b"Mario receives damage.",
            
        b"menu_damage_flower":
            b"Recover 1 FP whenever\n"
            b"Mario receives damage.",
            
        b"msg_damage_flower_p":
            b"Recover 1 FP whenever your\n"
            b"partner receives damage.",
            
        b"menu_damage_flower_p":
            b"Recover 1 FP whenever your\n"
            b"partner receives damage.",
            
        b"menu_heart_suitoru":
            b"Drop Mario's Attack power by\n"
            b"1 but regain 1 HP per attack.\n"
            b"Wearing multiple copies raises\n"
            b"HP more, for no extra ATK loss.",
            
        b"menu_heart_suitoru_p":
            b"Drop your ally's Attack power\n"
            b"by 1 but regain 1 HP per attack.\n"
            b"Wearing multiple copies raises\n"
            b"HP more, for no extra ATK loss.",

        b"menu_flower_suitoru":
            b"Drop Mario's Attack power by\n"
            b"1 but regain 1 FP per attack.\n"
            b"Wearing multiple copies raises\n"
            b"FP more, for no extra ATK loss.",
            
        b"menu_flower_suitoru_p":
            b"Drop your ally's Attack power\n"
            b"by 1 but regain 1 FP per attack.\n"
            b"Wearing multiple copies raises\n"
            b"FP more, for no extra ATK loss.",

        b"msg_2kai_item":
            b"During battle, Mario can use\n"
            b"multiple items in one turn.",

        b"msg_2kai_item_p":
            b"During battle, allies can use\n"
            b"multiple items in one turn.",
            
        b"menu_2kai_item":
            b"Wear this to become able to\n"
            b"use two or three items on\n"
            b"Mario's turn, for a cost\n"
            b"of 4 or 8 FP.",

        b"menu_2kai_item_p":
            b"Wear this to allow partners\n"
            b"to use two or three items\n"
            b"on their turn, for a cost\n"
            b"of 4 or 8 FP.",
            
        b"menu_subayaku_kawaru":
            b"Allow your ally to attack\n"
            b"even after changing partners.\n"
            b"Switching costs 1 FP initially,\n"
            b"and increases on every use.",
        
        b"msg_toughen_up":
            b"Wear this to add Toughen Up\n"
            b"to Mario's Tactics menu.",
            
        b"msg_toughen_up_p":
            b"Wear this to add Toughen Up\n"
            b"to partners' Tactics menu.",
            
        b"msg_super_start":
            b"Grants a bit of Star Power\n"
            b"when starting a battle.",
            
        b"msg_perfect_power":
            b"Increases attack power by 1\n"
            b"when Mario is at full HP.",
            
        b"msg_perfect_power_p":
            b"Increases attack power by 1\n"
            b"when allies are at full HP.",
            
        b"msg_toughen_up_menu":
            b"Wear this to add Toughen Up\n"
            b"to Mario's Tactics menu.\n"
            b"This uses 1 FP to raise DEF\n"
            b"by 2 points for a turn.\n"
            b"Wearing more copies raises\n"
            b"the effect and FP cost.",
            
        b"msg_toughen_up_p_menu":
            b"Wear this to add Toughen Up\n"
            b"to partners' Tactics menu.\n"
            b"This uses 1 FP to raise DEF\n"
            b"by 2 points for a turn.\n"
            b"Wearing more copies raises\n"
            b"the effect and FP cost.",
            
        b"btl_hlp_cmd_operation_super_charge":
            b"Briefly increases DEF by\n"
            b"more than Defending.",    
        
        b"menu_tamatsuki_jump":
            b"Wear this to use Tornado\n"
            b"Jump, a 2-FP move which can\n"
            b"damage and dizzy airborne\n"
            b"enemies if executed well.\n"
            b"Wearing more copies of the\n"
            b"badge increases its FP\n"
            b"cost and attack power.",
            
        b"menu_tsuranuki_naguri":
            b"Wear this to use Piercing\n"
            b"Blow, a 2-FP move which\n"
            b"deals damage that pierces\n"
            b"enemy defenses.\n"
            b"Wearing more copies of the\n"
            b"badge increases its FP\n"
            b"cost and attack power.",
            
        b"menu_fire_naguri":
            b"Wear this to use Fire Drive,\n"
            b"a 3-FP move which deals \n"
            b"fire damage and Burn status\n"
            b"to all grounded enemies.\n"
            b"Wearing more copies of the\n"
            b"badge increases its FP\n"
            b"cost and attack power.",
            
        b"menu_ice_naguri":
            b"Wear this to use Ice Smash,\n"
            b"a 2-FP move which deals\n"
            b"ice damage and may give its\n"
            b"target the Freeze status.\n"
            b"Wearing more copies of the\n"
            b"badge increases its FP\n"
            b"cost and status duration.",
            
        b"menu_charge":
            b"Wear this to add Charge\n"
            b"to Mario's Tactics menu.\n"
            b"This uses 2 FP to increase\n"
            b"the next move's ATK by 2.\n"
            b"Wearing more copies raises\n"
            b"the effect and FP cost.",
            
        b"menu_charge_p":
            b"Wear this to add Charge\n"
            b"to partners' Tactics menu.\n"
            b"This uses 2 FP to increase\n"
            b"the next move's ATK by 2.\n"
            b"Wearing more copies raises\n"
            b"the effect and FP cost.",
            
        b"msg_treasure_map":
            b"Allows Mario to use the move\n"
            b"Sweet Treat.",
            
        b"msg_diamond_star":
            b"Allows Mario to use the move\n"
            b"Earth Tremor.",
            
        b"msg_emerald_star":
            b"Allows Mario to use the move\n"
            b"Clock Out, and raises max SP.",
            
        b"msg_gold_star":
            b"Allows Mario to use the move\n"
            b"Power Lift, and raises max SP.",
            
        b"msg_ruby_star":
            b"Allows Mario to use the move\n"
            b"Art Attack, and raises max SP.",
            
        b"msg_sapphire_star":
            b"Allows Mario to use the move\n"
            b"Sweet Feast, and raises max SP.",
            
        b"msg_garnet_star":
            b"Allows Mario to use the move\n"
            b"Showstopper, and raises max SP.",
            
        b"msg_crystal_star":
            b"Allows Mario to use the move\n"
            b"Supernova, and raises max SP.",
        
        # Tower reward-related information.
        
        b"tot_reward_hpplus":       b"HP Up",
        b"tot_rewarddesc_hpplus":
            b"Levels up Mario's HP.",
            
        b"tot_reward_fpplus":       b"FP Up",
        b"tot_rewarddesc_fpplus":
            b"Levels up Mario's FP.",
            
        b"tot_reward_bpplus":       b"BP Up",
        b"tot_rewarddesc_bpplus":
            b"Levels up Mario's BP.",
        
        b"tot_reward_hpplusp":       b"HP Up P",
        b"tot_rewarddesc_hpplusp":
            b"Levels up partners' HP.",
        
        b"tot_reward_sack":         b"Strange Sack",
        b"tot_rewarddesc_sack":
            b"Increases the maximum number\n"
            b"of items you can carry.",
            
        b"tot_reward_getparty":
            b"<system>\n<p>\n%s joined your party!\n<k>",
            
        b"tot_reward_learnmove":
            b"<system>\n<p>\nYou learned %s!\n<k>",
            
        b"tot_reward_upgrademove":
            b"<system>\n<p>\n%s's level\nwent up by 1!\n<k>",
            
        # Moves and status effects.
        
        b"in_konran_hammer":        b"Shrink Smash",
        b"tot_ptr1_ironbonk":       b"Ironbonk",
        b"tot_ptr1_scope_out":      b"Scope Out",
        b"tot_ptr2_withdraw":       b"Withdraw",
        b"tot_ptr2_bulkup":         b"Bulk Up",
        b"tot_ptr3_blizzard":       b"Blizzard",
        b"tot_ptr3_thunder":        b"Thunder Storm",
        b"tot_ptr4_egg":            b"Egg Barrage",
        b"tot_ptr4_spit":           b"Lickety Spit",
        b"tot_ptr4_gulp":           b"Gulp",
        b"tot_ptr5_curse":          b"Curse",
        b"tot_ptr5_neutralize":     b"Neutralize",
        b"tot_ptr6_poisonbomb":     b"Poison Bomb",
        b"tot_ptr6_megatonbomb":    b"Megaton Bomb",
        b"tot_ptr7_embargo":        b"Embargo",
        b"tot_ptr7_smokebomb":      b"Smoke Bomb",
            
        b"msg_super_genki":
            b"Restores Mario and his ally's\n"
            b"HP and FP over 5 turns.",
            
        b"tot_ptr1_scope_out_effect_msg":
            b"Scoped! Your next attack is\n"
            b"guaranteed to connect!",
            
        b"tot_ptr5_infatuate_effect_msg":
            b"Infatuated! The enemy will\n"
            b"fight for your side now!",
            
        b"msg_st_chg_allergy":
            b"Allergic! Status effects\n"
            b"cannot be changed!",
            
        b"msg_gatsun_jump":
            b"Attack an enemy with a single\n"
            b"powerful stomp, softening them.",
            
        b"msg_nemurase_fumi":
            b"Attack an enemy with a stomp\n"
            b"that can cause drowziness.",
            
        b"msg_tamatsuki_jump":
            b"Hit all enemies with a gust of\n"
            b"wind, dizzying aerial foes.",
            
        b"msg_kaiten_hammer":
            b"Strike an enemy, knocking it\n"
            b"into the enemies behind it.",
            
        b"msg_ultra_hammer":
            b"Strike an enemy, knocking it\n"
            b"into the enemies behind it.",
            
        b"msg_gatsun_naguri":
            b"Pound an enemy with a powerful,\n"
            b"defense-piercing strike.",
            
        b"msg_konran_hammer":
            b"Strike an enemy with a blow\n"
            b"that may shrink them.",
            
        b"msg_ice_naguri":
            b"Strike an enemy with a blow\n"
            b"that may freeze them solid.",
            
        b"msg_jishin_attack":
            b"Attack all grounded enemies,\n"
            b"piercing their defense.",
            
        b"msg_pkr_monosiri":
            b"View enemies' descriptions\n"
            b"and see their stats in battle.",
            
        b"tot_ptr1_ironbonk_desc":
            b"Attack with an invulnerable,\n"
            b"defense-piercing Headbonk.",
            
        b"tot_ptr1_scope_out_desc":
            b"Scope out an enemy, ensuring\n"
            b"your next attack lands.",
            
        b"msg_pnk_normal_attack":
            b"Attack the front enemy with\n"
            b"a spinning shell drive.",
            
        b"msg_pnk_syubibin_koura":
            b"Attack all grounded enemies\n"
            b"with a spinning shell.",
            
        b"tot_ptr2_withdraw_desc":
            b"Make Koops retreat into his\n"
            b"shell, impervious to attack.",
            
        b"tot_ptr2_bulkup_desc":
            b"Raise an ally's Attack and\n"
            b"Defense power for a while.",
            
        b"msg_pwd_kumogakure":
            b"Envelop the party in a fog,\n"
            b"causing foes to sometimes miss.",
            
        b"tot_ptr3_blizzard_desc":
            b"Attack all enemies with a\n"
            b"blast of icy wind.",
            
        b"tot_ptr3_thunder_desc":
            b"Strike all enemies with\n"
            b"powerful bolts of lightning!",
            
        b"tot_ptr4_egg_desc":
            b"Throw a volley of eggs at\n"
            b"foes, dealing light damage.",
            
        b"tot_ptr4_spit_desc":
            b"Spit the front enemy into all\n"
            b"ground-bound enemies behind it.",
            
        b"tot_ptr4_gulp_desc":
            b"Swallow an enemy in one gulp,\n"
            b"stealing their held item.",
            
        b"tot_ptr5_curse_desc":
            b"Cast a hex on the enemy to\n"
            b"slow their movements.",
            
        b"tot_ptr5_neutralize_desc":
            b"Make the enemy unable to\n"
            b"change their status.",
            
        b"msg_ptr_meromero_kiss":
            b"Blow a kiss to an enemy to try\n"
            b"to win them to your side.",
            
        b"msg_pbm_jigen_bakudan":
            b"Throw a volley of time bombs\n"
            b"that explode on the next turn.",
            
        b"tot_ptr6_poisonbomb_desc":
            b"Throw a volley of bombs that\n"
            b"explode in a noxious blast.",
            
        b"tot_ptr6_megatonbomb_desc":
            b"Arm a bomb that does massive\n"
            b"damage to all combatants.",
            
        b"msg_pch_madowaseru":
            b"Tease enemies with some sly\n"
            b"moves, confusing them.",
            
        b"tot_ptr7_embargo_desc":
            b"Distract foes and scatter\n"
            b"their held items and badges.",
            
        b"tot_ptr7_smokebomb_desc":
            b"Lightly damage and dizzy foes\n"
            b"with a burst of smoke.",
            
        b"msg_pch_kiss":
            b"Replenish the party's HP\n"
            b"with a friendly kiss.",
            
        b"tot_ptr3_thunder_ac":
            b"Press <icon PAD_A 0.6 1 2 6>"
            b"<icon PAD_B 0.6 1 2 6>"
            b"<icon PAD_Y 0.6 1 2 6>"
            b"<icon PAD_X 0.6 1 2 6> repeatedly\n"
            b"in quick succession!",
            
        # Shortened move names
        
        b"tot_mj_spring_abb":   b"Spring J.",
        b"tot_mj_pjump_abb":    b"Power J.",
        b"tot_mj_multi_abb":    b"Multib.",
        b"tot_mj_pbounce_abb":  b"Power B.",
        b"tot_mj_sleep_abb":    b"Sleep S.",
        b"tot_mj_tjump_abb":    b"Tor. J.",
        
        b"tot_mh_super_abb":    b"Super H.",
        b"tot_mh_ultra_abb":    b"Ultra H.",
        b"tot_mh_power_abb":    b"Power S.",
        b"tot_mh_shrink_abb":   b"Shrink S.",
        b"tot_mh_quake_abb":    b"Quake H.",
        
        b"tot_ms_1_abb":        b"Sweet Tr.",
        b"tot_ms_2_abb":        b"Earth Tr.",
        b"tot_ms_6_abb":        b"Sweet F.",
        b"tot_ms_7_abb":        b"Showst.",
        
        b"tot_ptr4_norm_abb":   b"Ground P.",
        b"tot_ptr4_spit_abb":   b"Lickety Sp.",
        b"tot_ptr4_egg_abb":    b"Egg Barr.",
        b"tot_ptr6_bsquad_abb": b"Bomb Sq.",
        b"tot_ptr6_poison_abb": b"Poison B.",
        b"tot_ptr7_smokeb_abb": b"Smoke B.",
        
        # Upgrade descriptions
        
        b"tot_upg_none":
            b"This move can't be upgraded.",
        
        b"tot_upg_damage":
            b"Upgrading this move will\n"
            b"increase its damage output.",
            
        b"tot_upg_status":
            b"Upgrading this move will\n"
            b"increase its status duration.",
            
        b"tot_upg_statuseffect":
            b"Upgrading this move will\n"
            b"increase its status power.",
            
        b"tot_upg_statuschance":
            b"Upgrading this move will\n"
            b"increase its status chance.",
        
        b"tot_upg_blizzard":
            b"Upgrading this move ups its\n"
            b"damage and freeze duration.",
            
        b"tot_upg_hits":
            b"Upgrading this move will\n"
            b"increase the number of hits.",
            
        b"tot_upg_shellsh":
            b"Upgrading this move will\n"
            b"increase the shield's max HP.",
            
        b"tot_upg_multi":
            b"Upgrading this move will\n"
            b"allow you to target all foes.",
            
        b"tot_upg_bombs":
            b"Upgrading this move will\n"
            b"increase the number of bombs.",
            
        b"tot_upg_smooch":
            b"Upgrading this move will\n"
            b"increase the max HP healed.",
            
        b"tot_upg_sac1":
            b"Upgrading this move will\n"
            b"increase targets' value.",
            
        # Menu text
        
        b"tot_winsel_titlemove":      b"Move",
        b"tot_winsel_whichunlock":    b"Which move?",
        
        # Enemy names + Tattle info (TODO)
        
        b"btl_un_monban":             b"Craw",
            
        # Achievements (TODO)
        
        b"in_ach_1":    b"Chest Collector",
        b"in_ach_2":    b"Badge Buff",
        b"in_ach_3":    b"Tattletale",
        
        b"msg_ach_1":
            b"Commemorates those that have\n"
            b"collected all unique rewards.",
            
        b"msg_ach_2":
            b"Commemorates those that have\n"
            b"collected one of every badge.",
            
        b"msg_ach_3":
            b"Commemorates those that have\n"
            b"Tattled every kind of enemy.",
        
    },
    
    # Tower lobby placeholder
    'gon_00': {
        # Area tattle. (TODO: Write final script)
        
        b"msg_kuri_map":
            b"Placeholder area tattle \xd0\n<k>",
    },
    
    # Tower floor 1; TODO: Copy to other tower maps as needed.
    'gon_01': {
        # Area tattle. (TODO: Write final script)
        
        b"msg_kuri_map":
            b"Placeholder area tattle \xd0\n<k>",
        
        # NPC tattles.

        b"\x8d\x73\x8f\xa4\x90\x6c":
            b"<keyxon>\nThat's Charlieton, the\n"
            b"merchant.<dkey><wait 300></dkey> I guess he sells his\n"
            b"wares here, too.\n<k>\n<p>\n"
            b"But...<wait 100>it looks like his stuff\n"
            b"gets more expensive the\n"
            b"deeper he is in the Pit.\n<k>\n<p>\n"
            b"I'm happy to be able to shop\n"
            b"here and all, but sheesh...\n"
            b"<wait 300>\n"
            b"Well, it's your money.\n<k>",
            
        b"\x83\x70\x83\x8f\x81\x5b\x83\x5f\x83\x45\x83\x93\x89\xae":
            b"<keyxon>\nThat's Chet Rippo, the\n"
            b"adjuster.<dkey><wait 300></dkey> Seems he's found a\n"
            b"new niche here in the Pit.\n<k>\n<p>\n"
            b"He'll take your unwanted\n"
            b"items, badges, and level-ups\n"
            b"in exchange for a few coins.\n<k>\n<p>\n"
            b"Me, <wait 200>I wouldn't trust this guy\n"
            b"personally, Mario, but I guess\n"
            b"if you're desperate...\n<k>",
        
        # NPC text.
        
        # Boss fights.
        
        b"tot_dragon_debug":
            b"Placeholder text %d-%d<k>",
        
        # Generic
        
        b"tot_npc_yesnoopt":
            b"<select 0 1 0 40>\nYes\nNo",
        
        # Charlieton
        
        b"tot_charlieton_intro":
            b"Hey hey HEY-YO!<wait 250> I've got ALL\n"
            b"the hottest new products\n"
            b"right here.<wait 250> Look no further!\n<k>\n<p>\n"
            b"You got the fevered look of\n"
            b"a man with a mind for goods!\n"
            b"<wait 250>\n"
            b"Wanna buy something?\n<o>",

        b"tot_charlieton_decline":
            b"<p>\n"
            b"Oh, not interested, my man?\n"
            b"<wait 250>\n"
            b"That's too bad, seriously...\n"
            b"<wait 250>\n"
            b"It's all hot...<wait 250>I mean rare...<wait 250>stuff!\n"
            b"<k>\n<p>\n"
            b"And you never know when\n"
            b"you're gonna wish you had\n"
            b"some of this gear, my man...\n<k>",

        b"tot_charlieton_nocoins":
            b"Whoa whoa whoa WHOA!\n"
            b"<wait 250>\n"
            b"You don't have enough coins,\n"
            b"my man.\n<k>",

        b"tot_charlieton_itemdesc":
            b"That's my man, right there!\n"
            b"<wait 300>\n"
            b"You've got an eye for quality,\n"
            b"and that's no lie!\n<k>\n<p>\n"
            b"That <ITEM> is going\n"
            b"for a low price of <NUM> coin<S>!\n"
            b"<wait 250>\n"
            b"Wanna buy it?\n<o>",

        b"tot_charlieton_success":
            b"<p>\n"
            b"Thank you SO very much!\n"
            b"<wait 250>\n"
            b"You got yourself a good deal,\n"
            b"there, I kid you not!\n<k>\n<p>\n"
            b"You are SET for the future\n"
            b"with that item, my man!\n"
            b"<wait 250>\n"
            b"Come back real soon!\n<k>",

        b"tot_charlieton_buyanother":
            b"<p>\n"
            b"Thank you SO very much!\n"
            b"<wait 250>\n"
            b"And I've still got oodles\n"
            b"of super-hot items for you!\n<k>\n<p>\n"
            b"Wanna see?\n<o>",
            
        b"tot_charlieton_full_inv":
            b"<p>\n"
            b"Ooh, you can't carry any more \n"
            b"stuff, my man. You sure you\n"
            b"want to buy this anyway?\n<o>",
            
        # Miscellaneous.
            
        b"pit_move_level":
            b"<system>\n<p>\n"
            b"You obtained a Lv. 2 Star Power!\n"
            b"Tap left or right to select which\n"
            b"power level you want to use.\n<k>",
        
        b"pit_disabled_return":
            b"<system>\n<p>\nYou can't leave the Infinite Pit!\n<k>",
        
        b"rippo_confirm_bp":
            b"<p>\nI'll give you 39 coins for 3 BP.\n"
            b"<wait 350>You won't be able to get it back,\n"
            b"so are you absolutely sure?\n<o>",
            
        b"rippo_confirm_fp":
            b"<p>\nI'll give you 39 coins for 5 FP.\n"
            b"<wait 350>You won't be able to get it back,\n"
            b"so are you absolutely sure?\n<o>",
            
        b"rippo_confirm_hp":
            b"<p>\nI'll give you 39 coins for 5 HP.\n"
            b"<wait 350>You won't be able to get it back,\n"
            b"so are you absolutely sure?\n<o>",
            
        b"rippo_exit":
            b"<p>\nNow get outta here!\n<k>",
            
        b"rippo_intro":
            b"<shake>\n"
            b"Hee hee hee!</shake><wait 300> I can help\n"
            b"lighten your load if you've\n"
            b"got junk weighing you down!\n<k>\n<p>\n"
            b"What're you selling?\n<o>",
            
        b"rippo_item_different":
            b"<p>\nHmph. Anything else tickle \n"
            b"your fancy, then?\n<k>",
            
        b"rippo_stat_different":
            b"<p>\nHmph. Anything else tickle \n"
            b"your fancy, then?\n<o>",
            
        b"rippo_item_ok":
            b"I'm afraid I can only spare\n"
            b"a mere <NUM> coin<S> for your\n"
            b"<ITEM>. OK?\n<o>",
            
        b"rippo_item_thanks_last":
            b"<p>\nIf you don't have anything\n"
            b"else for me, then get\n"
            b"outta here!\n<k>",
            
        b"rippo_item_thanks_next":
            b"<p>\nYeah, yeah. <wait 500>You got any\n"
            b"other goods?\n<o>",
            
        b"rippo_no_badges":
            b"<p>\nI don't see any of those\n"
            b"in your stash. <wait 300>Quit wastin'\n"
            b"both our time!\n<k>",
            
        b"rippo_no_items":
            b"<p>\nI don't see any of those\n"
            b"in your stash. <wait 300>Quit wastin'\n"
            b"both our time!\n<k>",
            
        b"rippo_no_free_bp":
            b"<p>\nYou've got too many badges\n"
            b"equipped! <wait 300>Make some space\n"
            b"first, <wait 200>sheesh!\n<k>",
            
        b"rippo_no_stats":
            b"<p>\n<dynamic 3>Ha,</dynamic> <wait 200>are you kidding?\n"
            b"<wait 300>Go level up some, <wait 150>then we\n"
            b"can talk business!\n<k>",
            
        b"rippo_stat_menu":
            b"<select 0 3 0 40>\nHP\nFP\nBP",
            
        b"rippo_stat_thanks_last":
            b"<p>\nI'm finished. <wait 500>So... \n"
            b"<wait 300>Unless you need anything\n"
            b"else, get outta here!\n<k>",
            
        b"rippo_stat_thanks_next":
            b"<p>\nDone. <wait 500>So... \n"
            b"<wait 300>Got any other stat points\n"
            b"you aren't usin'?\n<o>",
            
        b"rippo_stat_too_low":
            b"<p>\nYou're looking too low on\n"
            b"that already! <wait 250>Perhaps\n"
            b"something else?\n<o>",
            
        b"rippo_which_item":
            b"<p>\nWhaddya want to sell?\n<k>",
            
        b"rippo_which_stat":
            b"<p>\nWhaddya want to sell?\n<o>",
            
        b"rippo_yes_no":
            b"<select 0 1 0 40>\nYes\nNo",
    }
}

class UpdateTextFilesError(Exception):
    def __init__(self, message=""):
        self.message = message

def _UpdateStrings(in_dir, out_dir):
    for file in g_StringMap:
        # Read the original strings for the given file, if it exists.
        strings = {}
        if file == 'global' and os.path.exists(in_dir / f'{file}.txt'):
            with open(str(in_dir / f'{file}.txt'), 'rb') as infile:
                # Keys and values are all delimited by nulls.
                split_kvs = infile.read().split(b'\x00')
                x = 0
                # There should be an empty string key at the end of the file.
                while len(split_kvs[x]):
                    strings[split_kvs[x]] = split_kvs[x+1]
                    x += 2
        
        # Add / update strings from map.
        for (k, v) in g_StringMap[file].items():
            strings[k] = v
        
        # Write strings out to file.
        if strings:
            with open(str(out_dir / f'{file}.txt'), 'wb') as outfile:
                # Write key+value separated by nulls.
                for (k,v) in strings.items():
                    outfile.write(b'%s\x00%s\x00' % (k, v))
                # Write final null at end.
                outfile.write(b'\x00')

def main(argc, argv):
    in_dir = FLAGS.GetFlag("in_msg_dir")
    if not in_dir or not os.path.exists(Path(in_dir)):
        raise UpdateTextFilesError("--in_msg_dir must point to a valid directory.")
    in_dir = Path(in_dir)
    
    out_dir = FLAGS.GetFlag("out_msg_dir")
    if not out_dir or not os.path.exists(Path(out_dir)):
        raise UpdateTextFilesError("--out_msg_dir must point to a valid directory.")
    out_dir = Path(out_dir)
    
    _UpdateStrings(in_dir, out_dir)


if __name__ == "__main__":
    (argc, argv) = FLAGS.ParseFlags(sys.argv[1:])
    main(argc, argv)