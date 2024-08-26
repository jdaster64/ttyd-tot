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
        # Generic placeholder string.
        b"tot_msg_dummy":       b"Placeholder",
        
        # Item & badge names.
        b"in_cake":                 b"Strawberry Cake",
        b"in_suitoru":              b"HP-Sucker",
        b"in_nancy_tea":            b"Maple Tea",
        b"in_first_love_pudding":   b"Sweet Pudding",
        b"in_peach_talt":           b"Sour Tart",
        b"in_kararina_pasta":       b"Hottest Dog",
        b"in_2bai_damage":          b"No Pain, No Gain",
        b"in_toughen_up":           b"Toughen Up",
        b"in_toughen_up_p":         b"Toughen Up P",
        b"in_super_start":          b"Super Start",
        b"in_perfect_power":        b"Perfect Power",
        b"in_perfect_power_p":      b"Perfect Power P",
        b"in_pity_star":            b"Pity Star",
        b"in_pity_star_p":          b"Pity Star P",
        b"tot_key_name":            b"Tower Key",
        b"tot_mkey_name":           b"Master Key",
        b"in_super_peekaboo":       b"Stat Master",
        b"in_bgm":                  b"Music Toggle",
        b"in_itemselect":           b"Item Selector",
        b"in_badgeselect":          b"Badge Selector",
        b"in_attack_fx":            b"Attack FX",
        b"in_m_emblem":             b"M Palette",
        b"in_y_emblem":             b"Y Palette",
        
        # Special item descriptions.
        
        b"msg_super_coin":
            b"A powerful object that ranks\n"
            b"up a move of your choice.",

        b'msg_star_piece':
            b'A capricious object that ranks\n'
            b'up your moves at random.',
            
        b"msg_star_piece_inhub":
            b"A glittering trinket that might\n"
            b"be of value to some merchants.",
            
        b"msg_custom_super_boots":
            b"A stronger pair of boots.",
            
        b"msg_custom_ultra_boots":
            b"An even stronger pair of boots.",
            
        b"msg_custom_super_hammer":
            b"A more powerful hammer.",
            
        b"msg_custom_ultra_hammer":
            b"An even more powerful hammer.",
            
        b"tot_key_desc":
            b"Opens a single locked door.\n"
            b"Works on non-midboss rooms.",
            
        b"tot_mkey_desc":
            b"Opens a single locked door.\n"
            b"Works on any room.",

        b'msg_HP_mieru':
            b"Shows enemies' HP bar.",

        b'msg_super_peekaboo':
            b"Shows enemies' current ATK and\n"
            b"DEF stats under their HP.",

        b'msg_timing_oshieru':
            b'Shows an indicator for when\n'
            b'to perform Stylish commands.',

        b'msg_bgm':
            b"Toggle the background music\n"
            b"on or off.",

        b'msg_itemselect':
            b"Choose a preferred loadout\n"
            b"of up to 6 items.",

        b'msg_badgeselect':
            b"Choose a preferred loadout\n"
            b"of up to 6 badges.",

        b'msg_attack_fx':
            b"Select which sound effects\n"
            b"play when Mario attacks.",

        b'msg_y_emblem':
            b"Select which Yoshi colors\n"
            b"might appear in the tower.",

        b'msg_m_emblem':
            b"Select what outfit Mario\n"
            b"will wear.",
            
        # Normal item descriptions.
        
        b"msg_kaminari_gorogoro":
            b"Strikes a single enemy with\n"
            b"lightning, dealing 5 damage.",
        
        b"msg_kaminari_dokkan":
            b"Strikes all enemies with\n"
            b"lightning, dealing 5 damage.",
        
        b"msg_kirakira_otoshi":
            b"Drops a shower of stars\n"
            b"on enemies for 6 damage.",
        
        b"msg_koori_no_ibuki":
            b"Deals 3 ice damage to all foes,\n"
            b"and might freeze them.",
        
        b"msg_fire_flower":
            b"Deals 3 fire damage to all foes,\n"
            b"and might burn them.",
        
        b"msg_yurayura_jishin":
            b"Hits all ground-bound foes for\n"
            b"6 damage, and can flip them.",

        b'list_yurayura_jishin':
            b"Hits all ground-bound foes for\n"
            b"6 damage, and can flip them.\n"
            b'Crafted from the raw material\n'
            b'found in a POW Block.',

        b'msg_pow_block':
            b'Hits all foes for 2 damage,\n'
            b'and can flip certain enemies.',

        b'msg_dekadeka_drink':
            b"Makes you Huge, temporarily\n"
            b'increasing power by 50%.',

        b'msg_kachikachi_koura':
            b'Boosts your Defense power\n'
            b'by 3 for a short while.',
            
        b"msg_teki_kyouka":
            b"Enemies drop extra coins, but\n"
            b"temporarily gain +3 ATK.",

        b'msg_minimini_kun':
            b'Briefly shrinks enemies,\n'
            b'reducing damage by 50%.',

        b'msg_ultra_kinoko':
            b'A feel-great mushroom.\n'
            b'Replenishes 30 HP.',

        b'msg_royal_jelly':
            b'A highly nutritious snack.\n'
            b'Restores 30 FP.',

        b'msg_hotdog':
            b"A tasty hot dog that restores\n"
            b'5 HP and 5 FP.',
        
        b"msg_cake":
            b"Scrumptious strawberry cake.\n"
            b"Heals 5 to 30 HP and FP.",

        b'msg_coconuts':
            b"Hard fruit that's probably\n"
            b"better tossed than eaten.",

        b'msg_wonder_egg':
            b'A mystical egg that restores\n'
            b'a third of your Star Power.',

        b'msg_red_kararin':
            b'A red-hot sauce that charges\n'
            b"your next attack's power by 3.",
            
        b"msg_shikaeshi_no_kona":
            b"Direct attackers take back\n"
            b"the same damage they deal.",
            
        b"msg_kame_no_noroi":
            b"Has a chance of inducing Slow \n"
            b"status on all foes.",

        b'list_kinoko_itame':
            b'A tasty mushroom dish that\n'
            b'replenishes 8 HP and 2 FP.\n'
            b'Made by cooking a Mushroom.',

        b'msg_kinoko_itame':
            b'A tasty mushroom dish that\n'
            b'replenishes 8 HP and 2 FP.',

        b'list_kinoko_foil_yaki':
            b'A hearty mushroom dish that\n'
            b'replenishes 15 HP and 5 FP.\n'
            b'Made by cooking a Super\n'
            b'Shroom or Volt Shroom.',

        b'msg_kinoko_foil_yaki':
            b'A hearty mushroom dish that\n'
            b'replenishes 15 HP and 5 FP.',

        b'list_kinoko_steak':
            b'A supreme mushroom dish that\n'
            b'replenishes 45 HP and 10 FP.\n'
            b'Made by cooking an Ultra\n'
            b'Shroom.',

        b'msg_kinoko_steak':
            b'A supreme mushroom dish that\n'
            b'replenishes 45 HP and 10 FP.',

        b'list_honey_kinoko':
            b'A sweet-topped mushroom that\n'
            b'replenishes 5 HP and 5 FP.\n'
            b'Made by cooking a Mushroom\n'
            b'or Honey Syrup.',

        b'msg_honey_kinoko':
            b'A sweet-topped mushroom that\n'
            b'replenishes 5 HP and 5 FP.',

        b'list_maple_kinokoS':
            b'A sweet-topped mushroom that\n'
            b'replenishes 10 HP and 10 FP.\n'
            b'Made by cooking a Super\n'
            b'Shroom or Maple Syrup.',

        b'msg_maple_kinokoS':
            b'A sweet-topped mushroom that\n'
            b'replenishes 10 HP and 10 FP.',

        b'list_royal_kinokoZ':
            b'A sweet-topped mushroom that\n'
            b'replenishes 30 HP and 30 FP.\n'
            b'Made by cooking an Ultra\n'
            b"Shroom or Jammin' Jelly.",

        b'msg_royal_kinokoZ':
            b'A sweet-topped mushroom that\n'
            b'replenishes 30 HP and 30 FP.',

        b'list_nancy_dinner':
            b'A nice meal made by Zess T.\n'
            b'Replenishes 15 HP and 15 FP.\n'
            b'Made by mixing two healing\n'
            b'items with small recovery.',

        b'msg_nancy_dinner':
            b'A nice meal made by Zess T.\n'
            b'Replenishes 15 HP and 15 FP.',

        b'list_nancy_special':
            b'A tasty meal made by Zess T.\n'
            b'Replenishes 25 HP and 25 FP.\n'
            b'Made by mixing two healing\n'
            b'items with moderate recovery.',

        b'msg_nancy_special':
            b'A tasty meal made by Zess T.\n'
            b'Replenishes 25 HP and 25 FP.',

        b'list_nancy_delux':
            b"A feast made by Zess T.\n"
            b'Replenishes 50 HP and 50 FP.\n'
            b'Made by mixing two healing\n'
            b'items with strong recovery.',

        b'msg_nancy_delux':
            b"A feast made by Zess T.\n"
            b'Replenishes 50 HP and 50 FP.',

        b'list_nancy_dynamite':
            b'Explosives made by Zess T.\n'
            b'Deals 10 damage to all foes.\n'
            b'Made by mixing two rare\n'
            b'attacking items.',

        b'msg_nancy_dynamite':
            b'Explosives made by Zess T.\n'
            b'Deals 10 damage to all foes.',

        b'list_nancy_tea':
            b'A maple-sweetened tea that\n'
            b'replenishes 20 FP.\n'
            b'Made by cooking Maple Syrup.',

        b'msg_nancy_tea':
            b'A maple-sweetened tea that\n'
            b'replenishes 20 FP.',

        b'list_space_food':
            b'Dry food that makes your\n'
            b'status unable to change.\n'
            b'Made by cooking a Dried\n'
            b'Shroom or Tasty Tonic.',

        b'msg_space_food':
            b'Dry food that makes your\n'
            b'status unable to change.',

        b'list_snow_rabbit':
            b'A captivating dessert that\n'
            b'heals 30 HP but freezes you.\n'
            b'Made by cooking an Ice Storm.',

        b'msg_snow_rabbit':
            b'A captivating dessert that\n'
            b'heals 30 HP but freezes you.',

        b'list_coconuts_bomb':
            b'An explosive Coconut that\n'
            b'deals 7 damage to one enemy.\n'
            b'Made by cooking a Coconut.',

        b'msg_coconuts_bomb':
            b'An explosive Coconut that\n'
            b'deals 7 damage to one enemy.',

        b'list_kachikachi_dish':
            b'Hard food that damages and\n'
            b'may soften an enemy.\n'
            b'Made by cooking a Courage\n'
            b'Shell.',

        b'msg_kachikachi_dish':
            b'Hard food that damages and\n'
            b'may soften an enemy.',

        b'list_kinoko_cake':
            b'A hearty mushroom cake.\n'
            b'Replenishes 30 HP and 10 FP.\n'
            b'Made by redecorating a\n'
            b'Strawberry Cake.',

        b'msg_kinoko_cake':
            b'A hearty mushroom cake.\n'
            b'Replenishes 30 HP and 10 FP.',

        b'list_mousse_cake':
            b'A moist, fluffy cake.\n'
            b'Replenishes 10 HP and 30 FP.\n'
            b'Made by redecorating a\n'
            b'Strawberry Cake.',

        b'msg_mousse_cake':
            b'A moist, fluffy cake.\n'
            b'Replenishes 10 HP and 30 FP.',

        b'list_fruit_parfait':
            b'Dessert made to last! Heals 5\n'
            b'HP and FP, and more over time.\n'
            b'Made by cooking an Ice Storm.',

        b'msg_fruit_parfait':
            b'Dessert made to last! Heals 5\n'
            b'HP and FP, and more over time.',

        b'list_bomb_egg':
            b'An explosive egg that deals\n'
            b'6 damage to one enemy.\n'
            b'Made by cooking a Mystic Egg.',

        b'msg_bomb_egg':
            b'An explosive egg that deals\n'
            b'6 damage to one enemy.',

        b'list_tea_kinoko':
            b'Mellow soup that heals 20 HP\n'
            b'and FP, but drops ATK and DEF.\n'
            b'Made by cooking a Mini Mr.\n'
            b'Mini or Mr. Softener.',

        b'msg_tea_kinoko':
            b'Mellow soup that heals 20 HP\n'
            b'and FP, but drops ATK and DEF.',

        b'list_poisoned_kinoko':
            b'A dubious mushroom. It might\n'
            b"poison you, or heal you to full!\n"
            b'Made by cooking an HP-Sucker\n'
            b'or a Life Shroom.',

        b'msg_poisoned_kinoko':
            b'A dubious mushroom. It might\n'
            b"poison you, or heal to full!",

        b'list_first_love_pudding':
            b'Amazing pudding brimming\n'
            b'with positive energy.\n'
            b'Made by cooking an item\n'
            b'that gives a helpful status.',

        b'msg_first_love_pudding':
            b'Amazing pudding brimming\n'
            b'with positive energy.',

        b'list_peach_talt':
            b'A tart-tasting pastry that\n'
            b'spreads negative energy.\n'
            b'Made by cooking an item\n'
            b'that gives a harmful status.',

        b'msg_peach_talt':
            b'A tart-tasting pastry that\n'
            b'spreads negative energy.',

        b'list_starry_dinner':
            b'An out-of-this-world dish that\n'
            b'slowly recovers HP, FP and SP!\n'
            b'Made by cooking a Shooting\n'
            b'Star or slow-healing item.',

        b'msg_starry_dinner':
            b'An out-of-this-world dish that\n'
            b'slowly recovers HP, FP and SP!',

        b'list_last_dinner':
            b'A pitch-black stew that\n'
            b'yields incredible results.\n'
            b"Made by mixing a dangerous\n"
            b'and a helpful item.',

        b'msg_last_dinner':
            b'A pitch-black stew that\n'
            b'yields incredible results.',

        b'list_kararina_pasta':
            b'A spicy coney that heals 7 HP\n'
            b'and 7 FP, and gives +3 Charge.\n'
            b'Made by cooking a Hot Dog\n'
            b'or Hot Sauce.',

        b'msg_kararina_pasta':
            b'A spicy coney that heals 7 HP\n'
            b'and 7 FP, and gives +3 Charge.',

        b'list_biribiri_candy':
            b'Candy that restores 15 FP\n'
            b'and applies Electric status.\n'
            b'Made by cooking an item\n'
            b'with ties to electricity.',

        b'msg_biribiri_candy':
            b'Candy that restores 15 FP\n'
            b'and applies Electric status.',
            
        b'list_fire_candy':
            b'A treat that heals 30 FP,\n'
            b'but has a burning aftertaste.\n'
            b'Made by cooking a Fire\n'
            b'Flower.',

        b'msg_fire_candy':
            b'A treat that heals 30 FP,\n'
            b'but has a burning aftertaste.',

        b'list_honey_candy':
            b'Sweet candy that\n'
            b'replenishes 15 FP.\n'
            b'Made by cooking a Honey\n'
            b'Syrup.',

        b'msg_honey_candy':
            b'Sweet candy that\n'
            b'replenishes 15 FP.',

        b'list_coconut_candy':
            b'Hard candy that heals\n'
            b'3 HP and 15 FP.\n'
            b'Made by cooking a Coconut.',

        b'msg_coconut_candy':
            b'Hard candy that heals\n'
            b'3 HP and 15 FP.',

        b'list_royal_candy':
            b'Fruit-flavored candy that\n'
            b'restores 50 FP.\n'
            b"Made by cooking a Jammin'\n"
            b"Jelly.",

        b'msg_royal_candy':
            b'Fruit-flavored candy that\n'
            b'restores 50 FP.',

        b'list_healthy_salad':
            b'Healthy food that heals 10\n'
            b'HP and FP, and cures ailments.\n'
            b'Made by cooking Tasty Tonic.',

        b'msg_healthy_salad':
            b'Healthy food that heals 10\n'
            b'HP and FP, and cures ailments.',
            
        # Badge descriptions.

        b'menu_power_plus':
            b"Boosts Mario's attack power\n"
            b'by 1.',

        b'msg_power_plus':
            b"Boosts Mario's attack power\n"
            b'by 1.',

        b'menu_power_plus_p':
            b"Boosts allies' attack power\n"
            b'by 1.',

        b'msg_power_plus_p':
            b"Boosts allies' attack power\n"
            b'by 1.',

        b'msg_jump_only':
            b"+1 Jump ATK, but can't use\n"
            b"Hammer abilities.",

        b'menu_jump_only':
            b"+1 Jump ATK, but can't use\n"
            b"Hammer abilities.",

        b'msg_hammer_only':
            b"+1 Hammer ATK and single hits\n"
            b"become throws, but can't Jump.",

        b'menu_hammer_only':
            b"+1 Hammer ATK and single hits\n"
            b"become throws, but can't Jump.",

        b'menu_nice_de_bougyo':
            b"Successful guards reduce\n"
            b"Mario's damage by 1 more.",

        b'msg_nice_de_bougyo':
            b"Successful guards reduce\n"
            b"Mario's damage by 1 more.",

        b'menu_nice_de_bougyo_p':
            b"Successful guards reduce\n"
            b"allies' damage by 1 more.",

        b'msg_nice_de_bougyo_p':
            b"Successful guards reduce\n"
            b"allies' damage by 1 more.",
            
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

        b'menu_itsumo_genki':
            b'Makes Mario immune to all\n'
            b'negative status effects.',

        b'msg_itsumo_genki':
            b'Makes Mario immune to all\n'
            b'negative status effects.',

        b'menu_itsumo_genki_p':
            b'Makes allies immune to all\n'
            b'negative status effects.',

        b'msg_itsumo_genki_p':
            b'Make allies immune to all\n'
            b'negative status effects.',

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
            
        b"msg_subayaku_kawaru":
            b"Swapping partners costs FP,\n"
            b"but doesn't consume a turn.",
            
        b"menu_subayaku_kawaru":
            b"Swapping partners costs FP,\n"
            b"but doesn't consume a turn.\n"
            b"Switching costs 1 FP initially,\n"
            b"and increases per use up to 5.",
        
        b"msg_toughen_up":
            b"Increases the defense given by\n"
            b"Mario's Defend command by 1.",
            
        b"msg_toughen_up_p":
            b"Increases the defense given by\n"
            b"allies' Defend command by 1.",
            
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
            b"Increases the defense given by\n"
            b"Mario's Defend command by 1.",
            
        b"msg_toughen_up_p_menu":
            b"Increases the defense given by\n"
            b"allies' Defend command by 1.",
            
        b"msg_pity_star":
            b"Increases Star Power gained\n"
            b"when enemies attack Mario.",
            
        b"msg_pity_star_p":
            b"Increases Star Power gained\n"
            b"when enemies attack allies.",
            
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
            b"<system>\n<p>\n%s's max level\nwent up by 1!\n<k>",
            
        b"tot_reward_upgrademove_inbattle":
            b"<col c00000ff>\n%s</col>\n's max level\nwent up by 1!",
        
        b"tot_reward_fullrecovery":
            b"<system>\n<p>\nYour party fully recovered!\n<k>",
            
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

        b'msg_genkigenki':
            b"Restores Mario and his ally's\n"
            b'HP and FP, and cures ailments.',
            
        b"msg_super_genki":
            b"Restores Mario and his ally's\n"
            b"HP and FP over 5 turns.",

        b'msg_bakugame':
            b'Immobilizes enemies and slows\n'
            b'bosses for a few turns.',
            
        b"tot_sp_regen_effect":
            b"Star Power will regenerate\n"
            b"over a few turns.",
            
        b"tot_ptr1_scope_out_effect_msg1":
            b"Scoped! The next attack is\n"
            b"guaranteed to land!",
            
        b"tot_ptr1_scope_out_effect_msg2":
            b"Scoped! The next attack and\n"
            b"non-KO status will succeed!",
            
        b"tot_ptr5_infatuate_effect_msg":
            b"Infatuated! The enemy will\n"
            b"fight for your side now!",

        b'msg_st_chg_fire':
            b'Burned! The fire will deal\n'
            b'steady damage and sap ATK!',

        b'msg_st_chg_poison':
            b'Poisoned! The toxins will\n'
            b'deal increasing damage!',
            
        b"msg_st_chg_allergy":
            b"Allergic! Status effects\n"
            b"cannot be changed!",

        b'msg_st_chg_freeze':
            b"Frozen! Breaking hits will\n"
            b'deal double damage!',

        b'msg_st_chg_big':
            b'Huge! Attack power is\n'
            b'now boosted by 50%!',

        b'msg_st_chg_small':
            b'Tiny! Attack power has\n'
            b'now dropped by 50%!',
            
        b"tot_status_withdraw":
            b"Invulnerable to all\n"
            b"attacks this turn!",

        b'tot_selerr_infatuate':
            b"Already Infatuated an enemy!",

        b'tot_selerr_megaton_bomb':
            b"Megaton Bomb already deployed!",
            
        b"msg_gatsun_jump":
            b"Attack an enemy with a single\n"
            b"powerful stomp, softening them.",
            
        b"msg_nemurase_fumi":
            b"Attack an enemy with a stomp\n"
            b"that can cause drowziness.",
            
        b"msg_tamatsuki_jump":
            b"Hit all enemies with a gust of\n"
            b"wind, dizzying aerial foes.",
            
        b"msg_jyabara_jump":
            b"Attack an enemy with a\n"
            b"three-hit Spring Jump combo.",
            
        b"msg_normal_hammer":
            b"Hit an enemy with the Hammer.",
            
        b"msg_kaiten_hammer":
            b"Strike an enemy, knocking it\n"
            b"into the enemies behind it.",
            
        b"msg_ultra_hammer":
            b"Hit an enemy twice, knocking it\n"
            b"into the enemies behind it.",
            
        b"msg_gatsun_naguri":
            b"Pound an enemy with a strong\n"
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
            b"Ensures the next attack will\n"
            b"successfully hit its target.",
            
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
            b"Makes the target immune to all\n"
            b"status effects for a while.",
            
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
        
        b"tot_toughen_up_abb":  b"Tough. Up",
        
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
            
        b"tot_upg_scope_out":
            b"Upgrading this move makes it\n"
            b"also guarantee most statuses.",
        
        b"tot_upg_veil":
            b"Upgrading this move makes it\n"
            b"grant Mario an extra turn.",
            
        b"tot_upg_shellsh":
            b"Upgrading this move will\n"
            b"increase the shield's max HP.",
            
        b"tot_upg_multi":
            b"Upgrading this move allows\n"
            b"it to hit multiple targets.",
            
        b"tot_upg_bombs":
            b"Upgrading this move will\n"
            b"increase the number of bombs.",
            
        b"tot_upg_tease":
            b"Upgrading this move makes\n"
            b"each action likelier to fail.",
            
        b"tot_upg_smooch":
            b"Upgrading this move will\n"
            b"increase the max HP healed.",
            
        b"tot_upg_sac1":
            b"Upgrading this move will\n"
            b"increase targets' value.",
            
        # Menu text
        
        b"tot_menu_spaceused":        b"Items",
        
        b"tot_winsel_titlemove":      b"Move",
        b"tot_winsel_whichunlock":    b"Which move?",
        b"tot_winsel_titlestat":      b"Stat Lvl.",
        b"tot_winsel_tradewhichstat": b"Trade which?",
        b"tot_winsel_whichduds":      b"Which getup?",
        b"tot_winsel_whichones":      b"Which ones?",
        
        # TODO: Move this and other Gold Fuzzy dialogue to the final boss area.
        
        b"tot_gfz_call_horde":
            b"Meee-OOOF!<wait 250> Hey!<wait 250> Ow!\n"
            b"<wait 250>\nYou chumps are tougher than\n"
            b"you look...\n<k>\n<p>\n"
            b"But I'm not through yet!\n<wait 250>\n"
            b"Everyone! GET 'EM!!!\n<k>",
            
        # Generic Map text.
        
        b"tot_lock_claimchest":
            b"<system>\n<p>\n"
            b"Claim a reward from one of the\n"
            b"chests to proceed!\n<k>",
        
        b"tot_lock_defeatenemies":
            b"<system>\n<p>\n"
            b"Defeat the enemy and claim\n"
            b"the rewards to proceed!\n<k>",
        
        b"tot_quitearly":
            b"<system>\n<p>\n"
            b"Do you really want to end this\n"
            b"run early and return to the\n"
            b"lobby?\n<o>",
        
        b"tot_gameover":
            b"<system>\n<p>\n"
            b"Do you want to continue from\n"
            b"after the last boss, or give up\n"
            b"and return to the lobby?\n<o>",
            
        b"tot_gameover_opt":
            b"<select 0 1 0 40>\nContinue\nGive Up",
            
        # Pause menu stuff.
        
        # Mario menu.

        b'msg_menu_mario_num_sp':
            b'Floors',

        b'msg_menu_star_power':
            b'The maximum Star Power Mario\n'
            b"has available for using Specials.",

        b'msg_menu_mario_coin':
            b'The number of coins Mario\n'
            b'has to buy items and badges.',

        b'msg_menu_mario_kakera':
            b'Star Pieces Mario has\n'
            b'collected.',

        b'msg_menu_mario_scoin':
            b'Shine Sprites Mario has\n'
            b'collected.',

        b'msg_menu_mario_star_p':
            b'How many floors Mario has\n'
            b'ascended on this run.',
            
        # Item loadout selection menu.
        
        b'tot_loadoutsel_add':
            b'Add Item',
        
        b'tot_loadoutsel_remove':
            b'Remove',
        
        b'tot_loadoutsel_back':
            b'Back',
        
        # Journal menu.
        
        b"msg_menu_kiroku_ryori":
            b'Inspect the items Mario has\n'
            b'collected here.',
        
        b"msg_menu_move_log":
            b'Information about the moves\n'
            b'Mario and party can perform.',
        
        b"msg_menu_records":
            b"Records of past runs' stats and\n"
            b'your overall completion progress.',
        
        b"msg_menu_achievements":
            b"Review your achievements and\n"
            b'the rewards earned from them.',
            
        b"tot_movelog_jump":        b"Jump Moves",
        b"tot_movelog_hammer":      b"Hammer Moves",
        b"tot_movelog_special":     b"Special Moves",
        b"tot_movelog_found":       b"Lvls.",
        b"tot_movelog_used":        b"Used",
        b"tot_movelog_stylish":     b"Styl.",
        b"tot_movelog_movecount":   b"Moves",
        
        # Level descriptions for moves in Moves log.
        
        # Placeholders.
        b"tot_movelog_d001": b"\nLv. 1: ???\n",
        b"tot_movelog_d002": b"\nLv. 2: ???\n",
        b"tot_movelog_d003": b"\nLv. 3: ???",
        
        # Jump moves.
        b"tot_movelog_d011":
            b"\nLv. 1: Deals 2+2 damage\n"
            b"for no cost.",
        b"tot_movelog_d012":
            b"\nLv. 2: Deals 3+3 damage\n"
            b"for 2 FP.",
        b"tot_movelog_d013":
            b"\nLv. 3: Deals 4+4 damage\n"
            b"for 4 FP.",
        b"tot_movelog_d021":
            b"\nLv. 1: Deals 2+4 damage\n"
            b"for 2 FP.",
        b"tot_movelog_d022":
            b"\nLv. 2: Deals 3+6 damage\n"
            b"for 4 FP.",
        b"tot_movelog_d023":
            b"\nLv. 3: Deals 4+8 damage\n"
            b"for 7 FP.",
        b"tot_movelog_d031":
            b"\nLv. 1: Deals 2+2+4 damage\n"
            b"for 4 FP.",
        b"tot_movelog_d032":
            b"\nLv. 2: Deals 3+3+6 damage\n"
            b"for 6 FP.",
        b"tot_movelog_d033":
            b"\nLv. 3: Deals 4+4+8 damage\n"
            b"for 9 FP.",
        b"tot_movelog_d041":
            b"\nLv. 1: Deals 6 damage and\n"
            b"3-turn -3 DEF for 2 FP.",
        b"tot_movelog_d042":
            b"\nLv. 2: Deals 9 damage and\n"
            b"3-turn -3 DEF for 4 FP.",
        b"tot_movelog_d043":
            b"\nLv. 3: Deals 12 damage and\n"
            b"3-turn -3 DEF for 6 FP.",
        b"tot_movelog_d051":
            b"\nLv. 1: Deals 2 damage to\n"
            b"each enemy for 3 FP.",
        b"tot_movelog_d052":
            b"\nLv. 2: Deals 3 damage to\n"
            b"each enemy for 4 FP.",
        b"tot_movelog_d053":
            b"\nLv. 3: Deals 4 damage to\n"
            b"each enemy for 6 FP.",
        b"tot_movelog_d061":
            b"\nLv. 1: Deals 1 damage per\n"
            b"hit, diminishing, for 3 FP.",
        b"tot_movelog_d062":
            b"\nLv. 2: Deals 2 damage per\n"
            b"hit, diminishing, for 5 FP.",
        b"tot_movelog_d063":
            b"\nLv. 3: Deals 3 damage per\n"
            b"hit, diminishing, for 7 FP.",
        b"tot_movelog_d071":
            b"\nLv. 1: Deals 2+2 damage and\n"
            b"3-turn Sleep for 2 FP.",
        b"tot_movelog_d072":
            b"\nLv. 1: Deals 2+2 damage and\n"
            b"5-turn Sleep for 4 FP.",
        b"tot_movelog_d073":
            b"\nLv. 1: Deals 2+2 damage and\n"
            b"7-turn Sleep for 6 FP.",
        b"tot_movelog_d081":
            b"\nLv. 1: Deals 4 damage, and\n"
            b"2 to all others for 3 FP.",
        b"tot_movelog_d082":
            b"\nLv. 2: Deals 6 damage, and\n"
            b"3 to all others for 5 FP.",
        b"tot_movelog_d083":
            b"\nLv. 3: Deals 8 damage, and\n"
            b"4 to all others for 7 FP.",
        # Hammer moves.
        b"tot_movelog_d111":
            b"\n\nLv. 1: Deals 4 damage\n"
            b"for no cost.",
        b"tot_movelog_d112":
            b"\nLv. 2: Deals 6 damage\n"
            b"for 2 FP.",
        b"tot_movelog_d113":
            b"\nLv. 3: Deals 8 damage\n"
            b"for 4 FP.",
        b"tot_movelog_d121":
            b"\nLv. 1: Deals 4 damage, and\n"
            b"2 to foes behind for 2 FP.",
        b"tot_movelog_d122":
            b"\nLv. 2: Deals 6 damage, and\n"
            b"3 to foes behind for 4 FP.",
        b"tot_movelog_d123":
            b"\nLv. 3: Deals 8 damage, and\n"
            b"4 to foes behind for 6 FP.",
        b"tot_movelog_d131":
            b"\nLv. 1: Deals 4+4 damage, and\n"
            b"2 to foes behind for 4 FP.",
        b"tot_movelog_d132":
            b"\nLv. 2: Deals 6+6 damage, and\n"
            b"3 to foes behind for 6 FP.",
        b"tot_movelog_d133":
            b"\nLv. 3: Deals 8+8 damage, and\n"
            b"4 to foes behind for 9 FP.",
        b"tot_movelog_d141":
            b"\nLv. 1: Deals 6 DEF-piercing\n"
            b"damage for 2 FP.",
        b"tot_movelog_d142":
            b"\nLv. 2: Deals 9 DEF-piercing\n"
            b"damage for 4 FP.",
        b"tot_movelog_d143":
            b"\nLv. 3: Deals 12 DEF-piercing\n"
            b"damage for 6 FP.",
        b"tot_movelog_d151":
            b"\nLv. 1: Deals 4 damage and\n"
            b"2-turn Shrink for 2 FP.",
        b"tot_movelog_d152":
            b"\nLv. 2: Deals 4 damage and\n"
            b"3-turn Shrink for 4 FP.",
        b"tot_movelog_d153":
            b"\nLv. 3: Deals 4 damage and\n"
            b"4-turn Shrink for 6 FP.",
        b"tot_movelog_d161":
            b"\nLv. 1: Deals 4 damage and\n"
            b"2-turn Freeze for 3 FP.",
        b"tot_movelog_d162":
            b"\nLv. 2: Deals 4 damage and\n"
            b"3-turn Freeze for 5 FP.",
        b"tot_movelog_d163":
            b"\nLv. 3: Deals 4 damage and\n"
            b"4-turn Freeze for 7 FP.",
        b"tot_movelog_d171":
            b"\nLv. 1: Deals 2 damage to\n"
            b"grounded enemies for 3 FP.",
        b"tot_movelog_d172":
            b"\nLv. 2: Deals 4 damage to\n"
            b"grounded enemies for 5 FP.",
        b"tot_movelog_d173":
            b"\nLv. 3: Deals 6 damage to\n"
            b"grounded enemies for 7 FP.",
        b"tot_movelog_d181":
            b"\nLv. 1: Hits for 3 (diminishing)\n"
            b"and 3-turn Burn for 3 FP.",
        b"tot_movelog_d182":
            b"\nLv. 2: Hits for 5 (diminishing)\n"
            b"and 3-turn Burn for 5 FP.",
        b"tot_movelog_d183":
            b"\nLv. 3: Hits for 7 (diminishing)\n"
            b"and 3-turn Burn for 7 FP.",
        # Special moves.
        b"tot_movelog_d211":
            b"\nLv. 1: Heal up to 7 HP apiece\n"
            b"and 8 FP, for 1 SP.",
        b"tot_movelog_d212":
            b"\nLv. 2: Heal up to 15 HP apiece\n"
            b"and 14 FP, for 2 SP.",
        b"tot_movelog_d213":
            b"\nLv. 3: Heal up to 24 HP apiece\n"
            b"and 21 FP, for 3 SP.",
        b"tot_movelog_d221":
            b"\nLv. 1: Deals 1 damage + 1 / bar,\n"
            b"up to a total of 4, for 1 SP.",
        b"tot_movelog_d222":
            b"\nLv. 2: Deals 2 damage + 1 / bar,\n"
            b"up to a total of 6, for 2 SP.",
        b"tot_movelog_d223":
            b"\nLv. 3: Deals 3 damage + 1 / bar,\n"
            b"up to a total of 8, for 3 SP.",
        b"tot_movelog_d231":
            b"\nLv. 1: Stops enemies and slows\n"
            b"bosses for 2 turns, for 2 SP.",
        b"tot_movelog_d232":
            b"\nLv. 2: Stops enemies and slows\n"
            b"bosses for 3 turns, for 3 SP.",
        b"tot_movelog_d233":
            b"\nLv. 3: Stops enemies and slows\n"
            b"bosses for 4 turns, for 4 SP.",
        b"tot_movelog_d241":
            b"\nLv. 1: Earn +1 ATK or DEF per\n"
            b"6 arrows shot, for 2 SP.",
        b"tot_movelog_d242":
            b"\nLv. 2: Earn +1 ATK or DEF per\n"
            b"5 arrows shot, for 3 SP.",
        b"tot_movelog_d243":
            b"\nLv. 3: Earn +1 ATK or DEF per\n"
            b"4 arrows shot, for 4 SP.",
        b"tot_movelog_d251":
            b"\nLv. 1: Deal up to 2 damage per\n"
            b"enemy encircled, for 3 SP.",
        b"tot_movelog_d252":
            b"\nLv. 2: Deal up to 3 damage per\n"
            b"enemy encircled, for 5 SP.",
        b"tot_movelog_d253":
            b"\nLv. 3: Deal up to 4 damage per\n"
            b"enemy encircled, for 6 SP.",
        b"tot_movelog_d261":
            b"\nLv. 1: Heal about 3-4 HP and\n"
            b"FP for 5 turns, for 2 SP.",
        b"tot_movelog_d262":
            b"\nLv. 2: Heal about 4-6 HP and\n"
            b"FP for 5 turns, for 3 SP.",
        b"tot_movelog_d263":
            b"\nLv. 3: Heal about 5-7 HP and\n"
            b"FP for 5 turns, for 5 SP.",
        b"tot_movelog_d271":
            b"\nLv. 1: Instantly defeat foes\n"
            b"with a 65% base rate for 2 SP.",
        b"tot_movelog_d272":
            b"\nLv. 2: Instantly defeat foes\n"
            b"with a 90% base rate for 4 SP.",
        b"tot_movelog_d273":
            b"\nLv. 3: Instantly defeat foes\n"
            b"with a 115% base rate for 5 SP.",
        b"tot_movelog_d281":
            b"\nLv. 1: Deal up to 10 damage\n"
            b"to all foes for 4 SP.",
        b"tot_movelog_d282":
            b"\nLv. 2: Deal up to 20 damage\n"
            b"to all foes for 6 SP.",
        b"tot_movelog_d283":
            b"\nLv. 3: Deal up to 30 damage\n"
            b"to all foes for 7 SP.",
        # Goombella moves.
        b"tot_movelog_d311":
            b"\nLv. 1: Deals 2+2 damage\n"
            b"for no cost.",
        b"tot_movelog_d312":
            b"\nLv. 2: Deals 3+3 damage\n"
            b"for 2 FP.",
        b"tot_movelog_d313":
            b"\nLv. 3: Deals 4+4 damage\n"
            b"for 4 FP.",
        b"tot_movelog_d331":
            b"\nLv. 1: Deals 2+2 DEF-piercing\n"
            b"damage for 2 FP.",
        b"tot_movelog_d332":
            b"\nLv. 2: Deals 3+3 DEF-piercing\n"
            b"damage for 4 FP.",
        b"tot_movelog_d333":
            b"\nLv. 3: Deals 4+4 DEF-piercing\n"
            b"damage for 6 FP.",
        b"tot_movelog_d341":
            b"\nLv. 1: Ensures the next attack\n"
            b"against the target will land.",
        b"tot_movelog_d342":
            b"\nLv. 2: Ensures the next attack\n"
            b"will land and inflict its status.",
        b"tot_movelog_d351":
            b"\nLv. 1: Deals 1 damage per\n"
            b"hit, diminishing, for 3 FP.",
        b"tot_movelog_d352":
            b"\nLv. 2: Deals 2 damage per\n"
            b"hit, diminishing, for 5 FP.",
        b"tot_movelog_d353":
            b"\nLv. 3: Deals 3 damage per\n"
            b"hit, diminishing, for 7 FP.",
        # Koops moves.
        b"tot_movelog_d411":
            b"\nLv. 1: Deals 3 damage\n"
            b"for no cost.",
        b"tot_movelog_d412":
            b"\nLv. 2: Deals 5 damage\n"
            b"for 2 FP.",
        b"tot_movelog_d413":
            b"\nLv. 3: Deals 8 damage\n"
            b"for 4 FP.",
        b"tot_movelog_d421":
            b"\nLv. 1: Deals 2 damage to\n"
            b"grounded foes for 3 FP.",
        b"tot_movelog_d422":
            b"\nLv. 2: Deals 3 damage to\n"
            b"grounded foes for 4 FP.",
        b"tot_movelog_d423":
            b"\nLv. 3: Deals 5 damage to\n"
            b"grounded foes for 6 FP.",
        b"tot_movelog_d441":
            b"\nLv. 1: Shields Mario for\n"
            b"up to two hits for 4 FP.",
        b"tot_movelog_d442":
            b"\nLv. 2: Shields Mario for\n"
            b"up to three hits for 6 FP.",
        b"tot_movelog_d443":
            b"\nLv. 3: Shields Mario for\n"
            b"up to four hits for 8 FP.",
        b"tot_movelog_d451":
            b"\nLv. 1: Gives +2 ATK and DEF\n"
            b"for up to four turns for 3 FP.",
        b"tot_movelog_d452":
            b"\nLv. 2: Gives +3 ATK and DEF\n"
            b"for up to four turns for 6 FP.",
        b"tot_movelog_d453":
            b"\nLv. 3: Gives +4 ATK and DEF\n"
            b"for up to four turns for 9 FP.",
        b"tot_movelog_d461":
            b"\nLv. 1: Deals 4 piercing damage\n"
            b"to grounded foes for 5 FP.",
        b"tot_movelog_d462":
            b"\nLv. 2: Deals 6 piercing damage\n"
            b"to grounded foes for 7 FP.",
        b"tot_movelog_d463":
            b"\nLv. 3: Deals 8 piercing damage\n"
            b"to grounded foes for 9 FP.",
        # Flurrie moves.
        b"tot_movelog_d511":
            b"\nLv. 1: Deals 3 damage\n"
            b"for no cost.",
        b"tot_movelog_d512":
            b"\nLv. 2: Deals 5 damage\n"
            b"for 2 FP.",
        b"tot_movelog_d513":
            b"\nLv. 3: Deals 8 damage\n"
            b"for 4 FP.",
        b"tot_movelog_d531":
            b"\nLv. 1: Drains 4 HP from an\n"
            b"enemy for 3 FP.",
        b"tot_movelog_d532":
            b"\nLv. 2: Drains 6 HP from an\n"
            b"enemy for 5 FP.",
        b"tot_movelog_d533":
            b"\nLv. 3: Drains 8 HP from an\n"
            b"enemy for 7 FP.",
        b"tot_movelog_d551":
            b"\nLv. 1: Deals 3 piercing damage\n"
            b"and 1-turn Freeze for 5 FP.",
        b"tot_movelog_d552":
            b"\nLv. 2: Deals 4 piercing damage\n"
            b"and 2-turn Freeze for 7 FP.",
        b"tot_movelog_d553":
            b"\nLv. 3: Deals 5 piercing damage\n"
            b"and 3-turn Freeze for 9 FP.",
        # Yoshi moves.
        b"tot_movelog_d611":
            b"\nLv. 1: Hits up to 4 times for\n"
            b"diminishing damage for no cost.",
        b"tot_movelog_d612":
            b"\nLv. 2: Hits up to 5 times for\n"
            b"diminishing damage for 2 FP.",
        b"tot_movelog_d613":
            b"\nLv. 3: Hits up to 6 times for\n"
            b"diminishing damage for 4 FP.",
        b"tot_movelog_d621":
            b"\nLv. 1: Deals 4 damage, and\n"
            b"2 to foes behind for 2 FP.",
        b"tot_movelog_d622":
            b"\nLv. 2: Deals 6 damage, and\n"
            b"3 to foes behind for 4 FP.",
        b"tot_movelog_d623":
            b"\nLv. 3: Deals 8 damage, and\n"
            b"4 to foes behind for 6 FP.",
        b"tot_movelog_d631":
            b"\nLv. 1: Toss up to 3 eggs\n"
            b"for 2 damage for 3 FP.",
        b"tot_movelog_d632":
            b"\nLv. 2: Toss up to 4 eggs\n"
            b"for 2 damage for 5 FP.",
        b"tot_movelog_d633":
            b"\nLv. 3: Toss up to 5 eggs\n"
            b"for 2 damage for 7 FP.",
        b"tot_movelog_d651":
            b"\nLv. 1: Toss up to 2 eggs\n"
            b"for 3-turn Shrink for 3 FP.",
        b"tot_movelog_d652":
            b"\nLv. 2: Toss up to 3 eggs\n"
            b"for 3-turn Shrink for 4 FP.",
        b"tot_movelog_d653":
            b"\nLv. 3: Toss up to 4 eggs\n"
            b"for 3-turn Shrink for 5 FP.",
        b"tot_movelog_d661":
            b"\nLv. 1: Hits up to 4 times for\n"
            b"diminishing damage for 4 FP.",
        b"tot_movelog_d662":
            b"\nLv. 2: Hits up to 5 times for\n"
            b"diminishing damage for 6 FP.",
        b"tot_movelog_d663":
            b"\nLv. 3: Hits up to 6 times for\n"
            b"diminishing damage for 8 FP.",
        # Vivian moves.
        b"tot_movelog_d711":
            b"\nLv. 1: Deals 3 damage and\n"
            b"2-turn Burn for no cost.",
        b"tot_movelog_d712":
            b"\nLv. 2: Deals 5 damage and\n"
            b"2-turn Burn for 2 FP.",
        b"tot_movelog_d713":
            b"\nLv. 3: Deals 7 damage and\n"
            b"2-turn Burn for 4 FP.",
        b"tot_movelog_d721":
            b"\nLv. 1: Inflicts 3-turn Slow\n"
            b"on a single enemy for 2 FP.",
        b"tot_movelog_d722":
            b"\nLv. 2: Inflicts 3-turn Slow\n"
            b"on all enemies for 5 FP.",
        b"tot_movelog_d731":
            b"\nLv. 1: Inflicts 3-turn Allergic\n"
            b"on any single target for 2 FP.",
        b"tot_movelog_d732":
            b"\nLv. 2: Inflicts 3-turn Allergic\n"
            b"on either side for 5 FP.",
        b"tot_movelog_d741":
            b"\nLv. 1: Hide for a turn, and skip\n"
            b"the next turn, for 1 FP.",
        b"tot_movelog_d742":
            b"\nLv. 2: Hide for a turn, and grant\n"
            b"Mario an extra move, for 6 FP.",
        b"tot_movelog_d751":
            b"\nLv. 1: Deals 3 piercing damage\n"
            b"and 3-turn Burn for 4 FP.",
        b"tot_movelog_d752":
            b"\nLv. 2: Deals 5 piercing damage\n"
            b"and 3-turn Burn for 6 FP.",
        b"tot_movelog_d753":
            b"\nLv. 3: Deals 7 piercing damage\n"
            b"and 3-turn Burn for 8 FP.",
        # Bobbery moves.
        b"tot_movelog_d811":
            b"\nLv. 1: Deals 3 damage\n"
            b"for no cost.",
        b"tot_movelog_d812":
            b"\nLv. 2: Deals 5 damage\n"
            b"for 2 FP.",
        b"tot_movelog_d813":
            b"\nLv. 3: Deals 8 damage\n"
            b"for 4 FP.",
        b"tot_movelog_d821":
            b"\nLv. 1: Throw two bombs for\n"
            b"3 piercing damage, for 2 FP.",
        b"tot_movelog_d822":
            b"\nLv. 2: Throw three bombs for\n"
            b"3 piercing damage, for 4 FP.",
        b"tot_movelog_d823":
            b"\nLv. 3: Throw four bombs for\n"
            b"3 piercing damage, for 6 FP.",
        b"tot_movelog_d841":
            b"\nLv. 1: Throw two bombs, which\n"
            b"inflict 5-Turn Poison, for 3 FP.",
        b"tot_movelog_d842":
            b"\nLv. 2: Throw three bombs, which\n"
            b"inflict 5-Turn Poison, for 5 FP.",
        b"tot_movelog_d843":
            b"\nLv. 3: Throw four bombs, which\n"
            b"inflict 5-Turn Poison, for 7 FP.",
        b"tot_movelog_d851":
            b"\nLv. 1: Deals 5 damage to all\n"
            b"enemies for 6 FP.",
        b"tot_movelog_d852":
            b"\nLv. 2: Deals 7 damage to all\n"
            b"enemies for 8 FP.",
        b"tot_movelog_d853":
            b"\nLv. 3: Deals 9 damage to all\n"
            b"enemies for 10 FP.",
        # Ms. Mowz moves.
        b"tot_movelog_d911":
            b"\nLv. 1: Deals 3 piercing hits for\n"
            b"diminishing damage for no cost.",
        b"tot_movelog_d912":
            b"\nLv. 2: Deals 4 piercing hits for\n"
            b"diminishing damage for 2 FP.",
        b"tot_movelog_d913":
            b"\nLv. 3: Deals 5 piercing hits for\n"
            b"diminishing damage for 4 FP.",
        b"tot_movelog_d931":
            b"\nLv. 1: 2-turn Confuse, with a\n"
            b"failed act rate of 50% for 2 FP.",
        b"tot_movelog_d932":
            b"\nLv. 2: 2-turn Confuse, with a\n"
            b"failed act rate of 70% for 4 FP.",
        b"tot_movelog_d933":
            b"\nLv. 3: 2-turn Confuse, with a\n"
            b"failed act rate of 90% for 7 FP.",
        b"tot_movelog_d951":
            b"\nLv. 1: Deals 3 piercing damage\n"
            b"and 50% 1-turn Dizzy for 4 FP.",
        b"tot_movelog_d952":
            b"\nLv. 2: Deals 4 piercing damage\n"
            b"and 50% 1-turn Dizzy for 5 FP.",
        b"tot_movelog_d953":
            b"\nLv. 3: Deals 5 piercing damage\n"
            b"and 50% 1-turn Dizzy for 6 FP.",
        b"tot_movelog_d961":
            b"\nLv. 1: Heal up to 5 HP for\n"
            b"both Mario and Mowz for 4 FP.",
        b"tot_movelog_d962":
            b"\nLv. 2: Heal up to 10 HP for\n"
            b"both Mario and Mowz for 7 FP.",
        b"tot_movelog_d963":
            b"\nLv. 3: Heal up to 15 HP for\n"
            b"both Mario and Mowz for 10 FP.",
            
            
        # Records log text.
        
        b"tot_recn_overall":            b"Completion",
        b"tot_recn_playtime":           b"Total Play Time",
        b"tot_recn_completion_pct":     b"Completion %",
        b"tot_recn_item_pct":           b"Items Log",
        b"tot_recn_badge_pct":          b"Badge Log",
        b"tot_recn_move_pct":           b"Moves Log",
        b"tot_recn_tattle_pct":         b"Tattle Log",
        b"tot_recn_achievement_pct":    b"Achievement Log",
        b"tot_recn_hub_pct":            b"Hub Progression",
        b"tot_recn_hub":                b"Hub Progression",
        b"tot_recn_hub_items":          b"Items Bought",
        b"tot_recn_hub_badges":         b"Badges Bought",
        b"tot_recn_hub_keyitems":       b"Key Items Bought",
        b"tot_recn_hub_options":        b"Options Bought",
        b"tot_recn_hub_marioskins":     b"???",
        b"tot_recn_hub_yoshiskins":     b"???",
        b"tot_recn_hub_attackfx":       b"???",
        b"tot_recn_runs":               b"Run Completions",
        b"tot_recn_half_wins":          b"Half Length",
        b"tot_recn_full_wins":          b"Full Length",
        b"tot_recn_ex_wins":            b"EX Difficulty",
        b"tot_recn_continues":          b"Total Continues",
        b"tot_recn_times":              b"Best Times (IGT)",
        b"tot_recn_half_time":          b"Half Length",
        b"tot_recn_full_time":          b"Full Length",
        b"tot_recn_ex_time":            b"EX Difficulty",
        b"tot_recn_runstats_1":         b"Run Stats (1/3)",
        b"tot_recn_floors":             b"Floors Cleared",
        b"tot_recn_turns":              b"Turns Spent",
        b"tot_recn_runaway":            b"Times Ran Away",
        b"tot_recn_kills":              b"Foes Defeated",
        b"tot_recn_edamage":            b"Enemy Damage",
        b"tot_recn_pdamage":            b"Player Damage",
        b"tot_recn_coinsearned":        b"Coins Earned",
        b"tot_recn_coinsspent":         b"Coins Spent",
        b"tot_recn_runstats_2":         b"Run Stats (2/3)",
        b"tot_recn_fpspent":            b"FP Spent",
        b"tot_recn_spspent":            b"SP Spent",
        b"tot_recn_superguards":        b"Superguards",
        b"tot_recn_conditions":         b"Conditions Met",
        b"tot_recn_starpieces":         b"Star Pieces",
        b"tot_recn_shinesprites":       b"Shine Sprites",
        b"tot_recn_itemsused":          b"Items Used",
        b"tot_recn_itemsbought":        b"Items Bought",
        b"tot_recn_runstats_3":         b"Run Stats (3/3)",
        b"tot_recn_wonky":              b"Wonky Deals",
        b"tot_recn_dazzle":             b"Dazzle Deals",
        b"tot_recn_rippo":              b"Chet Rippo Deals",
        b"tot_recn_lumpy":              b"Lumpy Deals",
        b"tot_recn_grubba":             b"Grubba Deals",
        b"tot_recn_doopliss":           b"Doopliss Deals",
        b"tot_recn_mover":              b"Mover Deals",
        b"tot_recn_zess":               b"Zess T. Deals",

        b"tot_rech_progression":
            b"Total playtime and Journal\n"
            b"completion progress.",
        b"tot_rech_hub_pct":
            b"Unlockables purchased from\n"
            b"various vendors in Petalburg.",
        b"tot_rech_wins":
            b"How many successful runs were\n"
            b"completed, and best clear times.",
        b"tot_rech_runstats":
            b"Various stats aggregated across\n"
            b"all of your past runs.",
            
            
        # Achievement descriptions.
        
        b"tot_achd_00":
            b"Perform all Stylish commands on\n"
            b"20 different moves.",
        b"tot_achd_01":
            b"Buy a total of 50 items from\n"
            b"Charlieton.",
        b"tot_achd_02":
            b"Deal with a total of 10 NPCs.",
        b"tot_achd_03":
            b"Find all partners.",
        b"tot_achd_04":
            b"Use 10 different Lvl. 3 moves.",
        b"tot_achd_05":
            b"Defeat 50 different types of\n"
            b"midbosses.",
        b"tot_achd_06":
            b"Superguard 100 attacks in total.",
        b"tot_achd_07":
            b"Earn a total of 10,000 coins.",
        b"tot_achd_08":
            b"Deal a total of 50,000 damage\n"
            b"to enemies.",
        b"tot_achd_09":
            b"Clear Hooktail's tower for the\n"
            b"first time.",
        b"tot_achd_10":
            b"Clear a Half-length tower in\n"
            b"1:00:00 with default settings.",
        b"tot_achd_11":
            b"Clear a Half-length tower in\n"
            b"0:40:00 with default settings.",
        b"tot_achd_12":
            b"Clear Gloomtail's tower for the\n"
            b"first time.",
        b"tot_achd_13":
            b"Clear a Full-length tower in\n"
            b"2:00:00 with default settings.",
        b"tot_achd_14":
            b"Clear a Full-length tower in\n"
            b"1:30:00 with default settings.",
        b"tot_achd_15":
            b"Finish an EX-difficulty tower run\n"
            b"with default settings.",
        b"tot_achd_16":
            b"Clear a EX-difficulty run in\n"
            b"3:00:00 with default settings.",
        b"tot_achd_17":
            b"Clear a EX-difficulty run in\n"
            b"2:15:00 with default settings.",
        b"tot_achd_18":
            b"Finish a run, making deals with\n"
            b"NPCs on 7 or more rest floors.",
        b"tot_achd_19":
            b"Finish a run with maximum\n"
            b"Action Command difficulty.",
        b"tot_achd_20":
            b"Finish a run with no partners.",
        b"tot_achd_21":
            b"Finish a run with all 7 partners\n"
            b"in your party.",
        b"tot_achd_22":
            b"Finish a default run without\n"
            b"Jumping or Hammering in battle.",
        b"tot_achd_23":
            b"Finish a default run without\n"
            b"ever using an item.",
        b"tot_achd_24":
            b"Finish a default run without\n"
            b"ever equipping a badge.",
        b"tot_achd_25":
            b"Finish a run with no moves able\n"
            b"to be upgraded further.",
        b"tot_achd_26":
            b"Finish a run without failing a\n"
            b"bonus condition.",
        b"tot_achd_27":
            b"Finish a default run, spending\n"
            b"3 or fewer turns on every floor.",
        b"tot_achd_28":
            b"Finish a default run without\n"
            b"taking any damage.",
        b"tot_achd_29":
            b"Finish a run with 2x or more\n"
            b"enemy HP and ATK.\n"
            b"All other settings must be\n"
            b"set to the default.",
        b"tot_achd_30":
            b"Finish a run with one of Mario's\n"
            b"stats at a maximum of 1.\n"
            b"Settings must be default, aside\n"
            b"from setting HP/FP/BP to 0.",
        b"tot_achd_31":
            b"Finish a run with two of Mario's\n"
            b"stats at a maximum of 1.\n"
            b"Settings must be default, aside\n"
            b"from setting HP/FP/BP to 0.",
        b"tot_achd_32":
            b"Beat a floor with a fainted\n"
            b"partner.",
        b"tot_achd_33":
            b"Finish a fight after a Bandit\n" 
            b"flees with an item.",
        b"tot_achd_34":
            b"Detonate a Bomb Squad bomb\n"
            b"early with Fire Drive.",
        b"tot_achd_35":
            b"Break an enemy's Frozen status\n"
            b"with a hit dealing 20+ damage.",
        b"tot_achd_36":
            b"Deal 50 damage to a single foe\n"
            b"using Poison status.",
        b"tot_achd_37":
            b"Defeat a Shrunk midboss with\n"
            b"Gale Force or Gulp.",
        b"tot_achd_38":
            b"Get at least a GREAT rating on\n"
            b"Flurrie's Thunder Storm.",
        b"tot_achd_39":
            b"Take a hit from a fully charged\n"
            b"Megaton Bomb.",
        b"tot_achd_40":
            b"Use a Trade Off item on turn 1\n"
            b"against the final boss.",
        b"tot_achd_41":
            b"Superguard one of the dragons'\n"
            b"bite attack.",
        b"tot_achd_42":
            b"Clear Charlieton's inventory on\n"
            b"a floor with Limited stock.",
        b"tot_achd_43":
            b"Buy a Star Piece from Dazzle\n"
            b"for 100 or more coins.",
        b"tot_achd_44":
            b"Exchange all your levels in all\n"
            b"stats with Chet Rippo.",
        b"tot_achd_45":
            b"Double your coins with Lumpy\n"
            b"twice or more in a single run.",
        b"tot_achd_46":
            b"Use a Tower Key to skip a floor\n"
            b"with chests already accessible.",
        b"tot_achd_47":
            b"Have Zess T. cook one of her\n"
            b"signature items.",
        b"tot_achd_48":
            b"Have Zess T. transform an item\n"
            b"with a Point Swap.",
        b"tot_achd_49":
            b"Get 10 Shine Sprites in a single\n"
            b"tower run.",
        b"tot_achd_50":
            b"Reach 999 coins on hand within\n"
            b"a tower run.",
        b"tot_achd_51":
            b"Purchase 5 of each type of\n"
            b"cosmetic upgrade.",
        b"tot_achd_52":
            b"Unlock all tower run options.",
        b"tot_achd_53":
            b"Obtain all Key Items.",
        b"tot_achd_54":
            b"Purchase 10 items and 10 badges\n"
            b"from the Petalburg shop.",
        b"tot_achd_55":
            b"Purchase all items and badges\n"
            b"from the Petalburg shop.",
        b"tot_achd_56":
            b"Log all normal and rare items\n"
            b"in the Items log.",
        b"tot_achd_57":
            b"100% complete the Items log.",
        b"tot_achd_58":
            b"100% complete the Badges log.",
        b"tot_achd_59":
            b"Log all regular enemies in the\n"
            b"Tattle log.",
        b"tot_achd_60":
            b"100% complete the Tattle log.",
        b"tot_achd_61":
            b"100% complete the Moves log.",
        b"tot_achd_62":
            b"???", 
            #  b"Beat the secret final boss.",
        b"tot_achd_63":
            b"Finish 10 tower runs across\n"
            b"all difficulty settings.",
        b"tot_achd_64":
            b"Finish 25 tower runs across\n"
            b"all difficulty settings.",
        b"tot_achd_65":
            b"Complete all other achievements.\n",
        b"tot_achd_66":
            b"???",
            # Placeholder;
            # b"Spend exactly 417 coins in a\n"
            # b"single tower run.",
        b"tot_achd_67":
            b"???", 
            # Placeholder;
            # "Deal 1,000 or more damage with\n"
            # b"Infatuated foes in a default run.",
        b"tot_achd_68":
            b"???", 
            # Placeholder;
            # b"Finish a run with all of Mario's\n"
            # b"stats at a maximum of 1.\n"
            # b"Settings must be default, aside\n"
            # b"from setting HP/FP/BP to 0.",
        b"tot_achd_69":
            b"???", 
            # Placeholder;
            # b"Take exactly 654 total damage\n"
            # b"in a single tower run.",
        
        # Achievement menu option names.
        
        b"tot_acho_charlieton":     b"Charlieton Stock",
        b"tot_acho_npcchoice":      b"NPC Choice",
        b"tot_acho_partner":        b"Partner Choice",
        b"tot_acho_acdiff":         b"AC Difficulty",
        b"tot_acho_superguardcost": b"Superguard Cost",
        b"tot_acho_audiencethrow":  b"Audience Throws",
        b"tot_acho_infinitebp":     b"Infinite BP",
        b"tot_acho_randomdamage":   b"Damage Variance",
        b"tot_acho_revive":         b"Partner Revival",
        b"tot_acho_bandit":         b"Bandit Escape",
        b"tot_acho_invincrease":    b"S. Sack Size",
        b"tot_acho_customloadout":  b"Custom Loadouts",
        b"tot_acho_obfuscated":     b"Obfuscated Items",
        b"tot_acho_secretboss":     b"???",  # Placeholder; b"Gold Fuzzy Boss",
        
        b"tot_ach_usehammer":
            b"Use a Hammer to unlock an\n"
            b"achievement's reward early.",
        b"tot_ach_unbreakable":
            b"This reward cannot be\n"
            b"unlocked early.",
        
        # Enemy names
        
        b"btl_un_monban":           b"Craw",
        b"btl_un_sinnosuke":        b"H. Bob-omb",
        b"btl_un_hyper_sinnosuke":  b"Cosmic Boo",
        
        # TODO: Shortened Tattles for all enemies - b"btl_hlp_monban", etc.
        
        b'btl_hlp_monban':
            b"That's a Craw.\n"
            b'<wait 250>\n'
            b'These guards are pretty stoic,\n'
            b'no-nonsense types.\n<k>\n<p>\n',
        
        b'btl_hlp_sinnosuke':
            b"That's a Hyper Bob-omb.\n"
            b'<wait 250>\n'
            b'Massive destructive power<wait 250>, now\n'
            b'in a trendy hot-pink package!\n<k>\n<p>\n',
        
        b'btl_hlp_hyper_sinnosuke':
            b"That's a Cosmic Boo.\n"
            b'<wait 250>\n'
            b"A giant purple Boo spoken of only\n"
            b"in legend, but I guess it's real!\n<k>\n<p>\n",
        
        b'btl_hlp_iron_sinemon':
            b"That's an Iron Cleft.\n"
            b'<wait 250>\n'
            b'These guys are so tough,<wait 50> that\n'
            b'any hit only leaves a scratch!\n<k>\n<p>\n',
        
        # TODO: Menu Tattles for all enemies. - b"menu_enemy_001", etc.
        
        # Cosmetic names and descriptions.
        
        b'tot_cos0_06_g':     b'FX Pack 01',
        b'tot_cos0_07_g':     b'FX Pack 02',
        b'tot_cos0_08_g':     b'FX Pack 03',
        b'tot_cos0_09_g':     b'FX Pack 04',
        b'tot_cos0_10_g':     b'FX Pack 05',
        b'tot_cos0_11_g':     b'FX Pack 06',
        b'tot_cos0_12_g':     b'FX Pack 07',
        b'tot_cos0_13_g':     b'FX Pack 08',
        b'tot_cos0_14_g':     b'FX Pack 09',
        b'tot_cos0_15_g':     b'FX Pack 10',
        
        b'tot_cos0_01':     b'Ding FX',
        b'tot_cos0_02':     b'Froggy FX',
        b'tot_cos0_03':     b'Squeaky FX',
        b'tot_cos0_04':     b'Peach FX',
        b'tot_cos0_05':     b'Bowser FX',
        b'tot_cos0_06':     b'???',
        b'tot_cos0_07':     b'???',
        b'tot_cos0_08':     b'???',
        b'tot_cos0_09':     b'???',
        b'tot_cos0_10':     b'???',
        b'tot_cos0_11':     b'???',
        b'tot_cos0_12':     b'???',
        b'tot_cos0_13':     b'???',
        b'tot_cos0_14':     b'???',
        b'tot_cos0_15':     b'???',
        b'tot_cos0_16':     b'???',
        b'tot_cos0_17':     b'???',
        b'tot_cos0_18':     b'???',
        b'tot_cos0_19':     b'???',
        b'tot_cos0_20':     b'???',
        b'tot_cos0_21':     b'???',
        b'tot_cos0_22':     b'???',
        b'tot_cos0_23':     b'???',
        b'tot_cos0_24':     b'???',
        b'tot_cos0_25':     b'???',
        
        b'tot_cos1_01':     b'Mario',
        b'tot_cos1_02':     b'Luigi',
        b'tot_cos1_03':     b'Wario',
        b'tot_cos1_04':     b'Waluigi',
        b'tot_cos1_05':     b'Fire Mario',
        b'tot_cos1_06':     b'Ice Mario',
        b'tot_cos1_07':     b'Bubble Mario',
        b'tot_cos1_08':     b'Superball Mario',
        b'tot_cos1_09':     b'Flying Mario',
        b'tot_cos1_10':     b'Mario Bros.',
        b'tot_cos1_11':     b'Classic Luigi',
        b'tot_cos1_12':     b'SMB1 Mario',
        b'tot_cos1_13':     b'SMBDX Luigi',
        b'tot_cos1_14':     b'SMB3 Mario',
        b'tot_cos1_15':     b'SMW Mario',
        b'tot_cos1_16':     b'Mario Golf 1',
        b'tot_cos1_17':     b'Mario Golf 2',
        b'tot_cos1_18':     b'Smash Bros. 1',
        b'tot_cos1_19':     b'Smash Bros. 2',
        b'tot_cos1_20':     b'Maker Mario',
        b'tot_cos1_21':     b'Toadette',
        b'tot_cos1_22':     b'#417',
        b'tot_cos1_23':     b'Shadowy Mario',
        b'tot_cos1_24':     b'Silver',
        b'tot_cos1_25':     b'Gold',
        b'tot_cos1_26':     b'Platinum',
        
        b'tot_cos2_01':     b'Green',
        b'tot_cos2_02':     b'Red',
        b'tot_cos2_03':     b'Blue',
        b'tot_cos2_04':     b'Orange',
        b'tot_cos2_05':     b'Pink',
        b'tot_cos2_06':     b'Black',
        b'tot_cos2_07':     b'White',
        b'tot_cos2_08':     b'Brown',
        b'tot_cos2_09':     b'Scarlet',
        b'tot_cos2_10':     b'Yellow',
        b'tot_cos2_11':     b'Lime',
        b'tot_cos2_12':     b'Teal',
        b'tot_cos2_13':     b'Indigo',
        b'tot_cos2_14':     b'Purple',
        b'tot_cos2_15':     b'Lilac',
        b'tot_cos2_16':     b'Fuchsia',
        b'tot_cos2_17':     b'Grey',
        b'tot_cos2_18':     b'#654',
        b'tot_cos2_19':     b'Silver',
        b'tot_cos2_20':     b'Gold',
        b'tot_cos2_21':     b'Platinum',
        
        b'tot_cos0_01_h':
            b"Gives Mario's attacks a tinny,\n"
            b"dinging sound.",
        b'tot_cos0_02_h':
            b"Gives Mario's attacks a\n"
            b"croaking sound.",
        b'tot_cos0_03_h':
            b"Gives Mario's attacks a\n"
            b"mouse-like squeaking sound.",
        b'tot_cos0_04_h':
            b"Gives Mario's attacks a\n"
            b"cheerful laughing sound.",
        b'tot_cos0_05_h':
            b"Gives Mario's attacks a\n"
            b"dreadful roaring sound.",
        b"tot_cos0_06_h":
            b"Placeholder",
        b"tot_cos0_07_h":
            b"Placeholder",
        b"tot_cos0_08_h":
            b"Placeholder",
        b"tot_cos0_09_h":
            b"Placeholder",
        b"tot_cos0_10_h":
            b"Placeholder",
        b"tot_cos0_11_h":
            b"Placeholder",
        b"tot_cos0_12_h":
            b"Placeholder",
        b"tot_cos0_13_h":
            b"Placeholder",
        b"tot_cos0_14_h":
            b"Placeholder",
        b"tot_cos0_15_h":
            b"Placeholder",
        b"tot_cos0_16_h":
            b"Placeholder",
        b"tot_cos0_17_h":
            b"Placeholder",
        b"tot_cos0_18_h":
            b"Placeholder",
        b"tot_cos0_19_h":
            b"Placeholder",
        b"tot_cos0_20_h":
            b"Placeholder",
        b"tot_cos0_21_h":
            b"Placeholder",
        b"tot_cos0_22_h":
            b"Placeholder",
        b"tot_cos0_23_h":
            b"Placeholder",
        b"tot_cos0_24_h":
            b"Placeholder",
        b"tot_cos0_25_h":
            b"Placeholder",
            
        b"tot_cos1_01_h":
            b"Red shirt, blue overalls.\n"
            b"Unquestionably iconic!",
        b"tot_cos1_02_h":
            b"Garb of exorcists, housekeepers,\n"
            b"and famous detectives' assisants.",
        b"tot_cos1_03_h":
            b"The real Wario's getup could\n"
            b"never be this affordable.",
        b"tot_cos1_04_h":
            b"The perfect fit for shrinking far,\n"
            b"far back into the shadows.",
        b"tot_cos1_05_h":
            b"Shiny white and red duds.\n"
            b"THE original power-up.",
        b"tot_cos1_06_h":
            b"Wear these to look like one\n"
            b"cool customer.",
        b"tot_cos1_07_h":
            b"The newest flower power in town.\n"
            b"Hot pink and red's a choice.",
        b"tot_cos1_08_h":
            b"Honestly, it's anyone's guess if\n"
            b"the color on this one's accurate.",
        b"tot_cos1_09_h":
            b"Way too cool-looking to have its\n"
            b"power-up only appear in one level.",
        b"tot_cos1_10_h":
            b"Going all the way back to\n"
            b"Brooklyn with this one!",
        b"tot_cos1_11_h":
            b"Luigi's original white-and-green\n"
            b"style, no Fire Flower required!",
        b"tot_cos1_12_h":
            b"Earthy tones and nary a bit of\n"
            b"blue in sight!",
        b"tot_cos1_13_h":
            b"Luigi cutting in on his bro's style,\n"
            b"and getting in on some VS. action!",
        b"tot_cos1_14_h":
            b"Darker overalls to account for\n"
            b"the halogen stagelights!",
        b"tot_cos1_15_h":
            b"16-bits means so many more off-reds"
            b"and blues to choose from!",
        b"tot_cos1_16_h":
            b"Taking a break from tennis\n"
            b"refereeing to hit the links!",
        b"tot_cos1_17_h":
            b"Casual blues, perfect for"
            b"putting greens!",
        b"tot_cos1_18_h":
            b"All the longtime Smash fans still\n"
            b'call this outfit "brown" out of habit.',
        b"tot_cos1_19_h":
            b"Mario's classic colors with a\n"
            b"cotton-candy twist!",
        b"tot_cos1_20_h":
            b"The perfect look for construction\n"
            b"work, preferably with a hard-hat.",
        b"tot_cos1_21_h":
            b"Fun fact: Toadette ditched her\n"
            b"orange dress right after this game!",
        b"tot_cos1_22_h":
            b"Inspired by world champions' and\n"
            b"mod creators' Pika-clone of choice.",
        b"tot_cos1_23_h":
            b"Remembering names was never\n"
            b"so easy!",
        b"tot_cos1_24_h":
            b"Nothing showier being a living\n"
            b"trophy, even if it's only 2nd!",
        b"tot_cos1_25_h":
            b"All that glitters is you! Wear\n"
            b"this golden sheen with pride!",
        b"tot_cos1_26_h":
            b"Wow, you're a super player!\n"
            b"You've really earned this!",
            
        b"tot_cos2_01_h":
            b"The classic green Yoshi.\n"
            b"You've seen them all over!",
        b"tot_cos2_02_h":
            b"This red Yoshi can't spit fire,\n"
            b"but he's still a hothead!",
        b"tot_cos2_03_h":
            b"Hop on this blue Yoshi to fly\n"
            b"(in terms of speed, at least).",
        b"tot_cos2_04_h":
            b"Unlike Bowser Jr.'s illusions,\n"
            b"this Yoshi isn't water-soluble.",
        b"tot_cos2_05_h":
            b"A cute pastel color that\n"
            b"belies this Yoshi's toughness!",
        b"tot_cos2_06_h":
            b"A rare variety of Yoshi; like you,\n"
            b"they're exceptionally speedy!",
        b"tot_cos2_07_h":
            b"A rare variety of Yoshi known\n"
            b"for their love of spicy stuff!",
        b"tot_cos2_08_h":
            b"A rugged rocky-brown look, right\n"
            b"out of the prehistoric era.",
        b"tot_cos2_09_h":
            b"A vermilion hue reminiscent of\n"
            b"vast pools of lava.",
        b"tot_cos2_10_h":
            b"A sunny look that gives you the\n"
            b"confidence to storm fortresses.",
        b"tot_cos2_11_h":
            b"A chartreuse hue that might be\n"
            b"found in forest underbrush.",
        b"tot_cos2_12_h":
            b"A bluish green, evoking faraway\n"
            b"islands full of tasty treats.",
        b"tot_cos2_13_h":
            b"A striking hue fit for a king, or\n"
            b"at least the king of dinosaurs.",
        b"tot_cos2_14_h":
            b"A royal color, perfect for\n"
            b"one rolling in ill-gotten riches.",
        b"tot_cos2_15_h":
            b"A soothing shade of lavender,\n"
            b"bringing to mind a soft rain.",
        b"tot_cos2_16_h":
            b"A shocking magenta that's\n"
            b"out of this world!",
        b"tot_cos2_17_h":
            b"A muted beige so samey it could\n"
            b"make your head spin.",
        b"tot_cos2_18_h":
            b"Inspired by a Pokemon starter\n"
            b"from an underappreciated region.",
        b"tot_cos2_19_h":
            b"A steed of literal silver's\n"
            b"nothing to sneeze at!",
        b"tot_cos2_20_h":
            b"The envy of Midas, and yet\n"
            b"you're literally untouchable!",
        b"tot_cos2_21_h":
            b"Thanks for playing this mod!\n"
            b"You've really earned this!",
            
        # Placeholder NPC dialogue (map-agnostic).
            
        b"tot_npc_generic":
            b"[Placeholder] I'm interactable.\n<k>",
    },
    
    # Tower lobby.
    'gon_00': {
        # Area tattle. (TODO: Write final script)
        
        b"msg_kuri_map":
            b"Placeholder area tattle \xd0\n<k>",
            
        # General lobby text.
        
        b"tot_lobby_confirmstart":
            b"<system>\n<p>\n"
            b"Are you prepared to start with\n"
            b"the options selected?\n<o>",
            
        b"tot_lobby_optyesno":
            b"<select 0 1 0 40>\nYes\nNo",
            
        # Run options menu.
            
        b"tot_winsel_runoptions_header":    b"Options",
        
        b"tot_optr_seed":               b"Seed",
        b"tot_optr_seed_random":        b"0 (Random)",
        b"tot_optr_preset":             b"Preset",
        b"tot_optr_preset_custom":      b"Custom",
        b"tot_optr_preset_default":     b"Default",
        b"tot_optr_difficulty":         b"Difficulty",
        b"tot_optr_diff_half":          b"Half (32 floors)",
        b"tot_optr_diff_full":          b"Full (64 floors)",
        b"tot_optr_diff_ex":            b"EX (64 floors)",
        b"tot_optr_timertype":          b"Timer Display",
        b"tot_optr_timer_none":         b"None",
        b"tot_optr_timer_igt":          b"In-Game Time",
        b"tot_optr_timer_rta":          b"Real-Time",
        b"tot_optr_startitems":         b"Starting Items",
        b"tot_optr_startitems_off":     b"None",
        b"tot_optr_startitems_basic":   b"Basic",
        b"tot_optr_startitems_strong":  b"Strong",
        b"tot_optr_startitems_random":  b"Random",
        b"tot_optr_startitems_custom":  b"Custom",
        b"tot_optr_drops":              b"Battle Drops",
        b"tot_optr_drops_def":          b"Default",
        b"tot_optr_drops_gated":        b"Condition-Gated",
        b"tot_optr_drops_noheld":       b"No Held, Bonus Only",
        b"tot_optr_drops_all":          b"All Items Drop",
        b"tot_optr_charlie":            b"Charlieton Stock",
        b"tot_optr_charlie_5":          b"Default",
        b"tot_optr_charlie_3":          b"Smaller",
        b"tot_optr_charlie_lim":        b"Limited",
        b"tot_optr_revive":             b"Partner Revival",
        b"tot_optr_mhp":                b"HP / Level",
        b"tot_optr_mfp":                b"FP / Level",
        b"tot_optr_mbp":                b"BP / Level",
        b"tot_optr_mbp_inf":            b"Infinite",
        b"tot_optr_php":                b"Party HP / Level",
        b"tot_optr_itemgain":           b"Items / Strange Sack",
        b"tot_optr_ehp":                b"Enemy HP %",
        b"tot_optr_eatk":               b"Enemy ATK %",
        b"tot_optr_supercost":          b"Superguard SP Cost",
        b"tot_optr_partner":            b"Starting Partner",
        b"tot_optr_partner_random":     b"Random",
        b"tot_optr_partner_1":          b"Goombella",
        b"tot_optr_partner_2":          b"Koops",
        b"tot_optr_partner_3":          b"Flurrie",
        b"tot_optr_partner_4":          b"Yoshi",
        b"tot_optr_partner_5":          b"Vivian",
        b"tot_optr_partner_6":          b"Bobbery",
        b"tot_optr_partner_7":          b"Ms. Mowz",
        b"tot_optr_maxpartners":        b"Max Partners",
        b"tot_optr_nopartners":         b"None",
        b"tot_optr_ac":                 b"AC Difficulty",
        b"tot_optr_ac_0":               b"3 Simplifiers",
        b"tot_optr_ac_1":               b"2 Simplifiers",
        b"tot_optr_ac_2":               b"1 Simplifier",
        b"tot_optr_ac_3":               b"Default",
        b"tot_optr_ac_4":               b"1 Unsimplifier",
        b"tot_optr_ac_5":               b"2 Unsimplifiers",
        b"tot_optr_ac_6":               b"3 Unsimplifiers",
        b"tot_optr_bandit":             b"Bandit Behavior",
        b"tot_optr_bandit_flee":        b"No Refight",
        b"tot_optr_bandit_fight":       b"Forced Refight",
        b"tot_optr_npc_1":              b"NPC Option 1",
        b"tot_optr_npc_2":              b"NPC Option 2",
        b"tot_optr_npc_3":              b"NPC Option 3",
        b"tot_optr_npc_4":              b"NPC Option 4",
        b"tot_optr_npc_wonky":          b"Wonky",
        b"tot_optr_npc_dazzle":         b"Dazzle",
        b"tot_optr_npc_chet":           b"Chet Rippo",
        b"tot_optr_npc_lumpy":          b"Lumpy",
        b"tot_optr_npc_doopliss":       b"Doopliss",
        b"tot_optr_npc_grubba":         b"Grubba",
        b"tot_optr_npc_mover":          b"Mover",
        b"tot_optr_npc_zess":           b"Zess T.",
        b"tot_optr_npc_random":         b"Random",
        b"tot_optr_npc_none":           b"None",
        b"tot_optr_chests":             b"No. of Chests",
        b"tot_optr_chests_default":     b"Balanced",
        b"tot_optr_damagevar":          b"Damage Variance",
        b"tot_optr_damagevar_0":        b"0%",
        b"tot_optr_damagevar_25":       b"25%",
        b"tot_optr_damagevar_50":       b"50%",
        b"tot_optr_audthrows":          b"Audience Throws",
        b"tot_optr_audthrows_off":      b"Normal",
        b"tot_optr_audthrows_on":       b"Random Items",
        b"tot_optr_itemshuffle":        b"Obfuscated Items",
        b"tot_optr_secretboss":         b"Secret Boss",
        b"tot_optr_secretboss_random":  b"Randomly Appears",
        b"tot_optr_secretboss_off":     b"Never Appears",
        b"tot_optr_secretboss_on":      b"Always Appears",
        
        b"tot_optr_off":                b"Off",
        b"tot_optr_on":                 b"On",
        
        b"tot_opth_seed":
            b"Seeds random events. Press X\n"
            b"to randomize, L/R to shift 10x.",
        
        b"tot_opth_preset":
            b"Whether to use custom settings\n"
            b"or a predetermined setup.",
        
        b"tot_opth_difficulty":
            b"Determines how many floors\n"
            b"tall the tower will be.",
        
        b"tot_opth_timertype":
            b"Whether to show IGT, RTA,\n"
            b"or no timer during the run.",
            
        b"tot_opth_startitems":
            b"Determines the items\n"
            b"you start the run with.",
        
        b"tot_opth_drops":
            b"Determines how you get item\n"
            b"drops from enemy battles.",
        
        b"tot_opth_charlie":
            b"Determines how well-stocked\n"
            b"Charlieton's shop is.",
        
        b"tot_opth_revive":
            b"Determines whether partners\n"
            b"revive to 1 HP after wins.",
        
        b"tot_opth_mhp":
            b"How many Heart Points each\n"
            b"stat level is worth.",
        
        b"tot_opth_mfp":
            b"How many Flower Points each\n"
            b"stat level is worth.",
        
        b"tot_opth_mbp":
            b"How many Badge Points each\n"
            b"stat level is worth.",
        
        b"tot_opth_php":
            b"How much HP partners gain\n"
            b"per stat level, on average.",
            
        b"tot_opth_itemgain":
            b"How many extra items each\n"
            b"Strange Sack can carry.",
        
        b"tot_opth_ehp":
            b"What percentage to scale\n"
            b"enemy health by.",
        
        b"tot_opth_eatk":
            b"What percentage to scale\n"
            b"enemies' attack power by.",
        
        b"tot_opth_supercost":
            b"How much Star Power it costs\n"
            b"to perform a Superguard.",
            
        b"tot_opth_partner":
            b"Which partner Mario receives\n"
            b"at the start of the run.",
            
        b"tot_opth_maxpartners":
            b"How many partners Mario\n"
            b"can recruit at once.",
            
        b"tot_opth_ac":
            b"How difficult Action Commands\n"
            b"are to perform.",
            
        b"tot_opth_bandit":
            b"What happens after finishing a\n"
            b"fight where a Bandit ran away.",
            
        b"tot_opth_npc_generic":
            b"Choose which NPCs can appear\n"
            b"on rest floors.",
            
        b"tot_opth_npc_wonky":
            b"He'll buy unwanted items or\n"
            b"badges for a small amount.",
            
        b"tot_opth_npc_dazzle":
            b"He'll sell you additional\n"
            b"Star Pieces.",
            
        b"tot_opth_npc_chet":
            b"He'll give you Shine Sprites\n"
            b"in exchange for lowered stats.",
            
        b"tot_opth_npc_lumpy":
            b"Loan him coins to get back\n"
            b"double, if you see him again.",
            
        b"tot_opth_npc_doopliss":
            b"He'll make enemies stronger,\n"
            b"but more chests will appear.",
            
        b"tot_opth_npc_grubba":
            b"He'll give you extra conditions\n"
            b"for double-or-nothing winnings.",
            
        b"tot_opth_npc_mover":
            b"He can sell you special keys\n"
            b"that let you skip encounters.",
            
        b"tot_opth_npc_zess":
            b"She'll use her cooking skills to\n"
            b"turn your items into rarer ones.",
        
        b"tot_opth_chests":
            b"How many chests there are to\n"
            b"choose from on every floor.",
            
        b"tot_opth_damagevar":
            b"Varies all damage by up to this\n"
            b"amount in either direction.",
            
        b"tot_opth_audthrows":
            b"Whether to make the audience\n"
            b"throw a random item every turn.",
            
        b"tot_opth_itemshuffle":
            b"Shuffles all items and badges'\n"
            b"names, icons and descriptions.",
            
        b"tot_opth_secretboss":
            b"Whether or not the secret,\n"
            b"alternate final boss can appear.",
            
        # Map text.
        
        b"tot_lobby_frontsign":
            b'<kanban>\n'
            b'<pos 116 10>\n"Battle Tower"\n'
            b'<pos 77 39>\nUnder Construction\n<k>',
        
        b"tot_lobby_backsign":
            b'<kanban>\n'
            b'Current seed: <col 0000ffff>\n'
            b'%09d\n</col>\n'
            b'Options: <col 0000ffff>\n'
            b'%s\n</col>\n'
            b'<k>',
        
        b"tot_lobby_reentry":
            b"<system>\n<p>\n"
            b"You can't re-enter that way!\n<k>",
    },
    
    # Tower floor 1; TODO: Copy to other tower maps as needed.
    'gon_01': {
        # Sign information. (TODO: Might not even exist in final game)

        b'tot_floor_sign':
            b'<kanban>\n'
            b'Seed: <col 0000ffff>\n'
            b'%09d\n</col>\n'
            b'Options: <col 0000ffff>\n'
            b'%s\n</col>\n'
            b'Floor: <col 0000ffff>\n'
            b'%02d\n</col>\n'
            b'<k>',
    
        # Area tattle. (TODO: Write final script)
        
        b"msg_kuri_map":
            b"Placeholder area tattle \xd0\n<k>",
        
        # NPC tattles.

        b"npc_shop":
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
            
        b"npc_lumpy":
            b"<keyxon>\n[Placeholder] That's Lumpy.\n"
            b"<dkey><wait 300></dkey>\n"
            b"If you invest coins, it might pay\n"
            b"back dividends later... <dkey><wait 200></dkey>or not.\n<k>",
            
        b"npc_doopliss":
            b"<keyxon>\n[Placeholder] That's Doopliss.\n"
            b"<dkey><wait 300></dkey>\n"
            b"He'll make the next floors tougher,\n"
            b"but more chests will appear.\n<k>",
            
        b"npc_grubba":
            b"<keyxon>\n[Placeholder] That's Grubba.\n"
            b"<dkey><wait 300></dkey>\n"
            b"Meeting his conditions yields\n"
            b"2x coins, but fail, and get 0!\n<k>",
            
        b"npc_chet":
            b"<keyxon>\n[Placeholder] That's Chet Rippo.\n"
            b"<dkey><wait 300></dkey>\n"
            b"You can pay him to swap levels\n"
            b"for extra Shine Sprites.\n<k>",
            
        b"npc_wonky":
            b"<keyxon>\n[Placeholder] That's Wonky.\n"
            b"<dkey><wait 300></dkey>\n"
            b"He'll take our unwanted items or\n"
            b"badges for a few coins.\n<k>",
            
        b"npc_dazzle":
            b"<keyxon>\n[Placeholder] That's Dazzle.\n"
            b"<dkey><wait 300></dkey>\n"
            b"He'll sell us extra Star Pieces\n"
            b"to help rank up our moves.\n<k>",
            
        b"npc_mover":
            b"<keyxon>\n[Placeholder] That's a Mover.\n"
            b"<dkey><wait 300></dkey>\n"
            b"Word has it, he can help us get\n"
            b"through the tower faster.\n<k>",
            
        b"npc_zess":
            b"<keyxon>\n[Placeholder] That's Zess T.\n"
            b"<dkey><wait 300></dkey>\n"
            b"She's a wonderful cook that can\n"
            b"turn our items into rarer ones.\n<k>",
            
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
        
        # Generic
        
        b"tot_npc_yesnoopt":
            b"<select 0 1 0 40>\nYes\nNo",
        
        # Charlieton
        
        b"tot_charlieton_nostock":
            b"Hey, thanks to you, I'm all\n"
            b"sold out, my man! <wait 250>You put\n"
            b"my kids through college!\n<k>",
        
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
            
        # Other NPCs
        
        b"tot_wonky_intro":
            b"<shake>\n"
            b"Hee hee hee!</shake><wait 300> I can help\n"
            b"lighten your load if you've\n"
            b"got junk weighing you down!\n<k>\n<p>\n"
            b"What're you selling?\n<o>",
        
        b"tot_wonky_topmenu":
            b"<select 0 2 0 40>\nItems\nBadges",
            
        b"tot_wonky_noitems":
            b"<p>\n[Placeholder] No items!\n<k>",
            
        b"tot_wonky_nobadges":
            b"<p>\n[Placeholder] No badges!\n<k>",
            
        b"tot_wonky_whichitem":
            b"<p>\n[Placeholder] Which one?\n<k>",
            
        b"tot_wonky_itemok":
            b"[Placeholder] How about I\n"
            b"give <NUM> coin<S> for your\n"
            b"<ITEM>. Deal?\n<o>",
            
        b"tot_wonky_itemdifferent":
            b"<p>\n[Placeholder] Anything else?\n<k>",
            
        b"tot_wonky_thanksnext":
            b"<p>\n[Placeholder] Any more?\n<o>",
            
        b"tot_wonky_thankslast":
            b"<p>\n[Placeholder] Thanks!\n<k>",
        
        b"tot_wonky_exit":
            b"<p>\n[Placeholder] Exit text\n<k>",
            
        b"tot_chet_intro":
            b"[Placeholder] I can give you\n"
            b"Shine Sprites in exchange for\n"
            b"your stats. Deal?\n<o>",
            
        b"tot_chet_nostats":
            b"<p>\n[Placeholder] No stats!\n<k>",
            
        b"tot_chet_nocoins":
            b"<p>\n[Placeholder] Not enough!\n<k>",
        
        b"tot_chet_whichstat":
            b"<p>\n[Placeholder] Which stat?\n<k>",
        
        b"tot_chet_different":
            b"<p>\n[Placeholder] Anything else?\n<k>",
        
        b"tot_chet_confirm":
            b"[Placeholder] Your %s?\n"
            b"It'll fall from level %d to %d\n"
            b"in exchange for a Shine Sprite.\n<k>\n<p>\n"
            b"The fee will be %d coins.\nDeal?\n<o>",
            
        b"tot_chet_decline":
            b"<p>\nThen get outta here!\n<k>",
            
        b"tot_chet_exit":
            b"<p>\nNow get outta here!\n<k>",
            
        b"tot_dazzle_intro":
            b"[Placeholder] I'm Dazzle,\n"
            b"Star Piece collector\n"
            b"extraordinaire!\n<k>\n<p>\n"
            b"I can let you have one\n"
            b"for free, but if you want\n"
            b"more, it'll cost you! Deal?\n<o>",
            
        b"tot_dazzle_offer":
            b"[Placeholder] I can let you\n"
            b"have another Star Piece for\n"
            b"%d coins. Deal?\n<o>",
            
        b"tot_dazzle_nocoins":
            b"<p>\n[Placeholder] Not enough!\n<k>",
            
        b"tot_dazzle_decline":
            b"<p>\n[Placeholder] Oh, too bad!\n<k>",
            
        b"tot_grubba_intro":
            b"[Placeholder] Want conditions\n"
            b"every floor? If you meet them,\n"
            b"you'll double your winnings!\n<k>\n<p>\n"
            b"If you don't meet them, you'll\n"
            b"wind up with a fat sack o' nothin'.\n"
            b"What say ya, Gonzales?\n<o>",
            
        b"tot_grubba_accept":
            b"<p>\n[Placeholder] Now, THAT's\n"
            b"what I'm talkin' about! Now get\n"
            b"out there and whomp 'em!\n<k>",
            
        b"tot_grubba_decline":
            b"<p>\n[Placeholder] Come on, son,\n"
            b"where's yer fightin' spirit?\n<k>",
            
        b"tot_grubba_active":
            b"[Placeholder] Get out there\n"
            b"and show those yokels what\n"
            b"yer made of, Gonzales!\n<k>",
            
        b"tot_doopliss_intro":
            b"[Placeholder] Hey, Slick!\n"
            b"Looking for a challenge?\n<k>\n<p>\n"
            b"I'll use my magic to make\n"
            b"the enemies on the next set of\n"
            b"floors extra tough.\n<k>\n<p>\n"
            b"In exchange, you might get\n"
            b"more rewards to choose from\n"
            b"after battle. Deal?\n<o>",
            
        b"tot_doopliss_accept":
            b"<p>\n[Placeholder] Hee hee!\n"
            b"Don't get clobbered out there!\n<k>",
            
        b"tot_doopliss_decline":
            b"<p>\n[Placeholder]\nAw, you're no fun!\n<k>",
            
        b"tot_doopliss_active":
            b"[Placeholder] Hee hee!\n"
            b"Don't get clobbered out there!\n<k>",
            
        b"tot_lumpy_intronocoin":
            b"[Placeholder] Hello! I'm Lumpy.\n"
            b"If you lend me all your coins,\n"
            b"I'll pay you interest later.\n<k>\n<p>\n"
            b"Or at least I would, if you\n"
            b"had any coins on you right now.\n"
            b"Good luck on your travels!\n<k>",
            
        b"tot_lumpy_intro":
            b"[Placeholder] Hello! I'm Lumpy.\n"
            b"If you lend me all your coins,\n"
            b"I'll pay you interest later.\n<k>\n<p>\n"
            b"Sound like a deal?\n<o>",
            
        b"tot_lumpy_reward":
            b"[Placeholder] Oh, hey!\n"
            b"Thanks for the loan earlier;\n"
            b"here it is with 100% interest!\n<k>\n<o>",
            
        b"tot_lumpy_goodluck":
            b"Good luck on your travels!\n<k>",
            
        b"tot_lumpy_goodluckadd":
            b"<p>\nGood luck on your travels!\n<k>",
            
        b"tot_lumpy_decline":
            b"<p>\nGood luck on your travels!\n<k>",
            
        b"tot_lumpy_accept":
            b"<p>\nThanks a million!\n"
            b"You won't regret it, I'm sure.\n"
            b"Until we cross paths again!\n<k>",
            
        b"tot_lumpy_doubleorno":
            b"<p>\nIf you'd care to lend me your\n"
            b"cash again, I can offer you the\n"
            b"same deal. What do you say?\n<o>",
            
        b"tot_mover_intro":
            b"Hey.<wait 250> Wassup?<wait 250> I'm a Mover.\n"
            b'<wait 250>\n'
            b'I know a few things about\n'
            b'getting through this tower.\n<k>\n<p>\n'
            b'For a few coins, I can get you\n'
            b'the key to where you wanna go.\n'
            b'<wait 250>\n'
            b"Whaddya thinkin'?\n<o>",
            
        b"tot_mover_menu":
            b"<select 0 2 0 40>\n"
            b"Tower Key (skips 1 non-midboss)\n"
            b"Master Key (skips any 1 floor)\n"
            b"I'm good, thanks!",
            
        b"tot_mover_decline":
            b"<p>\n[Placeholder] That's too bad.\n<k>",
            
        b"tot_mover_offer0":
            b"<p>\n[Placeholder] A Tower Key\n"
            b"is gonna run you %d coins.\n"
            b"<wait 250>Deal?\n<o>",
            
        b"tot_mover_offer1":
            b"<p>\n[Placeholder] A Master Key\n"
            b"is gonna run you %d coins.\n"
            b"<wait 250>Deal?\n<o>",
            
        b"tot_mover_nocoins":
            b'<p>\n'
            b'Hey! <wait 100>You need more coins.\n'
            b'<wait 250>\n'
            b"Sorry, but a guy's gotta make\n"
            b'a living, know what I mean?\n'
            b'<k>',

        b'tot_mover_success':
            b'[Placeholder] Awright! \n'
            b'<wait 150>My secret paths are...secret!<wait 250> \n'
            b"We never spoke, <wait 150> got it?\n<k>",
            
        b"tot_mover_active":
            b"[Placeholder] We never spoke,\n"
            b"<wait 150>got it?\n<k>",
            
        b"tot_mover_full_inv":
            b'<p>\n'
            b"Hey! <wait 100>You're looking loaded up\n"
            b'on gear already.<wait 250> You still sure\n'
            b"ya got room for this?\n<o>",
            
        b"tot_zess_intro":
            b"[Placeholder] I'm Zess T.<wait 250> \n"
            b"I love to cook! You provide the\n"
            b"items, I work my magic.\n<k>\n<p>\n"
            b"What can I do for you today?\n<o>",
            
        b"tot_zess_intronoitems":
            b"[Placeholder] I'm Zess T.<wait 250> \n"
            b"I love to cook! You provide the\n"
            b"items, I work my magic.\n<k>\n<p>\n"
            b"It seems you don't have any\n"
            b"I can work with at the moment,\n"
            b"though. Next time, then!\n<k>",
            
        b"tot_zess_decline":
            b"<p>\nOh, that's too bad!\n<k>",
            
        b"tot_zess_nochooserecipe":
            b"Indecisive, are we? <wait 250>Oh well,\n"
            b"I'll be here if you make up\n"
            b"your mind later...\n<k>",
            
        b"tot_zess_which1st":
            b"<p>\nWhich would you like to use?\n<k>",
            
        b"tot_zess_which2nd":
            b"Mm-hmm, got it. <wait 250>What else?\n<k>",
            
        b"tot_zess_whichrecipe":
            b"[Placeholder] I've got a\n"
            b"couple of dishes that use that.\n"
            b"<wait 250>Which one do you want?\n<k>",
        
        b"tot_zess_giveup":
            b"[Placeholder] What's wrong?\n"
            b"Still want me to cook?\n<o>",
            
        b"tot_zess_confirmcost":
            b"[Placeholder] All right, I'll\n"
            b"cook that right up for you\n"
            b"for %d coins.<wait 250> OK?\n<o>",
            
        b"tot_zess_declinecost":
            b"<p>\n[Placeholder] Hmph, my art\n"
            b"isn't free. <wait 250>Come back if you\n"
            b"want anything else.\n<k>",
            
        b"tot_zess_confirmswap":
            b"[Placeholder] All right, I'll\n"
            b"swap that %s\n"
            b"for you, free of charge. OK?\n<o>",
            
        b"tot_zess_cookagain":
            b"Want me to cook anything\n"
            b"else?\n<o>",
            
        b"tot_zess_nomoretocook":
            b"[Placeholder] Well, I've done\n"
            b"all I can do. <wait 200>Enjoy the fruit\n"
            b"of my culinary magic!\n<k>",
            
        # NPC-related menu / help text.
        
        b"tot_desc_chet_adjusthp":
            b"Lower Mario's HP by 1 level.",
        
        b"tot_desc_chet_adjustphp":
            b"Lower partner HP by 1 level.",
        
        b"tot_desc_chet_adjustfp":
            b"Lower FP by 1 level.",
        
        b"tot_desc_chet_adjustbp":
            b"Lower BP by 1 level.",
        
        b"tot_desc_chet_adjustnone":
            b"This stat can't be lowered\n"
            b"any further!",
            
        # Miscellaneous unused stuff from Infinite Pit.
            
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
    },
    
    # Tower final floor.
    'gon_05': {
    
        # Final results window.
        
        b"tot_winsel_runstats_topbar":
            b"Run Results",
            
        b"tot_winsel_runresults_header":
            b"Stats",
            
        b"tot_winsel_runsplits_header":
            b"Splits",
            
        b"tot_optr_turnsspent":         b"Total Turns Spent",
        b"tot_optr_turnsavg":           b"Average Turns",
        b"tot_optr_turnsmost":          b"Most Turns",
        b"tot_optr_battletimeavg":      b"Average Battle Time",
        b"tot_optr_battletimemost":     b"Longest Battle",
        b"tot_optr_timesran":           b"Times Ran Away",
        b"tot_optr_enemydamage":        b"Damage Dealt",
        b"tot_optr_playerdamage":       b"Damage Taken",
        b"tot_optr_itemsused":          b"Items Used",
        b"tot_optr_shinesprites":       b"Shine Sprites",
        b"tot_optr_starpieces":         b"Star Pieces",
        b"tot_optr_coinsearned":        b"Coins Earned",
        b"tot_optr_coinsspent":         b"Coins Spent",
        b"tot_optr_fpspent":            b"FP Spent",
        b"tot_optr_spspent":            b"SP Spent",
        b"tot_optr_superguards":        b"Superguards",
        b"tot_optr_conditionsmet":      b"Conditions Met",
        
        # Dragon field dialogue.
        
        b"tot_field_dragon00_00":
            b"[Placeholder] Encounter<k>",
        
        b"tot_field_dragon01_00":
            b"[Placeholder] Encounter<k>",
        
        b"tot_field_dragon02_00":
            b"[Placeholder] Encounter<k>",
        
        # Dragon fight dialogue.
        
        # Hooktail battle entry.
        b"tot_dragon00_00":
            b"[Placeholder] Battle entry<k>",
        
        # Hooktail low health.
        b"tot_dragon00_10":
            b"[Placeholder] Low health<k>",
        
        # Hooktail phase 2 start.
        b"tot_dragon00_20":
            b"[Placeholder] Phase 2<k>",
        
        # Hooktail phase 3 start (unused).
        b"tot_dragon00_30":
            b"[Placeholder] Phase 3<k>",
        
        # Hooktail megabreath (unused).
        b"tot_dragon00_40":
            b"[Placeholder] Megabreath!<k>",
        
        # Hooktail healing (unused).
        b"tot_dragon00_50":
            b"[Placeholder] Healing!<k>",
        
        # Hooktail bite reaction.
        b"tot_dragon00_60":
            b"[Placeholder] Bite gloating<k>",
        
        # Hooktail partner bite reaction.
        b"tot_dragon00_70":
            b"[Placeholder]\n"
            b"You can Superguard that btw<k>",
        
        # Hooktail death.
        b"tot_dragon00_80":
            b"[Placeholder] Dead<k>",
        
        # Hooktail fake death (unused).
        b"tot_dragon00_90":
            b"[Placeholder] Dead?",
        
        # Gloomtail battle entry.
        b"tot_dragon01_00":
            b"<boss>\n"
            b"So you've come this far to\n"
            b"challenge me again, mortals?\n"
            b"<k>\n"
            b"<p>\n"
            b"That was quite foolish, I fear.\n"
            b"<wait 250>\n"
            b"I shall not go so easy on you\n"
            b"this time!\n"
            b"<k>",
        
        # Gloomtail low health.
        b"tot_dragon01_10":
            b"<boss>\n"
            b"Mmmmph...<wait 250> How can this be...\n"
            b"<k>",
        
        # Gloomtail phase 2 start.
        b"tot_dragon01_20":
            b"<boss>\n"
            b"GWAAAAH HA HA HA HA!\n"
            b"<wait 250>\n"
            b"You are tough meat, my little\n"
            b"tasty morsels!\n"
            b"<k>\n"
            b"<p>\n"
            b"But how will you fare\n"
            b"against THIS!!!\n"
            b"<k>",
        
        # Gloomtail phase 3 start.
        b"tot_dragon01_30":
            b"<boss>\n"
            b"Gwuhhh...\n"
            b"<k>\n"
            b"<p>\n"
            b"You are not the easy prey\n"
            b"you appear to be...\n"
            b"<k>\n"
            b"<p>\n"
            b"I would never be able to show\n"
            b"my face again if I was bested\n"
            b"again by you whelps...\n"
            b"<k>\n"
            b"<p>\n"
            b"...So I will show you the true\n"
            b"extent of my power!\n"
            b"<k>",
        
        # Gloomtail megabreath.
        b"tot_dragon01_40":
            b"<boss>\n"
            b"<dynamic 3>\n"
            b"<scale 2>\n"
            b"<pos 15 10>\n"
            b"MEGABREATH!\n"
            b"<k>",
        
        # Gloomtail healing (unused).
        b"tot_dragon01_50":
            b"Placeholder text 1-50<k>",
        
        # Gloomtail bite reaction (unused).
        b"tot_dragon01_60":
            b"Placeholder text 1-60<k>",
        
        # Gloomtail partner bite reaction (unused).
        b"tot_dragon01_70":
            b"Placeholder text 1-70<k>",
        
        # Gloomtail death.
        b"tot_dragon01_80":
            b"<boss>\n"
            b"No... <wait 250>No...<wait 250> It can't be true!\n"
            b"<k>",
        
        # Gloomtail fake death.
        b"tot_dragon01_90":
            b"<boss>\n"
            b"[Placeholder]\n"
            b"No... <wait 250>No...<wait 250> It can't be true!\n"
            b"<k>",
        
        # Bonetail battle entry.
        b"tot_dragon02_00":
            b"[Placeholder] Battle entry<k>",
        
        # Bonetail low health.
        b"tot_dragon02_10":
            b"[Placeholder] Low health<k>",
        
        # Bonetail phase 2 start.
        b"tot_dragon02_20":
            b"[Placeholder] Phase 2<k>",
        
        # Bonetail phase 3 start (unused).
        b"tot_dragon02_30":
            b"[Placeholder] Phase 3<k>",
        
        # Bonetail megabreath (unused).
        b"tot_dragon02_40":
            b"[Placeholder] Megabreath!<k>",
        
        # Bonetail healing.
        b"tot_dragon02_50":
            b"[Placeholder] Healing!<k>",
        
        # Bonetail bite reaction (unused).
        b"tot_dragon02_60":
            b"[Placeholder] Bite<k>",
        
        # Bonetail partner bite reaction (unused).
        b"tot_dragon02_70":
            b"[Placeholder] Reaction<k>",
        
        # Bonetail death.
        b"tot_dragon02_80":
            b"[Placeholder] Dead<k>",
        
        # Bonetail fake death (unused).
        b"tot_dragon02_90":
            b"[Placeholder] Dead?",
    },
    
    # West side of Petalburg hub.
    'gon_10': {
    
        # Shopkeeper.
    
        b"tot_shopkeep_00":
            b"It's <NUM> coin<S> for\n"
            b"<AN_ITEM>.\n<wait 250>\n"
            b"Would you like one?\n<o>",
    
        b"tot_shopkeep_01":
            b"<p>\n"
            b"Oops! <wait 250>Oh, gee, I'm sorry.\n"
            b"<wait 250>\n"
            b"You don't have enough coins.\n<k>",
    
        b"tot_shopkeep_11":
            b"<p>\nThank you so very much!\n<k>",
    
        b"tot_shopkeep_22":
            b"<p>\nCome again, OK?\n<k>",
    
        b"tot_shopkeep_24":
            b"<select 0 1 0 40>\nYes\nNo",
    },
    
    # East side of Petalburg hub.
    'gon_11': {
    },
    
    # Opening cutscene in Petal Meadows.
    'gon_12': {
    
        # Area tattle.
        
        b"msg_kuri_map":
            b"<keyxon>\n"
            b"Ah, the Petal Meadows.\n"
            b"<dkey><wait 300></dkey>\n"
            b"A plateau surrounded by hills\n"
            b"far to the east of Rogueport.\n<k>\n<p>\n"
            b"It's mild and pleasant here\n"
            b"all year round.<wait 250> I love it!\n"
            b"<wait 300>\n"
            b"Doesn't it feel nice?\n<k>",
    
        # Opening cutscene text.
            
        b"stg1_hei_00":
            b"It's great that you could come\n"
            b"and visit, Mario! <wait 250>I know that\n"
            b"hero work can be a handful!\n<k>\n<p>\n"
            b"I wonder what all the folks in\n"
            b"Petalburg are up to these days.\n"
            b"Ooh, I can't wait!\n<k>",
            
        b"stg1_hei_01":
            b"Mario, <wait 150>that scary sound and\n"
            b"ominous shadow couldn't mean\n"
            b"what I think it does, <wait 250>could it?<wait 250> \n<k>\n<p>\n"
            b"Oh, no! <wait 100>It IS her!\n"
            b"<wait 250>Mario! Look!!!\n<k>",
            
        b"stg1_hei_02":
            b"I thought we taught Hooktail\n"
            b"a lesson last time! <wait 150>What's she\n"
            b"doing back here now?\n<k>\n<p>\n"
            b"I've got a bad feeling about\n"
            b"this, Mario. <wait 250>Let's investigate and\n"
            b"make sure everything's OK! \n<k>",

    },
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