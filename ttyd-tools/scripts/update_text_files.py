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
        
        b"msg_toughen_up":
            b"Wear this to add Toughen Up\n"
            b"to Mario's Tactics menu.",
            
        b"msg_toughen_up_p":
            b"Wear this to add Toughen Up\n"
            b"to partners' Tactics menu.",
            
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
    'gon_13': {
        # 'Pit'-specific text (TODO: move to specific maps or global).
        b"pit_reward_party_join":
            b"<system>\n<p>\nYou got a new party member!\n<k>",
            
        b"pit_move_level":
            b"<system>\n<p>\n"
            b"You obtained a Lv. 2 Star Power!\n"
            b"Tap left or right to select which\n"
            b"power level you want to use.\n<k>",
            
        b"pit_chest_unclaimed":
            b"<system>\n<p>\nYou haven't claimed your\nreward!\n<k>",
        
        b"pit_disabled_return":
            b"<system>\n<p>\nYou can't leave the Infinite Pit!\n<k>",
            
        b"pit_charlieton_full_inv":
            b"<p>\n"
            b"Ooh, you can't carry any more \n"
            b"stuff, my man. You sure you\n"
            b"want to buy this anyway?\n<o>",
        
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
            
        b"msg_kuri_map":
            b"<keyxon>\nWe're in the Infinite Pit,\n"
            b"an endless series of trials\n"
            b"and enemy encounters.\n<k>\n<p>\n"
            b"It may be never-ending, but that\n"
            b"doesn't mean we can't give it\n"
            b"our best effort, right, Mario?\n<k>",
            
        # Chet Rippo NPC tattle.
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
    }
}

class UpdateTextFilesError(Exception):
    def __init__(self, message=""):
        self.message = message

def _UpdateStrings(in_dir, out_dir):
    for file in g_StringMap:
        # Read the original strings for the given file, if it exists.
        strings = {}
        if os.path.exists(in_dir / f'{file}.txt'):
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