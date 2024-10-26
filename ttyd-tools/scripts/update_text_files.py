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
    # Generic dummy string.
    b"tot_msg_dummy":       b"Placeholder",
        
    # Generic dummy NPC dialogue.
    b"tot_npc_generic":
        b"[Placeholder] I'm interactable.\n<k>",
        
    # File creation string.
    b"msg_mcard_make_data":
        b'<system>\n'
        b'<p>\n'
        b'<scaleX 0.9>\n'
        b'The Memory Card in Slot A does not\n'
        b'have a save file for Tower of Trials.\n'
        b'Create a Save File now?\n'
        b'</scale>\n'
        b'<o>',
    
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
        b"A glittering trinket, used to\n"
        b"trade with traveling merchants.",
        
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
    
    b"msg_jiwajiwa_kinoko":
        b"Replenishes 5 HP per turn\n"
        b"for the next 3 turns.",
    
    b"msg_jiwajiwa_syrup":
        b"Replenishes 5 FP per turn\n"
        b"for the next 3 turns.",
    
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
        
    b"msg_pnk_koura_tsuranuki":
        b"Drill through grounded enemies,\n"
        b"dealing more damage per hit.",
        
    b"msg_pwd_kumogakure":
        b"Envelop the party in a fog,\n"
        b"causing foes to sometimes miss.",
        
    b"tot_ptr3_blizzard_desc":
        b"Attack all enemies with a\n"
        b"blast of icy wind.",
        
    b"tot_ptr3_thunder_desc":
        b"Charge up a storm, releasing\n"
        b"powerful lightning after!",
        
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
        b"moves, heavily confusing them.",
        
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
    
    b"tot_ptr3_thund_abb":  b"T. Storm",
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
    
    b"tot_filemode_race":         b"Race Mode",
    b"tot_filemode_100":          b"100% Mode",
    b"tot_file_completion":       b"Completion",
        
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
        b"run and return to the lobby?\n"
        b"Doing so will forfeit all winnings.\n<o>",
    
    b"tot_gameover":
        b"<system>\n<p>\n"
        b"Do you want to continue from\n"
        b"after the last boss, or give up\n"
        b"and return to the lobby?\n<o>",
    
    b"tot_gameover_repeat":
        b"<p>\n"
        b"Do you want to continue from\n"
        b"after the last boss, or give up\n"
        b"and return to the lobby?\n<o>",
    
    b"tot_gameover_continueconf":
        b"<p>\n"
        b"Continuing this run will revert\n"
        b"your stats, items, & progress on\n"
        b"achievements. Are you sure?\n<o>",
    
    b"tot_gameover_giveupconf":
        b"<p>\n"
        b"Giving up will retain overall\n"
        b"progression status, but you will\n"
        b"forfeit all winnings. Are you sure?\n<o>",
        
    b"tot_gameover_opt":
        b"<select 0 1 0 40>\nContinue\nGive Up",
        
    b"tot_gameover_conf":
        b"<select 0 1 0 40>\nYes\nNo",
        
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
        b"for 6 FP.",
    b"tot_movelog_d031":
        b"\nLv. 1: Deals 4+2+4 damage\n"
        b"for 4 FP.",
    b"tot_movelog_d032":
        b"\nLv. 2: Deals 6+3+6 damage\n"
        b"for 6 FP.",
    b"tot_movelog_d033":
        b"\nLv. 3: Deals 8+4+8 damage\n"
        b"for 8 FP.",
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
        b"\nLv. 3: Deals 5 damage to\n"
        b"each enemy for 5 FP.",
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
        b"\nLv. 2: Deals 2+2 damage and\n"
        b"5-turn Sleep for 4 FP.",
    b"tot_movelog_d073":
        b"\nLv. 3: Deals 2+2 damage and\n"
        b"7-turn Sleep for 6 FP.",
    b"tot_movelog_d081":
        b"\nLv. 1: Deals 4 damage, and\n"
        b"2 to all others for 3 FP.",
    b"tot_movelog_d082":
        b"\nLv. 2: Deals 6 damage, and\n"
        b"3 to all others for 4 FP.",
    b"tot_movelog_d083":
        b"\nLv. 3: Deals 8 damage, and\n"
        b"5 to all others for 6 FP.",
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
        b"3 to foes behind for 3 FP.",
    b"tot_movelog_d123":
        b"\nLv. 3: Deals 8 damage, and\n"
        b"5 to foes behind for 5 FP.",
    b"tot_movelog_d131":
        b"\nLv. 1: Deals 4+4 damage, and\n"
        b"2 to foes behind for 4 FP.",
    b"tot_movelog_d132":
        b"\nLv. 2: Deals 6+6 damage, and\n"
        b"3 to foes behind for 6 FP.",
    b"tot_movelog_d133":
        b"\nLv. 3: Deals 8+8 damage, and\n"
        b"5 to foes behind for 8 FP.",
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
        b"with a 80% base rate for 3 SP.",
    b"tot_movelog_d272":
        b"\nLv. 2: Instantly defeat foes\n"
        b"with a 100% base rate for 4 SP.",
    b"tot_movelog_d273":
        b"\nLv. 3: Instantly defeat foes\n"
        b"with a 125% base rate for 5 SP.",
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
        b"damage for 3 FP.",
    b"tot_movelog_d332":
        b"\nLv. 2: Deals 3+3 DEF-piercing\n"
        b"damage for 4 FP.",
    b"tot_movelog_d333":
        b"\nLv. 3: Deals 4+4 DEF-piercing\n"
        b"damage for 5 FP.",
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
        b"grounded foes for 2 FP.",
    b"tot_movelog_d422":
        b"\nLv. 2: Deals 3 damage to\n"
        b"grounded foes for 3 FP.",
    b"tot_movelog_d423":
        b"\nLv. 3: Deals 5 damage to\n"
        b"grounded foes for 4 FP.",
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
        b"\nLv. 1: Deals 4 (and increasing)\n"
        b"DEF-piercing damage for 5 FP.",
    b"tot_movelog_d462":
        b"\nLv. 2: Deals 6 (and increasing)\n"
        b"DEF-piercing damage for 7 FP.",
    b"tot_movelog_d463":
        b"\nLv. 3: Deals 8 (and increasing)\n"
        b"DEF-piercing damage for 9 FP.",
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
        b"enemy for 4 FP.",
    b"tot_movelog_d533":
        b"\nLv. 3: Drains 8 HP from an\n"
        b"enemy for 5 FP.",
    b"tot_movelog_d541":
        b"\nApplies up to 4 turns of Dodgy\n"
        b"on Mario and Flurrie for 4 FP.",
    b"tot_movelog_d551":
        b"\nLv. 1: Deals 3 piercing damage\n"
        b"and 70%, 1t Freeze for 5 FP.",
    b"tot_movelog_d552":
        b"\nLv. 2: Deals 4 piercing damage\n"
        b"and 85%, 2t Freeze for 7 FP.",
    b"tot_movelog_d553":
        b"\nLv. 3: Deals 5 piercing damage\n"
        b"and 100%, 3t Freeze for 9 FP.",
    b"tot_movelog_d561":
        b"\nLv. 1: Deals 1 damage per cycle\n"
        b"to a single enemy, for 4 FP.",
    b"tot_movelog_d562":
        b"\nLv. 2: Deals 1 damage per cycle\n"
        b"to all enemies, for 9 FP.",
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
        b"4 to foes behind for 4 FP.",
    b"tot_movelog_d623":
        b"\nLv. 3: Deals 8 damage, and\n"
        b"6 to foes behind for 6 FP.",
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
        b"diminishing damage for 5 FP.",
    b"tot_movelog_d662":
        b"\nLv. 2: Hits up to 5 times for\n"
        b"diminishing damage for 6 FP.",
    b"tot_movelog_d663":
        b"\nLv. 3: Hits up to 6 times for\n"
        b"diminishing damage for 7 FP.",
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
        b"inflict 5-Turn Poison, for 2 FP.",
    b"tot_movelog_d842":
        b"\nLv. 2: Throw three bombs, which\n"
        b"inflict 5-Turn Poison, for 4 FP.",
    b"tot_movelog_d843":
        b"\nLv. 3: Throw four bombs, which\n"
        b"inflict 5-Turn Poison, for 6 FP.",
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
        b"failed act rate of 60% for 2 FP.",
    b"tot_movelog_d932":
        b"\nLv. 2: 1-turn Confuse, with a\n"
        b"failed act rate of 75% for 4 FP.",
    b"tot_movelog_d933":
        b"\nLv. 3: 1-turn Confuse, with a\n"
        b"failed act rate of 90% for 6 FP.",
    b"tot_movelog_d941":
        b"\nCosts 2 FP to use.",
    b"tot_movelog_d951":
        b"\nLv. 1: Deals 3 piercing damage\n"
        b"and 33% 1-turn Dizzy for 4 FP.",
    b"tot_movelog_d952":
        b"\nLv. 2: Deals 4 piercing damage\n"
        b"and 66% 1-turn Dizzy for 5 FP.",
    b"tot_movelog_d953":
        b"\nLv. 3: Deals 5 piercing damage\n"
        b"and 100% 1-turn Dizzy for 6 FP.",
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
    b"tot_recn_achievement_pct":    b"Achievements",
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
    b"tot_recn_times_rta":          b"Best Times (RTA)",
    b"tot_recn_times2":             b"*unseeded, default opts.",
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
    b"tot_recn_rcoinsearned":       b"Coins Collected",
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
        b"and best unseeded clear times.",
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
    b"tot_optr_coinsearned":        b"Coins Collected",
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
        b"Defeat 30 different types of\n"
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
        b"Finish a run with all bonus\n"
        b"conditions met (10 at minimum).",
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
        b"Deal a total of 50 damage to\n"
        b"one foe using Poison status.",
    b"tot_achd_37":
        b"Defeat a Shrunk midboss with\n"
        b"Gale Force or Gulp.",
    b"tot_achd_38":
        b"Get at least a GREAT rating\n"
        b"on Flurrie's Thunder Storm.\n"
        b"(Or equivalent rating, if using\n"
        b"non-default AC difficulty).",
    b"tot_achd_39":
        b"Take a hit from a fully charged\n"
        b"Megaton Bomb.",
    b"tot_achd_40":
        b"Defeat the final boss after\n"
        b"using a Trade Off on turn 1.",
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
        b"Purchase 5 items and 5 badges\n"
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
        b"Infatuated foes in a tower run.",
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
    
    # Reworked in-battle Tattles for all enemies.
    
    b'btl_hlp_kuriboo':
        b"That's a Goomba.\n"
        b'<wait 250>\n'
        b"Umm... <wait 100>Yeah, I'm one of those,\n"
        b"in case you hadn't noticed.\n"
        b'<k>\n'
        b'<p>\n'
        b'Ahem... <wait 100>It says here: "Goombas\n'
        b'are underlings of underlings."\n'
        b'<dkey><wait 300></dkey>\n'
        b'...That is totally rude!\n'
        b'<k>\n'
        b'%s',

    b'btl_hlp_patakuri':
        b"That's a Paragoomba.\n"
        b'<wait 250>\n'
        b'Basically a Goomba with\n'
        b"wings.<wait 300> I'm jealous!\n"
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b"You can't hammer it while it's\n"
        b'flying, but rough it up and\n'
        b"it'll totally plummet!\n"
        b'<k>',

    b'btl_hlp_togekuri':
        b"That's a Spiky Goomba.\n"
        b'<wait 250>\n'
        b'...A spiky-headed Goomba.\n'
        b'<wait 250>\n'
        b'What a creative name.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'The addition of the spike\n'
        b"means you'll hurt your feet\n"
        b'if you jump on it. <wait 100>Duh!\n'
        b'<k>',

    b'btl_hlp_hyper_kuriboo':
        b"That's a Hyper Goomba.\n"
        b'<wait 250>\n'
        b"Which means...<wait 250> Umm...<wait 250> It's a\n"
        b'hyperactive Goomba.<wait 250> Duh!\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'But if this nut goes <shake>BIZZ-ZOW\n'
        b'</shake>\n'
        b'and charges up, his next\n'
        b'attack will be much stronger!\n'
        b'<k>\n'
        b'<p>\n'
        b"So you'd better wipe it out\n"
        b'between the time it charges\n'
        b'up and the time it attacks!\n'
        b'<k>',
        
    b'btl_hlp_hyper_patakuri':
        b"That's a Hyper Paragoomba.\n"
        b'<wait 250>\n'
        b"It's basically just a Hyper\n"
        b'Goomba with wings.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b"After charging up, you'll be\n"
        b'dealing with a much stronger\n'
        b'attack next turn.\n'
        b'<k>\n'
        b'<p>\n'
        b"Once you damage it, it'll lose\n"
        b'its wings and turn into a\n'
        b'regular Hyper Goomba.\n'
        b'<k>\n'
        b'<p>\n'
        b"Not that THAT'S all that\n"
        b'cool. <wait 250>Even normal Hyper\n'
        b'Goombas are dangerous.\n'
        b'<k>',

    b'btl_hlp_hyper_togekuri':
        b"That's a Hyper Spiky Goomba.\n"
        b'<wait 250>\n'
        b"It's a Hyper Goomba with a\n"
        b'spike on its head.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'After charging up, its Attack\n'
        b'power will more than double,\n'
        b"<wait 200>on top of the spiky boost!\n"
        b'<k>\n'
        b'<p>\n'
        b'Beat it before it attacks,\n'
        b"or you'll be in peril before\n"
        b'you know it, seriously.\n'
        b'<k>',

    b'btl_hlp_yami_kuriboo':
        b"That's a Gloomba.\n"
        b'<wait 250>\n'
        b'It likes dark, damp places.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b"It's stronger than a normal\n"
        b'Goomba, so be careful.\n'
        b'<k>\n'
        b'<p>\n'
        b"That's not a healthy color for\n"
        b"a Goomba, but it doesn't\n"
        b"mean it's sick or anything.\n"
        b'<k>\n'
        b'<p>\n'
        b"So don't show any mercy!\n"
        b'<k>',

    b'btl_hlp_yami_patakuri':
        b"That's a Paragloomba.\n"
        b'<wait 250>\n'
        b"It's a Gloomba with wings.\n"
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b"It's like a stronger version\n"
        b'of a Paragoomba.<wait 250> And maybe\n'
        b"that's all there is to it.\n"
        b'<k>\n'
        b'<p>\n'
        b'I wonder if living in here\n'
        b'ever makes its wings moldy?\n'
        b'<k>\n'
        b'<p><dynamic 3>\n'
        b'EEEEEEEEEEEEEEEEEYUCK!\n'
        b'</dynamic><wait 250>\n'
        b"It's too gross to even think\n"
        b'about! <wait 250>Bleck!\n'
        b'<k>',

    b'btl_hlp_yami_togekuri':
        b"That's a Spiky Gloomba.\n"
        b'<wait 250>\n'
        b"It's a Spiky Goomba that\n"
        b'likes dark places.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b"I'm sure you know this, but\n"
        b'try not to jump on the spike.\n'
        b'<k>\n'
        b'<p>\n'
        b"I wonder if I'd look like\n"
        b'that if I stayed in here\n'
        b'long enough?\n'
        b'<k>\n'
        b'<p>\n'
        b'<wait 300>\n'
        b"...I'd rather not think about\n"
        b'that.\n'
        b'<k>',

    b'btl_hlp_nokonoko':
        b"That's a Koopa Troopa.\n"
        b'<wait 250>\n'
        b"They've been around forever!\n"
        b'<wait 100>\n'
        b'Gotta respect the longevity!\n'
        b'<k>\n'
        b"%s"
        b'<p>\n'
        b'Their shells are hard, but\n'
        b'flip them over and their\n'
        b'Defense drops to zero.\n'
        b'<k>\n'
        b'<p>\n'
        b'And you know how to flip\n'
        b'them over, right?<wait 250> Just jump\n'
        b'on their heads!\n'
        b'<k>',

    b'btl_hlp_patapata':
        b"That's a Koopa Paratroopa.\n"
        b'<wait 250>\n'
        b"Well, umm<wait 10>.<wait 10>.<wait 10>.<wait 300> It's basically\n"
        b'a Koopa Troopa with wings.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'You can stomp on him, and\n'
        b"he'll plunge down and become\n"
        b'a plain Koopa Troopa.\n'
        b'<k>\n'
        b'<p>\n'
        b"Yeah, do that, and he's ours!\n"
        b'<wait 250>\n'
        b'Stomp again to flip him, and\n'
        b'<wait 250>\n'
        b'his arms and legs are useless!\n'
        b'<k>\n'
        b'<p>\n'
        b"<dynamic 3>Oops!</dynamic><wait 100> Sorry, that's not true.\n"
        b'<wait 300>\n'
        b'It looks like he can still\n'
        b'wiggle them around a bit...\n'
        b'<k>',

    b'btl_hlp_nokonoko_fighter':
        b"That's a KP Koopa.\n"
        b'<wait 250>\n'
        b"It's a Koopa Troopa of a\n"
        b'slightly different color.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'Looks like its abilities are\n'
        b'just like any Koopa...<wait 250> So he\n'
        b'oughta be a piece of cake!\n'
        b'<k>\n'
        b'<p>\n'
        b"Don't go easy just because\n"
        b'you know King K, Mario...\n'
        b'<wait 250>\n'
        b"That's what friends are for!\n"
        b'<k>',

    b'btl_hlp_patapata_fighter':
        b"That's a KP Paratroopa.\n"
        b'<wait 250>\n'
        b"It's a Koopa Paratroopa of a\n"
        b'different color.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'Its attacks are the same as\n'
        b'your average, garden-variety\n'
        b'Koopa Paratroopa.\n'
        b'<k>\n'
        b'<p>\n'
        b'So, just do the same thing as\n'
        b'always: stomp on it, flip it,\n'
        b'and drop its Defense to 0.\n'
        b'<k>\n'
        b'<p>\n'
        b'My Headbonk is pretty\n'
        b'effective against it, too.\n'
        b'<wait 250>\n'
        b"Come on!<wait 250> Let's get him!\n"
        b'<k>',

    b'btl_hlp_ura_noko':
        b"That's a Shady Koopa.\n"
        b'<wait 250>\n'
        b"It's yet another member of\n"
        b'the Koopa family tree.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'The difference between them\n'
        b'and other Koopas?<wait 250> They can\n'
        b'attack from their backs!\n'
        b'<k>\n'
        b'<p>\n'
        b'And, when they flip back up,\n'
        b'their Attack gets boosted\n'
        b'and they go totally ape!\n'
        b'<k>\n'
        b'<p>\n'
        b"...Wow!<wait 250> That's pretty tricky!\n"
        b'<wait 250>\n'
        b"No wonder they're called\n"
        b'Shady Koopas, huh?\n'
        b'<k>',

    b'btl_hlp_ura_pata':
        b"That's a Shady Paratroopa.\n"
        b'<wait 250>\n'
        b"It's a Shady Koopa with\n"
        b'wings, obviously.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'Unlike typical Koopas and\n'
        b'Paratroopas, its Attack jumps\n'
        b'when it flips up off its back.\n'
        b'<k>\n'
        b'<p>\n'
        b'This also says it has a move\n'
        b'that hits you and your ally\n'
        b'at once. Is that <dynamic 3>possible?\n'
        b'</dynamic>\n'
        b'<k>\n'
        b'<p>\n'
        b'Uh...<wait 250> I guess the best thing\n'
        b'to do is just beat it quickly\n'
        b'after we flip it.\n'
        b'<k>',



    b'btl_hlp_yami_noko':
        b"That's a Dark Koopa.\n"
        b'<wait 250>\n'
        b'It lives in dark places that\n'
        b'the light never reaches.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b"It's a bit tougher than a\n"
        b"regular Koopa, so you won't\n"
        b'be able to beat it easily.\n'
        b'<k>\n'
        b'<p>\n'
        b'But, it DOES have the same\n'
        b'weakness as a regular Koopa.\n'
        b'<wait 250>\n'
        b"Flip it over and it's helpless.\n"
        b'<k>\n'
        b'<p>\n'
        b'Why do you think it needs\n'
        b'sunglasses up here,\n'
        b"anyway? <wait 250>What's the point?\n"
        b'<k>',

    b'btl_hlp_yami_pata':
        b'This is a Dark Paratroopa.\n'
        b'<wait 250>\n'
        b'It lives in dark, damp places.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b"It's strong, but otherwise\n"
        b'just like other Paratroopas.\n'
        b'<k>\n'
        b'<p>\n'
        b"It's airborne, so try to\n"
        b'ground it first.\n'
        b'<k>',

    b'btl_hlp_togenoko':
        b"That's a Koopatrol.\n"
        b'<wait 250>\n'
        b'A Koopa Troopa who protects\n'
        b'himself with spiked armor.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'It attacks with its shell and\n'
        b'with its head, then sometimes\n'
        b'charges up for a fierce move.\n'
        b'<k>\n'
        b'<p>\n'
        b'Plus, if you take too long to\n'
        b"win, it'll call reinforcements.\n"
        b'<wait 250>\n'
        b'Yeah, sorta gnarly, huh?\n'
        b'<k>\n'
        b'<p>\n'
        b"It's one of the worst of\n"
        b"Bowser's guys.<wait 250> Koopa Troopas\n"
        b'dream of being Koopatrols.\n'
        b'<k>\n'
        b'<p>\n'
        b'...Hey, and by the way, what\n'
        b"do you think Bowser's doing\n"
        b'now, anyway? <wait 250>Eating?\n'
        b'<k>',

    b'btl_hlp_togenoko_ace':
        b"That's a Dark Koopatrol.\n"
        b'<wait 250>\n'
        b'These guys just totally ooze\n'
        b"toughness, don'tcha think?\n"
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'After it charges up power,\n'
        b'its next attack will be\n'
        b'devastating. Try to survive it.\n'
        b'<k>\n'
        b'<p>\n'
        b"It's almost like those red\n"
        b'eyes are there to warn you\n'
        b'just how tough it is...\n'
        b'<k>\n'
        b'<p>\n'
        b"Or maybe it just doesn't get\n"
        b'much sleep...\n'
        b'<k>',

    b'btl_hlp_honenoko':
        b"That's a Dull Bones.\n"
        b'<wait 250>\n'
        b'Sort of a skeleton thing.\n'
        b'<wait 250>\n'
        b'It was a Koopa Troopa...once.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'These creeps throw bones to\n'
        b'attack.<wait 250> Oh, and they build\n'
        b'reinforcements, too!\n'
        b'<k>\n'
        b'<p>\n'
        b'Their HP is pretty low, so\n'
        b'spread attacks are effective\n'
        b'at clearing them out.\n'
        b'<k>',

    b'btl_hlp_red_honenoko':
        b"That's a Red Bones!\n"
        b'<wait 250>\n'
        b'A little TOO red, I think.<wait 250> \n'
        b"No fashion sense, y'know?\n"
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'Unlike your average Dull\n'
        b'Bones, even if you pound it\n'
        b'to 0 HP and it crumbles...\n'
        b'<k>\n'
        b'<p>\n'
        b'It totally might revive!\n'
        b'<wait 100>\n'
        b'Like a zombie!\n'
        b'<k>\n'
        b'<p>\n'
        b'Oh, and one more thing!<wait 250> Once\n'
        b'its friends start to fall, it\n'
        b'may create some more!\n'
        b'<k>',

    b'btl_hlp_karon':
        b"That's a Dry Bones.\n"
        b'<wait 250>\n'
        b"It's a former Koopa whose\n"
        b'spirit animates its bones.\n'
        b'<k>\n'
        b'<p>\n'
        b'<shake>\n'
        b"Eeeeeek! <wait 100>That's so freaky!\n"
        b'</shake>\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'When its HP goes down to 0,\n'
        b'it collapses into a pile, but\n'
        b"it'll eventually rise again.\n"
        b'<k>\n'
        b'<p>\n'
        b'Fire and explosions will put\n'
        b'a permanent end to it\n'
        b'getting back up, though.\n'
        b'<k>\n'
        b'<p>\n'
        b'A Dry Bones will sometimes\n'
        b'build friends if it feels\n'
        b"it's outnumbered.\n"
        b'<k>\n'
        b'<p>\n'
        b"If you don't take them all\n"
        b"out close together, they'll\n"
        b'just keep coming back.\n'
        b'<k>\n'
        b'<p>\n'
        b"Let's wipe them out all at\n"
        b'once, and quick!<wait 250> These things\n'
        b'totally freak me out!\n'
        b'<k>',

    b'btl_hlp_black_karon':
        b"That's a Dark Bones.\n"
        b'<wait 250>\n'
        b"It's the baddest of the\n"
        b'Bones gang.\n'
        b'</shake>\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'When its HP goes down to 0,\n'
        b'it collapses into a pile, but\n'
        b"it'll eventually rise again.\n"
        b'<k>\n'
        b'<p>\n'
        b'Fire and explosions will put\n'
        b'a permanent end to it\n'
        b'getting back up, though.\n'
        b'<k>\n'
        b'<p>\n'
        b"Its HP is high, so it's hard\n"
        b'to take it down.\n'
        b'<k>\n'
        b'<p>\n'
        b'Like any other Bones, it\n'
        b'sometimes builds friends if\n'
        b'it feels outnumbered.\n'
        b'<k>\n'
        b'<p>\n'
        b"It's a pretty tough enemy.\n"
        b'<wait 250>\n'
        b'You better take it and its\n'
        b'buddies out all at once.\n'
        b'<k>',

    b'btl_hlp_hammer_bros':
        b"That's a Hammer Bro.\n"
        b'<wait 250>\n'
        b'You know him.<wait 250> He throws\n'
        b'hammers.<wait 250> Hence the name.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'When his HP drops and he\n'
        b"thinks he's in trouble, he'll\n"
        b'toss a hammer barrage.\n'
        b'<k>\n'
        b'<p>\n'
        b'Nothing about these guys has\n'
        b"changed: they're tough as\n"
        b'ever, so brace yourself!\n'
        b'<k>\n'
        b'<p>\n'
        b'Dang! <wait 250>This book always skips\n'
        b'the important stuff!<wait 250> How do\n'
        b'they carry endless hammers!\n'
        b'<k>',

    b'btl_hlp_boomerang_bros':
        b"That's a Boomerang Bro.\n"
        b'<wait 250>\n'
        b'This relative of the Hammer\n'
        b'Bros. prefers boomerangs.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'He attacks twice with his\n'
        b'boomerangs: <wait 250>on the way out\n'
        b'AND on the way back.\n'
        b'<k>\n'
        b'<p>\n'
        b'When his HP starts getting\n'
        b'low, he fights all the harder.\n'
        b'<wait 250>\n'
        b'Prepare for multiple attacks!\n'
        b'<k>',

    b'btl_hlp_fire_bros':
        b"That's a Fire Bro.\n"
        b'<wait 250>\n'
        b'This relative of the Hammer\n'
        b'Bros. spits out fire.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'If you get hit by a fireball,\n'
        b'you might catch on fire, so\n'
        b'guard well, OK?\n'
        b'<k>\n'
        b'<p>\n'
        b'When his HP gets low, he gets\n'
        b'desperate and totally starts\n'
        b'attacking like a fiend.\n'
        b'<k>',

    b'btl_hlp_jyugem':
        b"That's a Lakitu.\n"
        b'<wait 250>\n'
        b"It's a member of the Koopa\n"
        b'clan that rides on clouds.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b"If you stomp on it when it's\n"
        b"holding up a Spiny Egg, you'll\n"
        b"take damage, so DON'T do it!\n"
        b'<k>\n'
        b'<p>\n'
        b'Spiny Eggs can hatch into\n'
        b'Spinies, so beat the Lakitu\n'
        b'before fighting the Spinies.\n'
        b'<k>\n'
        b'<p>\n'
        b"Aw, this book doesn't answer\n"
        b'my real question:<wait 250> where does\n'
        b'it keep all those Spiny Eggs?\n'
        b'<k>',

    b'btl_hlp_togezo':
        b"That's a Spiny.\n"
        b'<wait 250>\n'
        b'Basically a spike-covered\n'
        b'Koopa. <wait 250>Ugh! <wait 250>Looks <wait 100><shake>painful</shake>!\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'These things have such high\n'
        b"Defense that you can't even\n"
        b'hurt them when they roll up.\n'
        b'<k>\n'
        b'<p>\n'
        b'So, when they go back to\n'
        b'normal, do all the damage\n'
        b'you can, and do it quickly!\n'
        b'<k>\n'
        b'<p>\n'
        b'If you flip them over, their\n'
        b"Defense drops to 0. <wait 250>THAT'S\n"
        b'the time to take them out.\n'
        b'<k>\n'
        b'<p>\n'
        b'Jumping on them is just plain\n'
        b'stupid, but a Special Move\n'
        b'or my Ironbonk should work.\n'
        b'<k>',

    b'btl_hlp_hyper_jyugem':
        b"That's a Dark Lakitu.\n"
        b'<wait 250>\n'
        b'It rides a rain cloud.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b"It attacks by throwing<wait 75>.<wait 75>.<wait 75>.\n"
        b"<wait 150><dynamic 3>pipes</dynamic> at you? <wait 250>That doesn't\n"
        b"seem right...\n"
        b'<k>\n'
        b'<p>\n'
        b"If you jump on it when it's\n"
        b"holding up a Spiny Egg, you'll\n"
        b'get hurt.\n'
        b'<k>\n'
        b'<p>\n'
        b'And sometimes the eggs it\n'
        b'throws turn into Sky-Blue\n'
        b'Spinies. <wait 250>Whoa!\n'
        b'<k>\n'
        b'<p>\n'
        b'If you only attack the\n'
        b"Spinies, you'll never win,\n"
        b'so go after the Dark Lakitu!\n'
        b'<k>',

    b'btl_hlp_hyper_togezo':
        b"That's a Sky-Blue Spiny.\n"
        b'<wait 250>\n'
        b'It appeared from an egg\n'
        b'thrown by the Dark Lakitu.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'Sometimes it balls up to\n'
        b'defend or stores energy for\n'
        b'an attack.\n'
        b'<k>\n'
        b'<p>\n'
        b'If you can, beat it and any\n'
        b'buddies it may have with a\n'
        b'special attack.\n'
        b'<k>',

    b'btl_hlp_met':
        b"That's a Buzzy Beetle.\n"
        b'<wait 250>\n'
        b"Awww...<wait 100> It's kinda cute!\n"
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b"It's got pretty good Defense...\n"
        b'<wait 250>\n'
        b"and fire and explosions don't\n"
        b'hurt it whatsoever.\n'
        b'<k>\n'
        b'<p>\n'
        b'But, if you jump on it, you\n'
        b'can flip it over and drop\n'
        b'its Defense down to 0.\n'
        b'<k>\n'
        b'<p>\n'
        b"Then it's at our mercy!<wait 250> Ha!\n"
        b'<wait 250>\n'
        b"Don't hesitate!<wait 250> Jump!<wait 250> Jump!\n"
        b'Jump jump jump!\n'
        b'<k>',

    b'btl_hlp_togemet':
        b"That's a Spike Top.\n"
        b'<wait 250>\n'
        b"It's a Buzzy Beetle with a\n"
        b'spike on its back.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'Like other Buzzies, fire and\n'
        b'explosions have absolutely no\n'
        b'effect on it.\n'
        b'<k>\n'
        b'<p>\n'
        b'But, you can flip it over to\n'
        b'drop its Defense to 0!\n'
        b'<k>\n'
        b'<p>\n'
        b"I'm sure you know this\n"
        b'already, but try not to jump\n'
        b'on it. <wait 250>That spike hurts.\n'
        b'<k>',

    b'btl_hlp_patamet':
        b"That's a Parabuzzy.\n"
        b'<wait 250>\n'
        b'A Buzzy Beetle with wings.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'Fire and explosions seem to\n'
        b"have no effect, so don't\n"
        b'bother, OK?\n'
        b'<k>\n'
        b'<p>\n'
        b'If you flip it over, its\n'
        b'Defense goes down to 0, so\n'
        b'jump on it first.\n'
        b'<k>',

    b'btl_hlp_patatogemet':
        b"That's a Spiky Parabuzzy.\n"
        b'<wait 250>\n'
        b"It's a Buzzy with a spike\n"
        b'and wings.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b"Fire and explosions don't\n"
        b'work on Buzzies, in case\n'
        b"you've forgotten.\n"
        b'<k>\n'
        b'<p>\n'
        b'...And this one flies in the air\n'
        b'AND has a spike.<wait 250> What a\n'
        b'total pain.\n'
        b'<k>\n'
        b'<p>\n'
        b"So... you'd better use an\n"
        b'item, Special Move, or I could\n'
        b'hit it with my Ironbonk!\n'
        b'<k>',

    b'btl_hlp_kamec':
        b"That's a Magikoopa.\n"
        b'<wait 250>\n'
        b"Y'know, a Koopa wizard.\n"
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b"It'll throw a load of pain our\n"
        b'way while using magic to help\n'
        b'its buddies.<wait 250> What a creep.\n'
        b'<k>\n'
        b'<p>\n'
        b"And when there's only one of\n"
        b'them, it splits up into multiple\n'
        b'copies to mess with us.\n'
        b'<k>\n'
        b'<p>\n'
        b"Stop this thing fast or we'll\n"
        b'be in a world of hurt!\n'
        b'<k>',

    b'btl_hlp_kamec_bunshin':
        b"That's a Magikoopa Copy.\n"
        b'<wait 250>\n'
        b"Yeah, attacking it won't do\n"
        b'any good whatsoever.\n'
        b'<k>',

    b'btl_hlp_kamec_red':
        b"That's a Red Magikoopa.\n"
        b'<wait 250>\n'
        b"It's a Koopa wizard dressed\n"
        b'in red.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'Besides using offensive magic,\n'
        b'it can also use magic to\n'
        b'boost Attack and Defense.\n'
        b'<k>\n'
        b'<p>\n'
        b'It also has slightly stronger\n'
        b'Attack power than other\n'
        b'types of Magikoopa.\n'
        b'<k>\n'
        b'<p>\n'
        b"When it's alone, this wizard\n"
        b'will use magic to multiply.\n'
        b'<k>\n'
        b'<p>\n'
        b"The fact that you can't tell\n"
        b'which is the real one after it\n'
        b'multiplies is super-annoying.\n'
        b'<k>\n'
        b'<p>\n'
        b'I have to say, though, that\n'
        b'shade of red is definitely\n'
        b'NOT its color.\n'
        b'<k>',

    b'btl_hlp_kamec_red_bunshin':
        b"That's a Red Magikoopa Copy.\n"
        b'<wait 250>\n'
        b"There's no point in attacking\n"
        b"it, since you can't hurt it.\n"
        b'<k>',

    b'btl_hlp_kamec_white':
        b"That's a White Magikoopa.\n"
        b'<wait 250>\n'
        b"It's a Koopa wizard dressed\n"
        b'in white.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'It attacks with magic and\n'
        b"can replenish its allies' HP.\n"
        b'<wait 250>\n'
        b'Better hit it first, huh?\n'
        b'<k>\n'
        b'<p>\n'
        b"It's also got a bit more HP than\n"
        b'other types of Magikoopa,\n'
        b'though. <wait 200>Ugh, how annoying!\n'
        b'<k>\n'
        b'<p>\n'
        b"Oh, this also says it'll use\n"
        b'magic to make copies of itself\n'
        b"when it's all alone.\n"
        b'<k>\n'
        b'<p>\n'
        b"...Which is totally lame, 'cause\n"
        b"then you can't tell the real\n"
        b'deal from the copy!\n'
        b'<k>',

    b'btl_hlp_kamec_white_bunshin':
        b"That's a White Magikoopa\n"
        b'Copy.<wait 250> No point in attacking\n'
        b"it, since you can't hurt it.\n"
        b'<k>',

    b'btl_hlp_kamec_green':
        b"That's a Green Magikoopa.\n"
        b'<wait 250>\n'
        b"It's a Koopa wizard dressed\n"
        b'in green.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'It has magic attacks and can\n'
        b'also magically electrify allies\n'
        b'and make them invisible.\n'
        b'<k>\n'
        b'<p>\n'
        b'It also has a bit of\n'
        b'Defense power, unlike other\n'
        b'types of Magikoopa.\n'
        b'<k>\n'
        b'<p>\n'
        b"Oh, this also says it'll use\n"
        b'magic to make copies of itself\n'
        b"when it's all alone.\n"
        b'<k>\n'
        b'<p>\n'
        b"...Which is totally lame, 'cause\n"
        b"then you can't tell the real\n"
        b'deal from the copy!\n'
        b'<k>',

    b'btl_hlp_kamec_green_bunshin':
        b"That's a Green Magikoopa\n"
        b'Copy.<wait 250> No point in attacking\n'
        b"it, since you can't hurt it.\n"
        b'<k>',
    
    b'btl_hlp_monban':
        b"That's a Craw.\n"
        b'<wait 250>\n'
        b'These guards are pretty\n'
        b'stoic, no-nonsense types.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b"You don't wanna jump on the\n"
        b"spear it's got pointing up.\n"
        b'<wait 250>\n'
        b"'Cause that...<wait 250>would hurt.\n"
        b'<k>\n'
        b'<p>\n'
        b'For a run-of-the-mill spear-\n'
        b"tosser, he sure looks like he's\n"
        b'got a POINT to make!<wait 250> Ha ha!\n'
        b'<k>',

    b'btl_hlp_dark_keeper':
        b"That's a Dark Craw.\n"
        b'<wait 250>\n'
        b'Yikes.<wait 250> What a ghastly-looking\n'
        b'spear-thrower.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'These guys have the usual\n'
        b'attacks, throwing spears\n'
        b'and charging into us...\n'
        b'<k>\n'
        b'<p>\n'
        b'He can also charge through\n'
        b'and hit us both! <wait 200>Watch for\n'
        b'a jump before he attacks!\n'
        b'<k>',

    b'btl_hlp_borodo':
        b"That's a Bandit.\n"
        b'<wait 250>\n'
        b'This scumbag tries to bump\n'
        b'you and grab coins. <wait 250>Jerk!\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'If you time your guard well\n'
        b"when he attacks, he won't be\n"
        b'able to steal anything.\n'
        b'<k>\n'
        b'<p>\n'
        b"Plus, the look on a Bandit's\n"
        b'face when you guard\n'
        b'successfully is priceless.\n'
        b'<k>\n'
        b'<p>\n'
        b'If a Bandit steals coins from\n'
        b'you, defeat him before he\n'
        b'flees to get your coins back.\n'
        b'<k>',

    b'btl_hlp_borodo_king':
        b"That's a Big Bandit.\n"
        b'<wait 250>\n'
        b'Basically, a boss among\n'
        b'ordinary Bandits.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b"Unlike other Bandits, he'll\n"
        b'steal your items as well as\n'
        b'your coins. <wait 250>Totally weak!\n'
        b'<k>\n'
        b'<p>\n'
        b'If you guard well when he\n'
        b"charges at you, he'll blow by\n"
        b'without stealing anything.\n'
        b'<k>\n'
        b'<p>\n'
        b"If he robs you, you won't get\n"
        b'your item back unless you\n'
        b"beat him right then. <wait 250>He'll run!\n"
        b'<k>',

    b'btl_hlp_badge_borodo':
        b"That's a Badge Bandit.\n"
        b'<wait 250>\n'
        b"He's a Bandit who'll go after\n"
        b'your badges. <wait 250>That jerk!\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'He sometimes steals a badge\n'
        b'when he tackles you, so use\n'
        b'your Guard carefully.\n'
        b'<k>\n'
        b'<p>\n'
        b'Be sure to beat him quick\n'
        b'if you get robbed. <wait 250>If he runs,\n'
        b"that badge's gone for good!\n"
        b'<k>',

    b'btl_hlp_hinnya':
        b"That's a Spinia.\n"
        b'<wait 250>\n'
        b'A totally weird creature\n'
        b'made of thin, papery boards.\n'
        b'<k>\n'
        b'<p>\n'
        b'It certainly looks unique.\n'
        b'<wait 250>\n'
        b'Nobody knows much about\n'
        b'these creatures, actually.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'Its attacks are super-swift,\n'
        b'but it should be pretty easy.\n'
        b'<wait 250>\n'
        b'Just give it a "Hee-YAAA!"\n'
        b'<k>',

    b'btl_hlp_hannya':
        b"That's a Spania.\n"
        b'<wait 250>\n'
        b'A Spinia with spikes on its\n'
        b'head.<wait 250> It looks meaner, too.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'You oughta watch out for\n'
        b'the spikes on its head, but\n'
        b'otherwise, just whale on it.\n'
        b'<k>\n'
        b'<p>\n'
        b'I gotta be honest, though,\n'
        b'the way that thing spins\n'
        b'makes me wanna yack.\n'
        b'<k>',

    b'btl_hlp_hennya':
        b"That's a Spunia.\n"
        b'<wait 250>\n'
        b'Its body is made up of\n'
        b'springy discs.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'It may not look like much,\n'
        b"but it's pretty tough.\n"
        b'<k>\n'
        b'<p>\n'
        b"It won't come at you with\n"
        b'anything too weird, so just\n'
        b'hang in there, OK?\n'
        b'<k>',
        
    b'btl_hlp_chorobon':
        b"That's a Fuzzy.\n"
        b'<wait 250>\n'
        b'What a hyper little guy, huh?\n'
        b'<wait 250>\n'
        b'Cut back on the caffeine!\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'Those things suck up your HP\n'
        b'and use it to replenish their\n'
        b"own!<wait 250> Isn't that the worst?\n"
        b'<k>\n'
        b'<p>\n'
        b"I mean, EW! <wait 250>Doesn't that just\n"
        b'sound totally gross?\n'
        b'<k>\n'
        b'<p>\n'
        b'Anyway, guard against them\n'
        b'by pressing <icon ANM_PAD_A 0.7 -18 28 0> the\n'
        b'MOMENT they release you.\n'
        b'<k>\n'
        b'<p>\n'
        b'The timing is pretty hard to\n'
        b'master, so, uh, practice up!\n'
        b'<wait 250>\n'
        b'These things really suck...HP.\n'
        b'<k>',

    b'btl_hlp_green_chorobon':
        b"That's a Green Fuzzy.\n"
        b'<wait 250>\n'
        b"As you can see, it's a Fuzzy,\n"
        b"and it's green. <wait 250>Duh!\n"
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'This is similar to a normal\n'
        b'Fuzzy, so it replenishes its\n'
        b'own HP as it sucks out ours.\n'
        b'<k>\n'
        b'<p>\n'
        b'<shake>\n'
        b'EWWWWWWWWWWWWWWWW...\n'
        b'</shake><wait 250>\n'
        b'That is just the grossest\n'
        b'thing ever.\n'
        b'<k>\n'
        b'<p>\n'
        b"...If you don't find that idea\n"
        b"gross, seriously, there's\n"
        b'something wrong with you.\n'
        b'<k>\n'
        b'<p>\n'
        b'The main difference between\n'
        b'this and other Fuzzies is\n'
        b'that this one multiplies.\n'
        b'<k>',

    b'btl_hlp_flower_chorobon':
        b"That's a Flower Fuzzy.\n"
        b'<wait 250>\n'
        b'Wow, what a totally pretty\n'
        b"Fuzzy, don'tcha think?\n"
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'This thing attacks by boinging\n'
        b'in and sucking out FP.\n'
        b'<wait 250>\n'
        b'How uncool!<wait 250> We need FP!\n'
        b'<k>\n'
        b'<p>\n'
        b'Once it charges up its own\n'
        b'FP, it uses magical attacks.\n'
        b'<wait 250>\n'
        b'Better beat it before it does.\n'
        b'<k>\n'
        b'<p>\n'
        b'<dynamic 3>Oh!</dynamic> <wait 100>I just got it!\n'
        b'<k>\n'
        b'<p>\n'
        b"It's a Flower Fuzzy because it\n"
        b'sucks your FP (Flower Points)!\n'
        b'<wait 250>\n'
        b'Duuuuuuh!<wait 250> Hee hee!\n'
        b'<k>',

    b'btl_hlp_sambo':
        b"That's a Pokey.\n"
        b'<wait 250>\n'
        b"It's a cactus ghoul that's got\n"
        b'nasty spines all over its body.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'Look at those spines... <wait 250>Those\n'
        b'would TOTALLY hurt.<wait 250> If you\n'
        b"stomp on it, you'll regret it.\n"
        b'<k>\n'
        b'<p>\n'
        b'Pokeys attack by lobbing\n'
        b'parts of their bodies and by\n'
        b'charging at you...\n'
        b'<k>\n'
        b'<p>\n'
        b'They can even call friends in\n'
        b'for help, so be quick about\n'
        b'taking them out.\n'
        b'<k>',

    b'btl_hlp_sambo_mummy':
        b"That's a Poison Pokey.\n"
        b'<wait 250>\n'
        b"As you probably guessed, it's\n"
        b'a poisonous Pokey.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'If you get poisoned, your HP\n'
        b'will drain faster and faster,\n'
        b"so you'll want to heal ASAP!\n"
        b'<k>\n'
        b'<p>\n'
        b"I'm sure you can see this, but\n"
        b'they have spines all over,\n'
        b"so DON'T touch them.\n"
        b'<k>\n'
        b'<p>\n'
        b'They may be smiling, but that\n'
        b"doesn't mean they're happy\n"
        b'to see you!\n'
        b'<k>',

    b'btl_hlp_monochrome_pakkun':
        b"That's a Pale Piranha.\n"
        b'<wait 250>\n'
        b'You know about these guys.\n'
        b'<wait 250>\n'
        b'The famous Piranha Plants.\n'
        b'<k>\n'
        b'<p>\n'
        b'This colorless subspecies\n'
        b'is adapted to Boggly Woods.\n'
        b'<wait 300>Seems to thrive here, though!\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'If you try to jump on them,\n'
        b"they'll totally chomp on you.\n"
        b'<k>',

    b'btl_hlp_poison_pakkun':
        b"That's a Putrid Piranha.\n"
        b'<wait 250>\n'
        b"It's a poisonous Piranha Plant.\n"
        b'<wait 300>\n'
        b'That color is totally sickly...\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'It bites, AND it attacks with\n'
        b'super-rank poison breath.\n'
        b'<wait 250>Breath mint, table four!\n'
        b'<k>\n'
        b'<p>\n'
        b'If you get poisoned, your HP\n'
        b'will drain faster over time,\n'
        b"so, <wait 100>y'know, <wait 150>try not to.\n"
        b'<k>',

    b'btl_hlp_ice_pakkun':
        b"That's a Frost Piranha.\n"
        b'<wait 250>\n'
        b"It's a cool customer with\n"
        b'strong ice powers.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'Its biting attack sometimes\n'
        b'freezes us, so try to\n'
        b'immobilize it first.\n'
        b'<k>\n'
        b'<p>\n'
        b"It also has a frosty breath\n"
        b"attack that can freeze us\n"
        b"both at once! <wait 250>\n"
        b"<shake>\n"
        b"Brrrrrrrr....\n"
        b"</shake>\n"
        b'<k>\n'
        b'<p>\n'
        b"It's weak against fire attacks,\n"
        b'too, so use them if you can!\n'
        b'<k>',

    b'btl_hlp_pakkun_flower':
        b"That's a Piranha Plant.\n"
        b'<wait 250>\n'
        b'In fact, I think this is the\n'
        b'strongest type of them all.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'It may look like a normal\n'
        b"Piranha Plant, but don't be\n"
        b"fooled! <wait 250>It's super-tough!\n"
        b'<k>\n'
        b'<p>\n'
        b"These guys must have some\n"
        b"Venus Fire Trap DNA, <wait 200>'cause\n"
        b"they can spit fire as well!\n"
        b'<k>\n'
        b'<p>\n'
        b'If we get beaten by a flower,\n'
        b"we'll never hear the end of\n"
        b'it, know what I mean?\n'
        b'<k>',

    b'btl_hlp_pansy':
        b"That's a Crazee Dayzee.\n"
        b'<wait 250>\n'
        b'It may look totally cute,\n'
        b"but it's a serious foe.\n"
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'What you gotta worry about\n'
        b'is its lullaby.<wait 250> That tune will\n'
        b'totally zonk you out.\n'
        b'<k>\n'
        b'<p>\n'
        b'If you manage to guard well,\n'
        b'though, you might not pass\n'
        b'out.<wait 250> Try to learn the timing.\n'
        b'<k>\n'
        b'<p>\n'
        b'Oh, and when its HP runs low,\n'
        b'it runs away, so try to KO\n'
        b'it in one fell swoop.\n'
        b'<k>',

    b'btl_hlp_twinkling_pansy':
        b"That's an Amazy Dayzee.\n"
        b'<wait 250>\n'
        b'This mystical Dayzee is like,\n'
        b'the rarest thing ever.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'Since it has such high HP\n'
        b'and runs away really quickly,\n'
        b"it's almost impossible to beat.\n"
        b'<k>\n'
        b'<p>\n'
        b"Plus, it has massive ATK,\n"
        b"so if we can't beat it quick,\n"
        b"we'd better hope it scrams.\n"
        b'<k>',

    b'btl_hlp_piders':
        b"That's a Pider.\n"
        b'<wait 250><dynamic 3>\n'
        b'ICK!</dynamic><wait 250> They gross me out.\n'
        b'<k>\n'
        b'%s\n'
        b'<p>\n'
        b'Besides its normal attacks,\n'
        b'it might spit three web-wads\n'
        b'at you consecutively.\n'
        b'<k>\n'
        b'<p>\n'
        b"You'd better have good\n"
        b'timing if you wanna guard\n'
        b'against its attacks.\n'
        b'<k>\n'
        b'<p>\n'
        b'It also says here that the\n'
        b"Punies can't stand Piders.\n"
        b'<wait 250>\n'
        b"So I'm not the only one!\n"
        b'<k>\n'
        b'<p>\n'
        b'Just the sight of these things\n'
        b'makes Punies flee in terror.\n'
        b'<wait 250>\n'
        b'I know how they feel!\n'
        b'<k>',

    b'btl_hlp_churantalar':
        b"That's an Arantula.\n"
        b'<wait 250>\n'
        b"It's a spiderlike creature\n"
        b'that thrives in hostile places.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b"It'll spit web-wads at you.\n"
        b'<wait 300>\n'
        b'Sometimes it even attacks\n'
        b'several times in a row.\n'
        b'<k>\n'
        b'<p>\n'
        b'That coloring freaks me out.\n'
        b'<wait 250>\n'
        b"...I'm kinda scared of spiders.\n"
        b'<wait 250>\n'
        b"So let's do this quickly!!\n"
        b'<k>',

    b'btl_hlp_basabasa':
        b"That's a Swooper.\n"
        b'<wait 250>\n'
        b"I bet they call it that 'cause\n"
        b'of the way it swoops around.\n'
        b'<k>\n'
        b'<p>\n'
        b'<wait 250><dynamic 3>\n'
        b"Wow!<wait 250> That's some AWESOME\n"
        b'naming work. <wait 250>Seriously!\n'
        b'</dynamic><wait 250>\n'
        b"It's <dynamic 3><wait 100>pure naming genius!!!\n"
        b'</dynamic>\n'
        b'<k>\n'
        b'<p>\n'
        b'So,<wait 100> anyway...\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b"Ground-level attacks can't\n"
        b'reach it up there, so whale\n'
        b'on it with Jumps and such!\n'
        b'<k>',

    b'btl_hlp_basabasa_chururu':
        b"That's a Swoopula.\n"
        b'<wait 250>\n'
        b'An airborne, bloodsucking,\n'
        b'batlike thing.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b"As if losing HP wasn't bad\n"
        b'enough, this little creep adds\n'
        b'yours to its own!\n'
        b'<k>\n'
        b'<p>\n'
        b'How totally, totally gross!\n'
        b'<wait 250>\n'
        b"Don't let it bite me, Mario!\n"
        b'<k>',

    b'btl_hlp_basabasa_green':
        b"That's a Swampire.\n"
        b'<wait 250>\n'
        b"It's a feared health-sucker\n"
        b'that hides in the darkness.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'It sucks health from its prey\n'
        b'to add to its own HP.\n'
        b'<k>\n'
        b'<p>\n'
        b'And that...<wait 100>totally grosses me\n'
        b"out like you wouldn't believe.\n"
        b'<k>\n'
        b'<p>\n'
        b'If you let it feast on you,\n'
        b'its HP will get really high.\n'
        b'<k>\n'
        b'<p>\n'
        b'Focus your attacks on it!\n'
        b'<k>',

    b'btl_hlp_monochrome_kurokumorn':
        b"That's a Dark Puff.\n"
        b'<wait 250>\n'
        b"It's basically a tiny, mean\n"
        b'thunderhead.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b"Sometimes it'll charge itself\n"
        b"with electricity. <wait 250>Don't touch\n"
        b'it when it does! <wait 250>Yowch!\n'
        b'<k>\n'
        b'<p>\n'
        b'You know what I mean, right?\n'
        b'<shake>Brzzzzzzzzzzzzzzzzzzzzzzzzt!\n'
        b'</shake>\n'
        b'<wait 250>\n'
        b'Shocking!\n'
        b'<k>\n'
        b'<p>\n'
        b"After it charges itself, it'll\n"
        b'totally zap you with lightning.\n'
        b'<wait 250>\n'
        b'Beat it as fast as you can.\n'
        b'<k>',

    b'btl_hlp_kurokumorn':
        b'This is a Ruff Puff.\n'
        b'<wait 250>\n'
        b"Don't confuse them with\n"
        b'Dark Puffs...\n'
        b'<k>\n'
        b'<p>\n'
        b"Although I don't think the\n"
        b'world would end if you DID\n'
        b'confuse them, but anyway...\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'They swoop at you or charge\n'
        b'themselves with electricity\n'
        b'before unleashing lightning.\n'
        b'<k>\n'
        b'<p>\n'
        b'Oh, and if you touch them\n'
        b"while they're charged, you'll\n"
        b'get a shock!\n'
        b'<k>',

    b'btl_hlp_bllizard':
        b"That's an Ice Puff.\n"
        b'<wait 250>\n'
        b"It's a mean snow cloud that\n"
        b'appears in cold areas.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'It swoops down and uses cold\n'
        b'breath to attack. <wait 250>Trust me,\n'
        b"it's NOT refreshing.\n"
        b'<k>\n'
        b'<p>\n'
        b'That cold breath can freeze\n'
        b'us, so try to avoid it.\n'
        b'<k>\n'
        b'<p>\n'
        b"Also, if we touch it when it's\n"
        b"storing cold energy, we'll get\n"
        b"hurt. <wait 250>Isn't that super-weak?\n"
        b'<k>\n'
        b'<p>\n'
        b"They're vulnerable to fire,\n"
        b"so let's try that, maybe.\n"
        b'<k>',

    b'btl_hlp_dokugassun':
        b"That's a Poison Puff.\n"
        b'<wait 250>\n'
        b'Basically just a puff of\n'
        b'poisonous air.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'These things charge you, but\n'
        b'they also save up toxins and\n'
        b'poison you with them. <wait 250>Scum!\n'
        b'<k>\n'
        b'<p>\n'
        b"Plus, you can't touch them\n"
        b"when they're saving up toxins\n"
        b"or you'll get hurt. <wait 250>Lame!\n"
        b'<k>',

    b'btl_hlp_teresa':
        b"That's a Boo.\n"
        b'<wait 250>\n'
        b"It's everyone's favorite ghost.\n"
        b'<wait 250>\n'
        b'...Well, most everyone...\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b"It's nothing to write home\n"
        b'about on the Attack side,\n'
        b'but it can turn invisible.\n'
        b'<k>\n'
        b'<p>\n'
        b'If it turns invisible, we\n'
        b"won't be able to hit it, so\n"
        b'beat it while you can see it.\n'
        b'<k>\n'
        b'<p>\n'
        b'Boos are kinda cute, but I\n'
        b"sure wouldn't want to meet\n"
        b'one in a dark alley.\n'
        b'<k>\n'
        b'<p>\n'
        b'If I ran into one in the\n'
        b'bathroom in the middle of\n'
        b"the night, I'd...\n"
        b'<k>\n'
        b'<p>\n'
        b'Well, never you mind what\n'
        b"I'd do.\n"
        b'<k>',
        
    b'btl_hlp_purple_teresa':
        b"That's a Dark Boo.\n"
        b'<wait 250>\n'
        b'Purple just looks...<wait 250>so wrong.\n'
        b'<wait 250>\n'
        b'These Boos are SO creepy.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'Its HP and Attack are high,\n'
        b"but they're just like ordinary\n"
        b'Boos otherwise.\n'
        b'<k>\n'
        b'<p>\n'
        b'So, just attack it like a\n'
        b'normal Boo.<wait 250> Get it before\n'
        b'it turns invisible!\n'
        b'<k>',

    b'btl_hlp_bubble':
        b"That's a Lava Bubble.\n"
        b'<wait 250>\n'
        b"It's a flame spirit.\n"
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'Its HP and Attack power may\n'
        b"be different from an Ember's,\n"
        b"but otherwise it's the same.\n"
        b'<k>\n'
        b'<p>\n'
        b'Since it is made of fire,\n'
        b"try not to touch it, 'cause\n"
        b"it'll burn you.\n"
        b'<k>\n'
        b'<p>\n'
        b"Apparently it's vulnerable to\n"
        b'explosions and ice attacks.\n'
        b'<k>\n'
        b'<p>\n'
        b'Oh, and if you get hit by a\n'
        b'flame attack, you might\n'
        b'catch fire, so guard well.\n'
        b'<k>',

    b'btl_hlp_hermos':
        b"That's an Ember.\n"
        b'<wait 250>\n'
        b'Sort of a pale-blue flame\n'
        b'spirit. <wait 250>Kind of spooky.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'Of course, since this thing is\n'
        b'a flame, touching it will hurt.\n'
        b'<wait 250>\n'
        b"Didn't Mama teach you that?\n"
        b'<k>\n'
        b'<p>\n'
        b"Looks like it's susceptible\n"
        b'to ice or explosive attacks,\n'
        b"though, so that's something.\n"
        b'<k>\n'
        b'<p>\n'
        b'If you get hit by a flame\n'
        b"attack, you'll catch fire,\n"
        b'so try to guard well.\n'
        b'<k>',

    b'btl_hlp_phantom':
        b"That's a Phantom Ember.\n"
        b'<wait 250>\n'
        b"It's an angry spirit born of\n"
        b'hatred and confusion.\n'
        b'<k>\n'
        b'<p>\n'
        b'<wait 150>\n'
        b"So, um...<wait 150> I'm not too cool\n"
        b'with this thing!\n'
        b'<wait 250>\n'
        b'<shake>\n'
        b'B-B-B-B-Bleck...\n'
        b'</shake>\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'If it attacks you with spirit\n'
        b"flames, you'll catch on fire.\n"
        b'<k>\n'
        b'<p>\n'
        b"Let's send this ghoul back\n"
        b'to the grave, OK?\n'
        b'<shake>\n'
        b'P-P-P-P-Please...\n'
        b'</shake>\n'
        b'<k>',

    b'btl_hlp_monochrome_sinemon':
        b"That's a Cleft.\n"
        b'<wait 250>\n'
        b'A rock-head jerk with spikes\n'
        b'on his noggin.<wait 250> What a rocker!\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b"Fire doesn't hurt it, but\n"
        b"any other piercing attacks\n"
        b"should be pretty effective.\n"
        b'<k>\n'
        b'<p>\n'
        b"You can also flip it with\n"
        b"bomb and earthquake-style\n"
        b"attacks to drop its DEF.\n"
        b'<k>',

    b'btl_hlp_hyper_sinemon':
        b"That's a Hyper Cleft.\n"
        b'<wait 250>\n'
        b"It's basically a Cleft that\n"
        b'uses charged-up moves.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'When it charges up, its\n'
        b'attacks get real strong.\n'
        b'<k>\n'
        b'<p>\n'
        b'Couple its rock-hardness with\n'
        b'its ability to charge up...\n'
        b'<wait 100>\n'
        b'and things get scary.\n'
        b'<k>\n'
        b'<p>\n'
        b"If you're confident, you may\n"
        b'wanna try doing Superguards\n'
        b'to send its attacks back...\n'
        b'<k>\n'
        b'<p>\n'
        b"If you can't, try flipping it\n"
        b"over, or using evasive actions\n"
        b"like Koops' Shell Shield.\n"
        b'<k>',

    b'btl_hlp_sinemon':
        b"That's a Moon Cleft.\n"
        b'<wait 250>\n'
        b"It's your basic Cleft living on\n"
        b'the moon.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'Defense is high, as usual...\n'
        b'<wait 250>\n'
        b"and fire attacks don't work\n"
        b'against it.\n'
        b'<k>\n'
        b'<p>\n'
        b'If you can turn it over with\n'
        b'an explosion, though, its\n'
        b'Defense goes down to 0.\n'
        b'<k>',
    
    b'btl_hlp_iron_sinemon':
        b"That's an Iron Cleft. <wait 250>Kind of\n"
        b"a smaller version of those\n"
        b"hard-heads from the Glitz Pit!\n"
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b"That DEF doesn't sound too\n"
        b"bad, but they're so tough, they\n"
        b"only take 1 damage a hit!\n"
        b'<k>\n'
        b'<p>\n'
        b"They're also immune to\n"
        b"most elemental attacks, like\n"
        b"fire, ice, and electricity!\n"
        b'<k>\n'
        b'<p>\n'
        b"Guess we'll have to whittle\n"
        b"them down slowly, but multi-\n"
        b"hit attacks should help!\n"
        b'<k>',

    b'btl_hlp_togedaruma':
        b"That's a Bristle.\n"
        b'<wait 250>\n'
        b'...Totally covered in spikes.\n'
        b'<wait 250>\n'
        b"<dynamic 3>They're so prickly!\n"
        b'</dynamic>\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'Even trying to get close\n'
        b'enough to whack it with a\n'
        b'hammer is dangerous!\n'
        b'<k>\n'
        b'<p>\n'
        b'Seriously, its spikes will\n'
        b'totally pop out!\n'
        b'<k>\n'
        b'<p>\n'
        b"You'll have to attack it\n"
        b"from a distance, or use an\n"
        b"item or Special Move!\n"
        b'<k>',

    b'btl_hlp_yamitogedaruma':
        b"That's a Dark Bristle.\n"
        b'<wait 250>\n'
        b"It's an ancient creature made\n"
        b'of rock.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b"You can't jump on it because\n"
        b"of its spikes, and you can't\n"
        b'approach due to its spears.\n'
        b'<k>\n'
        b'<p>\n'
        b'AND its Defense is high.\n'
        b'<k>\n'
        b'<p>\n'
        b"You'd better take it out\n"
        b'with special attacks or items.\n'
        b'<k>',

    b'btl_hlp_bomhei':
        b"That's a Bob-omb.\n"
        b'<wait 250>\n'
        b'It attacks by blowing itself\n'
        b'up. <wait 250>Yeah. <wait 250>Reeeeeeeal smart.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'A Bob-omb will get totally\n'
        b'mad if you damage it. They\n'
        b'have like, super-short fuses.\n'
        b'<k>\n'
        b'<p>\n'
        b"When it gets mad, it'll charge\n"
        b'and explode on its next turn.\n'
        b'<wait 250>\n'
        b'THAT attack...<wait 250>really hurts.\n'
        b'<k>\n'
        b'<p>\n'
        b"Oh, and if it's mad, it'll blow\n"
        b"up at the slightest contact,\n"
        b"dealing a lot of damage!\n"
        b"<k>\n"
        b"<p>\n"
        b"You can attack it from a\n"
        b"distance, but it'll still charge\n"
        b"at you in retaliation.\n"
        b"<k>\n"
        b"<p>\n"
        b"Your best bet is to take it\n"
        b"out in one go, incapacitate\n"
        b"it, or land a Superguard!\n"
        b"<k>\n",
    
    b'btl_hlp_sinnosuke':
        b"That's a Hyper Bob-omb.\n"
        b'<wait 250>\n'
        b'Massive destructive power,<wait 250> now\n'
        b'in a trendy hot-pink package!\n'
        b'<k>\n'
        b'%s'
        b"<p>\n"
        b"If you can believe it, these\n"
        b"guys are even more hot-headed\n"
        b"than regular Bob-ombs!\n"
        b"<k>\n"
        b"<p>\n"
        b"They'll ignite, and explode\n"
        b"on you the moment they get\n"
        b"to attack, even unprompted!\n"
        b"<k>\n"
        b"<p>\n"
        b"What's worse, if you get\n"
        b"it mad first, it'll charge its\n"
        b"ATK power before attacking!\n"
        b"<k>\n"
        b"<p>\n"
        b"If you don't have ATK to\n"
        b"take it out in one blow, you\n"
        b"better land that Superguard!\n"
        b"<k>",

    b'btl_hlp_heavy_bom':
        b"That's a Bulky Bob-omb.\n"
        b'<wait 250>\n'
        b"I think it's like other\n"
        b"Bob-ombs...<wait 300>but it's huge!\n"
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b"It's weird...<wait 250> It never attacks,\n"
        b"but once its fuse is lit, it'll\n"
        b'blow up on its own eventually.\n'
        b'<k>\n'
        b'<p>\n'
        b"I don't mind it taking itself\n"
        b'out of the battle, but that\n'
        b'explosion hurts us, too!\n'
        b'<k>\n'
        b'<p>\n'
        b"The more buffs you let it\n"
        b"get, the stronger its explosion\n"
        b"is, so watch out!\n"
        b'<k>\n'
        b'<p>\n'
        b'Fire and explosions light its\n'
        b'fuse, so I guess setting it\n'
        b'off early is one strategy...\n'
        b'<k>\n'
        b'<p>\n'
        b"You can run, too, but if you're\n"
        b'gonna fight, beat it before it\n'
        b'goes off!\n'
        b'<k>',

    b'btl_hlp_giant_bomb':
        b"That's a Bob-ulk.\n"
        b'<wait 250>\n'
        b"That's...<wait 250>easily the biggest\n"
        b"Bob-omb I've ever seen!\n"
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b"It won't attack, but once its\n"
        b"fuse is lit, it'll explode\n"
        b'after a while.\n'
        b'<k>\n'
        b'<p>\n'
        b"Its fully powered explosion\n"
        b"is absolutely devastating!\n"
        b"<wait 250>Seriously, it's no joke!\n"
        b'<k>\n'
        b'<p>\n'
        b'Just go all out to beat it.\n'
        b'<wait 250>\n'
        b'Pull out all the stops.\n'
        b'<k>\n'
        b'<p>\n'
        b'Or you could just set it off\n'
        b'early with fire or explosions.\n'
        b'<wait 250>\n'
        b'Your call.\n'
        b'<k>',

    b'btl_hlp_wanwan':
        b"That's a Chain-Chomp.\n"
        b'<wait 250>\n'
        b"I'd hate to get chomped by\n"
        b'those teeth. <wait 250>They look nasty.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'Its body is hard, so most\n'
        b"attacks won't do much.\n"
        b'<k>\n'
        b'<p>\n'
        b"Plus, you can't damage it\n"
        b'with fire and ice attacks.\n'
        b'<wait 250>\n'
        b'You can freeze it, though.\n'
        b'<k>\n'
        b'<p>\n'
        b'Luckily, it has low HP, so\n'
        b'you could take it down with\n'
        b'a special move or an item.\n'
        b'<k>\n'
        b'<p>\n'
        b'Chomp-Chomps are like, so\n'
        b'super-pumped all the time.\n'
        b'<wait 250>\n'
        b"Don't they ever get tired?\n"
        b'<k>',

    b'btl_hlp_burst_wanwan':
        b"That's a Red Chomp.\n"
        b'<wait 250>\n'
        b"It's a rabid, red, biting,\n"
        b'chewing, chomping fool!\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'Its attacks are so powerful,\n'
        b'we could be in a world of\n'
        b"hurt if we don't beat it fast!\n"
        b'<k>\n'
        b'<p>\n'
        b"...Which is why it's really,\n"
        b'REALLY lame that its\n'
        b'Defense is so high.\n'
        b'<k>\n'
        b'<p>\n'
        b"Its HP is relatively low,\n"
        b"so a strong piercing move or\n"
        b"two might do the trick...\n"
        b'<k>\n'
        b'<p>\n'
        b'Just look at that color!<wait 250> Think\n'
        b'someone painted it after it\n'
        b'fell asleep at a party?\n'
        b'<k>',

    b'btl_hlp_mahorn':
        b"That's a Wizzerd.\n"
        b'<wait 250>\n'
        b"It's a part-machine, part-\n"
        b'organic, centuries-old thing.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'It uses magic to attack,\n'
        b'heal, and alter your\n'
        b'condition, so stay on guard.\n'
        b'<k>\n'
        b'<p>\n'
        b'Its Defense is high, but we\n'
        b'can totally take this thing!\n'
        b'<k>',

    b'btl_hlp_super_mahorn':
        b"That's a Dark Wizzerd.\n"
        b'<wait 250>\n'
        b"It's a part-machine, part-\n"
        b'organic, centuries-old thing.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'It uses magic to attack and\n'
        b'to alter your condition,\n'
        b'so stay on guard.\n'
        b'<k>\n'
        b'<p>\n'
        b"If there's only one left,\n"
        b"it'll multiply itself to\n"
        b'confuse you.\n'
        b'<k>\n'
        b'<p>\n'
        b'I know they look totally\n'
        b"goofy, but they're actually\n"
        b'pretty tough enemies.\n'
        b'<k>',

    b'btl_hlp_mahorn_custom':
        b"That's an Elite Wizzerd.\n"
        b'<wait 250>\n'
        b'This is the top of the heap\n'
        b'for half-machine organisms.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'You can probably guess this,\n'
        b'but it uses various magic\n'
        b'moves in battle.\n'
        b'<k>\n'
        b'<p>\n'
        b"And, if it's alone, it'll create\n"
        b'illusions of itself.\n'
        b'<k>\n'
        b'<p>\n'
        b'It has no real weakness.\n'
        b'<k>\n'
        b'<p>\n'
        b"So just use whatever you've\n"
        b'got to beat it, OK?\n'
        b'<wait 250>\n'
        b"Let's go, Mario!!\n"
        b'<k>',

    b'btl_hlp_gundan_zako':
        b"That's an X-Naut.\n"
        b'<wait 250>\n'
        b"Says here it's classified\n"
        b'as an "enemy combatant."\n'
        b'<k>\n'
        b'<p>\n'
        b'That description stinks...\n'
        b'<wait 250>\n'
        b"You think it means they're\n"
        b'just low-level foot soldiers?\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'They drink these potions\n'
        b'that make them all big and\n'
        b'burly...<wait 250>then they attack! \n'
        b'<k>\n'
        b'<p>\n'
        b"These guys aren't all that\n"
        b"tough, and should go down\n"
        b"to strong spread attacks!\n"
        b'<k>',

    b'btl_hlp_gundan_zako_magician':
        b"That's an X-Naut PhD.\n"
        b'<wait 250>\n'
        b'Guys like this invent all the\n'
        b'annoying things we fight.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'The chemicals do all kinds of\n'
        b'stuff, like making things\n'
        b'huge or burning people.\n'
        b'<k>\n'
        b'<p>\n'
        b'He may also use potions to\n'
        b'heal himself or make himself\n'
        b'impossible to hit.\n'
        b'<k>\n'
        b'<p>\n'
        b'X-Naut PhDs are REALLY\n'
        b'annoying,<wait 250> so take them out\n'
        b'before they boost themselves.\n'
        b'<k>',

    b'btl_hlp_gundan_zako_elite':
        b"That's an Elite X-Naut.\n"
        b'<wait 250>\n'
        b'This guy is the creme de la\n'
        b'creme of the X-Nauts.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b"He'll use regular attacks as\n"
        b'well as boosting his power\n'
        b'or size.\n'
        b'<k>\n'
        b'<p>\n'
        b'Elite X-Nauts are tough,\n'
        b'but you just have to smack\n'
        b'away until they go down.\n'
        b'<k>',

    b'btl_hlp_barriern':
        b"That's a Yux.\n"
        b'<wait 250>\n'
        b"Says here it's a creature\n"
        b'created in the X-Naut labs.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'According to this, attacks\n'
        b"and items won't affect it if\n"
        b'it has Mini-Yux around it.\n'
        b'<k>\n'
        b'<p>\n'
        b'So, if any Mini-Yux appear,\n'
        b'take those out first. <wait 250>Duh!\n'
        b'<k>',

    b'btl_hlp_barriern_petit':
        b"That's a Mini-Yux.\n"
        b'<wait 250>\n'
        b'A creature made to protect\n'
        b'a Yux, it can split into two.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'These twerps are the reason\n'
        b"you sometimes can't do any\n"
        b'damage to the main Yux.\n'
        b'<k>\n'
        b'<p>\n'
        b"They're a pain, but you HAVE\n"
        b'to beat them before the Yux.\n'
        b'<wait 250>\n'
        b"Any spread attack'll do...\n"
        b'<k>',

    b'btl_hlp_barriern_z':
        b"That's a Z-Yux.\n"
        b'<wait 250>\n'
        b"It's a genetic improvement\n"
        b'on the original Yux.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'Multiple Mini-Z-Yux will\n'
        b'appear to protect the main\n'
        b"unit, as you'd expect.\n"
        b'<k>\n'
        b'<p>\n'
        b'Sometimes they also restore\n'
        b'HP to the main unit, which\n'
        b'really burns me up!\n'
        b'<k>\n'
        b'<p>\n'
        b"It doesn't have much HP, but\n"
        b'those barriers often protect\n'
        b'it from attacks.\n'
        b'<k>',

    b'btl_hlp_barriern_z_petit':
        b"That's a Mini-Z-Yux.\n"
        b'<wait 250>\n'
        b'It lives to protect the Z-Yux.\n'
        b'<wait 250>\n'
        b'There can be as many as 4.\n'
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b"If you don't clear these guys\n"
        b"out, you'll never be able to\n"
        b'attack the Z-Yux.\n'
        b'<k>\n'
        b'<p>\n'
        b'Using a multiple-strike attack\n'
        b'to take them out all at once\n'
        b'is a totally good idea.\n'
        b'<k>\n'
        b'<p>\n'
        b'I mean, I respect them for\n'
        b'protecting their...<wait 250>whatever,\n'
        b'but they get NO mercy!\n'
        b'<k>',

    b'btl_hlp_barriern_custom':
        b"That's an X-Yux.\n"
        b"<wait 250>Strangely, supposedly it's\n"
        b"read 'Cross'-Yux, not 'Ex'.\n"
        b'<k>\n'
        b'<p>\n'
        b'Anyway, this is a new Yux\n'
        b'designed to protect extra-\n'
        b'important areas...\n'
        b'<k>\n'
        b'<p>\n'
        b'They were designed to be\n'
        b"unbeatable.<wait 250> They're not.\n"
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'It can produce two\n'
        b'Mini-X-Yuxes at once.\n'
        b'<k>\n'
        b'<p>\n'
        b'AND, it has an excellent\n'
        b'success rate of immobilizing\n'
        b'opponents.\n'
        b'<k>\n'
        b'<p>\n'
        b'We gotta do something about\n'
        b"that if we're gonna win.\n"
        b'<k>',

    b'btl_hlp_barriern_custom_satellite':
        b"That's a Mini-X-Yux.\n"
        b'<wait 250>\n'
        b'It lives to protect the X-Yux.\n'
        b'<wait 250>\n'
        b'Up to 4 can guard the X-Yux.\n'
        b'<k>\n'
        b"%s"
        b'<p>\n'
        b"You can't attack the main\n"
        b'unit until you clear these\n'
        b'annoying pests out.\n'
        b'<k>\n'
        b'<p>\n'
        b'So use multiple-strike attacks\n'
        b'to take them out all at once!\n'
        b'<k>',

    b'btl_hlp_atomic_teresa':
        b"That's an Atomic Boo.\n"
        b'<wait 250>\n'
        b"It's a giant Boo made up of\n"
        b'a ton of smaller Boos.\n'
        b'<k>\n'
        b"%s"
        b'<p>\n'
        b'This thing will try to smoosh\n'
        b'us. It can also split up and\n'
        b'send hundreds of Boos at us.\n'
        b'<k>\n'
        b'<p>\n'
        b'And, when we attack, we\n'
        b'might get so scared that we\n'
        b"get confused or can't move.\n"
        b'<k>\n'
        b'<p>\n'
        b"I sure wouldn't want to see\n"
        b'this thing standing behind me\n'
        b'in the middle of the night...\n'
        b'<k>\n'
        b'<p><shake>\n'
        b'Creeeeeepy...\n'
        b'<k>',

    b'btl_hlp_hyper_sinnosuke':
        b"That's a Cosmic Boo.\n"
        b'<wait 250>\n'
        b"A distant relative of\n"
        b"Atomic Boo.\n"
        b"<k>\n"
        b'<p>\n'
        b"I've only ever heard urban\n"
        b"legends of these things, but\n"
        b"I guess it's real after all!\n"
        b'<k>\n'
        b"%s"
        b'<p>\n'
        b"This thing's a fair bit\n"
        b"stronger than your garden-\n"
        b"variety Atomic Boo...\n"
        b'<k>\n'
        b'<p>\n'
        b"AND it can use any of its\n"
        b"attacks without taking a\n"
        b"turn to charge! <wait 250>Yikes!\n"
        b'<k>\n'
        b'<p>\n'
        b"C'mon, Mario, let's pound\n"
        b"this thing into atoms, <wait 200>or\n"
        b"space dust, or whatever!\n"
        b'<k>',

    b'btl_hlp_gonbaba':
        b"That's Hooktail! <wait 250>I thought\n"
        b"she was gone for good, but\n"
        b"she's one tough customer!\n"
        b"<k>\n"
        b"%s"
        b"<p>\n"
        b"She'll attack with stomps\n"
        b"and fiery breath attacks, and\n"
        b"maybe dirtier tricks as well!\n"
        b"<k>\n"
        b"<p>\n"
        b"Her flame breath only burns\n"
        b"hotter with revenge, and can\n"
        b"sap our ATK with Burn status!\n"
        b"<k>\n"
        b"<p>\n"
        b"Let's show her we're still\n"
        b"the boss around here, Mario!\n"
        b"<k>",

    b'btl_hlp_bunbaba':
        b"That's Gloomtail, <wait 250>Hooktail's\n"
        b"elder brother. <wait 250>He must really\n"
        b"<wait 250>care to come all this way!\n"
        b"<k>\n"
        b"%s"
        b"<p>\n"
        b"He'll bite or stomp you, and\n"
        b"<wait 250>\n"
        b"he may also attack with\n"
        b"noxious or fiery breath.\n"
        b"<k>\n"
        b"<p>\n"
        b"When his HP gets low, he may\n"
        b"throw in some other attacks,\n"
        b"as well. <wait 250>Wouldn't surprise me.\n"
        b"<k>\n"
        b"<p>\n"
        b"Especially watch out for his\n"
        b"megabreath move, 'cause the\n"
        b"word is, it's GNARLY!\n"
        b"<k>\n"
        b"<p>\n"
        b"He also Charges up for some\n"
        b"attacks, so you might want\n"
        b"to dodge those.\n"
        b"<k>",
       
    b'btl_hlp_zonbaba':
        b"That's Bonetail! <wait 250>Wow, I can't\n"
        b"imagine how that bag of bones\n"
        b"made it all the way up here.\n"
        b"<k>\n"
        b"%s"
        b"<p>\n"
        b"Their breath attacks can\n"
        b"inflict a ton of different\n"
        b"status effects, so watch out!\n"
        b"<k>\n"
        b"<p>\n"
        b"If their HP falls far enough\n"
        b"below yours, they might heal\n"
        b"a bunch of HP at once!\n"
        b"<k>\n"
        b"<p>\n"
        b"Come on, Mario, we're this\n"
        b"close to finishing the fight!\n"
        b"Don't let up now!\n"
        b"<k>",

    b'btl_hlp_chorobon_gundan':
        b"It's a Fuzzy Horde!\n"
        b'<wait 250>\n'
        b"Look at 'em all!<wait 250> What a mob\n"
        b"scene!<wait 100><dynamic 3> It's crazy!</dynamic> \n"
        b'<k>\n'
        b'%s'
        b'<p>\n'
        b'Fuzzies come at you all at\n'
        b'once and attack in order.\n'
        b'<wait 250>\n'
        b'Says here not to freak out!\n'
        b'<k>\n'
        b'<p>\n'
        b'It also says that if you use\n'
        b'good timing to defend, you\n'
        b"won't take much damage.\n"
        b'<k>\n'
        b'<p>\n'
        b"I'm thinking it might just be\n"
        b'faster to whip the ringleader,\n'
        b"though, y'know?\n"
        b'<k>',
        
    b'btl_hlp_chorobon_gundan':
        b"It's a Fuzzy Horde!\n"
        b"<wait 250>\n"
        b"Look at 'em all!<wait 250> What a mob\n"
        b"scene!<wait 100><dynamic 3> It's crazy!</dynamic> \n"
        b"<k>\n"
        b"%s"
        b"<p>\n"
        b"Fuzzies come at you all at\n"
        b"once and attack in order.\n"
        b"<wait 250>\n"
        b"Says here not to freak out!\n"
        b"<k>\n"
        b"<p>\n"
        b"As the horde thins out,\n"
        b"they can attack with wildly\n"
        b"varying timing. Stay focused!\n"
        b"<k>\n"
        b"<p>\n"
        b"It'd be best if we can boost\n"
        b"our DEF, so we take less\n"
        b"damage on every hit.\n"
        b"<k>\n"
        b"<p>\n"
        b"We could attack Gold Fuzzy,\n"
        b"but he'll restore more HP if\n"
        b"the horde stays healthy!\n"
        b"<k>\n"
        b"<p>\n"
        b"If we manage to defeat him,\n"
        b"the horde should flee\n"
        b"immediately, though!\n"
        b"<k>",

    b'btl_hlp_gold_chorobon':
        b"That's Gold Fuzzy.\n"
        b"<wait 250>\n"
        b"Can't say I expected to run\n"
        b"into him all the way up here!\n"
        b"<k>\n"
        b"%s"
        b"<p>\n"
        b"Honestly, I don't know\n"
        b"what to expect from him.\n"
        b"He was pretty weak before...\n"
        b"<k>\n"
        b"<p>\n"
        b"But if he could whip a dragon\n"
        b"by himself, he must have\n"
        b"seriously leveled up!\n"
        b"<k>\n"
        b"<p>\n"
        b"Whatever the case, we'd\n"
        b"better stay on our toes!\n"
        b"<k>",
    
    # TODO: Maybe menu Tattles for all enemies - b"menu_enemy_001", etc.
    
    b'menu_enemy_400':
        b"[Dummy] Fuzzy Horde",
        
    # Misc. strings.
    
    b"msg_nameent_3":
        b"Choose a name for the seed!",
}

g_TowerLobbyStrings = {
        
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
    b"tot_optr_seed_random":        b"Random on start",
    b"tot_optr_preset":             b"Option Preset",
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
    b"tot_optr_secretboss_random":  b"Rarely Appears",
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
        b"magic, Contact Crusher!\n<k>\n<p>\n"
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

    # Boss field dialogue.
    
    # Hooktail before battle.
    b"tot_di030100_00":
        b"<boss>\n"
        b"Ah, <wait 250>so the mustachioed one\n"
        b"finally returns!\n"
        b"<k>",
    # + variants for later visits...
    b"tot_di030110_00":
        b"<boss>\n"
        b"Back again for more, are we?\n"
        b"<k>",
    
    # Gloomtail before battle.
    b"tot_di040100_00":
        b"<boss>\n"
        b"<dynamic 3>Halt!</dynamic> <wait 250>Who dares approach?\n"
        b"<k>",
    # + variants for later visits...
    b"tot_di040110_00":
        b"<boss>\n"
        b"Oho, you challenge me again?\n"
        b"<k>",
        
    # Dragon before battle with Gold Fuzzy.
    b"tot_di030300_00":
        b"<boss>\n"
        b"Oog...\n"
        b"<k>",
        
    # Gold Fuzzy before battle.
    b"tot_di060100_00":
        b"<wave>Meeeeeeeeeeeork!</wave> \n"
        b"<k>\n"
        b"<p>\n"
        b"Yooooou! <wait 250>You thought you\n"
        b"were so <dynamic 3>slick</dynamic>, beating us up\n"
        b"at the fortress before!\n"
        b"<k>\n"
        b"<p>\n"
        b"Well, there's strength in\n"
        b"numbers, bucko! <wait 250>And we're\n"
        b"stronger than ever before!\n"
        b"<k>\n"
        b"<p>\n"
        b"Get ready for a world of\n"
        b"hurt, buddy! <wait 250>It's go time!\n"
        b"<k>",
    # + variants for later visits...
    b"tot_di060110_00":
        b"You! <wait 250>You're back again for\n"
        b"more punishment, eh?\n"
        b"<k>\n"
        b"<p>\n"
        b"Well, last time was a fluke,\n"
        b"believe me! <wait 200>No way we lose to\n"
        b"your type again!\n"
        b"<k>",
        
    # Gold Fuzzy after battle.
    b"tot_di060300_00":
        b"Gack!\n"
        b"<k>",
        
    
    # Boss battle dialogue.
    
    # Hooktail battle entry.
    b"tot_di031000_00":
        b"<boss>\n"
        b"So, Mustache, you've returned\n"
        b"at last! <wait 250>I expected you might\n"
        b"try to do as much...\n"
        b"<k>\n"
        b"<p>\n"
        b"I reckoned that if the threat\n"
        b"of this tower wouldn't draw you,\n"
        b"<wait 250>the allure of treasure would!\n"
        b"<k>\n"
        b"<p>\n"
        b"You might not have fully bested\n"
        b"me before, <wait 200>but I recognize that\n"
        b"I still sold you quite short.\n"
        b"<k>\n"
        b"<p>\n"
        b"Despite my prior tricks, we\n"
        b"dragons do try to be honorable\n"
        b"sorts, <wait 250>so how about this...\n"
        b"<k>\n"
        b"<p>\n"
        b"I shall remain here at the top\n"
        b"of my tower, as long as you\n"
        b"continue to challenge me.\n"
        b"<k>\n"
        b"<p>\n"
        b"In exchange, I shall fight you\n"
        b"tooth and nail. <wait 250>I assure you, I'll\n"
        b"not fall as easily as before!\n"
        b"<k>\n"
        b"<p>\n"
        b"Now then, mustachioed one,\n"
        b"<wait 250>we fight!\n"
        b"<k>",
    # + variants for later visits...
    b"tot_di031010_00":
        b"<boss>\n"
        b"So, you've returned again!\n"
        b"<wait 250>Let's not waste any time, then.\n"
        b"<wait 250>Have at you!\n"
        b"<k>",
    
    # Hooktail low health.
    b"tot_di031100_00":
        b"<boss>\n"
        b"Ugggh... <wait 250>You still put up quite\n"
        b"a fight, indeed...\n"
        b"<k>",
    
    # Hooktail phase 2 start.
    b"tot_di031200_00":
        b"<boss>\n"
        b"You may put up quite a fight,\n"
        b"<wait 200>but I'm not through yet!\n"
        b"<wait 250>Take this, little morsels!\n"
        b"<k>",
    
    # Hooktail - cutscene after first time bite lands.
    b"tot_di031600_00":
        b"<boss>\n"
        b"Gwa-HA! <wait 250>How do you like that,\n"
        b"you puny pugilists? <wait 250>You're no\n"
        b"match for brute strength!\n"
        b"<k>",
    
    # Hooktail - partner bite reactions.
    b"tot_di031700_00_kur":
        b"You big bully! <wait 250>Ooh, I'm boiling...\n"
        b"<wait 300>Next time she tries that, sock\n"
        b"her in the face, <wait 200>got it?\n"
        b"<k>",
    b"tot_di031700_00_nok":
        b"Man, she's really going on...\n"
        b"<wait 300>I bet we could probably hit her\n"
        b"back if she tries it again!\n"
        b"<k>",
    b"tot_di031700_00_win":
        b'Hmph, "brute" is the word;\n'
        b"<wait 250>that much we agree on.\n"
        b"<k>\n"
        b"<p>\n"
        b"Next time that blowhard\n"
        b"comes at us, dear, let's strike,\n"
        b"<wait 200>and still that brute's tongue!\n"
        b"<k>",
    b"tot_di031700_00_yos":
        b"Man,<wait 150> oh,<wait 150> <dynamic 3>MAN</dynamic>, can she talk!\n"
        b"<wait 250>What a bag of wind!\n"
        b"<k>\n"
        b"<p>\n"
        b"Y'know, Gonzales, <wait 200>I'll bet if we\n"
        b"tried to counter her next time,\n"
        b"she won't see it coming!\n"
        b"<k>",
    b"tot_di031700_00_viv":
        b"Oh dear, <wait 250>that's pretty rude\n"
        b"of her, wouldn't you say, Mario?\n"
        b"<k>\n"
        b"<p>\n"
        b"Come on, Mario, I won't stand\n"
        b"for her bullying! Punch her the\n"
        b"next time she tries that!\n"
        b"<k>",
    b"tot_di031700_00_bom":
        b"I say, chap, that's rather\n"
        b"unsporting of her, eh wot?\n"
        b"<k>\n"
        b"<p>\n"
        b"Such a blabbering mouth\n"
        b"deserves a sharp comeback,\n"
        b"wouldn't you say?\n"
        b"<k>",
    b"tot_di031700_00_chu":
        b"Oh, dearie, we really ought\n"
        b"to have seen that coming...\n"
        b"<k>\n"
        b"<p>\n"
        b"Let's keep on our toes\n"
        b"next time, and hit her right\n"
        b"in the kisser!\n"
        b"<k>",
    
    # Hooktail death.
    b"tot_di031800_00":
        b"<boss>\n"
        b"Nooo....! <wait 250>Gloomtail, <wait 150>brother,\n"
        b"<wait 250>you must avenge me again!\n"
        b"<k>",
    # + variants for later visits...
    b"tot_di031810_00":
        b"<boss>\n"
        b"Nooo....! <wait 250>Urp... <wait 150>bested again...\n"
        b"<k>",
    
    # Gloomtail battle entry.
    b"tot_di041000_00":
        b"<boss>\n"
        b"<dynamic 3>You!</dynamic> <wait 250>You mortals dared to\n"
        b"make a mockery of my dear\n"
        b"sister, once again!\n"
        b"<k>\n"
        b"<p>\n"
        b"And yet I'm to understand\n"
        b"this was a fair challenge,\n"
        b"<wait 250>and of her own choosing...\n"
        b"<k>\n"
        b"<p>\n"
        b"Be that as it might, daring to\n"
        b"challenge me as well was quite\n"
        b"foolish, I assure you.\n"
        b"<k>\n"
        b"<p>\n"
        b"I shall use every ounce of my\n"
        b"power to fell you this time!\n"
        b"<k>",
    # + variants for later visits...
    b"tot_di041010_00":
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
    b"tot_di041100_00":
        b"<boss>\n"
        b"Mmmmph...<wait 250> How can this be...\n"
        b"<k>",
    
    # Gloomtail phase 2 start.
    b"tot_di041200_00":
        b"<boss>\n"
        b"GWAAAAH HA HA HA HA!\n"
        b"<wait 250>\n"
        b"You are as tough as ever, my\n"
        b"little tasty morsels!\n"
        b"<k>\n"
        b"<p>\n"
        b"But how will you fare\n"
        b"against THIS!!!\n"
        b"<k>",
    
    # Gloomtail phase 3 start.
    b"tot_di041300_00":
        b"<boss>\n"
        b"Gwuhhh...\n"
        b"<k>\n"
        b"<p>\n"
        b"You whelps are still proving\n"
        b"more troublesome than I had\n"
        b"anticipated...\n"
        b"<k>\n"
        b"<p>\n"
        b"...So I will show you the true\n"
        b"extent of my power!\n"
        b"<k>",
    
    # Gloomtail megabreath.
    b"tot_di041400_00":
        b"<boss>\n"
        b"<dynamic 3>\n"
        b"<scale 2>\n"
        b"<pos 15 10>\n"
        b"MEGABREATH!\n"
        b"<k>",
    # + alternates with very low chance of appearing...
    b"tot_di041410_00":
        b"<boss>\n"
        b"<scale 0.5>MEGABREATH---!<wait 400> \n"
        b"<scale 1>Gack... <wait 250>Sorry... <wait 250>my voice is gone\n"
        b"today, it seems...\n"
        b"<k>\n"
        b"<p>\n"
        b"<scale 0.5>Ahem...<wait 350> \n"
        b"<dynamic 3>\n"
        b"<scale 2>\n"
        b"<pos 15 10>\n"
        b"MEGABREATH!\n"
        b"<k>",
    b"tot_di041411_00":
        b"<boss>\n"
        b"<scale 2>GIGABREATH!\n"
        b"<pos 0 62>\n"
        b"<wait 250>\n"
        b"<scale 0.5>Mmm... <wait 140>perhaps a bit too much?\n"
        b"<k>",
    b"tot_di041412_00":
        b"<boss>\n"
        b"<scale 2>MEGA-GEOFF!\n"
        b"<pos 0 62>\n"
        b"<wait 250>\n"
        b"<scale 0.5>Wait, that's not quite right...\n"
        b"<k>",
    
    # Gloomtail death.
    b"tot_di041800_00":
        b"<boss>\n"
        b"No... <wait 250>No...<wait 250> It can't be true!\n"
        b"<k>",
    # + variants for later visits...
    b"tot_di041810_00":
        b"<boss>\n"
        b"Ugh...<wait 250> ngh... <wait 350>Well met, <wait 200>mustache!\n"
        b"<k>",
    
    # Gloomtail fake death.
    b"tot_di041900_00":
        b"<boss>\n"
        b"Curses! <wait 250>No... <wait 250>I can't have been\n"
        b"undone, yet again!\n"
        b"<k>",
    
    # Bonetail battle entry + partner reactions.
    b"tot_di051000_00":
        b"<majo>\n"
        b"<col ffffffff>\n"
        b"<shake>\n"
        b"\n"
        b"AROOOOOOOOOOOOO!</shake>\n"
        b"<k>",
    b"tot_di051000_01_kur":
        b"Jeepers! <wait 250>It's that hunk of\n"
        b"bones from the Pit! <wait 250>How'd it\n"
        b"even GET up here?\n"
        b"<k>",
    b"tot_di051000_01_nok":
        b"Yikes! <wait 250>Did they lug that all\n"
        b"the way up HERE, or did it get\n"
        b"here on its own? <wait 350>Spooky...\n"
        b"<k>",
    b"tot_di051000_01_win":
        b"My word! <wait 250>I can't imagine how\n"
        b"THIS lumbering mass of bones\n"
        b"made it up here...\n"
        b"<k>",
    b"tot_di051000_01_yos":
        b"Yow! <wait 250>I knew that bag of bones\n"
        b"was one tough customer, but\n"
        b"making the trip up HERE?\n"
        b"<k>",
    b"tot_di051000_01_viv":
        b"Oh dear! <wait 250>Did they drag this\n"
        b"poor thing up here? <wait 250>Unless it \n"
        b"made it on its own... <wait 250>my!\n"
        b"<k>",
    b"tot_di051000_01_bom":
        b"Well, this is rather a surprise,\n"
        b"I do say... <wait 250>How in blazes did this\n"
        b"make the journey up here...!?\n"
        b"<k>",
    b"tot_di051000_01_chu":
        b"My, how fascinating! <wait 250>I suppose\n"
        b"it goes to show you can't count\n"
        b"anyone out easily...\n"
        b"<k>",
    # + alternate, with no partner commentary...
    b"tot_di051010_00":
        b"<majo>\n"
        b"<col ffffffff>\n"
        b"<shake>\n"
        b"\n"
        b"AROOOOOOOOOOOOO!</shake>\n"
        b"<k>",
    
    # Bonetail low health.
    b"tot_di051100_00":
        b"<majo>\n"
        b"<col ffffffff>\n"
        b"<shake>\n"
        b"\n"
        b"A<wait 100>rooo<wait 100>ooo<wait 100>.......</shake>\n"
        b"<k>",
    b"tot_di051100_01_kur":
        b"Mario, look! I think we've worn\n"
        b"them down... <wait 250>just a bit more, and\n"
        b"they oughtta be finished!\n"
        b"<k>",
    b"tot_di051100_01_nok":
        b"Boy, that thing's looking pretty\n"
        b"shabby, I think. <wait 250>C'mon, Mario, let's\n"
        b"keep it up just a bit longer!\n"
        b"<k>",
    b"tot_di051100_01_win":
        b"That poor beast looks quite\n"
        b"worn out, dear. Let's hurry it up\n"
        b"and give them some rest!\n"
        b"<k>",
    b"tot_di051100_01_yos":
        b"Come on, Gonzales, we've got\n"
        b"'em on the ropes! <wait 250>We can't\n"
        b"give up now!\n"
        b"<k>",
    b"tot_di051100_01_viv":
        b"It seems that they're getting\n"
        b"quite tired... <wait 250>Don't give up now,\n"
        b"Mario!\n"
        b"<k>",
    b"tot_di051100_01_bom":
        b"Come on, old boy! <wait 250>We're this\n"
        b"close to bringing this fell beast\n"
        b"to its knees!\n"
        b"<k>",
    b"tot_di051100_01_chu":
        b"It appears we're wearing them\n"
        b"down, dearie. <wait 250>Let's keep it up,\n"
        b"and dispose of them quickly!\n"
        b"<k>",
    
    # Bonetail phase 2 start.
    b"tot_di051200_00":
        b"<majo>\n"
        b"<col ffffffff>\n"
        b"<shake>\n"
        b"\n"
        b"AROOOOOOOOOOOOO!</shake>\n"
        b"<k>",
    
    # Bonetail healing.
    b"tot_di051500_00":
        b"<majo>\n"
        b"<col ffffffff>\n"
        b"<shake>\n"
        b"\n"
        b"AROOOOOOOOOOOOO!</shake>\n"
        b"<k>",
    
    # Bonetail death.
    b"tot_di051800_00":
        b"<majo>\n"
        b"<col ffffffff>\n"
        b"<shake>\n"
        b"\n"
        b"A<wait 100>rooo<wait 100>rooo<wait 100>............<wait 100>rooooo......\n"
        b"<k>",
    
    # Gold Fuzzy mid-fight dialogue.
    b"tot_di061000_00":
        b"Meee-OOOF!<wait 250> Hey!<wait 250> Ow!\n"
        b"<wait 250>\nYou chumps are tougher than\n"
        b"you look...\n<k>\n<p>\n"
        b"But I'm not through yet!\n<wait 250>\n"
        b"Everyone! GET 'EM!!!\n<k>",
}

g_PetalburgWestStrings = {

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
        b"<p>\n"
        b"If you've come across an item\n"
        b"or badge in the tower, you\n"
        b"might see it on the shelf here.\n"
        b"<k>\n"
        b"<p>\n"
        b"If something catches your eye,\n"
        b"just walk up to it and I can\n"
        b"sell it for a top-notch price!\n"
        b"<k>\n"
        b"<p>\n"
        b"If there's another item you\n"
        b"know you've seen before, but\n"
        b"it isn't on display right now,\n"
        b"<k>\n"
        b"<p>\n"
        b"read that sign back there to\n"
        b"get it on back-order. It'll cost\n"
        b"you a bit extra, though!\n"
        b"<k>\n"
        b"<p>\n"
        b"Oh, one more thing; the items\n"
        b"and badges you buy here can't\n"
        b"be used here in town...\n"
        b"<k>\n"
        b"<p>\n"
        b"But if you buy enough of each,\n"
        b"I'll give you a special reward\n"
        b"to view your collection!\n"
        b"<k>\n"
        b"<p>\n"
        b"You can also use it to select\n"
        b"a few to bring into the tower\n"
        b"on your next run!\n"
        b"<k>\n"
        b"<p>\n"
        b"Want me to run that all\n"
        b"by you again?\n"
        b"<o>",

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
        
    # Sign outside of shop.
    b"tot_shop_signtut":
        b"<kanban>\n"
        b"Welcome to Niff T.'s Shop,\n"
        b"your one-shop stop for all kinds\n"
        b"of useful items and badges!\n"
        b"<k>\n"
        b"<p>\n"
        b"Buy items and badges here to\n"
        b"add them to your permanent\n"
        b"collection!\n"
        b"<k>\n"
        b"<p>\n"
        b"Buy 5 each of items and badges,\n"
        b"and you'll be able to pick some\n"
        b"to bring into the tower!\n"
        b"<k>",
        
    # NPC A (Koopa near west entrance) dialogue.

    # "First visit" cutscene.
    
    b"tot_di011100_00":
        b"Oh, Mario! <wait 100>You got here\n"
        b"just in time! <wait 200>I suppose you're\n"
        b"aware that dragon's back!?\n<k>",
    
    b"tot_di011100_01":
        b"Oh, it's terrible! It seems\n"
        b"the castle's been fortified\n"
        b"considerably since last time.\n<k>\n<p>\n"
        b"Tons of rooms, <wait 100>overrun with\n"
        b"all sorts of enemies, <wait 200>and\n"
        b"who knows what else?\n<k>\n<p>\n"
        b"I don't know what she's\n"
        b"planning, <wait 150>but something's\n"
        b"gotta be done about it!\n<k>",
    
    b"tot_di011100_02":
        b"I'm glad to hear you say\n"
        b"that, Mario. <wait 150>We all owe you\n"
        b"an eternal debt already!\n<k>\n<p>\n"
        b"I know there's not much we\n"
        b"can do to help right now, <wait 150>but\n"
        b"we're behind you all the way!\n<k>\n<p>\n"
        b"Please do come on back if you\n"
        b"run into trouble! <wait 200>Rest assured,\n"
        b"we'll keep this town safe.\n<k>",
        
    # Post-first run and second run cutscenes.
    
    b"tot_di011200_00":
        b"Oh, hi! <wait 150>Glad to see you're\n"
        b"still doing well, Mario. <wait 250>How did\n"
        b"things go at the castle?\n"
        b"<k>",

    b"tot_di011200_01":
        b"<dynamic 3>What?</dynamic> <wait 250>You're saying you beat\n"
        b"Hooktail, <wait 150>but her elder brother\n"
        b"is coming to avenge her?\n"
        b"<k>",

    b"tot_di011200_02":
        b"I see... <wait 250>So all they want is\n"
        b"a challenge, and they're not\n"
        b"up to anything sinister?\n"
        b"<k>\n"
        b"<p>\n"
        b"Well, that's a great relief.\n"
        b"<wait 250>We knew we could count on\n"
        b"you to pull through, Mario!\n"
        b"<k>\n"
        b"<p>\n"
        b"Still, Hooktail's no pushover.\n"
        b"<wait 250>If her brother's even tougher,\n"
        b"you'd better be prepared!\n"
        b"<k>\n"
        b"<p>\n"
        b"Good news is, I heard the\n"
        b"shop's just been re-opened!\n"
        b"<wait 250>Why don't you pop on by?\n"
        b"<k>\n"
        b"<p>\n"
        b"If you need anything else,\n"
        b"try asking around town. <wait 250>I'm sure\n"
        b"lots of folks will have advice!\n"
        b"<k>",

    b"tot_di011300_00":
        b"Oh, welcome back, Mario!\n"
        b"<wait 250>I suppose things went all right\n"
        b"again this time?\n"
        b"<k>",

    b"tot_di011300_01":
        b"You've defeated both\n"
        b"Hooktail and Gloomtail, eh?\n"
        b"<wait 250>That's wonderful to hear!\n"
        b"<k>\n"
        b"<p>\n"
        b"I imagine they're the hardy\n"
        b"sorts, though. <wait 250>They'll be ready\n"
        b"to scrap again anytime!\n"
        b"<k>\n"
        b"<p>\n"
        b"If it's not too much to ask,\n"
        b"<wait 200>could you keep challenging them\n"
        b"whenever you have the time?\n"
        b"<k>",

    b"tot_di011300_02":
        b"Ah, I knew I could count on\n"
        b"you, Mario. <wait 250>I'm sure everyone'll\n"
        b"greatly appreciate it.\n"
        b"<k>\n"
        b"<p>\n"
        b"By the way, it seems some\n"
        b"traveling vendors just moved in\n"
        b"while you were away...\n"
        b"<k>\n"
        b"<p>\n"
        b"They're over in the square\n"
        b"to the east, and have some\n"
        b"fancy trinkets for sale.\n"
        b"<k>\n"
        b"<p>\n"
        b"They only accept a special\n"
        b"sort of currency that I don't\n"
        b"imagine most folks here have.\n"
        b"<k>\n"
        b"<p>\n"
        b'Some sort of "star bits", or\n'
        b"something like that. <wait 250>Perhaps\n"
        b"you've come across them?\n"
        b"<k>",
    
    # Default message, and general comments on previous run.

    b"tot_di011000_00":   
        b"Keep up the good work,\n"
        b"Mario! <wait 250>We know you can do it!\n"
        b"<k>",

    b"tot_di011400_00":
        b"How's it going, Mario?\n"
        b"<wait 250>Ah, not so well? <wait 200>Well, that's\n"
        b"a shame...\n"
        b"<k>\n"
        b"<p>\n"
        b"Better luck next time!\n"
        b"<k>",

    b"tot_di011401_00":
        b"How's it going, Mario?\n"
        b"<wait 250>Ah, so you were defeated by\n"
        b"ol' Hooktail, eh?\n"
        b"<k>\n"
        b"<p>\n"
        b"Well, at least you made it\n"
        b"there, and lost fair and square.\n"
        b"<wait 250>Better luck next time!\n"
        b"<k>",

    b"tot_di011402_00":
        b"How's it going, Mario?\n"
        b"<wait 250>Ah, met your match against\n"
        b"Gloomtail, you say?\n"
        b"<k>\n"
        b"<p>\n"
        b"Well, he's certainly tough,\n"
        b"I imagine. <wait 250>But you've beaten\n"
        b"him before, so don't give up!\n"
        b"<k>",

    b"tot_di011403_00":
        b"How's it going, Mario?\n"
        b"<wait 250>Beaten by... Bonetail, is it?\n"
        b"<k>\n"
        b"<p>\n"
        b"I can't say I've heard that\n"
        b"name. <wait 250>Definitely sounds like\n"
        b"a formidable foe, though.\n"
        b"<k>\n"
        b"<p>\n"
        b"Good thing you got to work\n"
        b"up to fighting it by beating\n"
        b"its siblings first, right?\n"
        b"<k>",

    b"tot_di011404_00":
        b"How's it going, Mario?\n"
        b"<wait 250>Mobbed by %s,\n"
        b"you say? Fascinating...\n"
        b"<k>\n"
        b"<p>\n"
        b"I thought things had seemed\n"
        b"awfully quiet at Shhwonk\n"
        b"Fortress these days...\n"
        b"<k>\n"
        b"<p>\n"
        b"Well, I'm sure you'll find out\n"
        b"how to lick them yet, Mario!\n"
        b"<wait 250>Go get 'em!\n"
        b"<k>",

    b"tot_di011405_00":
        b"How's it going, Mario?\n"
        b"<wait 250>Yikes, ended up on the wrong\n"
        b"end of Bobbery's explosives?\n"
        b"<k>\n"
        b"<p>\n"
        b"Man, those things can be\n"
        b"dangerous, I get it. <wait 250>Maybe try\n"
        b"to be more careful next time!\n"
        b"<k>",

    b"tot_di011410_00":
        b"How's it going, Mario?\n"
        b"<wait 300>Your last run got cut short\n"
        b"by %s, eh?\n"
        b"<k>\n"
        b"<o>",

    b"tot_di011411_00":
        b"How's it going, Mario?\n"
        b"<wait 300>You don't say? <wait 150>KO'd by\n"
        b"%s...\n"
        b"<k>\n"
        b"<o>",

    b"tot_di011412_00":
        b"How's it going, Mario?\n"
        b"<wait 250>I see, <wait 200>you were done in by\n"
        b"%s...\n"
        b"<k>\n"
        b"<o>",

    b"tot_di011420_00":
        b"<p>\n"
        b"Yikes, that's gotta be\n"
        b"embarrassing for you.\n"
        b"<wait 250>Well, better luck next time!\n"
        b"<k>",

    b"tot_di011421_00":
        b"<p>\n"
        b"Hey man, I get it. <wait 250>We all\n"
        b"have our off days, y'know?\n"
        b"<wait 250>Well, better luck next time!\n"
        b"<k>",

    b"tot_di011422_00":
        b"<p>\n"
        b"Wow, that's rough. <wait 250>Who'd have\n"
        b"thought they'd be so tough...\n"
        b"<wait 250>Well, better luck next time!\n"
        b"<k>",

    b"tot_di011423_00":
        b"<p>\n"
        b"Understandable, they can\n"
        b"be pretty tough customers.\n"
        b"<wait 250>Well, better luck next time!\n"
        b"<k>",

    b"tot_di011424_00":
        b"<p>\n"
        b"Oof, that's rough. <wait 250>Those guys\n"
        b"are no joke, I imagine.\n"
        b"<wait 250>Well, better luck next time!\n"
        b"<k>",

    b"tot_di011425_00":
        b"<p>\n"
        b"Yow, I wouldn't want to have\n"
        b"to face those either.\n"
        b"<wait 250>Well, better luck next time!\n"
        b"<k>",

    b"tot_di011430_00":
        b"How's it going, Mario?\n"
        b"<wait 250>Finished another run? That's\n"
        b"good to hear...\n"
        b"<k>\n"
        b"<p>\n"
        b"Keep up the good work,\n"
        b"Mario! <wait 250>We know you can do it!\n"
        b"<k>",

    b"tot_di011431_00":
        b"Wow, Mario! I heard you\n"
        b"took on the dragons' extra-\n"
        b"difficult tower challenge!\n"
        b"<k>\n"
        b"<p>\n"
        b"That must have been a\n"
        b"really tough feat! <wait 250>You should\n"
        b"feel quite accomplished.\n"
        b"<k>\n"
        b"<p>\n"
        b"Keep up the good work,\n"
        b"Mario! <wait 250>We know you can do it!\n"
        b"<k>",

    b"tot_di011432_00":
        b"Wow, back already, Mario?\n"
        b"<wait 250>That's got to be the fastest\n"
        b"run you've done yet!\n"
        b"<k>\n"
        b"<p>\n"
        b"Keep up the good work,\n"
        b"Mario! <wait 250>We know you can do it!\n"
        b"<k>",

    b"tot_di011433_00":
        b"Hey, Mario! <wait 250>It seems you're\n"
        b"really ramping up the intensity\n"
        b"of these challenges!\n"
        b"<k>\n"
        b"<p>\n"
        b"Keep up the good work,\n"
        b"Mario! <wait 250>We know you can do it!\n"
        b"<k>",
        
    # NPC F (Bub-ulber) dialogue.
    
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
    
    # NPC C - Little Toad
    
    b"tot_di003000_00":
        b"Hey, man, what's up?\n"
        b"<wait 250>I've been playing a ton of\n"
        b"different games lately...\n"
        b"<k>\n"
        b"<p>\n"
        b"Come by some time and we\n"
        b"can chat about 'em all!\n"
        b"<k>",

    b"tot_di003100_00":
        b"Hey, what's up? <wait 250>I've been\n"
        b"checking out some classic\n"
        b"game mods recently.\n"
        b"<k>\n"
        b"<p>\n"
        b"You ever play the original\n"
        b'"Pit Randomizer"' b" for TTYD?\n"
        b"It's so cool!\n"
        b"<k>\n"
        b"<p>\n"
        b"Choosing the right items\n"
        b"and badges to carry over\n"
        b"between runs is crucial...\n"
        b"<k>\n"
        b"<p>\n"
        b"I'm on my ninth time through\n"
        b"this run and enemies are\n"
        b"doing <dynamic 3>triple</dynamic> damage!\n"
        b"<k>\n"
        b"<p>\n"
        b"I'm going to try to keep it\n"
        b"up as long as I can!\n"
        b"<k>",

    b"tot_di003101_00":
        b"Hey, what's shaking?\n"
        b"<wait 250>I've been playing Nintendo 64\n"
        b"games again recently.\n"
        b"<k>\n"
        b"<p>\n"
        b"This " b'"Black Pit"' b" mod's really\n"
        b"got it all! <wait 250>Stylishes, <wait 150>costumes,\n"
        b"<wait 150>a bunch of achievements...\n"
        b"<k>\n"
        b"<p>\n"
        b"I beat every achievement\n"
        b"I could without upgrading HP\n"
        b"once! Man, what a fun time!\n"
        b"<k>\n"
        b"<p>\n"
        b"I oughtta try beating the\n"
        b"Roguelike mode with all my\n"
        b"stats at minimum sometime.\n"
        b"<k>\n"
        b"<p>\n"
        b"Might even be a secret\n"
        b"achievement for it! <wait 150>Probably\n"
        b"not, but you never know...\n"
        b"<k>",

    b"tot_di003102_00":
        b"What's up, man? <wait 250>I've been\n"
        b"getting back into" b' "Infinite Pit"\n'
        b"these days!\n"
        b"<k>\n"
        b"<p>\n"
        b"I could try to go for a\n"
        b"super-long run and get to,\n"
        b"like, floor 2,500 or such...\n"
        b"<k>\n"
        b"<p>\n"
        b"But I keep coming up with\n"
        b"weird combos of options I\n"
        b"want to try out instead!\n"
        b"<k>\n"
        b"<p>\n"
        b"I've also been training up\n"
        b"for doing races with friends\n"
        b"on the same starting seed!\n"
        b"<k>\n"
        b"<p>\n"
        b"My best time's somewhere\n"
        b"in the 1:40's, but I bet I'll get\n"
        b"a sub-90 with some luck!\n"
        b"<k>",

    b"tot_di003103_00":
        b"Hey, I gotta say, nothing like\n"
        b"playing a good old GameCube\n"
        b"classic sometimes, right?\n"
        b"<k>\n"
        b"<p>\n"
        b"I've been grinding checkbox\n"
        b"completion for the City Trial\n"
        b"mode in Kirby Air Ride!\n"
        b"<k>\n"
        b"<p>\n"
        b"Man, I love the sound it\n"
        b"makes when stuff unlocks!\n"
        b"<wait 250><dynamic 3>BAM!</dynamic>\n"
        b" <wait 250><dynamic 3>BASH!</dynamic>\n"
        b" <wait 250><dynamic 3>DOOSH!</dynamic> \n"
        b"<k>\n"
        b"<p>\n"
        b"They've really gotta make\n"
        b"a sequel sometime!\n"
        b"<k>",

    b"tot_di003104_00":
        b"What's up, man? <wait 250>I've been\n"
        b"playing my Nintendo Switch\n"
        b"a lot recently.\n"
        b"<k>\n"
        b"<p>\n"
        b"I didn't expect to play much\n"
        b"of Kirby Fighters 2, but the\n"
        b"story mode is awesome!\n"
        b"<k>\n"
        b"<p>\n"
        b"It's kind of a Rogue-like deal\n"
        b"where you climb up a tower\n"
        b"and fight a bunch of guys...\n"
        b"<k>\n"
        b"<p>\n"
        b"Then you get to pick a\n"
        b"power-up to upgrade yourself\n"
        b"with after every floor!\n"
        b"<k>\n"
        b"<p>\n"
        b"Man, that's fun! They should\n"
        b"put Rogue-like modes like that\n"
        b"into more games!\n"
        b"<k>",

    b"tot_di003105_00":
        b"I've gotta say, Hades might\n"
        b"be one of my favorite games\n"
        b"of all time!\n"
        b"<k>\n"
        b"<p>\n"
        b"There's like, a billion ways\n"
        b"a build can shake out with\n"
        b"all those boons and such.\n"
        b"<k>\n"
        b"<p>\n"
        b"Building your loadout up\n"
        b"one boon or one power-up at\n"
        b"a time is totally rad, too!\n"
        b"<k>\n"
        b"<p>\n"
        b"And man, trying to clear high\n"
        b"Heat runs is going to bring me\n"
        b"back over and over again!\n"
        b"<k>\n"
        b"<p>\n"
        b"I gotta check out the sequel\n"
        b"in early access one of these\n"
        b"days! Maybe it's even better!\n"
        b"<k>",

    b"tot_di003200_00":
        b"Hey, man! <wait 250>I've been playing\n"
        b"my Nintendo Switch a lot\n"
        b"again recently.\n"
        b"<k>\n"
        b"<p>\n"
        b"You know they re-released\n"
        b"The Thousand-Year Door on\n"
        b"Switch? It totally rocks!\n"
        b"<k>\n"
        b"<p>\n"
        b"I beat the whole game in\n"
        b"less than a week, including\n"
        b"all the bonus stuff!\n"
        b"<k>\n"
        b"<p>\n"
        b"Not going to spoil it, but man,\n"
        b"that new fight they added\n"
        b"was something else!\n"
        b"<k>\n"
        b"<p>\n"
        b"Never would've found it\n"
        b"myself without hearing about\n"
        b"it from friends, though.\n"
        b"<k>\n"
        b"<p>\n"
        b"I gotta be on the lookout\n"
        b"for secret fights like that\n"
        b"in other games, now!\n"
        b"<k>",
        
    # Innkeeper.
    
    b"tot_di004000_00":
        b"Hi, Mario! <wait 250>You're looking fit\n"
        b"now, so I imagine you won't\n"
        b"need our services...\n"
        b"<k>\n"
        b"<p>\n"
        b"Actually... <wait 250>truthfully, no one's\n"
        b"used our inn in a long time.\n"
        b"<k>\n"
        b"<p>\n"
        b"Since the shop overstocked\n"
        b"on Sleepy Sheep, we've just\n"
        b"been giving them away!\n"
        b"<k>\n"
        b"<p>\n"
        b"Anyway<wait 100>.<wait 100>.<wait 100>.<wait 200> if you ever need\n"
        b"tips on how to stay healthy,\n"
        b"feel free to ask me!\n"
        b"<k>",

    b"tot_di004100_00":
        b"Hi, Mario! <wait 250>I hope you've been\n"
        b"on the lookout for dangerous\n"
        b"status conditions...\n"
        b"<k>\n"
        b"<p>\n"
        b"Poison status is no joke!\n"
        b"It starts fairly weak, but its\n"
        b"damage builds up over time.\n"
        b"<k>\n"
        b"<p>\n"
        b"You'll also take more damage\n"
        b"if you get afflicted by it\n"
        b"multiple times in a row...\n"
        b"<k>\n"
        b"<p>\n"
        b"If you cure it, or if it runs its\n"
        b"course, the stacking effects\n"
        b"will reset, thankfully!\n"
        b"<k>\n"
        b"<p>\n"
        b"Not that I'm recommending it,\n"
        b"but I'll bet repeated Poison on\n"
        b"high-HP foes is effective!\n"
        b"<k>",

    b"tot_di004101_00":
        b"Hi, Mario! <wait 250>I hope you've been\n"
        b"on the lookout for dangerous\n"
        b"status conditions...\n"
        b"<k>\n"
        b"<p>\n"
        b"Like Burn status! If you're\n"
        b"burned, you'll be less effective\n"
        b"on offense AND defense.\n"
        b"<k>\n"
        b"<p>\n"
        b"You'll take 1 damage per\n"
        b"turn, and your attacks will\n"
        b"also do 1 damage less!\n"
        b"<k>\n"
        b"<p>\n"
        b"At least it protects you from\n"
        b"being frozen for a while. Still,\n"
        b"better cure it if you can!\n"
        b"<k>",

    b"tot_di004102_00":
        b"Hi, Mario! <wait 250>I hope you've been\n"
        b"on the lookout for dangerous\n"
        b"status conditions...\n"
        b"<k>\n"
        b"<p>\n"
        b"Getting frozen solid doesn't\n"
        b"do much by itself, but it leaves\n"
        b"you incredibly vulnerable!\n"
        b"<k>\n"
        b"<p>\n"
        b"One hit will thaw you out\n"
        b"immediately, but that hit will\n"
        b"do double damage!\n"
        b"<k>\n"
        b"<p>\n"
        b"Be very careful around icy\n"
        b"foes or Ice Storm items, or your\n"
        b"HP can go just like that!\n"
        b"<k>",

    b"tot_di004103_00":
        b"Hi, Mario! <wait 250>I hope you've been\n"
        b"on the lookout for dangerous\n"
        b"status conditions...\n"
        b"<k>\n"
        b"<p>\n"
        b"Being shrunk with Tiny status\n"
        b"doesn't hurt, but it'll halve\n"
        b"your attacks' damage!\n"
        b"<k>\n"
        b"<p>\n"
        b"Similarly, attacks from Huge\n"
        b"foes will do 50% more damage\n"
        b"than usual! <wait 150>Be careful!\n"
        b"<k>\n"
        b"<p>\n"
        b"Try to keep an eye out for\n"
        b"ways to use Huge and Tiny\n"
        b"status against foes instead!\n"
        b"<k>",

    b"tot_di004104_00":
        b"Hi, Mario! <wait 250>I hope you've been\n"
        b"on the lookout for dangerous\n"
        b"status conditions...\n"
        b"<k>\n"
        b"<p>\n"
        b"Stop status is the most\n"
        b"dangerous of all, leaving you\n"
        b"completely helpless!\n"
        b"<k>\n"
        b"<p>\n"
        b"Worst of all, midboss enemies\n"
        b"can't be Stopped at all, only\n"
        b"inflicted with Slow instead!\n"
        b"<k>\n"
        b"<p>\n"
        b"If you know an enemy can\n"
        b"inflict it, try using the Allergic\n"
        b"status to prevent it!\n"
        b"<k>\n"
        b"<p>\n"
        b"Naturally, the Feeling Fine\n"
        b"badge will work even better,\n"
        b"if you have it.\n"
        b"<k>",
        
    # NPC B - Toad on westside

    b"tot_di002000_00":
        b"I've been itching to add to\n"
        b"my collection of rare items\n"
        b"and badges lately...\n"
        b"<k>\n"
        b"<p>\n"
        b"But the shop over there's\n"
        b"been closed for ages!\n"
        b"<wait 250>Isn't that just my luck?\n"
        b"<k>\n"
        b"<p>\n"
        b"Oh well, I'm always down to\n"
        b"chat about my collection\n"
        b"anyway. Come by any time!\n"
        b"<k>",

    b"tot_di002100_00":
        b"One of my favorite items\n"
        b"I picked up recently is this\n"
        b"Mystic Egg, see?\n"
        b"<k>\n"
        b"<p>\n"
        b"It refills a third of your\n"
        b"max Star Power when used\n"
        b"in battle! <wait 250>Neat, huh?\n"
        b"<k>\n"
        b"<p>\n"
        b"I'm sure this'd make a great\n"
        b"ingredient for cooking, too, but\n"
        b"personally I like it as is!\n"
        b"<k>",

    b"tot_di002101_00":
        b"I recently got hold of a rare\n"
        b"item called a Fire Pop!\n"
        b"Ever seen one like it?\n"
        b"<k>\n"
        b"<p>\n"
        b"I've had to resist the urge\n"
        b"to nibble on it, as I've heard\n"
        b"it'll give you a nasty burn!\n"
        b"<k>\n"
        b"<p>\n"
        b"I imagine it'd still be just as\n"
        b"filling as other treats if you're\n"
        b"desperate, though.\n"
        b"<k>\n"
        b"<p>\n"
        b"I wonder if there are any\n"
        b"other dangerous desserts like\n"
        b"this out there...\n"
        b"<k>",

    b"tot_di002102_00":
        b"The fancy desserts they've\n"
        b"started carrying are really\n"
        b"something else!\n"
        b"<k>\n"
        b"<p>\n"
        b"Sweet Pudding has an\n"
        b"indescribable flavor. You'll feel\n"
        b"fantastic after eating it!\n"
        b"<k>\n"
        b"<p>\n"
        b"Sour Tart is much more of an\n"
        b"acquired taste, though. Could\n"
        b"really throw you for a loop.\n"
        b"<k>\n"
        b"<p>\n"
        b"Both of them have a really\n"
        b"unique flavor, though. It's like a\n"
        b"new experience every time!\n"
        b"<k>",

    b"tot_di002103_00":
        b"I just got this super-rare\n"
        b'item called a "Meteor Meal".\n'
        b"<wait 200>Check it out!\n"
        b"<k>\n"
        b"<p>\n"
        b"It restores 5 HP and FP for\n"
        b"a few turns, which's already\n"
        b"really nice...\n"
        b"<k>\n"
        b"<p>\n"
        b"But it also restores a third\n"
        b"of your SP on each of those\n"
        b"turns, as well!\n"
        b"<k>\n"
        b"<p>\n"
        b"Pardon the pun, <wait 150>but that's\n"
        b"out of this world! <wait 250>I wonder if\n"
        b"I could make one myself...\n"
        b"<k>",

    b"tot_di002104_00":
        b"Check out this thing I got\n"
        b"from a friend in Glitzville! <wait 150>They\n"
        b'call it the "Hottest Dog"!\n'
        b"<k>\n"
        b"<p>\n"
        b"It refills a bit of HP and FP,\n"
        b"but it also charges up your\n"
        b"next attack's power by 3!\n"
        b"<k>\n"
        b"<p>\n"
        b"Man, what won't they think\n"
        b"of next?\n"
        b"<k>",

    b"tot_di002105_00":
        b"I saved up a bunch to get\n"
        b"these awesome badges called\n"
        b"Jumpman and Hammerman.\n"
        b"<k>\n"
        b"<p>\n"
        b"Jumpman powers up your\n"
        b"jump attacks, but you won't\n"
        b"be able to use hammers.\n"
        b"<k>\n"
        b"<p>\n"
        b"Gotta commit to the\n"
        b'"Jumpman" brand, you know?\n'
        b"<k>\n"
        b"<p>\n"
        b"Hammerman's the opposite,\n"
        b"but it also makes your hammer\n"
        b"lighter, or something...\n"
        b"<k>\n"
        b"<p>\n"
        b"You'll be able to chuck your\n"
        b"Hammers at any enemy,\n"
        b"regardless of their position!\n"
        b"<k>\n"
        b"<p>\n"
        b"Come to think of it, I can't\n"
        b"jump very high, and I don't\n"
        b"own a Hammer anyway.\n"
        b"<k>",

    b"tot_di002106_00":
        b"Man, those Danger badges\n"
        b"sound really top-notch, but\n"
        b"they're too pricy for me.\n"
        b"<k>\n"
        b"<p>\n"
        b"Getting an extra 1 ATK in\n"
        b"Danger or 3 ATK in Peril sounds\n"
        b"nice, but also pretty scary.\n"
        b"<k>\n"
        b"<p>\n"
        b"I'll stick with my shiny new\n"
        b'"Perfect Power" badge instead.\n'
        b"It boosts ATK at full HP!\n"
        b"<k>\n"
        b"<p>\n"
        b"It definitely seems to\n"
        b"promote a healthier lifestyle,\n"
        b"I guess...\n"
        b"<k>\n"
        b"<p>\n"
        b"I just gotta make sure that\n"
        b"I don't obsess TOO much about\n"
        b"being healthy, y'know.\n"
        b"<k>",

    b"tot_di002107_00":
        b"I picked up this fancy badge\n"
        b'called "Quick Change" a while\n'
        b"back. It seems pretty nifty...\n"
        b"<k>\n"
        b"<p>\n"
        b"You get to shuffle your party\n"
        b"around all you want, and they\n"
        b"even keep their status.\n"
        b"<k>\n"
        b"<p>\n"
        b"You'll need to spend a few\n"
        b"Flower Points to swap, though,\n"
        b"so it still takes strategy.\n"
        b"<k>\n"
        b"<p>\n"
        b"Definitely not something\n"
        b"to go and use willy-nilly,\n"
        b"anyway.\n"
        b"<k>",

    b"tot_di002108_00":
        b"In case any trouble comes\n"
        b"back to town, I've got a bunch\n"
        b"of badges to hunker down!\n"
        b"<k>\n"
        b"<p>\n"
        b"Defend Plus'll give you 1 point\n"
        b"of good ol' DEF. Doesn't work on\n"
        b"piercing attacks, though.\n"
        b"<k>\n"
        b"<p>\n"
        b"Damage Dodge makes your\n"
        b"guards cut damage by 1 more,\n"
        b"even on piercing moves!\n"
        b"<k>\n"
        b"<p>\n"
        b"P-Down, D-Up is similar, and\n"
        b"works even if you don't guard,\n"
        b"but it also saps your power.\n"
        b"<k>\n"
        b"<p>\n"
        b"Toughen Up gives +1 DEF to\n"
        b"your 'Defend' action. It's niche,\n"
        b"but really cheap to equip!\n"
        b"<k>\n"
        b"<p>\n"
        b"So many different ways to\n"
        b"reduce damage, right? Maybe\n"
        b"I should try to pick a fight...\n"
        b"<k>",

    b"tot_di002109_00":
        b"Check out these nifty Star\n"
        b"Power-increasing badges I've\n"
        b"just added to my collection!\n"
        b"<k>\n"
        b"<p>\n"
        b"Super Appeal makes your\n"
        b"appeals to the crowd net you\n"
        b"an extra half-unit of SP.\n"
        b"<k>\n"
        b"<p>\n"
        b"Pity Star increases the SP\n"
        b"the crowd gives you after\n"
        b"taking a hit from an enemy.\n"
        b"<k>\n"
        b"<p>\n"
        b"Strong Start's my favorite.\n"
        b"It straight-up gives you some\n"
        b"SP at the start of a fight!\n"
        b"<k>",
        
    # NPC D (Poster Koopa).
    
    b"tot_di009000_00":
        b"Oh! <wait 200>A visitor? <wait 200>Well, make\n"
        b"yourself comfortable. It's not\n"
        b"much, but it's my place.\n"
        b"<k>\n"
        b"<p>\n"
        b"Going off to fight a dragon,\n"
        b"you say? <wait 250>That sounds intense.\n"
        b"<wait 250>Well, <wait 150>best of luck to you!\n"
        b"<k>\n"
        b"<p>\n"
        b"Honestly, <wait 250>sometimes I think\n"
        b"I need to try to go achieve\n"
        b"something like that, <wait 150>y'know?\n"
        b"<k>\n"
        b"<p>\n"
        b"Just sitting here on the\n"
        b"sidelines isn't all that fulfilling.\n"
        b"<wait 250>Nothing like you, obviously.\n"
        b"<k>\n"
        b"<p>\n"
        b"I tell you what, though. <wait 200>If you\n"
        b"find yourself struggling to\n"
        b"achieve something difficult...\n"
        b"<k>\n"
        b"<p>\n"
        b"Come by and have a chat!\n"
        b"Maybe we can come up with\n"
        b"a strategy together!\n"
        b"<k>",

    b"tot_di009118_00":
        b"Oh, it's you again. <wait 250>You say\n"
        b"you want to make deals with\n"
        b"7 NPCs in one tower run?\n"
        b"<k>\n"
        b"<p>\n"
        b"That sounds pretty doable...\n"
        b"<wait 250>They do appear on pretty much\n"
        b"every shop floor, after all.\n"
        b"<k>\n"
        b"<p>\n"
        b"You could make things just\n"
        b"a bit easier by selecting who\n"
        b"you want to see, I guess.\n"
        b"<k>\n"
        b"<p>\n"
        b"Like Wonky and Dazzle, for\n"
        b"instance, since they'll help you\n"
        b"out for pretty much free.\n"
        b"<k>\n"
        b"<p>\n"
        b"Up to you if you'd rather just\n"
        b"deal with whoever decides to\n"
        b"show up, though.\n"
        b"<k>",

    b"tot_di009122_00":
        b"Oh, it's you again. <wait 250>You say\n"
        b"you don't want to use Jumps\n"
        b"or Hammers in battle?\n"
        b"<k>\n"
        b"<p>\n"
        b"Well, your partners should\n"
        b"still give you plenty of variety\n"
        b"of moves, at least.\n"
        b"<k>\n"
        b"<p>\n"
        b"Plus, you'll have your turns\n"
        b"free to Appeal and use all the\n"
        b"Special Moves you want.\n"
        b"<k>\n"
        b"<p>\n"
        b"You can also first strike with\n"
        b"Jump and Hammer and still\n"
        b"fulfill the letter of the law...\n"
        b"<k>\n"
        b"<p>\n"
        b"Even just a First Strike from\n"
        b"Spin Jump or Ultra Hammer\n"
        b"still makes a big difference!\n"
        b"<k>",

    b"tot_di009125_00":
        b"Oh, it's you again. <wait 250>You say\n"
        b"you've got to finish a run with\n"
        b"all your moves at max level?\n"
        b"<k>\n"
        b"<p>\n"
        b"That sounds fun! <wait 250>Having\n"
        b"access to a bunch of powerful\n"
        b"moves is always nice.\n"
        b"<k>\n"
        b"<p>\n"
        b"I imagine the easiest way to\n"
        b"accomplish that is to do a run\n"
        b"with 3 chests per floor...\n"
        b"<k>\n"
        b"<p>\n"
        b"That'll give you more chances\n"
        b"to take a Star Piece or Shine\n"
        b"Sprite reward.\n"
        b"<k>\n"
        b"<p>\n"
        b"Besides that, <wait 150>it'll guarantee\n"
        b"you won't be forced to take a\n"
        b"move if you don't want it.\n"
        b"<k>\n"
        b"<p>\n"
        b"Should be a piece of cake!\n"
        b"<wait 200>Just resist the urge to take\n"
        b"too many new moves.\n"
        b"<k>",

    b"tot_di009126_00":
        b"Oh, it's you again. <wait 250>You say\n"
        b"you want to win a run without\n"
        b"failing any bonus conditions?\n"
        b"<k>\n"
        b"<p>\n"
        b"Sounds pretty touch-and-go...\n"
        b"<wait 250>Those conditions can be pretty\n"
        b"stringent sometimes.\n"
        b"<k>\n"
        b"<p>\n"
        b"Like turn count ones, for\n"
        b"example. <wait 150>Burn and Poison tick\n"
        b"at the <dynamic 3>start</dynamic> of a turn!\n"
        b"<k>\n"
        b"<p>\n"
        b"As a rule of thumb, any\n"
        b'"action"-based conditions only\n'
        b"consider selections in battle.\n"
        b"<k>\n"
        b"<p>\n"
        b"So, like, a Hammer first strike\n"
        b"won't count against you only\n"
        b"using Jump and Defend.\n"
        b"<k>\n"
        b"<p>\n"
        b"Likewise, Superguards and\n"
        b"other counterattack statuses\n"
        b"don't count as " b'"attacks".\n'
        b"<k>\n"
        b"<p>\n"
        b"Any move you choose that\n"
        b"can inflict damage or negative\n"
        b"status counts, though.\n"
        b"<k>\n"
        b"<p>\n"
        b"Most of the rest should be\n"
        b"pretty self-explanatory, even\n"
        b"if they're a bit tricky.\n"
        b"<k>\n"
        b"<p>\n"
        b"Oh! <wait 250>It's worth noting that\n"
        b"you can't fail a condition by\n"
        b"running away.\n"
        b"<k>\n"
        b"<p>\n"
        b"So if you know you can pass\n"
        b"a condition with a fresh start,\n"
        b"just run and try again!\n"
        b"<k>\n"
        b"<p>\n"
        b"Of course, sometimes there\n"
        b"might be a condition you can't\n"
        b"finish no matter what...\n"
        b"<k>\n"
        b"<p>\n"
        b"In that case, you'll just have\n"
        b"to give up and start another\n"
        b"run. <wait 250>That's the way it goes!\n"
        b"<k>",

    b"tot_di009127_00":
        b"Oh, it's you again. <wait 250>You say\n"
        b"you want to beat every floor\n"
        b"in 3 turns or fewer?\n"
        b"<k>\n"
        b"<p>\n"
        b"That's not a lot of time to\n"
        b"set up, especially since scouting\n"
        b"and running takes a turn...\n"
        b"<k>\n"
        b"<p>\n"
        b"If you're trying it on Hooktail's\n"
        b"tower, I imagine most floors\n"
        b"won't give too much trouble.\n"
        b"<k>\n"
        b"<p>\n"
        b"It's mostly the bosses that'll\n"
        b"be tough to take down that\n"
        b"quickly, then?\n"
        b"<k>\n"
        b"<p>\n"
        b"Obviously you'll want a lot\n"
        b"of ATK buffs or high-powered,\n"
        b"multi-hitting moves.\n"
        b"<k>\n"
        b"<p>\n"
        b"Using Power Jump to remove\n"
        b"an enemy's high DEF could\n"
        b"synergize well, I imagine...\n"
        b"<k>\n"
        b"<p>\n"
        b"And Freeze status doubling\n"
        b"the damage of the next hit\n"
        b"could be super strong as well.\n"
        b"<k>\n"
        b"<p>\n"
        b"Hmm... <wait 250>if you're really lucky,\n"
        b"maybe Charlieton could sell\n"
        b"you some Poison Shrooms.\n"
        b"<k>\n"
        b"<p>\n"
        b"They're kind of unreliable,\n"
        b"but cutting half an enemy's\n"
        b"HP is hard to beat if it works.\n"
        b"<k>\n"
        b"<p>\n"
        b"That's about all I've got...\n"
        b"<wait 250>Let me know how it goes!\n"
        b"<k>",

    b"tot_di009128_00":
        b"Oh, it's you again. <wait 250>You say\n"
        b"you want to do a whole tower\n"
        b"run without taking damage?\n"
        b"<k>\n"
        b"<p>\n"
        b"Wow, that sounds rough...\n"
        b"There're so many ways to\n"
        b"take just a little hit.\n"
        b"<k>\n"
        b"<p>\n"
        b"Well, if you can get enough\n"
        b"ATK power, you could try\n"
        b"one-shotting floors.\n"
        b"<k>\n"
        b"<p>\n"
        b"You can also try beating foes\n"
        b"one by one, leaving ones with\n"
        b"lower ATK or harmless items.\n"
        b"<k>\n"
        b"<p>\n"
        b"Point Swap doesn't count as\n"
        b"damage, so you can safely set\n"
        b"up Peril status for extra ATK!\n"
        b"<k>\n"
        b"<p>\n"
        b"You'll have to avoid causing\n"
        b"stage hazards if you're relying\n"
        b"on P-Up, D-Down, though!\n"
        b"<k>\n"
        b"<p>\n"
        b"I reckon it's probably more\n"
        b"worth getting Damage Dodge\n"
        b"and P-Down, D-Up, if you can.\n"
        b"<k>\n"
        b"<p>\n"
        b"Those'll reduce damage even\n"
        b"from DEF-piercing attacks\n"
        b"like Fuzzies' and Dayzees'.\n"
        b"<k>\n"
        b"<p>\n"
        b"As for partners, Shell Shield\n"
        b"or Withdraw are useful for foes\n"
        b"that only hit the front...\n"
        b"<k>\n"
        b"<p>\n"
        b"Rally Wink or Veil Lv. 2 could\n"
        b"be nice, too, if you need to act\n"
        b"twice in a row, Mario.\n"
        b"<k>\n"
        b"<p>\n"
        b"Of course, there's always\n"
        b"Boo's Sheets for complete\n"
        b"immunity, if you can get them.\n"
        b"<k>\n"
        b"<p>\n"
        b"<dynamic 3>Whew!</dynamic> <wait 250>That's a lot of stuff\n"
        b"to keep in mind.<wait 300> Good luck,\n"
        b"<wait 250>hopefully it goes well!\n"
        b"<k>",

    b"tot_di009129_00":
        b"Oh, it's you again. <wait 250>You say\n"
        b"you're trying to win a run\n"
        b"at 200% intensity or higher?\n"
        b"<k>\n"
        b"<p>\n"
        b"That sounds pretty tough.\n"
        b"At least you have a lot of\n"
        b"options to choose from...\n"
        b"<k>\n"
        b"<p>\n"
        b"Buffing enemies' HP or ATK\n"
        b"to double will get you almost\n"
        b"halfway there by itself!\n"
        b"<k>\n"
        b"<p>\n"
        b"If you can do without a stat,\n"
        b"<wait 200>then dropping it to 0 will also\n"
        b"add a decent chunk.\n"
        b"<k>\n"
        b"<p>\n"
        b"Plus, you have Charlieton's\n"
        b"limited stock, <wait 200>1-chest rewards,\n"
        b"<wait 200>conditional item drops...\n"
        b"<k>\n"
        b"<p>\n"
        b"Yeah, there should be a lot\n"
        b"of different ways you can go\n"
        b"about it, <wait 100>for sure.\n"
        b"<k>\n"
        b"<p>\n"
        b"I wonder how high an intensity\n"
        b"level you could beat a run with?\n"
        b"<wait 350>250%? <wait 350>300%? <wait 350>Hmm...\n"
        b"<k>",
 
    b"tot_di009130_00":
        b"Oh, it's you again. <wait 250>You say\n"
        b"you're trying to win with your\n"
        b"HP, FP or BP at minimum?\n"
        b"<k>\n"
        b"<p>\n"
        b"Interesting... I guess so long\n"
        b"as you have HP, you'll be able\n"
        b"to take a few hits.\n"
        b"<k>\n"
        b"<p>\n"
        b"FP gives you access to your\n"
        b"full suite of moves, naturally,\n"
        b"so it's a good one to have.\n"
        b"<k>\n"
        b"<p>\n"
        b"And BP is obviously great at\n"
        b"buffing you passively with\n"
        b"stat increases and so forth.\n"
        b"<k>\n"
        b"<p>\n"
        b"I guess with two of the three\n"
        b"to cover you, doing without one\n"
        b"shouldn't be too bad, right?\n"
        b"<k>",
    
    b"tot_di009131_00":
        b"Oh, it's you again. <wait 250>You say\n"
        b"you're trying to finish a run\n"
        b"with two stats at minimum?\n"
        b"<k>\n"
        b"<p>\n"
        b"Wow, that's tough. <wait 250>So you\n"
        b"basically have to choose just\n"
        b"one of HP, FP or BP to rely on.\n"
        b"<k>\n"
        b"<p>\n"
        b"HP seems like a decent pick,\n"
        b"as you'll probably be able to\n"
        b"take a few hits.\n"
        b"<k>\n"
        b"<p>\n"
        b"But FP could give you access\n"
        b"to a bunch of powerful moves,\n"
        b"and buffs like Shell Shield.\n"
        b"<k>\n"
        b"<p>\n"
        b"And of course, BP allows for\n"
        b"all sorts of flexibility. <wait 200>Plus, you'll\n"
        b"always have Perfect Power!\n"
        b"<k>\n"
        b"<p>\n"
        b"I guess any of those could\n"
        b"work if you commit to them.\n"
        b"<k>\n"
        b"<p>\n"
        b"Plus, you can always rely\n"
        b"on your partners' HP to\n"
        b"protect you from the front!\n"
        b"<k>",

    b"tot_di009140_00":
        b"Oh, it's you again. <wait 250>You say\n"
        b"you want to use a Trade Off\n"
        b"turn 1 against the final boss?\n"
        b"<k>\n"
        b"<p>\n"
        b"That sounds pretty risky!\n"
        b"<wait 250>An extra 3 ATK for the whole\n"
        b"fight is no joke...\n"
        b"<k>\n"
        b"<p>\n"
        b"You'd better be prepared\n"
        b"to take them down quickly, or\n"
        b"have a strong defense ready...\n"
        b"<k>\n"
        b"<p>\n"
        b"Oh, <wait 200>or you could maybe try\n"
        b"Neutralizing the boss with\n"
        b"Allergic status first!\n"
        b"<k>\n"
        b"<p>\n"
        b"It's not guaranteed to work,\n"
        b"but it could save you a lot of\n"
        b"trouble if it does!\n"
        b"<k>",

    b"tot_di009141_00":
        b"Oh, it's you again. <wait 250>You say\n"
        b"you want to Superguard\n"
        b"one of the dragons' bites?\n"
        b"<k>\n"
        b"<p>\n"
        b"How incredibly specific!\n"
        b"<wait 250>And tricky, presumably. <wait 150>They\n"
        b"do attack pretty quickly!\n"
        b"<k>\n"
        b"<p>\n"
        b"If you can't react to it,\n"
        b"it might be worth studying\n"
        b"their attack patterns.\n"
        b"<k>\n"
        b"<p>\n"
        b"Maybe one of them always\n"
        b"uses a bite attack under\n"
        b"predictable conditions!\n"
        b"<k>",
    
    b"tot_di009149_00":
        b"Oh, it's you again. <wait 250>You say\n"
        b"you want to collect 10 Shine\n"
        b"Sprites in a single run?\n"
        b"<k>\n"
        b"<p>\n"
        b"That's certainly a lot of\n"
        b"Shine Sprites. <wait 250>Probably more\n"
        b"than you'll see in chests...\n"
        b"<k>\n"
        b"<p>\n"
        b"You'll have to take every\n"
        b"one you find. <wait 250>Might need Chet\n"
        b"Rippo to help get the rest...\n"
        b"<k>\n"
        b"<p>\n"
        b"You'll want to take extra\n"
        b"stat increase rewards, if that's\n"
        b"the case. <wait 250>Yup!\n"
        b"<k>",
    
    b"tot_di009150_00":
        b"Oh, it's you again. <wait 250>You say\n"
        b"you want to reach 999 coins\n"
        b"on hand in one tower run?\n"
        b"<k>\n"
        b"<p>\n"
        b"Wow, that's a lot of cash.\n"
        b"<wait 250>I don't know if you'd get there\n"
        b"even without spending any...\n"
        b"<k>\n"
        b"<p>\n"
        b"You could try using Trade Off\n"
        b"to make enemies drop more\n"
        b"coins, of course...\n"
        b"<k>\n"
        b"<p>\n"
        b"Or get help from other folks\n"
        b"like Wonky or Grubba. <wait 250>Or Lumpy,\n"
        b"especially, <wait 200>if you're lucky.\n"
        b"<k>\n"
        b"<p>\n"
        b"Oh, <wait 200>if you come across the\n"
        b'"No Pain, No Gain" ' b"badge, that's\n"
        b"probably your best bet.\n"
        b"<k>\n"
        b"<p>\n"
        b"It'll double enemies' coins,\n"
        b"but also double your damage,\n"
        b"so you'll have to be careful!\n"
        b"<k>\n"
        b"<p>\n"
        b"That's about all I've got. <wait 300>Say,\n"
        b"<wait 200>this could also help if you want\n"
        b"to buy a lot of items at once...\n"
        b"<k>",

    b"tot_di009152_00":
        b"Oh, it's you again. <wait 250>You say\n"
        b"you want to get access to\n"
        b"every tower run option?\n"
        b"<k>\n"
        b"<p>\n"
        b"Well, most of them are locked\n"
        b"behind achievements. <wait 250>But you\n"
        b"probably knew that already...\n"
        b"<k>\n"
        b"<p>\n"
        b"Hmm... <wait 250>maybe try talking to\n"
        b"Bub-ulber down in the flower\n"
        b"patch just south of here?\n"
        b"<k>\n"
        b"<p>\n"
        b"I know he's knowledgeable\n"
        b"about seeds, <wait 200>so maybe he can\n"
        b"also help you out with this.\n"
        b"<k>",

    b"tot_di009156_00":
        b"Oh, it's you again. <wait 250>You say\n"
        b"you're trying to find all the\n"
        b"common and rare items?\n"
        b"<k>\n"
        b"<p>\n"
        b"I see. <wait 250>Yeah, rare items\n"
        b"definitely are hard to come by,\n"
        b"as the name suggests.\n"
        b"<k>\n"
        b"<p>\n"
        b"They're less likely to show up\n"
        b"on enemies, and Charlieton\n"
        b"doesn't stock as many...\n"
        b"<k>\n"
        b"<p>\n"
        b"If you run into Zess T., though,\n"
        b"I'm pretty sure she can cook\n"
        b"you anything you're missing.\n"
        b"<k>\n"
        b"<p>\n"
        b"In fact, the Items journal log\n"
        b"lists what ingredients any rare\n"
        b"item can be made from.\n"
        b"<k>\n"
        b"<p>\n"
        b"So if there's an common item\n"
        b"that isn't listed in any of the\n"
        b"items you've collected's list...\n"
        b"<k>\n"
        b"<p>\n"
        b"Well, that's<wait 150> <dynamic 3>a</dynamic> <wait 200>way to find\n"
        b"them, at least. <wait 250>Of course, you'll\n"
        b"run into them eventually.\n"
        b"<k>\n"
        b"<p>\n"
        b"Come to think of it, Zess T.'s\n"
        b"specialty items are probably\n"
        b"worth collecting as well...\n"
        b"<k>",

    b"tot_di009300_00":
        b"Oh, it's you again. <wait 250>Welcome!\n"
        b"<k>\n"
        b"<p>\n"
        b"It seems you probably don't\n"
        b"need my help at the moment.\n"
        b"<wait 250>That's good, I guess...\n"
        b"<k>",

    b"tot_di009301_00":
        b"Oh, it's you again. <wait 250>Welcome!\n"
        b"<k>\n"
        b"<p>\n"
        b"It seems to me that you've\n"
        b"accomplished just about\n"
        b"anything you could wish for...\n"
        b"<k>\n"
        b"<p>\n"
        b"You won't be needing my help\n"
        b"anymore, then. <wait 250>Maybe this is\n"
        b"the time I need to find myself!\n"
        b"<k>\n"
        b"<p>\n"
        b"Right now, just staying in\n"
        b"here seems pretty cozy, though.\n"
        b"Perhaps another time...\n"
        b"<k>",  

}

g_PetalburgEastStrings = {
    # Regular NPCs.
    
    # NPC G (Blue Bub-ulb).

    b"tot_di010100_00":
        b"This town has been a place\n"
        b"of peaceful happiness, and I\n"
        b"hope it remains that way...\n"
        b"<k>\n"
        b"<p>\n"
        b"I understand that things\n"
        b"seem bad right now, but I see\n"
        b"that you're free of worry.\n"
        b"<k>\n"
        b"<p>\n"
        b"Myself, I also think that\n"
        b"perhaps that tower isn't as\n"
        b"bad as it seems to be...\n"
        b"<k>\n"
        b"<p>\n"
        b"Perhaps you'll find treasures\n"
        b"worth your while in there, and\n"
        b"our town can stay at peace.\n"
        b"<k>",

    b"tot_di010101_00":
        b"Ah, you've returned. <wait 250>And it\n"
        b"seems I was correct about\n"
        b"the tower's treasures, no?\n"
        b"<k>\n"
        b"<p>\n"
        b"I'm sure most of us here will\n"
        b"be happy simply with you\n"
        b"around to keep the peace.\n"
        b"<k>\n"
        b"<p>\n"
        b"But it seems your heart is\n"
        b"set on finding worth in\n"
        b"something within the tower.\n"
        b"<k>\n"
        b"<p>\n"
        b"Be that the case, it'd be a\n"
        b"disservice if I didn't help you\n"
        b"find your inner happiness...\n"
        b"<k>\n"
        b"<p>\n"
        b"Do come by again, <wait 250>and I'll\n"
        b"do my part to help you find\n"
        b"what you treasure most.\n"
        b"<k>",

    b"tot_di010200_00":
        b"If anything brings happiness,\n"
        b"<wait 200>surely it's the companionship\n"
        b"of a good friend, <wait 150>no?\n"
        b"<k>\n"
        b"<p>\n"
        b"Your friend, Goombella,\n"
        b"for one; <wait 250>she's quite capable\n"
        b"at taking on single foes.\n"
        b"<k>\n"
        b"<p>\n"
        b"Her Ironbonk can pierce\n"
        b"through any hazards, and her\n"
        b"Multibonk is devastating!\n"
        b"<k>\n"
        b"<p>\n"
        b"Besides that, she's just as\n"
        b"good at giving you support\n"
        b"back with her Rally Wink!\n"
        b"<k>\n"
        b"<o>",

    b"tot_di010201_00":
        b"If anything brings happiness,\n"
        b"<wait 200>surely it's the companionship\n"
        b"of a good friend, <wait 150>no?\n"
        b"<k>\n"
        b"<p>\n"
        b"Your friend, Koops, for one;\n"
        b"<wait 250>he's quite quick, capable\n"
        b"and courageous.\n"
        b"<k>\n"
        b"<p>\n"
        b"He'll throw himself at all of\n"
        b"your foes without hesitation,\n"
        b"and at little cost.\n"
        b"<k>\n"
        b"<p>\n"
        b"And he can support both\n"
        b"you and himself defensively\n"
        b"in several ways...\n"
        b"<k>\n"
        b"<p>\n"
        b"Shell Shield for your safety,\n"
        b"<wait 200>Withdraw for his own, <wait 200>Bulk Up\n"
        b"for a boost to ATK and DEF...\n"
        b"<k>\n"
        b"<o>",

    b"tot_di010202_00":
        b"If anything brings happiness,\n"
        b"<wait 200>surely it's the companionship\n"
        b"of a good friend, <wait 150>no?\n"
        b"<k>\n"
        b"<p>\n"
        b"Your friend, Flurrie, for one;\n"
        b"<wait 250>she's a formidable force for\n"
        b"foes to contend with.\n"
        b"<k>\n"
        b"<p>\n"
        b"Her control of weather can\n"
        b"batter foes all at once in a\n"
        b"number of ways...\n"
        b"<k>\n"
        b"<p>\n"
        b"Freezing winds with Blizzard,\n"
        b"<wait 200>shocking Thunder Storms,\n"
        b"<wait 200>or Gale Force typhoons...\n"
        b"<k>\n"
        b"<o>",

    b"tot_di010203_00":
        b"If anything brings happiness,\n"
        b"<wait 200>surely it's the companionship\n"
        b"of a good friend, <wait 150>no?\n"
        b"<k>\n"
        b"<p>\n"
        b"Your Yoshi friend, for one;\n"
        b"<wait 250>he may be small, but he has\n"
        b"the guts of a champion.\n"
        b"<k>\n"
        b"<p>\n"
        b"His strikes may not seem\n"
        b"powerful, but he'll dish them\n"
        b"out in a relentless flurry.\n"
        b"<k>\n"
        b"<p>\n"
        b"Provide him with supporting\n"
        b"power, and those repeated\n"
        b"strikes will be unmatched!\n"
        b"<k>\n"
        b"<o>",

    b"tot_di010204_00":
        b"If anything brings happiness,\n"
        b"<wait 200>surely it's the companionship\n"
        b"of a good friend, <wait 150>no?\n"
        b"<k>\n"
        b"<p>\n"
        b"Your friend, Vivian, for one;\n"
        b"<wait 250>she's quite a charming\n"
        b"individual.\n"
        b"<k>\n"
        b"<p>\n"
        b"Her Curse, Neutralize, and\n"
        b"burn effects will take the\n"
        b"edge off of enemies' attacks...\n"
        b"<k>\n"
        b"<p>\n"
        b"And her Infatuate power\n"
        b"can even win them over to\n"
        b"your cause!\n"
        b"<k>\n"
        b"<o>",

    b"tot_di010205_00":
        b"If anything brings happiness,\n"
        b"<wait 200>surely it's the companionship\n"
        b"of a good friend, <wait 150>no?\n"
        b"<k>\n"
        b"<p>\n"
        b"Your friend, Bobbery, for one;\n"
        b"<wait 250>he's hale and hearty, and no\n"
        b"doubt has a storied past.\n"
        b"<k>\n"
        b"<p>\n"
        b"His time-activated devices\n"
        b"can be of great strategic\n"
        b"utility, <wait 150>if used well...\n"
        b"<k>\n"
        b"<p>\n"
        b"And the raw power of his\n"
        b"Bob-ombast and Megaton Bomb\n"
        b"are hard to match!\n"
        b"<k>\n"
        b"<o>",

    b"tot_di010206_00":
        b"If anything brings happiness,\n"
        b"<wait 200>surely it's the companionship\n"
        b"of a good friend, <wait 150>no?\n"
        b"<k>\n"
        b"<p>\n"
        b"Ms. Mowz, for one; <wait 250>she's not\n"
        b"easily won over, and can be\n"
        b"quite a thorn in foes' side.\n"
        b"<k>\n"
        b"<p>\n"
        b"She's adept at using enemies'\n"
        b"items to your advantage, <wait 150>with\n"
        b"Kiss Thief and Embargo...\n"
        b"<k>\n"
        b"<p>\n"
        b"And a well-honed Tease is\n"
        b"especially effective at\n"
        b"disrupting foes' plans.\n"
        b"<k>\n"
        b"<o>",

    b"tot_di010207_00":
        b"I have to imagine mastering\n"
        b"new skills is bound to bring\n"
        b"you great happiness,<wait 150> no?\n"
        b"<k>\n"
        b"<p>\n"
        b"Your jumping abilities seem\n"
        b"especially good for focusing\n"
        b"damage on a single foe.\n"
        b"<k>\n"
        b"<p>\n"
        b"A Power Jump can drop\n"
        b"an enemy's defenses with\n"
        b"a single strong strike...\n"
        b"<k>\n"
        b"<p>\n"
        b"Then Power Bounce, <wait 200>or\n"
        b"Spin or Spring Jump, <wait 150>can deal\n"
        b"with foes with ease.\n"
        b"<k>\n"
        b"<o>",

    b"tot_di010208_00":
        b"I have to imagine mastering\n"
        b"new skills is bound to bring\n"
        b"you great happiness,<wait 150> no?\n"
        b"<k>\n"
        b"<p>\n"
        b"Your Hammer abilities are\n"
        b"versatile, supporting you in\n"
        b"various ways.\n"
        b"<k>\n"
        b"<p>\n"
        b"Super and Ultra Hammer\n"
        b"deal heavy damage to a foe,\n"
        b"chipping away at the rest...\n"
        b"<k>\n"
        b"<p>\n"
        b"Ice Smash and Shrink Smash\n"
        b"can dash an enemy's defensive\n"
        b"or offensive potential...\n"
        b"<k>\n"
        b"<p>\n"
        b"And Power Smash can cut\n"
        b"through foes' defense with a\n"
        b"mighty, piercing strike!\n"
        b"<k>\n"
        b"<o>",

    b"tot_di010209_00":
        b"I have to imagine mastering\n"
        b"new skills is bound to bring\n"
        b"you great happiness,<wait 150> no?\n"
        b"<k>\n"
        b"<p>\n"
        b"Your Special Moves may\n"
        b"take effort to use, but are\n"
        b"life-saving at the right time.\n"
        b"<k>\n"
        b"<p>\n"
        b"Sweet Treat and Feast can\n"
        b"bring you back to health, or\n"
        b"keep you healthy for a while...\n"
        b"<k>\n"
        b"<p>\n"
        b"Clock Out and Showstopper\n"
        b"can take the edge off of a\n"
        b"tough enemy encounter...\n"
        b"<k>\n"
        b"<p>\n"
        b"And attacking powers can\n"
        b"clean up in no time, if you've\n"
        b"suffered heavy damage!\n"
        b"<k>\n"
        b"<o>",

    b"tot_di010210_00":
        b"What brings more happiness\n"
        b"than staying in good health?\n"
        b"<wait 250>Surely nothing, <wait 150>no?\n"
        b"<k>\n"
        b"<p>\n"
        b"An extra level of HP can\n"
        b"make all the difference in the\n"
        b"world against a tough foe.\n"
        b"<k>\n"
        b"<p>\n"
        b"Especially if that foe can\n"
        b"render you helpless with status\n"
        b"like Stop and Freeze!\n"
        b"<k>\n"
        b"<o>",

    b"tot_di010211_00":
        b"What brings more happiness\n"
        b"than keeping friends in good\n"
        b"spirits? <wait 250>Surely nothing, <wait 150>no?\n"
        b"<k>\n"
        b"<p>\n"
        b"Extra HP for your partners\n"
        b"can help bolster your team's\n"
        b"survivability with ease!\n"
        b"<k>\n"
        b"<p>\n"
        b"It also lends extra leeway\n"
        b"between a state of Peril, and\n"
        b"being unable to fight!\n"
        b"<k>\n"
        b"<o>",

    b"tot_di010212_00":
        b"What brings more happiness\n"
        b"than a chance to show your\n"
        b"skills? <wait 250>Surely nothing, <wait 150>no?\n"
        b"<k>\n"
        b"<p>\n"
        b"Extra levels of Flower Points\n"
        b"will give you more chances to\n"
        b"use your special skills...\n"
        b"<k>\n"
        b"<p>\n"
        b"Increase it enough, and\n"
        b"you'll be able to use cheaper\n"
        b"skills almost any time!\n"
        b"<k>\n"
        b"<p>\n"
        b"Alternatively, if the situation\n"
        b"is dire, you can unleash many\n"
        b"powerful moves in a row!\n"
        b"<k>\n"
        b"<o>",

    b"tot_di010213_00":
        b"Finding the right badge\n"
        b"can give a great sense of\n"
        b"satisfaction, <wait 150>no?\n"
        b"<k>\n"
        b"<p>\n"
        b"If that's the case, surely\n"
        b"increasing Badge Points will\n"
        b"increase it in equal measure.\n"
        b"<k>\n"
        b"<p>\n"
        b"Surely it must be frustrating\n"
        b"to have a collection of useful\n"
        b"badges that you can't equip!\n"
        b"<k>\n"
        b"<o>",

    b"tot_di010214_00":
        b"Having a useful array of\n"
        b"staple goods on hand is\n"
        b"always quite nice, <wait 150>no?\n"
        b"<k>\n"
        b"<p>\n"
        b"If you find extra pouches to\n"
        b"store items in, they might be\n"
        b"worth your while to pick up.\n"
        b"<k>\n"
        b"<p>\n"
        b"You can certainly make do\n"
        b"with only a few, <wait 150>but it never\n"
        b"hurts to be prepared.\n"
        b"<k>\n"
        b"<o>",

    b"tot_di010215_00":
        b"Coins might not be able to\n"
        b"buy happiness, <wait 200>but they still\n"
        b"provide some security, <wait 150>no?\n"
        b"<k>\n"
        b"<p>\n"
        b"It may feel like a waste to\n"
        b"go for them in favor of things\n"
        b"with more immediate benefit...\n"
        b"<k>\n"
        b"<p>\n"
        b"But don't underestimate\n"
        b"what you can get from having\n"
        b"extra buying power early on!\n"
        b"<k>\n"
        b"<p>\n"
        b"You might be able to afford\n"
        b"powerful badges, or make more\n"
        b"deals with friendly guests.\n"
        b"<k>\n"
        b"<o>",

    b"tot_di010216_00":
        b"Surely, honing your skills\n"
        b"to their greatest potential\n"
        b"must bring happiness, <wait 150>no?\n"
        b"<k>\n"
        b"<p>\n"
        b"Star Pieces may be flighty,\n"
        b"<wait 200>but extra skill levels are still\n"
        b"skill levels, all the same!\n"
        b"<k>\n"
        b"<p>\n"
        b"Plus, <wait 200>the chance of getting\n"
        b"multiple moves upgraded at\n"
        b"once is tempting, <wait 250>isn't it?\n"
        b"<k>\n"
        b"<o>",

    b"tot_di010217_00":
        b"Surely, honing your skills\n"
        b"to their greatest potential\n"
        b"must bring happiness, <wait 150>no?\n"
        b"<k>\n"
        b"<p>\n"
        b"Shine Sprites give you the\n"
        b"best shot at honing the exact\n"
        b"skill you want to improve.\n"
        b"<k>\n"
        b"<p>\n"
        b"Plus, <wait 200>they have the buying\n"
        b"power of multiple Star Pieces\n"
        b"for vendors here in town.\n"
        b"<k>\n"
        b"<o>",

    b"tot_di010218_00":
        b"Finding the right badge\n"
        b"can give a great sense of\n"
        b"satisfaction, <wait 150>no?\n"
        b"<k>\n"
        b"<p>\n"
        b"Some special badges out\n"
        b"there are especially worth\n"
        b"cherishing if you find them...\n"
        b"<k>\n"
        b"<p>\n"
        b"Unique badges like Double Dip\n"
        b"and Quick Change you won't\n"
        b"be able to find just anywhere.\n"
        b"<k>\n"
        b"<p>\n"
        b"If you see one of these,\n"
        b"<wait 200>consider that you might not\n"
        b"get another shot at them!\n"
        b"<k>\n"
        b"<o>",

    b"tot_di010219_00":
        b"Finding the right badge\n"
        b"can give a great sense of\n"
        b"satisfaction, <wait 150>no?\n"
        b"<k>\n"
        b"<p>\n"
        b"Not every badge has to be\n"
        b"super-unique, <wait 150>like Feeling Fine\n"
        b"or Spike Shield, though...\n"
        b"<k>\n"
        b"<p>\n"
        b"If you find a badge offered\n"
        b"for free that you really like,\n"
        b"<wait 150>or fits nicely in a strategy...\n"
        b"<k>\n"
        b"<p>\n"
        b"There's no shame in taking it,\n"
        b"<wait 150>even if you might be able to\n"
        b"find more of them later!\n"
        b"<k>\n"
        b"<o>",

    b"tot_di010300_00":
        b"<p>\n"
        b"Why, it seems you've given\n"
        b"them pretty much no thought\n"
        b"at all! <wait 300>How shocking...\n"
        b"<k>\n"
        b"<p>\n"
        b"Well, <wait 200>perhaps you know better,\n"
        b"<wait 150>but I believe it couldn't hurt to\n"
        b"explore new things.\n"
        b"<k>",

    b"tot_di010301_00":
        b"<p>\n"
        b"I guess you don't find them\n"
        b"as fulfilling as other things,\n"
        b"though. <wait 300>I see...\n"
        b"<k>\n"
        b"<p>\n"
        b"Well, <wait 200>perhaps you know better,\n"
        b"<wait 150>but I believe it couldn't hurt to\n"
        b"explore new things.\n"
        b"<k>",

    b"tot_di010302_00":
        b"<p>\n"
        b"It seems you've also given\n"
        b"them a fair deal of thought.\n"
        b"<wait 250>That's nice to hear!\n"
        b"<k>\n"
        b"<p>\n"
        b"All the same, <wait 200>I believe it's\n"
        b"always worth trying to explore\n"
        b"new things...\n"
        b"<k>",

    b"tot_di010303_00":
        b"<p>\n"
        b"It seems they're quite a\n"
        b"favorite of yours already.\n"
        b"<wait 250>That's nice to hear!\n"
        b"<k>\n"
        b"<p>\n"
        b"All the same, <wait 200>I believe it's\n"
        b"always worth trying to explore\n"
        b"new things...\n"
        b"<k>",

    # NPC H (Koopa).
    
    b"tot_di005000_00":
        b"Going to face that terrible\n"
        b"dragon again, Mario?\n"
        b"<wait 250>Good luck, you'll need it.\n"
        b"<k>\n"
        b"<p>\n"
        b"I've heard some nasty\n"
        b"enemies from all over are\n"
        b"swarming the place.\n"
        b"<k>\n"
        b"<p>\n"
        b"You go on ahead, but I'll\n"
        b"share whatever info I can\n"
        b"find on them later.\n"
        b"<k>",

    b"tot_di005100_00":
        b"Back in one piece, eh?\n"
        b"<wait 250>Don't suppose you ran into\n"
        b"any Piranha Plants, then.\n"
        b"<k>\n"
        b"<p>\n"
        b"I've heard they have nasty\n"
        b"breath attacks. Noxious gas,\n"
        b"icy wind, searing flames...\n"
        b"<k>\n"
        b"<p>\n"
        b"They're firmly rooted, too,\n"
        b"so they're not very prone\n"
        b"to fright or gale-force wind.\n"
        b"<k>\n"
        b"<p>\n"
        b"Supposedly they're all pretty\n"
        b"drowsy by nature, though,\n"
        b"so Sleep status couldn't hurt.\n"
        b"<k>",

    b"tot_di005101_00":
        b"Still alive to tell the tale?\n"
        b"<wait 250>Surely you steered clear of\n"
        b"hot-head Bob-ombs, then.\n"
        b"<k>\n"
        b"<p>\n"
        b"They're a lot less prone to\n"
        b"ineffectual blowing up than\n"
        b"before, you know.\n"
        b"<k>\n"
        b"<p>\n"
        b"If you hit 'em from a distance\n"
        b"when mad and don't outright\n"
        b"KO 'em, they counterattack!\n"
        b"<k>\n"
        b"<p>\n"
        b"Same goes for if you try to\n"
        b"light them on fire or such.\n"
        b"<dynamic 3><wait 200>BAM!</dynamic> <wait 150>Explosion in the kisser!\n"
        b"<k>\n"
        b"<p>\n"
        b"Only safe way to deal with\n"
        b"them is to KO 'em outright, or\n"
        b"stop them with status!\n"
        b"<k>\n"
        b"<p>\n"
        b"Otherwise, you'd best be\n"
        b"prepared to take the blow!\n"
        b"<k>",

    b"tot_di005102_00":
        b"Still kicking, eh?\n"
        b"<wait 250>Guess you avoided getting\n"
        b"barbed by an Iron Cleft.\n"
        b"<k>\n"
        b"<p>\n"
        b"These guys look like any\n"
        b"other Cleft, but they barely\n"
        b"flinch at any attack!\n"
        b"<k>\n"
        b"<p>\n"
        b"You can't flip 'em, they're\n"
        b"immune to elements, and they\n"
        b"only take chip damage!\n"
        b"<k>\n"
        b"<p>\n"
        b"You'll have to hit 'em with\n"
        b"a bunch of weak hits, like\n"
        b"Power Bounce or Stampede.\n"
        b"<k>\n"
        b"<p>\n"
        b"But make sure you've got\n"
        b"the power to get through\n"
        b"their natural DEF first!\n"
        b"<k>",

    b"tot_di005103_00":
        b"Still with us, eh?\n"
        b"<wait 250>Guess you didn't get speared\n"
        b"by a Dark Craw, then.\n"
        b"<k>\n"
        b"<p>\n"
        b"Craws have their standard\n"
        b"attacks, yes, but Dark Craws\n"
        b"have learned a new skill.\n"
        b"<k>\n"
        b"<p>\n"
        b"Sometimes, they can just up\n"
        b"and decide to ram through\n"
        b"your whole team! The nerve!\n"
        b"<k>\n"
        b"<p>\n"
        b"You can see it coming if\n"
        b"they do a little hop before\n"
        b"charging, though. Watch out!\n"
        b"<k>",

    b"tot_di005104_00":
        b"Alive and well, eh?\n"
        b"<wait 250>Guess you didn't get lured in\n"
        b"by an Amazy Dayzee.\n"
        b"<k>\n"
        b"<p>\n"
        b"You'll pretty rarely see them,\n"
        b"and even when you do, they'll\n"
        b"probably run off.\n"
        b"<k>\n"
        b"<p>\n"
        b"But if they don't, oh boy!\n"
        b"I wouldn't want to be there\n"
        b"to witness the carnage!\n"
        b"<k>\n"
        b"<p>\n"
        b"If you're not rolling high on\n"
        b"ATK, you might want to keep\n"
        b"a Fright Mask handy.\n"
        b"<k>",

    b"tot_di005105_00":
        b"Doing fine, I see?\n"
        b"<wait 250>Guess you didn't run afoul of\n"
        b"the Hammer Bros. and co.\n"
        b"<k>\n"
        b"<p>\n"
        b"Hammer, Fire, and Boomerang\n"
        b"Bros. all have high HP and DEF,\n"
        b"but fairly modest ATK.\n"
        b"<k>\n"
        b"<p>\n"
        b"Don't let your guard down!\n"
        b"If you get them to half HP,\n"
        b"they'll attack like maniacs!\n"
        b"<k>\n"
        b"<p>\n"
        b"If you don't want a zillion\n"
        b"attacks to the face, pay close\n"
        b"attention to their health!\n"
        b"<k>",

    b"tot_di005106_00":
        b"Right as rain, are we?\n"
        b"<wait 250>Seems you haven't been hexed\n"
        b"by a Magikoopa, then.\n"
        b"<k>\n"
        b"<p>\n"
        b"The usual, blue-robed types\n"
        b"have a variety of magical\n"
        b"spells I'm sure you're used to.\n"
        b"<k>\n"
        b"<p>\n"
        b"The rarer-colored ones are\n"
        b"specialists, though, and only\n"
        b"appear as supporting foes.\n"
        b"<k>\n"
        b"<p>\n"
        b"Red ones have higher ATK\n"
        b"and only use Huge and DEF-\n"
        b"buffing magic.\n"
        b"<k>\n"
        b"<p>\n"
        b"Green ones have DEF and\n"
        b"use support magic, and White\n"
        b"have more HP and can heal.\n"
        b"<k>\n"
        b"<p>\n"
        b"If they decide to clone, you\n"
        b"can't tell them apart, so have\n"
        b"a spread attack handy!\n"
        b"<k>",
    
    # NPC I (Toad).
    
    b"tot_di006100_00":
        b"I can't bear to think of\n"
        b"what might be going on at\n"
        b"the castle right now...\n"
        b"<k>\n"
        b"<p>\n"
        b"And yet, my morbid curiosity\n"
        b"is making me really want to\n"
        b"go and check it out myself.\n"
        b"<k>\n"
        b"<p>\n"
        b"The sensible part of me\n"
        b"would really prefer not to\n"
        b"be chomped to bits, though.\n"
        b"<k>\n"
        b"<p>\n"
        b"Stay here and wonder...\n"
        b"<wait 350>or go, <wait 200>and possibly get beaten\n"
        b"to a pulp, or worse? "
        b"<wait 350>Hmmm<wait 150>.<wait 150>.<wait 150>.<wait 150>.<wait 150>\n"
        b"<k>",

    b"tot_di006101_00":
        b"So all Hooktail wanted was\n"
        b"a fair challenge, then?\n"
        b"<wait 250>That's a relief, I guess.\n"
        b"<k>\n"
        b"<p>\n"
        b"But her older brother's coming\n"
        b"too, and he's way stronger?\n"
        b"<wait 250>Yikes, that sounds scary.\n"
        b"<k>\n"
        b"<p>\n"
        b"What if he's not nearly as\n"
        b"fair a fighter? <wait 250>I don't wanna\n"
        b"think of what might happen...\n"
        b"<k>\n"
        b"<p>\n"
        b"Be brave and go face my\n"
        b"fears... <wait 200>or stay here and keep\n"
        b"worrying myself sick? "
        b"<wait 250>Hmmm<wait 100>.<wait 100>.<wait 100>.<wait 100>.\n"
        b"<k>",


    b"tot_di006102_00":
        b"Oh hey, Mario. <wait 200>Sounds like you\n"
        b"and Gloomtail tussled and you\n"
        b"came out on top? <wait 250>Bravo!\n"
        b"<k>\n"
        b"<p>\n"
        b"Well, I finally overcame my\n"
        b"fear and checked out the\n"
        b"tower outskirts for myself...\n"
        b"<k>\n"
        b"<p>\n"
        b"Turns out there's a little\n"
        b"sign out front with what looks\n"
        b"like a bunch of regulations...\n"
        b"<k>\n"
        b"<p>\n"
        b"Are you going to keep on\n"
        b"challenging those dragons with\n"
        b"ridiculous restrictions?\n"
        b"<k>\n"
        b"<p>\n"
        b"Is that so? Whatever floats\n"
        b"your boat, I guess. <wait 250>Well, good\n"
        b"luck to you, Mario!\n"
        b"<k>",

    b"tot_di006200_00":
        b"If I recall, one of the rules I\n"
        b"saw on that board had to do\n"
        b"with the items enemies drop?\n"
        b"<k>\n"
        b"<p>\n"
        b"You can choose whether to\n"
        b"get items naturally, or from\n"
        b"fulfilling extra conditions?\n"
        b"<k>\n"
        b"<p>\n"
        b"Man, that's wild. <wait 250>Seems like\n"
        b"a super arbitrary setup, but if\n"
        b"it keeps them entertained...\n"
        b"<k>",

    b"tot_di006201_00":
        b"One of the options I saw was\n"
        b"what items you're allowed to\n"
        b"enter the tower with, yes?\n"
        b"<k>\n"
        b"<p>\n"
        b"So you can start with a few\n"
        b"simple items, a stronger set\n"
        b"with a Life Shroom...\n"
        b"<k>\n"
        b"<p>\n"
        b"a random pool of items, or\n"
        b"even a hand-picked selection\n"
        b"of items and badges?\n"
        b"<k>\n"
        b"<p>\n"
        b"That last one seems like it'd\n"
        b"have a ton of impact! I'd be\n"
        b"interested to see your picks.\n"
        b"<k>",

    b"tot_di006202_00":
        b"So it seems you're able to\n"
        b"choose how many partners\n"
        b"you get in the tower?\n"
        b"<k>\n"
        b"<p>\n"
        b"That'd have some interesting\n"
        b"implications, it seems. I wonder\n"
        b"what choice would be best...\n"
        b"<k>\n"
        b"<p>\n"
        b"The higher the max, the more\n"
        b"your move pool would end\n"
        b"up getting diluted...\n"
        b"<k>\n"
        b"<p>\n"
        b"But on the other hand, having\n"
        b"only a couple might leave you\n"
        b"with no counter to a foe.\n"
        b"<k>\n"
        b"<p>\n"
        b"Then again, you also might\n"
        b"have more chances to choose\n"
        b"to skip a particular partner.\n"
        b"<k>\n"
        b"<p>\n"
        b"Come to think of it, I guess\n"
        b"I'm burying the lede here...\n"
        b"<k>\n"
        b"<p>\n"
        b"Why are your partners okay\n"
        b"with being stuffed in boxes\n"
        b"in the tower, anyhow?\n"
        b"<k>",

    b"tot_di006203_00":
        b"Did I see this right? <wait 200>Can you\n"
        b"outright choose how much HP,\n"
        b"FP, and such you have?\n"
        b"<k>\n"
        b"<p>\n"
        b"That seems needlessly nice;\n"
        b"I can't imagine enemies dealing\n"
        b"with 10 points per upgrade.\n"
        b"<k>\n"
        b"<p>\n"
        b"On the other hand, why would\n"
        b"you ever set one to 0? <wait 200>That\n"
        b"seems incredibly reckless.\n"
        b"<k>\n"
        b"<p>\n"
        b"You wouldn't dare try that,\n"
        b"right, Mario? <wait 250>What would you\n"
        b"even have to start with?\n"
        b"<k>",

    b"tot_di006204_00":
        b"I took another quick look at\n"
        b"that sign, and saw there're rules\n"
        b"around Charlieton's shop.\n"
        b"<k>\n"
        b"<p>\n"
        b"Is he in cahoots with the\n"
        b"dragons now? <wait 250>Now I've really\n"
        b"heard everything.\n"
        b"<k>\n"
        b"<p>\n"
        b"I'm not sure how small a stock\n"
        b'"Tiny" ' b"is, but I'm sure it'd hamper\n"
        b"your options a lot.\n"
        b"<k>\n"
        b"<p>\n"
        b"Having the option's gotta put\n"
        b"into perspective just how much\n"
        b"he carries normally, huh?\n"
        b"<k>",

    b"tot_di006205_00":
        b"Don't know if you're aware,\n"
        b"but I saw a bunch of random\n"
        b"rules scribbled in the margins.\n"
        b"<k>\n"
        b"<p>\n"
        b"Don't know if any of them are\n"
        b"technically on the books yet,\n"
        b"but they boggle my mind!\n"
        b"<k>\n"
        b"<p>\n"
        b'"Random damage variance"?\n'
        b'<wait 250>"Random audience throws"?\n'
        b'<wait 250>"<dynamic 3>Obfuscated</dynamic> items"?\n'
        b"<k>\n"
        b"<p>\n"
        b"Man, I don't know what you\n"
        b"all get out of this, but if it gets\n"
        b"someone their kicks, I guess...\n"
        b"<k>",
    
    # NPC K (Toad in east house).

    b"tot_di007000_00":
        b"Oh, hi there, Mario!\n"
        b"<wait 250>Lovely day today, isn't it?\n"
        b"<k>\n"
        b"<p>\n"
        b"Ever since your exploits at\n"
        b"Hooktail Castle, I've been\n"
        b"keeping up with your moves!\n"
        b"<k>\n"
        b"<p>\n"
        b"You and your partners have\n"
        b"really mastered the art of\n"
        b"combat, and with such style!\n"
        b"<k>\n"
        b"<p>\n"
        b"Come back any time and we\n"
        b"can chat about whatever\n"
        b"fancy moves you want!\n"
        b"<k>",

    b"tot_di007100_00":
        b"Oh, hi there, Mario!\n"
        b"<wait 250>Lovely out today, isn't it?\n"
        b"<k>\n"
        b"<p>\n"
        b"I've noticed you've perfected\n"
        b"your Spring Jump move\n"
        b"recently!\n"
        b"<k>\n"
        b"<p>\n"
        b"What a breathtaking \n"
        b"three-hit combo, and with\n"
        b"added zip, to boot!\n"
        b"<k>\n"
        b"<p>\n"
        b"<dynamic 3>Boom</dynamic><wait 125>\n"
        b"-<dynamic 3>bang</dynamic><wait 125>\n"
        b"-<dynamic 3>POW!</dynamic> \n"
        b"<wait 250>That's really showing your\n"
        b"enemies what for!\n"
        b"<k>",

    b"tot_di007101_00":
        b"Oh, hi there, Mario!\n"
        b"<wait 250>Wonderful day, isn't it?\n"
        b"<k>\n"
        b"<p>\n"
        b"I've noticed you're putting\n"
        b"a bit more <dynamic 3>oomph</dynamic> into your\n"
        b"Ultra Hammer these days!\n"
        b"<k>\n"
        b"<p>\n"
        b"That extra <dynamic 3>WHACK</dynamic> when the\n"
        b"enemy lands just looks so\n"
        b"satisfying!\n"
        b"<k>",

    b"tot_di007102_00":
        b"Oh, hi there, Mario!\n"
        b"<wait 250>Beautiful out, isn't it?\n"
        b"<k>\n"
        b"<p>\n"
        b"Your supplemental jump and\n"
        b"hammer moves are putting in\n"
        b"a lot more work now!\n"
        b"<k>\n"
        b"<p>\n"
        b"Power Jump inflicting Soft\n"
        b"and Power Smash piercing DEF\n"
        b"really help pack a punch!\n"
        b"<k>\n"
        b"<p>\n"
        b"Shrink Smash and Ice Smash\n"
        b"are devastating to unlucky\n"
        b"foes' offense and defense!\n"
        b"<k>\n"
        b"<p>\n"
        b"Even old Tornado Jump's\n"
        b"shining with its ground-bound\n"
        b"tornadoes and Dizzy status!\n"
        b"<k>\n"
        b"<p>\n"
        b"You really know how to put\n"
        b"the hurt on enemies with style!\n"
        b"<k>",

    b"tot_di007103_00":
        b"Oh, hi there, Mario!\n"
        b"<wait 250>I've been scoping out your\n"
        b"partners' moves as well!\n"
        b"<k>\n"
        b"<p>\n"
        b"Speaking of, Goombella's\n"
        b"really ingenious for coming up\n"
        b"with that Scope Out move!\n"
        b"<k>\n"
        b"<p>\n"
        b"Being able to land a move\n"
        b"on a dodgy enemy's a real boon,\n"
        b"let alone guarantee status!\n"
        b"<k>\n"
        b"<p>\n"
        b"Her invulnerable Ironbonk is\n"
        b"also super! Can't complain about\n"
        b"a move that ignores hazards!\n"
        b"<k>",

    b"tot_di007104_00":
        b"Oh, hi there, Mario!\n"
        b"<wait 250>It seems dear Koops has proven\n"
        b"himself to be quite strong!\n"
        b"<k>\n"
        b"<p>\n"
        b"He makes his shell-tossing\n"
        b"moves look as quick and easy\n"
        b"as ever...\n"
        b"<k>\n"
        b"<p>\n"
        b"And he's proven to make\n"
        b"good use of that hard shell\n"
        b"with Withdraw!\n"
        b"<k>\n"
        b"<p>\n"
        b"And Bulk Up, boosting ATK\n"
        b"and DEF with his raw courage!\n"
        b"He should be quite proud.\n"
        b"<k>",

    b"tot_di007105_00":
        b"Oh, hi there! <wait 250>My, Flurrie's\n"
        b"really been channeling her\n"
        b"elemental prowess lately!\n"
        b"<k>\n"
        b"<p>\n"
        b"Ice and electric storms that\n"
        b"cover the whole field?\n"
        b"What awesome power!\n"
        b"<k>",

    b"tot_di007106_00":
        b"Oh, hi there, Mario!\n"
        b"<wait 250>That Yoshi's really mastered\n"
        b"the art of egg-tossing!\n"
        b"<k>\n"
        b"<p>\n"
        b"Specializing into the more\n"
        b"damaging Egg Barrage or more\n"
        b"debilitating Mini-Egg is genius!\n"
        b"<k>\n"
        b"<p>\n"
        b"I'll bet both moves could\n"
        b"help make a single tough foe\n"
        b"a lot less threatening!\n"
        b"<k>",

    b"tot_di007107_00":
        b"Oh, hi there, Mario!\n"
        b"<wait 250>That sweet Vivian is as\n"
        b"charming as ever!\n"
        b"<k>\n"
        b"<p>\n"
        b"Her fire powers dropping\n"
        b"enemies' ATK has got to be\n"
        b"vital for defense, I imagine.\n"
        b"<k>\n"
        b"<p>\n"
        b"Neutralize preventing status\n"
        b"on any fighter's got to have\n"
        b"interesting uses, as well!\n"
        b"<k>\n"
        b"<p>\n"
        b"But nothing compares to\n"
        b"just winning an enemy over\n"
        b"with Infatuate, no doubt!\n"
        b"<k>",

    b"tot_di007108_00":
        b"Oh, hi there, Mario!\n"
        b"<wait 250>Admiral Bobbery's as explosive\n"
        b"a personality as ever!\n"
        b"<k>\n"
        b"<p>\n"
        b"His Bob-ombast looks even\n"
        b"stronger than ever at its\n"
        b"maximum power!\n"
        b"<k>\n"
        b"<p>\n"
        b"To say nothing of the utter,\n"
        b"reckless destruction of a fully\n"
        b"charged Megaton Bomb!\n"
        b"<k>\n"
        b"<p>\n"
        b"I should hope he knows how\n"
        b"to utilize it without disregard\n"
        b"to both of your safety!\n"
        b"<k>",

    b"tot_di007109_00":
        b"Oh, hi there, Mario!\n"
        b"<wait 250>Looks like Ms. Mowz is as\n"
        b"crafty as ever!\n"
        b"<k>\n"
        b"<p>\n"
        b"She's really perfected that\n"
        b"Tease move, confusing enemies\n"
        b"instead of just dizzying them.\n"
        b"<k>\n"
        b"<p>\n"
        b"And making enemies likelier\n"
        b"to act in confusion at higher\n"
        b"levels? How fiendish!\n"
        b"<k>\n"
        b"<p>\n"
        b"You could probably let a\n"
        b"group of foes take each other\n"
        b"down with that move alone!\n"
        b"<k>",
    
    # Mayor Kroop.
    
    b"tot_di008100_00":
        b"Hmph? <wait 250>Who izzat?\n"
        b"<wait 250>Oh, it's just you, Murphy.\n"
        b"<wait 250>How kind of you to drop by!\n"
        b"<k>\n"
        b"<p>\n"
        b"Word is that no-goodnik\n"
        b"dragon's up to something, and\n"
        b"yer fixin' to tell it what for!\n"
        b"<k>\n"
        b"<p>\n"
        b"Well, that's mighty nice\n"
        b"of you, Murphy! <wait 250>I'd go an' help\n"
        b"too, if I could...\n"
        b"<k>\n"
        b"<p>\n"
        b"But alas, <wait 100>my fightin' days are\n"
        b"long gone. <wait 250>Heck, <wait 100>I can barely\n"
        b"drag this old shell outta bed!\n"
        b"<k>\n"
        b"<p>\n"
        b"I tell you, it means the world\n"
        b"that you'd check up on an old\n"
        b"coot like me. <shake>*Sniff...*</shake> \n"
        b"<k>\n"
        b"<p>\n"
        b"I'll be rootin' for you, Murphy.\n"
        b"And I'll be here any time to\n"
        b"chat about the good ol' days.\n"
        b"<k>",

    b"tot_di008101_00":
        b"Oh, hello, Murphy. <wait 250>Whuzzat?\n"
        b"<wait 250>You say all that dragon\n"
        b"wanted was a challenge?\n"
        b"<k>\n"
        b"<p>\n"
        b"And you say you're doing\n"
        b"this for a reward and not\n"
        b"just to humor 'em?\n"
        b"<k>\n"
        b"<p>\n"
        b"Well I gotta say, Murphy,\n"
        b"I thought better of you!\n"
        b"<wait 250>Treasure, <wait 150>well, I never...\n"
        b"<k>\n"
        b"<p>\n"
        b"Ah, well, I guess the youth\n"
        b"of today can't afford to be\n"
        b"noble for noble's sake...\n"
        b"<k>\n"
        b"<p>\n"
        b"Why, I could regale you with\n"
        b"countless yarns of daring-do\n"
        b"from back in my salad years.\n"
        b"<k>\n"
        b"<p>\n"
        b"And for nothing but the thrill\n"
        b"of it! <wait 200>Yessir-ee... <wait 300>you come on\n"
        b"back, Murphy, and you'll see!\n"
        b"<k>",

    b"tot_di008200_00":
        b"Back in my day, I was as\n"
        b"hale as ever! I could've run\n"
        b"circles around any hooligans!\n"
        b"<k>\n"
        b"<p>\n"
        b"But I didn't think it fair to\n"
        b"come at those whippersnappers\n"
        b"with everything I had!\n"
        b"<k>\n"
        b"<p>\n"
        b"I didn't need all that fancy\n"
        b"FP and BP stuff, so I just set\n"
        b"it all aside! <wait 250>Yessir...\n"
        b"<k>\n"
        b"<p>\n"
        b"Sure, the fights were intense,\n"
        b"but a fight well fought is its\n"
        b"own reward, I say!\n"
        b"<k>",

    b"tot_di008201_00":
        b"Back in the old days, Murphy,\n"
        b"I got in scraps with some <dynamic 3>REAL</dynamic> \n"
        b"bad baddies, you see?\n"
        b"<k>\n"
        b"<p>\n"
        b"They could dish out and take\n"
        b"twice as many licks as any\n"
        b"upstarts you see nowadays!\n"
        b"<k>\n"
        b"<p>\n"
        b"And I relished the challenge!\n"
        b"<wait 250>Not for a reward, mind you,\n"
        b"<wait 200>but just for the sake of it!\n"
        b"<k>",

    b"tot_di008202_00":
        b"You young'uns have got it\n"
        b"real easy these days, y'know,\n"
        b"with all these fancy shops!\n"
        b"<k>\n"
        b"<p>\n"
        b"Back in my day, I took what\n"
        b"I could carry on my own shell\n"
        b"and my own two legs!\n"
        b"<k>\n"
        b"<p>\n"
        b"And if, <wait 250>mind you,<wait 100> <dynamic 3>if</dynamic>\n"
        b"<wait 350> I had the\n"
        b"fortune of finding a shopkeeper,\n"
        b"he was traveling just as light!\n"
        b"<k>\n"
        b"<p>\n"
        b"You ought to try and learn\n"
        b"to be content with what you\n"
        b"got, Murphy!\n"
        b"<k>\n"
        b"<p>\n"
        b"Spurn those super-stocked\n"
        b"shopkeepers' shelves, <wait 250>slough off\n"
        b"those Strange Sacks...\n"
        b"<k>\n"
        b"<p>\n"
        b"And for Peach's sake, don't\n"
        b"go in carrying your whole house\n"
        b"with you!\n"
        b"<k>",

    b"tot_di008203_00":
        b"You know, Murphy, you young\n"
        b"folk are spoiled for choice\n"
        b"these days!\n"
        b"<k>\n"
        b"<p>\n"
        b"When I was explorin' back in\n"
        b"my halcyon days, I was living\n"
        b"off the land!\n"
        b"<k>\n"
        b"<p>\n"
        b"Sometimes the only choice I\n"
        b"had was to press on or head\n"
        b"back, tail between my legs!\n"
        b"<k>\n"
        b"<p>\n"
        b"And by the stars, press on,\n"
        b"I did! Taking my licks as they\n"
        b"came all the way!\n"
        b"<k>\n"
        b"<p>\n"
        b"You oughtta learn to live\n"
        b"with the choices you're\n"
        b"presented with, Murphy!\n"
        b"<k>\n"
        b"<p>\n"
        b"And just to build character,\n"
        b"not because it gets you some\n"
        b"special achievement!\n"
        b"<k>",
    
    # Gatekeeper Koopa.
    
    b"tot_di012000_00":
        b"With that dragon causing\n"
        b"a ruckus, I can't let anyone\n"
        b"enter the town this way.\n"
        b"<k>\n"
        b"<p>\n"
        b"You're not missing much, at\n"
        b"any rate. <wait 250>The fortress's been\n"
        b"empty for some time.\n"
        b"<k>",
        
    b"tot_di012001_00":
        b"So you and the dragons\n"
        b"have reached an agreement,\n"
        b"and are in it just for sport?\n"
        b"<k>\n"
        b"<p>\n"
        b"That's a relief to hear.\n"
        b"<wait 250>All the same, I probably still\n"
        b"shouldn't let you pass...\n"
        b"<k>\n"
        b"<p>\n"
        b"Wouldn't want to let any\n"
        b"ruffians get through and\n"
        b"wreak havoc in town!\n"
        b"<k>",

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