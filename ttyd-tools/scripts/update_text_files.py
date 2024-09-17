#! /usr/bin/python3.6

"""Patches msg files used by TTYD, adding/updating particular text strings."""
# Jonathan Aldrich 2024-02-23

import os
import subprocess
import sys
from collections import ChainMap
from pathlib import Path

import flags    # jdalib

FLAGS = flags.Flags()

FLAGS.DefineString("in_msg_dir", "")
FLAGS.DefineString("out_msg_dir", "")

g_GlobalStrings = {
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
        
    b'msg_pch_heart_catch':
        b"Steal an item or badge from\n"
        b'any enemy.',
        
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
    b"tot_winsel_titleattackfx":  b"Attack FX",
    b"tot_winsel_titlecolor":     b"Color",
    b"tot_winsel_titlecostume":   b"Costume",
    b"tot_winsel_titlestat":      b"Stat Lvl.",
    b"tot_winsel_tradewhichstat": b"Trade which?",
    b"tot_winsel_whichduds":      b"Which getup?",
    b"tot_winsel_whichones":      b"Which ones?",
    
    # File select text.
    
    b"tot_file_completion":       b"Completion",
    
    # TODO: Move this and other Gold Fuzzy dialogue to the final boss map.
    
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
        b'Completion',

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
        b'Total progress on achievements,\n'
        b'Journal logs, and shop purchases.',
        
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
        b"Records of various stats and\n"
        b'overall completion progress.',
    
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
    
    # Dummy text for if you don't have a level unlocked yet.
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
    b"tot_movelog_d321":
        b"\nShows current HP, ATK & DEF\n"
        b"for all enemies of a given type.",
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
    b"tot_movelog_d361":
        b"\nCosts 4 FP to use.",
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
    b"tot_movelog_d431":
        b"\nCosts 2 FP to use.",
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
    b"tot_movelog_d521":
        b"\nCosts 5 FP. Works better on\n"
        b"foes with low HP or Tiny status.",
    b"tot_movelog_d531":
        b"\nLv. 1: Drains 4 HP from an\n"
        b"enemy for 3 FP.",
    b"tot_movelog_d532":
        b"\nLv. 2: Drains 6 HP from an\n"
        b"enemy for 5 FP.",
    b"tot_movelog_d533":
        b"\nLv. 3: Drains 8 HP from an\n"
        b"enemy for 7 FP.",
    b"tot_movelog_d541":
        b"\nApplies up to 4 turns of Dodgy\n"
        b"on Mario and Flurrie for 4 FP.",
    b"tot_movelog_d551":
        b"\nLv. 1: Deals 3 piercing damage\n"
        b"and 1-turn Freeze for 5 FP.",
    b"tot_movelog_d552":
        b"\nLv. 2: Deals 4 piercing damage\n"
        b"and 2-turn Freeze for 7 FP.",
    b"tot_movelog_d553":
        b"\nLv. 3: Deals 5 piercing damage\n"
        b"and 3-turn Freeze for 9 FP.",
    b"tot_movelog_d561":
        b"\nDeals up to 10 piercing damage\n"
        b"on all foes for 9 FP.",
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
    b"tot_movelog_d641":
        b"\nCosts 5 FP. Works better on\n"
        b"foes with low HP or Tiny status.",
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
    b"tot_movelog_d761":
        b"\nCosts 5 FP to use. Only one foe\n"
        b"can be Infatuated at a time.",
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
    b"tot_movelog_d831":
        b"\nCounters enemy direct attacks\n"
        b"for up to 4 turns, for 4 FP.",
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
    b"tot_movelog_d861":
        b"\nDeals up to 20 damage to all\n"
        b"combatants for 10 FP.",
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
    b"tot_movelog_d921":
        b"\nCosts 5 FP. Ignores contact\n"
        b"hazards, except lit Bob-ombs.",
    b"tot_movelog_d931":
        b"\nLv. 1: 1-turn Confuse, with a\n"
        b"failed act rate of 50% for 2 FP.",
    b"tot_movelog_d932":
        b"\nLv. 2: 1-turn Confuse, with a\n"
        b"failed act rate of 70% for 4 FP.",
    b"tot_movelog_d933":
        b"\nLv. 3: 1-turn Confuse, with a\n"
        b"failed act rate of 90% for 7 FP.",
    b"tot_movelog_d941":
        b"\nCosts 2 FP to use.",
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
    b"tot_recn_runstats_1":         b"Run Stats (1/2)",
    b"tot_recn_runstats_2":         b"Run Stats (2/2)",
    b"tot_recn_runs":               b"Run Completions",
    b"tot_recn_aggstats_1":         b"Overall Stats (1/3)",
    b"tot_recn_aggstats_2":         b"Overall Stats (2/3)",
    b"tot_recn_aggstats_3":         b"Overall Stats (3/3)",
    b"tot_recn_aggstats_4":         b"Overall Stats (4/4)",  # Unused.
    b"tot_recn_hub":                b"Hub Progression",
    b"tot_recn_overall":            b"Overall Progress",
    
    b"tot_recn_playtime":           b"Total Play Time",
    b"tot_recn_completion_pct":     b"Completion %",
    b"tot_recn_achievement_pct":    b"Achievement Log",
    b"tot_recn_item_pct":           b"Items Log",
    b"tot_recn_badge_pct":          b"Badge Log",
    b"tot_recn_tattle_pct":         b"Tattle Log",
    b"tot_recn_move_pct":           b"Moves Obtained",
    b"tot_recn_move_cmp_pct":       b"Move Completion",
    b"tot_recn_hub_pct":            b"Completion %",
    b"tot_recn_hub_keyitems":       b"Key Items",
    b"tot_recn_hub_items":          b"Items Bought",
    b"tot_recn_hub_badges":         b"Badges Bought",
    b"tot_recn_hub_options":        b"Options Bought",
    b"tot_recn_hub_marioskins":     b"Mario Costumes",
    b"tot_recn_hub_yoshiskins":     b"Yoshi Colors",
    b"tot_recn_hub_attackfx":       b"Attack FX",
    b"tot_recn_half_wins":          b"Hooktail",
    b"tot_recn_full_wins":          b"Gloomtail",
    b"tot_recn_ex_wins":            b"EX Difficulty",
    b"tot_recn_intensity":          b"Max Intensity",
    b"tot_recn_times":              b"Best Times (IGT)",
    b"tot_recn_half_time":          b"Hooktail",
    b"tot_recn_full_time":          b"Gloomtail",
    b"tot_recn_ex_time":            b"EX Difficulty",
    b"tot_recn_attempts":           b"Total Attempts",
    b"tot_recn_clears":             b"Total Clears",
    b"tot_recn_floors":             b"Floors Cleared",
    b"tot_recn_continues":          b"Total Continues",
    b"tot_recn_turns":              b"Turns Spent",
    b"tot_recn_runaway":            b"Times Ran Away",
    b"tot_recn_kills":              b"Foes Defeated",
    b"tot_recn_midkills":           b"Unique Midbosses",
    b"tot_recn_edamage":            b"Enemy Damage",
    b"tot_recn_pdamage":            b"Player Damage",
    b"tot_recn_fpspent":            b"FP Spent",
    b"tot_recn_spspent":            b"SP Spent",
    b"tot_recn_superguards":        b"Superguards",
    b"tot_recn_conditions":         b"Conditions Met",
    b"tot_recn_mcoinsearned":       b"Coins Won",
    b"tot_recn_mspearned":          b"Star Pcs. Won",
    b"tot_recn_rcoinsearned":       b"Coins Earned",
    b"tot_recn_rcoinsspent":        b"Coins Spent",
    b"tot_recn_starpieces":         b"Star Pieces",
    b"tot_recn_shinesprites":       b"Shine Sprites",
    b"tot_recn_itemsused":          b"Items Used",
    b"tot_recn_itemsbought":        b"Items Bought",
    b"tot_recn_wonky":              b"Wonky Deals",
    b"tot_recn_dazzle":             b"Dazzle Deals",
    b"tot_recn_rippo":              b"Chet Rippo Deals",
    b"tot_recn_lumpy":              b"Lumpy Deals",
    b"tot_recn_grubba":             b"Grubba Deals",
    b"tot_recn_doopliss":           b"Doopliss Deals",
    b"tot_recn_mover":              b"Mover Deals",
    b"tot_recn_zess":               b"Zess T. Deals",
    b"tot_recn_unkdeals":           b"??? Deals",

    b"tot_rech_progression":
        b"Total playtime and Journal\n"
        b"completion progress.",
    b"tot_rech_hub_pct":
        b"Unlockables purchased from\n"
        b"various vendors in Petalburg.",
    b"tot_rech_wins":
        b"How many runs were completed,\n"
        b"and fastest default clear times.",
    b"tot_rech_runstats":
        b"Various stats for your current\n"
        b"attempted tower run.",
    b"tot_rech_aggstats":
        b"Various stats aggregated across\n"
        b"all of your past runs.",

    # Records Log + Final results window.
    
    b"tot_winsel_runstats_topbar":
        b"Run Results",
        
    b"tot_winsel_runresults_header":
        b"Stats",
        
    b"tot_winsel_runsplits_header":
        b"Splits",
        
    b"tot_winsel_runrewards_header":
        b"Rewards",
        
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
    b"tot_optr_npcsmet":            b"NPC Deals Taken",
    b"tot_optr_itemsbought":        b"Items Bought",
    b"tot_optr_continues":          b"Continues",
        
        
    # Achievement descriptions.
    
    b"tot_achd_00":
        b"Perform all Stylish commands on\n"
        b"15 different moves.",
    b"tot_achd_01":
        b"Buy a total of 25 items from\n"
        b"Charlieton.",
    b"tot_achd_02":
        b"Make deals with a total of\n"
        b"10 NPCs across all tower runs.",
    b"tot_achd_03":
        b"Recruit all partners.",
    b"tot_achd_04":
        b"Use 10 different Lvl. 3 moves.",
    b"tot_achd_05":
        b"Defeat 40 different types of\n"
        b"midbosses.",
    b"tot_achd_06":
        b"Superguard 100 attacks in total.",
    b"tot_achd_07":
        b"Win a total of 10,000 coins\n"
        b"from tower runs.",
    b"tot_achd_08":
        b"Deal a total of 15,000 damage\n"
        b"to enemies.",
    b"tot_achd_09":
        b"Clear Hooktail's tower for the\n"
        b"first time.",
    b"tot_achd_10":
        b"Clear Hooktail's tower in\n"
        b"1:00:00 with default settings.",
    b"tot_achd_11":
        b"Clear Hooktail's tower in\n"
        b"0:40:00 with default settings.",
    b"tot_achd_12":
        b"Clear Gloomtail's tower for the\n"
        b"first time.",
    b"tot_achd_13":
        b"Clear Gloomtail's tower in\n"
        b"2:00:00 with default settings.",
    b"tot_achd_14":
        b"Clear Gloomtail's tower in\n"
        b"1:30:00 with default settings.",
    b"tot_achd_15":
        b"Finish an EX-difficulty tower run\n"
        b"with default settings.",
    b"tot_achd_16":
        b"Clear an EX-difficulty run in\n"
        b"3:00:00 with default settings.",
    b"tot_achd_17":
        b"Clear an EX-difficulty run in\n"
        b"2:20:00 with default settings.",
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
        b"3 or fewer turns on each floor.\n"
        b"Running away from battle still\n"
        b"counts any full or partial turns\n"
        b"spent toward that floor's total.",
    b"tot_achd_28":
        b"Finish a default run without\n"
        b"taking any damage.",
    b"tot_achd_29":
        b"Finish a run with an intensity\n"
        b"rating of 200% or higher.",
    b"tot_achd_30":
        b"Finish a run with one of Mario's\n"
        b"stats at a maximum of 1.\n"
        b"Settings must be default, aside\n"
        b"from forcing any stat to 0.\n"
        b"You can also start at default,\n"
        b"using Chet Rippo to sell stats.",
    b"tot_achd_31":
        b"Finish a run with two of Mario's\n"
        b"stats at a maximum of 1.\n"
        b"Settings must be default, aside\n"
        b"from forcing any stat to 0.\n"
        b"You can also start at default,\n"
        b"using Chet Rippo to sell stats.",
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
        b"Log all common and rare items\n"
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
        b"Beat the secret final boss.",
    b"tot_achd_63":
        b"Finish 10 tower runs across\n"
        b"all difficulty settings.",
    b"tot_achd_64":
        b"Finish 25 tower runs across\n"
        b"all difficulty settings.",
    b"tot_achd_65":
        b"Complete all other achievements.\n",
    b"tot_achd_66":
        b"Spend exactly 417 coins in a\n"
        b"single tower run.",
    b"tot_achd_67":
        b"Deal 1,000 or more damage with\n"
        b"Infatuated foes in a default run.",
    b"tot_achd_68":
        b"Finish a run with all of Mario's\n"
        b"stats at a maximum of 1.\n"
        b"Settings must be default, aside\n"
        b"from forcing any stat to 0.\n"
        b"You can also start at default,\n"
        b"using Chet Rippo to sell stats.",
    b"tot_achd_69":
        b"Take exactly 654 total damage\n"
        b"in a single tower run.",
    
    # Achievement menu option names.
    
    b"tot_acho_charlieton":     b"Charlieton Stock",
    b"tot_acho_npcchoice":      b"NPC Choice",
    b"tot_acho_partner":        b"Partner Choice",
    b"tot_acho_acdiff":         b"AC Difficulty",
    b"tot_acho_superguardcost": b"Superguard Cost",
    b"tot_acho_audiencethrow":  b"Audience Throws",
    b"tot_acho_randomdamage":   b"Damage Variance",
    b"tot_acho_revive":         b"Partner Revival",
    b"tot_acho_bandit":         b"Bandit Escape",
    b"tot_acho_invincrease":    b"S. Sack Size",
    b"tot_acho_obfuscated":     b"Obfuscated Items",
    b"tot_acho_secretboss":     b"Secret Boss",
    b"tot_acho_infinitebp":     b"BP: Infinite",
    b"tot_acho_customloadout":  b"Items: Custom",
    
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
        b'These guys are so tough,\n<wait 50>\n'
        b'any hit only leaves a scratch!\n<k>\n<p>\n',
    
    # TODO: Menu Tattles for all enemies. - b"menu_enemy_001", etc.
    
    b'menu_enemy_400':
        b"[Placeholder] Fuzzy Horde",
    
    # Cosmetic names and descriptions.
    
    b'tot_cos0_06_g':     b'Animal FX',
    b'tot_cos0_07_g':     b'Character FX',
    b'tot_cos0_08_g':     b'Crowd FX',
    b'tot_cos0_09_g':     b'Digital FX',
    b'tot_cos0_10_g':     b'Cartoon FX 1',
    b'tot_cos0_11_g':     b'Cartoon FX 2',
    b'tot_cos0_12_g':     b'Cartoon FX 3',
    b'tot_cos0_13_g':     b'Variety FX',
    b'tot_cos0_14_g':     b'Peach+Bowser FX',
    
    b'tot_cos0_01':     b'Ding FX',
    b'tot_cos0_02':     b'Froggy FX',
    b'tot_cos0_03':     b'Squeaky FX',
    b'tot_cos0_04':     b'Tweet FX',
    b'tot_cos0_05':     b'Crow FX',
    b'tot_cos0_06':     b'Oink FX',
    b'tot_cos0_07':     b'Smorg FX',
    b'tot_cos0_08':     b'Doopliss FX',
    b'tot_cos0_09':     b'Bandit FX',
    b'tot_cos0_10':     b'Scream FX',
    b'tot_cos0_11':     b'Whistle FX',
    b'tot_cos0_12':     b'Techy FX',
    b'tot_cos0_13':     b'Retro FX',
    b'tot_cos0_14':     b'1-UP FX',
    b'tot_cos0_15':     b'Pop FX',
    b'tot_cos0_16':     b'Zip FX',
    b'tot_cos0_17':     b'Boom FX',
    b'tot_cos0_18':     b'Boing FX',
    b'tot_cos0_19':     b'Crash FX',
    b'tot_cos0_20':     b'Punch FX',
    b'tot_cos0_21':     b'Wacky FX',
    b'tot_cos0_22':     b'Raspberry FX',
    b'tot_cos0_23':     b'Jingle FX',
    b'tot_cos0_24':     b'Crystal FX',
    b'tot_cos0_25':     b'Warp FX',
    b'tot_cos0_26':     b'Peach FX',
    b'tot_cos0_27':     b'Bowser FX',
    
    b'tot_cos1_01':     b'Mario',
    b'tot_cos1_02':     b'Luigi',
    b'tot_cos1_03':     b'Wario',
    b'tot_cos1_04':     b'Waluigi',
    b'tot_cos1_05':     b'Fire Mario',
    b'tot_cos1_06':     b'Ice Mario',
    b'tot_cos1_07':     b'Bubble Mario',
    b'tot_cos1_08':     b'Superball Mario',
    b'tot_cos1_09':     b'Flying Mario',
    b'tot_cos1_10':     b'Classic Mario',
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
        b"Gives Mario's attacks a\n"
        b"tinny, dinging sound.",
    b'tot_cos0_02_h':
        b"Gives Mario's attacks a\n"
        b"croaking sound.",
    b'tot_cos0_03_h':
        b"Gives Mario's attacks a\n"
        b"mouse-like squeaking sound.",
    b'tot_cos0_04_h':
        b"Gives Mario's attacks a\n"
        b"cheerful tweeting sound.",
    b'tot_cos0_05_h':
        b"Gives Mario's attacks a\n"
        b"doleful cawing sound.",
    b"tot_cos0_06_h":
        b"Gives Mario's attacks a\n"
        b"greedy oinking sound.",
    b"tot_cos0_07_h":
        b"Gives Mario's attacks an\n"
        b"unnerving swarming sound.",
    b"tot_cos0_08_h":
        b"Gives Mario's attacks an\n"
        b"evil cackling sound.",
    b"tot_cos0_09_h":
        b"Gives Mario's attacks a\n"
        b"chuckling sound.",
    b"tot_cos0_10_h":
        b"Gives Mario's attacks a\n"
        b"screaming sound.",
    b"tot_cos0_11_h":
        b"Gives Mario's attacks a\n"
        b"whistling sound.",
    b"tot_cos0_12_h":
        b"Gives Mario's attacks a\n"
        b"variety of digital sounds.",
    b"tot_cos0_13_h":
        b"Gives Mario's attacks a\n"
        b"variety of familiar sounds.",
    b"tot_cos0_14_h":
        b"Gives Mario's attacks a\n"
        b"re-invigorating sound.",
    b"tot_cos0_15_h":
        b"Gives Mario's attacks a\n"
        b"variety of popping sounds.",
    b"tot_cos0_16_h":
        b"Gives Mario's attacks a\n"
        b"variety of zippy sounds.",
    b"tot_cos0_17_h":
        b"Gives Mario's attacks a\n"
        b"booming explosive sound.",
    b"tot_cos0_18_h":
        b"Gives Mario's attacks a\n"
        b"variety of bouncy sounds.",
    b"tot_cos0_19_h":
        b"Gives Mario's attacks a\n"
        b"variety of crashing sounds.",
    b"tot_cos0_20_h":
        b"Gives Mario's attacks a\n"
        b"meaty punching sound.",
    b"tot_cos0_21_h":
        b"Gives Mario's attacks a\n"
        b"variety of cartoony sounds.",
    b"tot_cos0_22_h":
        b"Gives Mario's attacks an\n"
        b"odiferous sound?",
    b"tot_cos0_23_h":
        b"Gives Mario's attacks a\n"
        b"quizzical jingle.",
    b"tot_cos0_24_h":
        b"Gives Mario's attacks a\n"
        b"variety of crystalline sounds.",
    b"tot_cos0_25_h":
        b"Gives Mario's attacks a\n"
        b"60's sci-fi sound.",
    b"tot_cos0_26_h":
        b"Gives Mario's attacks a\n"
        b"positively peachy sound.",
    b"tot_cos0_27_h":
        b"Gives Mario's attacks a\n"
        b"dreadful roaring sound.",
        
    b"tot_cos1_01_h":
        b"Red shirt, blue overalls.\n"
        b"Unquestionably iconic!",
    b"tot_cos1_02_h":
        b"Garb of housekeepers, exorcists,\n"
        b"and detectives' assisants?",
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
        b"Wear this and you're sure to be\n"
        b"one cool customer.",
    b"tot_cos1_07_h":
        b"The newest flower-based power.\n"
        b"Hot pink and red's a choice.",
    b"tot_cos1_08_h":
        b"Honestly, it's anyone's guess if\n"
        b"the color on this one's accurate.",
    b"tot_cos1_09_h":
        b"Way too cool-looking a suit to\n"
        b"only appear in one level.",
    b"tot_cos1_10_h":
        b"Going all the way back to\n"
        b"Brooklyn with this one!",
    b"tot_cos1_11_h":
        b"Luigi rocking classic white and\n"
        b"green, no Fire Flower required!",
    b"tot_cos1_12_h":
        b"Earthy tones and nary a bit of\n"
        b"blue in sight!",
    b"tot_cos1_13_h":
        b"Luigi cutting in on his bro's style,\n"
        b"and getting in some VS. action!",
    b"tot_cos1_14_h":
        b"Darker overalls to account for\n"
        b"the halogen stagelights!",
    b"tot_cos1_15_h":
        b"16-bits means so many more off-\n"
        b"reds and blues to choose from!",
    b"tot_cos1_16_h":
        b"Taking a break from tennis\n"
        b"refereeing to hit the links!",
    b"tot_cos1_17_h":
        b"Casual blues to complement the\n"
        b"putting greens.",
    b"tot_cos1_18_h":
        b"Longtime Smash fans still call\n"
        b'this outfit "brown" out of habit.',
    b"tot_cos1_19_h":
        b"Mario's classic colors with a\n"
        b"cotton-candy twist!",
    b"tot_cos1_20_h":
        b"Perfect for construction work,\n"
        b"but preferably with a hard-hat.",
    b"tot_cos1_21_h":
        b"Fun fact: Toadette ditched her\n"
        b"orange dress right after TTYD!",
    b"tot_cos1_22_h":
        b"Inspired by world champions' and\n"
        b"modders' favorite Pokemon alike.",
    b"tot_cos1_23_h":
        b"Remembering names was never\n"
        b"so easy!",
    b"tot_cos1_24_h":
        b"Nothing showier than a living\n"
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
        b"Reminiscent of an under-\n"
        b"appreciated Pokemon region.",
    b"tot_cos2_19_h":
        b"A steed of literal silver's\n"
        b"nothing to sneeze at!",
    b"tot_cos2_20_h":
        b"The envy of Midas, and yet\n"
        b"you're literally untouchable!",
    b"tot_cos2_21_h":
        b"Thanks for playing this mod!\n"
        b"You've really earned this!",
        
    # Misc.
    
    b"msg_nameent_3":
        b"Choose a name for the seed!",
        
    # Placeholder NPC dialogue (map-agnostic).
        
    b"tot_npc_generic":
        b"[Placeholder] I'm interactable.\n<k>",
}

g_TowerLobbyStrings = {
    # Area tattle. (TODO: Write final script)
    
    b"msg_kuri_map":
        b"Placeholder area tattle \xd0\n<k>",
        
    # General lobby text.
    
    b"tot_lobby_confirmstart_noopt":
        b"<system>\n<p>\n"
        b"Are you prepared to start?\n<o>",
    
    b"tot_lobby_confirmstart":
        b"<system>\n<p>\n"
        b"Are you prepared to start with\n"
        b"the options selected?\n<o>",
        
    b"tot_lobby_optyesno":
        b"<select 0 1 0 40>\nYes\nNo",
        
    # Run options menu.
    
    b"tot_intensity_normal":
        b"Increase intensity for more \n"
        b"     end-of-run rewards!",
    
    b"tot_intensity_light":
        b"Well, hope you have fun, \n"
        b"          at least!",
    
    b"tot_intensity_heavy":
        b"Good luck! You're going \n"
        b"        to need it!",
        
    b"tot_winsel_runoptions_header":    b"Options",
    
    b"tot_optr_seed":               b"Set Seed",
    b"tot_optr_seed_random":        b"0 (Set on start)",
    b"tot_optr_preset":             b"Preset",
    b"tot_optr_preset_custom":      b"Custom",
    b"tot_optr_preset_default":     b"Default",
    b"tot_optr_difficulty":         b"Tower Type",
    b"tot_optr_diff_half":          b"Hooktail (32f)",
    b"tot_optr_diff_full":          b"Gloomtail (64f)",
    b"tot_optr_diff_ex":            b"EX Difficulty (64f)",
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
    b"tot_optr_charlie_2":          b"Tiny",
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
        b"X: Default        Y: Randomize\n"
        b"Z: Bub-ulb name L/R: Shift 10x",
    
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
        
    b"tot_lobby_opttutorial":
        b"<system>\n<p>\n"
        b"Use this sign to customize\n"
        b"various options for your next\n"
        b"tower run to your liking!\n<k>\n<p>\n"
        b"Press left/right to change an\n"
        b"option, X to revert it to its\n"
        b"default, and B to back out.\n<k>\n<p>\n"
        b"Certain options will increase\n"
        b"or decrease a run's Intensity,\n"
        b"affecting the rewards earned.\n<k>\n<p>\n"
        b"Shoot for 100% completion,\n"
        b"or experiment with different\n"
        b"options just for fun!\n<k>",
    
    b"tot_lobby_frontsign":
        b'<kanban>\n'
        b'<pos 116 10>\n"Battle Tower"\n'
        b'<pos 77 39>\nUnder Construction\n<k>',
    
    b"tot_lobby_backsign":
        b'<kanban>\n'
        b'Seed: <col 0000ffff>\n'
        b'%09d\n</col>\n'
        b'Options: <col 0000ffff>\n'
        b'%s\n</col>\n'
        b'<k>',
    
    b"tot_lobby_reentry":
        b"<system>\n<p>\n"
        b"You can't re-enter that way!\n<k>",
}

g_TowerStrings = {
    # NPC tattles.

    b"npc_shop":
        b"<keyxon>\nThat's Charlieton, the\n"
        b"merchant.<dkey><wait 300></dkey> I guess he sells his\n"
        b"wares here, too.\n<k>\n<p>\n"
        b"But... <wait 100>it looks like his stuff\n"
        b"gets more expensive the\n"
        b"higher we get in the tower.\n<k>\n<p>\n"
        b"I'm happy to be able to shop\n"
        b"here and all, but sheesh...\n"
        b"<wait 300>\n"
        b"Well, it's your money.\n<k>",
        
    b"npc_lumpy":
        b"That's Lumpy. <dkey><wait 300></dkey>\n"
        b"He says if\n"
        b"you invest in him, he'll pay\n"
        b"back big dividends later...\n<k>\n<p>\n"
        b"He does seem trustworthy,\n"
        b"but what if we don't end up\n"
        b"seeing him again? <wait 250>Hmm...\n<k>",
        
    b"npc_doopliss":
        b"<keyxon>\nThat's Doopliss. <dkey><wait 300></dkey>\n"
        b"His magic can\n"
        b"make more chests appear, but\n"
        b"will make enemies stronger!\n<k>\n<p>\n"
        b"What do you think, Mario?\n"
        b"<wait 150>Is it worth the risk?\n<k>",
        
    b"npc_grubba":
        b"<keyxon>\nThat's Grubba. <dkey><wait 300></dkey>\n"
        b"He'll give us\n"
        b"special fighting conditions\n"
        b"for the next few floors.\n<k>\n<p>\n"
        b"If we can pull them off, we'll\n"
        b"win double the normal coins,\n"
        b"but if we fail, we get nothing!\n<k>",
        
    b"npc_chet":
        b"<keyxon>\nThat's Chet Rippo. <dkey><wait 300></dkey>\n\n"
        b"He's absolutely loaded with\n"
        b"extra Shine Sprites!\n<k>\n<p>\n"
        b"We'll have to give up a level\n"
        b"of one of our stats if we want\n"
        b"them, though. <wait 100>Yikes!\n<k>\n<p>\n"
        b"At least he's not literally\n"
        b"asking for an arm and a leg,\n"
        b"I guess?\n<k>",
        
    b"npc_wonky":
        b"<keyxon>\nThat's Wonky. <dkey><wait 300></dkey>\n"
        b"He's really\n"
        b"gotten into collecting stuff,\n"
        b"but isn't very... <wait 150>discerning.\n<k>\n<p>\n"
        b"He'll take our unwanted items\n"
        b"or badges for a few coins.\n"
        b"<wait 250>One man's trash, as they say!\n<k>",
        
    b"npc_dazzle":
        b"<keyxon>\nThat's Dazzle. <dkey><wait 300></dkey>\n"
        b"He's hoping\n"
        b"to offload some of his Star\n"
        b"Piece collection for coins.\n<k>\n<p>\n"
        b"He'll give you a freebie to\n"
        b"start, but will raise his price\n"
        b"once he thinks you're hooked.\n<k>\n<p>\n"
        b"The places folks will go\n"
        b"these days to make money...\n"
        b"<wait 200>kinda wild, don'tcha think?\n<k>",
        
    b"npc_mover":
        b"<keyxon>\nHe's a Mover. <dkey><wait 300></dkey>\n"
        b"Word has it,\n"
        b"he cracked the tower's locks\n"
        b"the day it opened. <wait 150>Wow.\n<k>\n<p>\n"
        b"He's asking a hefty price for\n"
        b"his keys, but I dunno, <wait 150>skipping a\n"
        b"hard fight could be worth it!\n<k>",
        
    b"npc_zess":
        b"<keyxon>\nThat's Zess T., the chef.\n"
        b"<dkey><wait 300></dkey>\n"
        b"Hope she's still not mad about\n"
        b"that contact lens incident...\n<k>\n<p>\n"
        b"She's got an all-new recipe\n"
        b"book, and can cook stuff\n"
        b"using any common item!\n<k>\n<p>\n"
        b"She can also make signature\n"
        b"dishes with rarer items, and\n"
        b"maybe even wilder stuff!\n<k>",
    
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
        b"<p>\nYou haven't got any items!\n"
        b"<wait 250>Oh, don't tease me so!\n<k>",
        
    b"tot_wonky_nobadges":
        b"<p>\nMy friend, if you had any\n"
        b"badges on you, I'd know it,\n"
        b"<wait 200>believe me!\n<k>",
        
    b"tot_wonky_whichitem":
        b"<p>\nOoh, let me see! Which one?\n<k>",
        
    b"tot_wonky_itemok":
        b"Oh, that's nice! <wait 250>I'd say\n"
        b"your <ITEM> is\n"
        b"worth... <NUM> coin<S>, <wait 150>yes?\n<o>",
        
    b"tot_wonky_itemdifferent":
        b"<p>\nIs that so? How very sad...\n"
        b"<wait 250>Perhaps something else?\n<k>",
        
    b"tot_wonky_thanksnext":
        b"<p>\nAnything else I can take\n"
        b"off your hands?\n<o>",
        
    b"tot_wonky_thankslast":
        b"<p>\nDon't be a stranger!\n<k>",
    
    b"tot_wonky_exit":
        b"<p>\nDon't be a stranger!\n<k>",
        
    b"tot_chet_intro":
        b"My, <wait 150>ahem, <wait 150>mystical artifacts\n"
        b"aren't for just anyone to\n"
        b"walk up and stare at, <wait 150>bud.\n<k>\n<p>\n"
        b"I could let <wait 150>o<wait 110>n<wait 110>e of these\n"
        b"Shine Sprites go, but it'll cost\n"
        b"you a stat level. <wait 200>Deal?\n<o>",
        
    b"tot_chet_nostats":
        b"<p>\nYou don't even have any\n"
        b"stats to lower, man! <wait 250>Stop\n"
        b"wasting my time!\n<k>",
        
    b"tot_chet_nocoins":
        b"<p>\nYou're broke, buster! Hit\nthe road!\n<k>",
    
    b"tot_chet_whichstat":
        b"<p>\nAh, now we're in business!\n"
        b"Which stat are you gonna\n"
        b"give up?\n<k>",
    
    b"tot_chet_different":
        b"<p>\nHmph! <wait 200>Aren't we choosy?\n"
        b"<wait 250>Anything else, then?\n<k>",
    
    b"tot_chet_confirm":
        b"Your %s, eh? <wait 150>All right,\n"
        b"it'll fall from level %d to %d\n"
        b"in exchange for a Shine Sprite.\n<k>\n<p>\n"
        b"The adjustment fee will be\n"
        b"%d coins. <wait 200>Deal?\n<o>",
        
    b"tot_chet_decline":
        b"<p>\nThen get outta here!\n<k>",
        
    b"tot_chet_exit":
        b"<p>\nNow get outta here!\n<k>",
        
    b"tot_dazzle_intro":
        b"I'm Dazzle, the famous\n"
        b"Star Piece collector\n"
        b"extraordinaire!\n<k>\n<p>\n"
        b"I'll give you one of these\n"
        b"beauties for free, but any\n"
        b"more'll cost you! Want one?\n<o>",
        
    b"tot_dazzle_offer":
        b"Oh, what fun! I can let you\n"
        b"have another Star Piece for\n"
        b"%d coins. Deal?\n<o>",
        
    b"tot_dazzle_nocoins":
        b"<p>\nYou're too low on coins!\n"
        b"<wait 200>I can't let something this\n"
        b"pretty go for such a sum...\n<k>",
        
    b"tot_dazzle_decline":
        b"<p>\nOh, too bad! I can't imagine\n"
        b"the sorrow you must feel,\n"
        b"<wait 200>forsaken by the stars!\n<k>",
        
    b"tot_grubba_intro":
        b"Well, howdy, Gonzales! Fancy\n"
        b"meetin' you here in this here\n"
        b"tower! <wait 200>Howzabout a deal?\n<k>\n<p>\n"
        b"You keep up my patented,\n"
        b"crowd-pleasin' fighting rules,\n"
        b"<wait 150>and get 2x the coins! <wait 250>Wham!\n<k>\n<p>\n"
        b"If you don't meet 'em, you'll\n"
        b"wind up with a sack o' nothin'.\n"
        b"What say ya, Gonzales?\n<o>",
        
    b"tot_grubba_accept":
        b"<p>\nNow, <wait 100>THAT's <wait 100>what I like to\n"
        b"hear, champ! <wait 200>You get out\n"
        b"there and whoop 'em for me!\n<k>",
        
    b"tot_grubba_decline":
        b"<p>\nNow come on, son, <wait 150>where's\n"
        b"yer fightin' spirit?\n<k>",
        
    b"tot_grubba_active":
        b"Get out there and show\n"
        b"those yokels the stuff yer\n"
        b"yer made of, Gonzales!\n<k>",
        
    b"tot_doopliss_intro":
        b"Heya, Slick!\n"
        b"Looking for a challenge?\n<k>\n<p>\n"
        b"I'll use my magic to make\n"
        b"the enemies on the next set of\n"
        b"floors <wait 30>\n"
        b"<dynamic 3>extra</dynamic> tough!\n<k>\n<p>\n"
        b"In exchange, I'll make it so\n"
        b"more chests appear on every\n"
        b"floor. <wait 150>You game, pal?\n<o>",
        
    b"tot_doopliss_accept":
        b"<p>\nYou got it, Slick! Here goes!\n<k>",
        
    b"tot_doopliss_decline":
        b"<p>\nAw, you're no fun!\n<k>",
        
    b"tot_doopliss_active":
        b"Hee hee! Don't get clobbered\n"
        b"out there!\n<k>",
        
    b"tot_lumpy_intronocoin":
        b"Hello! I'm Lumpy. <wait 200>If you lend\n"
        b"me all your coins, I'll pay you\n"
        b"back double later, <wait 200>I promise!\n<k>\n<p>\n"
        b"Or at least I would, if you\n"
        b"had any coins on you right now.\n"
        b"Good luck on your travels!\n<k>",
        
    b"tot_lumpy_intro":
        b"Hello! I'm Lumpy. <wait 200>If you lend\n"
        b"me all your coins, I'll pay you\n"
        b"back double later, <wait 200>I promise!\n<k>\n<p>\n"
        b"Sound like a deal?\n<o>",
        
    b"tot_lumpy_reward":
        b"Oh, hey! Thanks for the\n"
        b"loan earlier. <wait 200>Here's the cash\n"
        b"back, with 100% interest!\n<k>\n<o>",
        
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
        b"<wait 250>I know a few things about\n"
        b"getting through this tower.\n<k>\n<p>\n"
        b"For a few coins, I can get you\n"
        b"the key to where you wanna go.\n"
        b"<wait 250>\nWhaddya thinkin'?\n<o>",
        
    b"tot_mover_menu":
        b"<select 0 2 0 40>\n"
        b"Tower Key (skips 1 non-midboss)\n"
        b"Master Key (skips any 1 floor)\n"
        b"I'm good, thanks!",
        
    b"tot_mover_decline":
        b"<p>\nAw, that's too bad.\n"
        b"<wait 250>Guess the secrets of the\n"
        b"tower will remain a mystery...\n<k>",
        
    b"tot_mover_offer0":
        b"<p>\nA Tower Key, huh?\n"
        b"That'll run you %d coins.\n"
        b"<wait 250>Do we have a deal?\n<o>",
        
    b"tot_mover_offer1":
        b"<p>\nAh, the Master Key.\n"
        b"That'll run you %d coins.\n"
        b"<wait 250>Do we have a deal?\n<o>",
        
    b"tot_mover_nocoins":
        b'<p>\n'
        b'Hey! <wait 100>You need more coins.\n'
        b"<wait 250>Sorry, but a guy's gotta make\n"
        b'a living, know what I mean?\n'
        b'<k>',

    b'tot_mover_success':
        b"Awright! <wait 150>My secret paths\n"
        b"are... <wait 150>secret!<wait 250> We never spoke,\n"
        b"<wait 150>capiche?\n<k>",
        
    b"tot_mover_active":
        b"We never spoke, <wait 150>got it?\n<k>",
        
    b"tot_mover_full_inv":
        b'<p>\n'
        b"Hey! <wait 100>You're looking loaded up\n"
        b"on gear already.<wait 250> You still sure\n"
        b"ya got room for this?\n<o>",
        
    b"tot_zess_intro":
        b"I'm Zess T.<wait 250> I love to cook!\n"
        b"<wait 200>You provide the items, and\n"
        b"I work my magic.\n<k>\n<p>\n"
        b"What can I do for you\n"
        b"today, <wait 200>Twinkle-toes?\n<o>",
        
    b"tot_zess_intronoitems":
        b"I'm Zess T.<wait 250> I love to cook!\n"
        b"<wait 200>You provide the items, and\n"
        b"I work my magic.\n<k>\n<p>\n"
        b"It seems you don't have any\n"
        b"I can work with at the moment,\n"
        b"though. Next time, Bigfoot!\n<k>",
        
    b"tot_zess_decline":
        b"<p>\nOh, that's too bad!\n<k>",
        
    b"tot_zess_nochooserecipe":
        b"Indecisive, are we? <wait 250>Oh well,\n"
        b"I'll be here if you make up\n"
        b"your mind later, Squashy...\n<k>",
        
    b"tot_zess_which1st":
        b"<p>\nWhich would you like to use?\n<k>",
        
    b"tot_zess_which2nd":
        b"Mm-hmm, got it. <wait 250>What else?\n<k>",
        
    b"tot_zess_onlyrecipe":
        b"Ah yes, I think I've got a\n"
        b"recipe for that. <wait 200>This tickle\n"
        b"your fancy, Mr. Left-feet?\n<k>",
        
    b"tot_zess_whichrecipe":
        b"I've got a couple dishes\n"
        b"that use that ingredient.\n"
        b"<wait 250>Which one do you want?\n<k>",
    
    b"tot_zess_giveup":
        b"What's wrong, Cap'n Klutz?\n"
        b"Still want me to cook or not?\n<o>",
        
    b"tot_zess_confirmcost":
        b"All right, Tootsies, I'll\n"
        b"cook that right up for you\n"
        b"for %d coins.<wait 250> OK?\n<o>",
        
    b"tot_zess_declinecost":
        b"<p>\nHmph, <wait 150>my art isn't free.\n"
        b"<wait 250>Come back if you change\n"
        b"your mind, Dr. Doofus!\n<k>",
        
    b"tot_zess_confirmswap":
        b"All right, I'll swap that\n"
        b"%s around\n"
        b"for you, free of charge. OK?\n<o>",
        
    b"tot_zess_cookagain":
        b"Want me to cook anything\n"
        b"else?\n<o>",
        
    b"tot_zess_nomoretocook":
        b"Well, I've done all I can do.\n"
        b"<wait 200>Enjoy the fruit of my culinary\n"
        b"magic, Contact-Lens Killer!\n<k>\n<p>\n"
        b"Okay, admittedly that one\n"
        b"was a bit on the nose...\n<k>",
        
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
}

g_Tower1Strings = {
    # Goombella area tattle.
    b"msg_kuri_map":
        b"This is Hooktail Castle. <wait 250>Well,\n"
        b"at least it was at some point...\n"
        b"<wait 200>it's more of a tower now.\n<k>\n<p>\n"
        b"No telling how far up this\n"
        b"goes, but let's stick it out and\n"
        b"show that dragon who's boss!\n<k>",
}

g_Tower2Strings = {
    # Goombella area tattle.
    b"msg_kuri_map":
        b"This is Hooktail Castle.\n"
        b"<wait 250>It seems the decor's changed...\n"
        b"<wait 200>looks kind of nice, honestly.\n<k>\n<p>\n"
        b"We've made a good bit of\n"
        b"progress, Mario, but we can't\n"
        b"stop now! <wait 200>C'mon!\n<k>",
}

g_Tower3Strings = {
    # Goombella area tattle.
    b"msg_kuri_map":
        b"This is Hooktail Castle.\n"
        b"<wait 250>Ugh, this color combination\n"
        b"is making me sick.\n<k>\n<p>\n"
        b"We've got to be more than\n"
        b"halfway up by now, right?\n"
        b"<wait 200>Let's keep powering through...\n<k>",
}

g_Tower4Strings = {
    # Goombella area tattle.
    b"msg_kuri_map":
        b"This is Hooktail Castle.\n"
        b"<wait 250>Wow, the atmosphere here's\n"
        b"really intense!\n<k>\n<p>\n"
        b"We must be close now, but\n"
        b"these fights are really tough!\n"
        b"<wait 200>Don't let your guard down!\n<k>",
}

g_TowerBossFloorStrings = {
    # Field dialogue.
    
    b"tot_field_dragon00_00":
        b"[Placeholder] Encounter<k>",
    
    b"tot_field_dragon01_00":
        b"[Placeholder] Encounter<k>",
    
    b"tot_field_dragon02_00":
        b"[Placeholder] Encounter<k>",
        
    b"tot_field_gfz_00":
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
}

g_PetalburgWestStrings = {

    # "First visit" cutscene.
    
    b"tot_town_firstvisit_00":
        b"Oh, Mario! <wait 100>You got here\n"
        b"just in time! <wait 200>I suppose you're\n"
        b"aware that dragon's back!?\n<k>",
    
    b"tot_town_firstvisit_01":
        b"Oh, it's terrible! It seems\n"
        b"the castle's been fortified\n"
        b"considerably since last time.\n<k>\n<p>\n"
        b"Tons of rooms, <wait 100>overrun with\n"
        b"all sorts of enemies, <wait 200>and\n"
        b"who knows what else?\n<k>\n<p>\n"
        b"I don't know what she's\n"
        b"planning, <wait 150>but something's\n"
        b"gotta be done about it!\n<k>",
    
    b"tot_town_firstvisit_02":
        b"I'm glad to hear you say\n"
        b"that, Mario. <wait 150>We all owe you\n"
        b"an eternal debt already!\n<k>\n<p>\n"
        b"I know there's not much we\n"
        b"can do to help right now, <wait 150>but\n"
        b"we're behind you all the way!\n<k>\n<p>\n"
        b"Please do come on back if you\n"
        b"run into trouble! <wait 200>Rest assured,\n"
        b"we'll keep this town safe.\n<k>",

    # Shopkeeper dialogue.

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

    b"tot_shopkeep_19":
        b"<p>\nYou've been quite the repeat\n"
        b"customer! As a reward, <k>",

    b"tot_shopkeep_22":
        b"<p>\nCome again, OK?\n<k>",

    b"tot_shopkeep_24":
        b"<select 0 1 0 40>\nYes\nNo",

    b"tot_shopkeep_30":
        b"<p>\nYou've been quite the repeat\n"
        b"customer! Here, take these as\n"
        b"a reward, on the house!\n<k>",

    b"tot_shopkeep_31":
        b"Thank you so very much!\n<k>",
        
    b"tot_shopkeep_closed":
        b"Hiya, Mario! <wait 200>I haven't had\n"
        b"any customers recently, <wait 150>so I\n"
        b"haven't set up shop just yet.\n<k>\n<p>\n"
        b"Come back in a little while,\n"
        b"and I'll make sure to have\n"
        b"everything shop-shape! <wait 200>Ha!\n<k>",

    b"tot_shopkeep_tut":
        b"Hiya, Mario! <wait 200>I've redone the\n"
        b"the place recently, <wait 100>so let me\n"
        b"give you a quick run-down.\n<k>\n<o>",
        
    b"tot_shopkeep_tut_body":
        b"<p>\nIf you've come across an item\n"
        b"or badge in the tower, you\n"
        b"might see it on the shelf here.\n<k>\n"
        b"<p>\nIf something catches your eye,\n"
        b"just walk up to it and I can\n"
        b"sell it for a top-notch price!\n<k>\n"
        b"<p>\nItems don't have any use here\n"
        b"in town, but you can select\n"
        b"a few to bring into the tower.\n<k>\n"
        b"<p>\nFinally, if there's an item you\n"
        b"know you've seen before, but\n"
        b"it isn't on display right now,\n<k>\n"
        b"<p>\nread that sign back there to\n"
        b"get it on back-order. It'll cost\n"
        b"you a bit extra, though!\n<k>\n"
        b"<p>\nWant me to run that all\n"
        b"by you again?\n<o>",

    b"tot_shopkeep_tutrepeat":
        b"<select 1 1 0 40>\nJust once more!\nNope, I'm good!",

    b"tot_shopkeep_nobackorder":
        b"I'm afraid there's nothing\n"
        b"ready for back-order right now.\n<wait 300>\n"
        b"Feel free to browse, though!\n<k>",

    b"tot_shopkeep_generic":
        b"Welcome! Feel free to browse\n"
        b"the items on display, or put in\n"
        b"a back-order from the sign.\n<k>",

    b"tot_shopkeep_yesno":
        b"<select 0 1 0 40>\nYes\nNo",
        
    # Bub-ulber dialogue.
    
    # Special dialogue.
    
    b"tot_bubulb_firsttalk":
        b"Top of the morning!\n"
        b"<wait 250>'Tis I, Bub-ulber.\n<k>\n<p>\n"
        b"This may not come as a\n"
        b"surprise, but I consider myself\n"
        b"a connoisseur of seeds.\n<k>\n<p>\n"
        b"Feel free to come by any\n"
        b"time you want and I'll share\n"
        b"my seeding knowledge!\n<k>",
    
    b"tot_bubulb_unlockseed":
        b"<p>\nBy the by, <wait 150>it seems that\n"
        b"you share my great interest\n"
        b"in all things seeds!\n<k>\n<p>\n"
        b"Why don't you come and\n"
        b"help me name one sometime?\n<k>\n<p>\n"
        b"You might find that you'll\n"
        b"come to better understand\n"
        b"them that way as well!\n<k>",
    
    b"tot_bubulb_seedtut":
        b"<system>\n<p>\n"
        b"You've unlocked the ability to\n"
        b"set a seed for your future\n"
        b"tower runs!\n<k>\n<p>\n"
        b"Using a set seed will guarantee\n"
        b"certain random aspects will\n"
        b"always play out consistently.\n<k>\n<p>\n"
        b"Use the run options sign to\n"
        b"enter a number, or talk to\n"
        b"Bub-ulber to choose a name!\n<k>",
    
    b"tot_bubulb_setseed":
        b"<p>\nOh, were you interested in\n"
        b"choosing a name for your\n"
        b"next seed?\n<o>",

    b"tot_bubulb_earlyend":
        b"<p>\nThat's all the advice I have\n"
        b"for now. <wait 250>Please do enjoy\n"
        b"this most glorious day!\n<k>",

    b"tot_bubulb_goodbye":
        b"<p>\nVery well then, <wait 200>please do\n"
        b"enjoy this most glorious day!\n<k>",

    b"tot_bubulb_nameset":
        b'"%s", eh?\n'
        b"That sounds like a fine name!\n"
        b"I'll be sure to use it next time.\n<k>",
    
    # Regular dialogue.
    
    b"tot_di001100_00":
        b"Good morning to you, sir!\n"
        b"<wait 250>Care to learn about how\n"
        b"seeds affect enemy groups?\n<k>\n<p>\n"
        b"Each type of enemy has its\n"
        b"own level ranging from 2-10,\n"
        b"indicating their toughness.\n<k>\n<p>\n"
        b"As you ascend the tower,\n"
        b"every floor has a quota of\n"
        b"enemy levels it's aiming for.\n<k>\n<p>\n"
        b"Once half of this target is\n"
        b"reached, enemies may stop\n"
        b"being added at any time.\n<k>\n<p>\n"
        b"This means the fight might\n"
        b"be easier, but you'll also get\n"
        b"fewer coins and chests!\n<k>\n<o>",
        
    b"tot_di001101_00":
        b"A glorious day to you, sir!\n"
        b"<wait 250>Care to learn about how\n"
        b"seeds affect reward types?\n<k>\n<p>\n"
        b"Every floor in the tower has\n"
        b"a selection of chest rewards,\n"
        b"<wait 150>generally from one to three.\n<k>\n<p>\n"
        b"Generally speaking, the\n"
        b"harder the enemies on a floor,\n"
        b"<wait 100>the more choices you'll have!\n<k>\n<p>\n"
        b"You might not have noticed\n"
        b"this, <wait 150>but only one stat level\n"
        b"increase can appear at once.\n<k>\n<p>\n"
        b"The same goes for moves;\n"
        b"<wait 150>you'll never see a Jump and\n"
        b"partner move together.\n<k>\n<o>",
    
    b"tot_di001102_00":
        b"A fantastic day to you, sir!\n"
        b"<wait 250>Care to learn about how\n"
        b"seeds affect partner pools?\n<k>\n<p>\n"
        b"As the name suggests, after\n"
        b"recruiting a certain number of\n"
        b"partners, only they'll appear.\n<k>\n<p>\n"
        b"Hooktail's tower has a 3-\n"
        b"partner pool, and the others\n"
        b"have a 4-partner pool.\n<k>\n<p>\n"
        b"This may seem limiting, <wait 150>but\n"
        b"it gives you more chances to\n"
        b"give them additional moves!\n<k>\n<p>\n"
        b"Waiting for your favorites\n"
        b"can work too, but will probably\n"
        b"net you fewer moves in total.\n<k>\n<o>",
    
    b"tot_di001103_00":
        b"A wonderful day to you, sir!\n"
        b"<wait 250>Care to learn about how\n"
        b"seeds affect partner moves?\n<k>\n<p>\n"
        b"You might have noticed\n"
        b"partners' moves have colored\n"
        b"orbs next to their names...\n<k>\n<p>\n"
        b"These indicate the move's\n"
        b"rarity, with blue for Tier 1,\n"
        b"green for 2, and red for 3.\n<k>\n<p>\n"
        b"You won't be offered higher-\n"
        b"rarity moves for the first\n"
        b"additional move on a partner.\n<k>\n<p>\n"
        b"However, the second and\n"
        b"third moves taken will always\n"
        b"offer a tier-2 and 3 option!\n<k>\n<p>\n"
        b"Partners have different\n"
        b"rarity distributions, so mind\n"
        b"that when aiming for a move!\n<k>\n<o>",
    
    b"tot_di001104_00":
        b"A cheery day to you, sir!\n"
        b"<wait 250>Care to learn about how\n"
        b"seeds affect Special moves?\n<k>\n<p>\n"
        b"Special Moves are offered\n"
        b"pretty rarely from chests,\n"
        b"signified by their Crystal Star.\n<k>\n<p>\n"
        b"Taking one will grant you its\n"
        b"move, and an additional unit\n"
        b"of max Star Power, up to 9.\n<k>\n<p>\n"
        b"Special Moves can only be\n"
        b"offered in an order allowing\n"
        b"them to be used at Lvl. 3.\n<k>\n<p>\n"
        b"Because of that, if you choose\n"
        b"not to take one, it'll appear\n"
        b"again next time you see one!\n<k>\n<p>\n"
        b"So if you want a chance at\n"
        b"later Special Moves, be sure\n"
        b"to take every one you see!\n<k>\n<o>",
    
    b"tot_di001105_00":
        b"Top of the morning!\n"
        b"<wait 250>Care to learn about how\n"
        b"seeds affect guest NPCs?\n<k>\n<p>\n"
        b"Every run's rest floors will\n"
        b"have guest cameos by up to\n"
        b"4 characters <wait 150>(3 for Hooktail).\n<k>\n<p>\n"
        b"Generally, you'll see them all\n"
        b"about equally, with none able\n"
        b"to appear more than 5 times.\n<k>\n<p>\n"
        b"However, some have a higher\n"
        b"likelihood of showing up only\n"
        b"a couple times, such as Lumpy.\n<k>\n<p>\n"
        b"If you're lucky enough to\n"
        b"see a guest you want, don't\n"
        b"hesitate to take their deal.\n<k>\n<p>\n"
        b"You might not get another\n"
        b"chance, after all!\n<k>\n<o>",
    
    b"tot_di001106_00":
        b"Good day to you, sir!\n"
        b"<wait 250>Care to learn about how\n"
        b"seeds affect items' rarity?\n<k>\n<p>\n"
        b"There are 40 types of\n"
        b'items that are "common", and\n'
        b'28 types that are "rare".\n<k>\n<p>\n'
        b"Enemies are several times\n"
        b"more likely to hold common\n"
        b"items than rare items.\n<k>\n<p>\n"
        b"Charlieton's shop also has\n"
        b"only a couple of them on sale\n"
        b"at any time.\n<k>\n<p>\n"
        b"You know, I've heard there's\n"
        b"a master chef that can turn\n"
        b"common items into rare items...\n<k>\n<p>\n"
        b"They might even be able to\n"
        b"whip up something super-special\n"
        b"if you have multiple rare ones!\n<k>\n<o>",
    
    b"tot_di001107_00":
        b"A pleasant day to you, sir!\n"
        b"<wait 250>Care to learn about how\n"
        b"seeds affect retries?\n<k>\n<p>\n"
        b"If things go sour on a tower\n"
        b"run, you can always try again\n"
        b"from the last Save Block.\n<k>\n<p>\n"
        b"You can also retry without\n"
        b"reloading, <wait 150>using checkpoints\n"
        b"saved after every boss.\n<k>\n<p>\n"
        b"If you do either, you'll notice\n"
        b"most things will play out the\n"
        b"same as before the retry.\n<k>\n<p>\n"
        b"This includes enemy groups,\n"
        b"<wait 150>item drops, <wait 150>bonus conditions,\n"
        b"<wait 150>and most types of rewards.\n<k>\n<p>\n"
        b"Feel free to use this to your\n"
        b"advantage if you're struggling\n"
        b"with a difficult run!\n<k>\n<o>",
    
    b"tot_di001108_00":
        b"A blessed day to you, sir!\n"
        b"<wait 250>Care to learn about how\n"
        b"seeds affect unique badges?\n<k>\n<p>\n"
        b"Most badges can be stacked,\n"
        b"and can appear on enemies\n"
        b"and as bonus rewards.\n<k>\n<p>\n"
        b"However, there are a few\n"
        b"special badges you can only\n"
        b"get a single copy of in a run.\n<k>\n<p>\n"
        b"These include Double Dip,\n"
        b"<wait 150>Spike Shield, <wait 150>Quick Change,\n"
        b"<wait 150>Zap Tap, and a few others.\n<k>\n<p>\n"
        b"These badges can only be\n"
        b"obtained as chest rewards, or\n"
        b"rarely from Charlieton's shop.\n<k>\n<p>\n"
        b"If you run into Wonky, you can\n"
        b"sell these like any other badge,\n"
        b"but they won't appear again!\n<k>\n<o>",
    
    b"tot_di001109_00":
        b"A fabulous day to you, sir!\n"
        b"<wait 250>Care to learn about how\n"
        b"broad seeds' influence is?\n<k>\n<p>\n"
        b"You might not be surprised\n"
        b"to find out that seeds control\n"
        b"enemy groups or rewards...\n<k>\n<p>\n"
        b"But did you know that seeds\n"
        b"affect starting items on the\n"
        b'"Random" setting as well?\n<k>\n<p>\n'
        b"The same goes for most other\n"
        b"sources of random items, like\n"
        b"Ms. Mowz's Kiss Thief.\n<k>\n<p>\n"
        b"I've heard that if you see all\n"
        b"kinds of items and badges, you\n"
        b"can shuffle their identities.\n<k>\n<p>\n"
        b"I'd bet you 1,000 coins that\n"
        b"even that shuffling process\n"
        b"is controlled by seeds!\n<k>\n<o>",
       
    # TODO: More Koopa dialogue, regular NPC dialogue.
}

g_PetalburgEastStrings = {
    # Cosmetic shopkeepers.

    b"tot_shopkeep_yesno":
        b"<select 0 1 0 40>\nYes\nNo",
        
    b"tot_cshopkeep_tut1":
        b"Hi, Mario! <wait 150>So nice of you to\n"
        b"take on those big, <wait 170>bad <wait 170>dragons\n"
        b"up in their fancy towers!\n<k>"
        b"<p>\nRemember us, the Traveling \n"
        b"Sisters 3?\n<k>",
        
    b"tot_cshopkeep_tut2":
        b"We're finally settling down here\n"
        b"in Petalburg, and we've got lots\n"
        b"of goodies for sale! \xd0\n<k>",
        
    b"tot_cshopkeep_tut3":
        b"We each specialize in different\n"
        b"types of special flair. <wait 250>Outfits,\n"
        b"<wait 120>musical trinkets,<wait 120> you name it!\n<k>\n"
        b"<p>\nWhy don't you bring us some\n"
        b"shiny Star Pieces and take\n"
        b"a look sometime? \xde\n<k>",
        
    b"tot_cshop0_10":
        b"Hi, Mario! <wait 150>I'm afraid I'm fresh\n"
        b"out of those musical doodads\n"
        b"right now! \xd0\n<k>",
        
    b"tot_cshop1_10":
        b"Hi, Mario, looking sharp!\n"
        b"<wait 200>I'm out of new outfits at\n"
        b"the moment.\n<k>"
        b"<p>\nYou're really rocking that\n"
        b"look right now, though!\n<k>",
        
    b"tot_cshop2_10":
        b"Hi, Mario! \xde\n"
        b"<wait 150>I haven't come across any\n"
        b"new Yoshi varieties recently!\n<k>"
        b"<p>\nPerhaps another time!\n<k>",
        
    b"tot_cshop0_11":
        b"Hi, Mario! <wait 150>Any of these\n"
        b"sound good to you? \xd0\n<k>",
        
    b"tot_cshop1_11":
        b"Hi, Mario! <wait 150>What new outfits\n"
        b"were you thinking about?\n<k>",
        
    b"tot_cshop2_11":
        b"Hi, Mario! <wait 150>I love seeing all the\n"
        b"different colors Yoshis come in.\n"
        b"Any catching your eye? \xde\n<k>",

    b"tot_cshop0_12":
        b"It's <NUM> Star Piece<S> for\n"
        b"the <ITEM> pack.\n<wait 250>\n"
        b"Sound good? \xd0\n<o>",

    b"tot_cshop1_12":
        b"It's <NUM> Star Piece<S> for\n"
        b"that <ITEM> outfit.\n<wait 250>\n"
        b"What do you think? Is it you?\n<o>",

    b"tot_cshop2_12":
        b"It's <NUM> Star Piece<S> for\n"
        b"the <ITEM> Yoshi variety.\n<wait 250>\n"
        b"What do you think? \xde\n<o>",
        
    b"tot_cshop0_13":
        b"<p>\nAhh, that's not quite\n"
        b"enough! <wait 200>So sorry! \xd0\n<k>",
        
    b"tot_cshop1_13":
        b"<p>\nCan't go that low, I'm afraid!\n<k>",
        
    b"tot_cshop2_13":
        b"<p>\nAfraid you're a little short,\n<wait 200>sorry! \xde\n<k>",
        
    b"tot_cshop0_15":
        b"<p>\nWant anything else? \xd0\n<k>",
        
    b"tot_cshop1_15":
        b"<p>\nAnything else suit you?\n<k>",
        
    b"tot_cshop2_15":
        b"<p>\nAny others look cool? \xde\n<k>",
        
    b"tot_cshop0_16":
        b"<p>\nThanks a bunch! \xd0\n<k>",
        
    b"tot_cshop1_16":
        b"<p>\nLooking good!\n<k>",
        
    b"tot_cshop2_16":
        b"<p>\nGlad I could help you! \xde\n<k>",
        
    b"tot_cshop0_17":
        b"Want anything else? \xd0\n<k>",
        
    b"tot_cshop1_17":
        b"Anything else suit you?\n<k>",
        
    b"tot_cshop2_17":
        b"Any others look cool? \xde\n<k>",
        
    b"tot_cshop0_18":
        b"Thanks a bunch! \xd0\n<k>",
        
    b"tot_cshop1_18":
        b"Looking good!\n<k>",
        
    b"tot_cshop2_18":
        b"Glad I could help! \xde\n<k>",
        
    b"tot_cshop0_20":
        b"Next time! \xd0\n<k>",
        
    b"tot_cshop1_20":
        b"See you around!\n<k>",
        
    b"tot_cshop2_20":
        b"After a while! \xde\n<k>",
        
    b"tot_cshop0_30":
        b"<p>\nAhh, I see; anything else? \xd0\n<k>",
        
    b"tot_cshop1_30":
        b"<p>\nNo dice, huh? <wait 200>Anything else\n"
        b"suit your fancy?\n<k>",
        
    b"tot_cshop2_30":
        b"<p>\nToo exotic? \xde\nWhat else, then?<k>",
        
    b"tot_cshop0_40":
        b"<p>\nThanks a bunch! \xd0\n"
        b"<wait 200>Here's a little trinket to keep\n"
        b"you organized, <wait 200>on the house!\n<k>",
        
    b"tot_cshop1_40":
        b"<p>\nFantastic choice! <wait 150>Here's a\n"
        b"little something to help manage\n"
        b"your wardrobe going forward.\n<k>",
        
    b"tot_cshop2_40":
        b"<p>\nGreat choice! <wait 200>Oh, <wait 100>this here'll\n"
        b"help you find different Yoshi\n"
        b"varieties out in the wild. \xde\n<k>",
}

g_OpeningCutsceneStrings = {
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
        b"I've got a really bad feeling\n"
        b"about this, Mario. <wait 250>We should\n"
        b"probably see what's up!\n<k>\n<p>\n"
        b"I'll go on ahead to the castle,\n"
        b"<wait 150>you go and check up on\n"
        b"the folks in town!\n<k>",
}

g_StringMap = {

    'global': g_GlobalStrings,
    
    # Tower lobby.
    'gon_00': g_TowerLobbyStrings,
    
    # Tower floors.
    'gon_01': ChainMap(g_TowerStrings, g_Tower1Strings),
    'gon_02': ChainMap(g_TowerStrings, g_Tower2Strings),
    'gon_03': ChainMap(g_TowerStrings, g_Tower3Strings),
    'gon_04': ChainMap(g_TowerStrings, g_Tower4Strings),
    
    # Tower boss floor.
    'gon_05': g_TowerBossFloorStrings,
    
    # West side of Petalburg hub.
    'gon_10': g_PetalburgWestStrings,
    'gon_11': g_PetalburgEastStrings,
    'gon_12': g_OpeningCutsceneStrings,
    
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