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
        
        # Special item descriptions.
        
        b"msg_super_coin":
            b"A powerful object that ranks\n"
            b"up a move of your choice.",

        b'msg_star_piece':
            b'A capricious object that ranks\n'
            b'up your moves at random.',
            
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
            b'Shroom.',

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

        b'menu_hammer_only':
            b'Increase hammer power by 1,\n'
            b'but lose the ability to jump.\n'
            b'Mario will throw his hammer\n'
            b'for single-target attacks.',

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
            b'Makes Mario immune to many\n'
            b'kinds of status effects.',

        b'msg_itsumo_genki':
            b'Makes Mario immune to many\n'
            b'kinds of status effects.',

        b'menu_itsumo_genki_p':
            b'Makes allies immune to many\n'
            b'kinds of status effects.',

        b'msg_itsumo_genki_p':
            b'Make allies immune to many\n'
            b'kinds of status effects.',

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
            b"run early and restart from\n"
            b"the lobby?\n<o>",
        
        b"tot_gameover":
            b"<system>\n<p>\n"
            b"Do you want to continue from\n"
            b"after the last boss, or give up\n"
            b"and restart from the lobby?\n<o>",
            
        b"tot_gameover_opt":
            b"<select 0 1 0 40>\nContinue\nGive Up",
        
        # Enemy names + Tattle info (TODO)
        
        b"btl_un_monban":             b"Craw",
        b"btl_un_sinnosuke":          b"H. Bob-omb",
        b"btl_un_hyper_sinnosuke":    b"Cosmic Boo",
        
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
        b"tot_optr_npc_wonky":          b"NPCs: Wonky",
        b"tot_optr_npc_dazzle":         b"NPCs: Dazzle",
        b"tot_optr_npc_chet":           b"NPCs: Chet Rippo",
        b"tot_optr_npc_lumpy":          b"NPCs: Lumpy",
        b"tot_optr_npc_doopliss":       b"NPCs: Doopliss",
        b"tot_optr_npc_grubba":         b"NPCs: Grubba",
        b"tot_optr_npc_mover":          b"NPCs: Mover",
        b"tot_optr_npc_zess":           b"NPCs: Zess T.",
        
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
            
        # Map text.
        
        b"tot_lobby_frontsign":
            b'<kanban>\n'
            b'<pos 116 10>\n"Battle Tower"\n'
            b'<pos 77 39>\nUnder Construction\n<k>',
        
        b"tot_lobby_backsign":
            b'<kanban>\n'
            b'Current seed: <col 0000ffff>\n'
            b'%09d\n</col>\n'
            b'Current options: <col 0000ffff>\n'
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
            
        b"tot_npc_generic":
            b"[Placeholder] I'm interactable.\n<k>",
        
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
            b"she doing back here now?\n<k>\n<p>\n"
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