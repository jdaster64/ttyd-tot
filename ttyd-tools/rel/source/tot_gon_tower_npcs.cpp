#include "tot_gon_tower_npcs.h"

#include "evt_cmd.h"
#include "mod.h"
#include "tot_generate_item.h"
#include "tot_manager_achievements.h"
#include "tot_manager_options.h"
#include "tot_manager_reward.h"
#include "tot_manager_timer.h"
#include "tot_state.h"
#include "tot_window_select.h"

#include <ttyd/evt_badgeshop.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_item.h>
#include <ttyd/evt_mario.h>
#include <ttyd/evt_msg.h>
#include <ttyd/evt_npc.h>
#include <ttyd/evt_pouch.h>
#include <ttyd/evt_shop.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evt_win.h>
#include <ttyd/evt_window.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/msgdrv.h>
#include <ttyd/npc_data.h>
#include <ttyd/npcdrv.h>
#include <ttyd/system.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::tot::gon {

namespace {

// Including entire namespaces for convenience.
using namespace ::ttyd::evt_eff;
using namespace ::ttyd::evt_item;
using namespace ::ttyd::evt_mario;
using namespace ::ttyd::evt_msg;
using namespace ::ttyd::evt_npc;
using namespace ::ttyd::evt_pouch;
using namespace ::ttyd::evt_shop;
using namespace ::ttyd::evt_snd;
using namespace ::ttyd::evt_sub;
using namespace ::ttyd::evt_win;
using namespace ::ttyd::evt_window;

using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::item_data::itemDataTable;
using ::ttyd::npc_data::npcTribe;
using ::ttyd::npcdrv::NpcSetupInfo;
using ::ttyd::system::qqsort;

namespace ItemType = ::ttyd::item_data::ItemType;
namespace NpcTribeType = ::ttyd::npc_data::NpcTribeType;

}  // namespace

namespace SecondaryNpcType {
    enum e {
        NONE = -1,
        WONKY = 0,
        DAZZLE,
        CHET_RIPPO,
        LUMPY,
        DOOPLISS,
        GRUBBA,
        MOVER,
        ZESS_T,
        
        NUM_NPC_TYPES,
        // Used for interpreting run options.
        CHOICE_RANDOM   = NUM_NPC_TYPES,
        CHOICE_NONE     = NUM_NPC_TYPES + 1,
    };
}

namespace RecipeItemRarity {
    enum e {
        INVALID = 0,
        COMMON,
        RARE,
        SPECIALTY,
    };
}

namespace RecipeItemType {
    enum e {
        INVALID = 0,
        HEALING,
        DAMAGE,
        SUPPORT,
        STATUS,
        MISC,
    };
}

namespace RecipeMode {
    enum e {
        INVALID = 0,
        SINGLE_ITEM,
        DOUBLE_ITEM,
        POINT_SWAP,
    };
}

// Holds information relevant to Zess T. cooking items.
struct RecipeInfo {
    int16_t recipe_1;
    int16_t recipe_2;
    int16_t swap_recipe;
    uint8_t item_rarity;
    uint8_t item_type;
};
// Recipe info (for items GOLD_BAR through FRESH_JUICE).
RecipeInfo g_RecipeInfo[] = {
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::ELECTRO_POP, ItemType::THUNDER_RAGE, ItemType::INVALID_ITEM, RecipeItemRarity::COMMON, RecipeItemType::DAMAGE, },
    { ItemType::ELECTRO_POP, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::COMMON, RecipeItemType::DAMAGE, },
    { ItemType::METEOR_MEAL, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::COMMON, RecipeItemType::DAMAGE, },
    { ItemType::SNOW_BUNNY, ItemType::FRUIT_PARFAIT, ItemType::FIRE_FLOWER, RecipeItemRarity::COMMON, RecipeItemType::DAMAGE, },
    { ItemType::FIRE_POP, ItemType::HOT_SAUCE, ItemType::ICE_STORM, RecipeItemRarity::COMMON, RecipeItemType::DAMAGE, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::RARE, RecipeItemType::DAMAGE, },
    { ItemType::LOVE_PUDDING, ItemType::INVALID_ITEM, ItemType::REPEL_CAPE, RecipeItemRarity::COMMON, RecipeItemType::SUPPORT, },
    { ItemType::ELECTRO_POP, ItemType::SHROOM_ROAST, ItemType::INVALID_ITEM, RecipeItemRarity::COMMON, RecipeItemType::SUPPORT, },
    { ItemType::LOVE_PUDDING, ItemType::INVALID_ITEM, ItemType::BOOS_SHEET, RecipeItemRarity::COMMON, RecipeItemType::SUPPORT, },
    { ItemType::PEACH_TART, ItemType::INVALID_ITEM, ItemType::SPITE_POUCH, RecipeItemRarity::COMMON, RecipeItemType::STATUS, },
    { ItemType::PEACH_TART, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::COMMON, RecipeItemType::STATUS, },
    { ItemType::EARTH_QUAKE, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::COMMON, RecipeItemType::DAMAGE, },
    { ItemType::PEACH_TART, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::COMMON, RecipeItemType::STATUS, },
    { ItemType::PEACH_TART, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::COMMON, RecipeItemType::STATUS, },
    { ItemType::LOVE_PUDDING, ItemType::INVALID_ITEM, ItemType::MINI_MR_MINI, RecipeItemRarity::COMMON, RecipeItemType::SUPPORT, },
    { ItemType::LOVE_PUDDING, ItemType::COURAGE_MEAL, ItemType::MR_SOFTENER, RecipeItemRarity::COMMON, RecipeItemType::SUPPORT, },
    { ItemType::POISON_SHROOM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::COMMON, RecipeItemType::DAMAGE, },
    { ItemType::PEACH_TART, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::COMMON, RecipeItemType::STATUS, },
    { ItemType::PEACH_TART, ItemType::SHROOM_BROTH, ItemType::POWER_PUNCH, RecipeItemRarity::COMMON, RecipeItemType::STATUS, },
    { ItemType::PEACH_TART, ItemType::SHROOM_BROTH, ItemType::COURAGE_SHELL, RecipeItemRarity::COMMON, RecipeItemType::STATUS, },
    { ItemType::HONEY_SHROOM, ItemType::SHROOM_FRY, ItemType::HONEY_SYRUP, RecipeItemRarity::COMMON, RecipeItemType::HEALING, },
    { ItemType::MAPLE_SUPER, ItemType::SHROOM_ROAST, ItemType::MAPLE_SYRUP, RecipeItemRarity::COMMON, RecipeItemType::HEALING, },
    { ItemType::JELLY_ULTRA, ItemType::SHROOM_STEAK, ItemType::JAMMIN_JELLY, RecipeItemRarity::COMMON, RecipeItemType::HEALING, },
    { ItemType::POISON_SHROOM, ItemType::INVALID_ITEM, ItemType::DRIED_SHROOM, RecipeItemRarity::COMMON, RecipeItemType::SUPPORT, },
    { ItemType::SPACE_FOOD, ItemType::INVALID_ITEM, ItemType::LIFE_SHROOM, RecipeItemRarity::COMMON, RecipeItemType::SUPPORT, },
    { ItemType::SPACE_FOOD, ItemType::HEALTHY_SALAD, ItemType::SPACE_FOOD, RecipeItemRarity::COMMON, RecipeItemType::SUPPORT, },
    { ItemType::HONEY_SHROOM, ItemType::HONEY_CANDY, ItemType::MUSHROOM, RecipeItemRarity::COMMON, RecipeItemType::HEALING, },
    { ItemType::MAPLE_SUPER, ItemType::ZESS_TEA, ItemType::SUPER_SHROOM, RecipeItemRarity::COMMON, RecipeItemType::HEALING, },
    { ItemType::JELLY_ULTRA, ItemType::JELLY_CANDY, ItemType::ULTRA_SHROOM, RecipeItemRarity::COMMON, RecipeItemType::HEALING, },
    { ItemType::METEOR_MEAL, ItemType::INVALID_ITEM, ItemType::GRADUAL_SYRUP, RecipeItemRarity::COMMON, RecipeItemType::HEALING, },
    { ItemType::METEOR_MEAL, ItemType::INVALID_ITEM, ItemType::SLOW_SHROOM, RecipeItemRarity::COMMON, RecipeItemType::HEALING, },
    { ItemType::SPICY_PASTA, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::COMMON, RecipeItemType::HEALING, },
    { ItemType::SHROOM_CAKE, ItemType::MOUSSE_CAKE, ItemType::INVALID_ITEM, RecipeItemRarity::COMMON, RecipeItemType::HEALING, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::COMMON, RecipeItemType::MISC, },
    { ItemType::PEACH_TART, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::COMMON, RecipeItemType::STATUS, },
    { ItemType::MYSTERY, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::COMMON, RecipeItemType::MISC, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::COCONUT_BOMB, ItemType::COCO_CANDY, ItemType::INVALID_ITEM, RecipeItemRarity::COMMON, RecipeItemType::DAMAGE, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::EGG_BOMB, ItemType::METEOR_MEAL, ItemType::INVALID_ITEM, RecipeItemRarity::COMMON, RecipeItemType::HEALING, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::SPICY_PASTA, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::COMMON, RecipeItemType::SUPPORT, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::LOVE_PUDDING, ItemType::INVALID_ITEM, ItemType::RUIN_POWDER, RecipeItemRarity::COMMON, RecipeItemType::SUPPORT, },
    { ItemType::PEACH_TART, ItemType::POISON_SHROOM, ItemType::INVALID_ITEM, RecipeItemRarity::COMMON, RecipeItemType::STATUS, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::HONEY_CANDY, RecipeItemRarity::RARE, RecipeItemType::HEALING, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::ZESS_TEA, RecipeItemRarity::RARE, RecipeItemType::HEALING, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::JELLY_CANDY, RecipeItemRarity::RARE, RecipeItemType::HEALING, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::RARE, RecipeItemType::HEALING, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::RARE, RecipeItemType::HEALING, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::RARE, RecipeItemType::HEALING, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::SPECIALTY, RecipeItemType::HEALING, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::SPECIALTY, RecipeItemType::HEALING, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::SPECIALTY, RecipeItemType::HEALING, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::SPECIALTY, RecipeItemType::DAMAGE, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::SHROOM_ROAST, RecipeItemRarity::RARE, RecipeItemType::HEALING, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::TASTY_TONIC, RecipeItemRarity::RARE, RecipeItemType::SUPPORT, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::FIRE_POP, RecipeItemRarity::RARE, RecipeItemType::HEALING, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::RARE, RecipeItemType::DAMAGE, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::RARE, RecipeItemType::DAMAGE, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::MOUSSE_CAKE, RecipeItemRarity::RARE, RecipeItemType::HEALING, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::SHROOM_CAKE, RecipeItemRarity::RARE, RecipeItemType::HEALING, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::RARE, RecipeItemType::SUPPORT, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::RARE, RecipeItemType::DAMAGE, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::RARE, RecipeItemType::HEALING, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::RARE, RecipeItemType::SUPPORT, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::PEACH_TART, RecipeItemRarity::RARE, RecipeItemType::SUPPORT, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::RARE, RecipeItemType::SUPPORT, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::SPECIALTY, RecipeItemType::SUPPORT, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::RARE, RecipeItemType::HEALING, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::LOVE_PUDDING, RecipeItemRarity::RARE, RecipeItemType::STATUS, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::RARE, RecipeItemType::HEALING, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::SNOW_BUNNY, RecipeItemRarity::RARE, RecipeItemType::HEALING, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::SHROOM_FRY, RecipeItemRarity::RARE, RecipeItemType::HEALING, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::RARE, RecipeItemType::HEALING, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::SHROOM_STEAK, RecipeItemRarity::RARE, RecipeItemType::HEALING, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::RARE, RecipeItemType::HEALING, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
    { ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, ItemType::INVALID_ITEM, RecipeItemRarity::INVALID, RecipeItemType::INVALID, },
};

// Global variables.
int32_t g_CurrentIngredients[2] = { 0, 0 };
int32_t g_RecipeModes[3];

// Declarations for USER_FUNCs.
EVT_DECLARE_USER_FUNC(evtTot_SelectCharlietonItems, 0)
EVT_DECLARE_USER_FUNC(evtTot_CheckCharlietonSoldOut, 1)
EVT_DECLARE_USER_FUNC(evtTot_TrackNpcAction, 2)
EVT_DECLARE_USER_FUNC(evtTot_CheckAnyStatsDowngradeable, 1)
EVT_DECLARE_USER_FUNC(evtTot_DowngradeStat, 1)
EVT_DECLARE_USER_FUNC(evtTot_GetChetCost, 1)
EVT_DECLARE_USER_FUNC(evtTot_GetDazzleCost, 1)
EVT_DECLARE_USER_FUNC(evtTot_GetMoverCost, 2)
EVT_DECLARE_USER_FUNC(evtTot_GetLumpyInfo, 2)
EVT_DECLARE_USER_FUNC(evtTot_ReturnLumpy, 0)
EVT_DECLARE_USER_FUNC(evtTot_GetRecipeOptionsString, 2)
EVT_DECLARE_USER_FUNC(evtTot_GetSelectedRecipeMode, 3)
EVT_DECLARE_USER_FUNC(evtTot_AddIngredient, 1)
EVT_DECLARE_USER_FUNC(evtTot_MakeIngredientList, 3)
EVT_DECLARE_USER_FUNC(evtTot_MakeRecipeList, 4)
EVT_DECLARE_USER_FUNC(evtTot_ResetIngredients, 0)
EVT_DECLARE_USER_FUNC(evtTot_GetRecipeResults, 3)
EVT_DECLARE_USER_FUNC(evtTot_GetItemName, 2)
EVT_DECLARE_USER_FUNC(evtTot_CheckNpcEffectEnabled, 2)
EVT_DECLARE_USER_FUNC(evtTot_EnableNpcEffect, 1)

// Declarations for NPCs.
extern int32_t g_SecondaryNpcTribeIndices[SecondaryNpcType::NUM_NPC_TYPES];
extern NpcSetupInfo g_SecondaryNpcTemplates[SecondaryNpcType::NUM_NPC_TYPES];
extern NpcSetupInfo g_NpcSetup[3];

// Generic move script that can be shared by all mobile npcs.
EVT_BEGIN(TowerNpc_GenericMove)
    USER_FUNC(evt_npc_flag_onoff, 1, PTR("me"), 0x20410)
    USER_FUNC(evt_npc_status_onoff, 1, PTR("me"), 2)
    // Pick random start direction.
    USER_FUNC(evt_sub_random, 360, LW(3))
LBL(0)
    // Flip around twice.
    USER_FUNC(evt_npc_reverse_ry, PTR("me"))
    WAIT_MSEC(500)
    USER_FUNC(evt_npc_reverse_ry, PTR("me"))
    WAIT_MSEC(1000)
    // Move in a new direction 120-240 degrees different from the last.
    USER_FUNC(evt_npc_get_position, PTR("me"), LW(0), LW(1), LW(2))
    USER_FUNC(evt_npc_get_loiter_dir, LW(3), FLOAT(120.0), FLOAT(240.0))
    USER_FUNC(evt_npc_add_dirdist, LW(0), LW(2), LW(3), FLOAT(100.0))
    USER_FUNC(evt_npc_move_position, PTR("me"), LW(0), LW(2), 2000, FLOAT(40.0), 4)
    GOTO(0)
    RETURN()
EVT_END()

// Generic talk script, placeholder for implementing new NPCs.
EVT_BEGIN(TowerNpc_GenericTalk)
    USER_FUNC(evt_msg_print, 0, PTR("tot_npc_generic"), 0, PTR("me"))
    RETURN()
EVT_END()

// Init script for Charlieton; generates item table.
EVT_BEGIN(TowerNpc_CharlietonInit)
    USER_FUNC(evtTot_SelectCharlietonItems)
    RETURN()
EVT_END()

// Talk script for Charlieton.
EVT_BEGIN(TowerNpc_CharlietonTalk)
    // Check for having no items left to sell.
    USER_FUNC(evtTot_CheckCharlietonSoldOut, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_msg_print, 0, PTR("tot_charlieton_nostock"), 0, PTR("me"))
        RETURN()
    END_IF()
    USER_FUNC(evt_msg_print, 0, PTR("tot_charlieton_intro"), 0, PTR("me"))
    USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_charlieton_decline"))
        RETURN()
    END_IF()
    USER_FUNC(evt_msg_continue)
LBL(0)
    USER_FUNC(evt_win_coin_on, 0, LW(12))
    USER_FUNC(evt_win_other_select, 
        (uint32_t)window_select::MenuType::TOT_CHARLIETON_SHOP)
    IF_EQUAL(LW(0), 0)
        USER_FUNC(evt_win_coin_off, LW(12))
        USER_FUNC(evt_msg_print, 0, PTR("tot_charlieton_decline"), 0, PTR("me"))
        RETURN()
    END_IF()
    USER_FUNC(evt_pouch_get_coin, LW(0))
    IF_SMALL(LW(0), LW(3))
        USER_FUNC(evt_win_coin_off, LW(12))
        USER_FUNC(evt_msg_print, 0, PTR("tot_charlieton_nocoins"), 0, PTR("me"))
        GOTO(0)
    END_IF()
    USER_FUNC(evt_msg_fill_num, 0, LW(14), PTR("tot_charlieton_itemdesc"), LW(3))
    USER_FUNC(evt_msg_fill_item, 1, LW(14), LW(14), LW(2))
    USER_FUNC(evt_msg_print, 1, LW(14), 0, PTR("me"))
    USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_win_coin_off, LW(12))
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_charlieton_decline"))
        RETURN()
    END_IF()
    IF_EQUAL(LW(1), (int32_t)ItemType::STAR_PIECE)
        // Give item directly.
        GOTO(10)
    END_IF()
    USER_FUNC(evt_pouch_add_item, LW(1), LW(0))
    IF_EQUAL(LW(0), -1)
        // Inventory full; prompt if the player wants to buy the item anyway.
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_charlieton_full_inv"))
        USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
        IF_EQUAL(LW(0), 1)  // Declined.
            USER_FUNC(evt_win_coin_off, LW(12))
            USER_FUNC(evt_msg_print_add, 0, PTR("tot_charlieton_decline"))
            RETURN()
        END_IF()
LBL(10)
        MUL(LW(3), -1)
        USER_FUNC(evt_pouch_add_coin, LW(3))
        USER_FUNC(evt_win_coin_wait, LW(12))
        WAIT_MSEC(200)
        USER_FUNC(evt_win_coin_off, LW(12))
        USER_FUNC(evtTot_AfterItemBought, LW(1))
        // Check for Charlieton being sold out.
        USER_FUNC(evtTot_CheckCharlietonSoldOut, LW(0))
        IF_EQUAL(LW(0), 1)
            USER_FUNC(evtTot_MarkCompletedAchievement, AchievementId::MISC_CHARLIETON_OUT_OF_STOCK)
        END_IF()
        // Close text dialog and give item directly.
        USER_FUNC(evt_msg_continue)
        USER_FUNC(evtTot_GetUniqueItemName, LW(0))
        USER_FUNC(evt_item_entry, LW(0), LW(1), FLOAT(0.0), FLOAT(-999.0), FLOAT(0.0), 17, -1, 0)
        USER_FUNC(evt_item_get_item, LW(0))
        RETURN()
    END_IF()
    MUL(LW(3), -1)
    USER_FUNC(evt_pouch_add_coin, LW(3))
    USER_FUNC(evt_win_coin_wait, LW(12))
    WAIT_MSEC(200)
    USER_FUNC(evt_win_coin_off, LW(12))
    USER_FUNC(evtTot_AfterItemBought, LW(1))
    // If out of coins, or Charlieton is out of items, end conversation.
    USER_FUNC(evtTot_CheckCharlietonSoldOut, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_charlieton_success"))
        USER_FUNC(evtTot_MarkCompletedAchievement, AchievementId::MISC_CHARLIETON_OUT_OF_STOCK)
        RETURN()
    END_IF()
    USER_FUNC(evt_pouch_get_coin, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_charlieton_success"))
        RETURN()
    END_IF()
    // Otherwise, offer to sell another.
    USER_FUNC(evt_msg_print_add, 0, PTR("tot_charlieton_buyanother"))
    USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_charlieton_decline"))
        RETURN()
    END_IF()
    USER_FUNC(evt_msg_continue)
    GOTO(0)
    RETURN()
EVT_END()

// Selling items event.
EVT_BEGIN(TowerNpc_SellItems)
    USER_FUNC(ttyd::evt_shop::sell_pouchcheck_func)
    IF_EQUAL(LW(0), 0)
        USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("tot_wonky_noitems"))
        RETURN()
    END_IF()
    USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("tot_wonky_whichitem"))
    USER_FUNC(ttyd::evt_window::evt_win_coin_on, 0, LW(8))
LBL(0)
    USER_FUNC(ttyd::evt_window::evt_win_item_select, 1, 3, LW(1), LW(4))
    IF_SMALL_EQUAL(LW(1), 0)
        USER_FUNC(ttyd::evt_window::evt_win_coin_off, LW(8))
        USER_FUNC(ttyd::evt_msg::evt_msg_print,
            0, PTR("tot_wonky_exit"), 0, PTR("npc_wonky"))
        RETURN()
    END_IF()
    USER_FUNC(ttyd::evt_shop::name_price, LW(1), LW(2), LW(3))
    USER_FUNC(ttyd::evt_msg::evt_msg_fill_num, 0, LW(14), PTR("tot_wonky_itemok"), LW(3))
    USER_FUNC(ttyd::evt_msg::evt_msg_fill_item, 1, LW(14), LW(14), LW(2))
    USER_FUNC(ttyd::evt_msg::evt_msg_print, 1, LW(14), 0, PTR("npc_wonky"))
    USER_FUNC(ttyd::evt_msg::evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("tot_wonky_itemdifferent"))
        GOTO(0)
    END_IF()
    USER_FUNC(ttyd::evt_pouch::N_evt_pouch_remove_item_index, LW(1), LW(4), LW(0))
    USER_FUNC(ttyd::evt_pouch::evt_pouch_add_coin, LW(3))
    USER_FUNC(evtTot_TrackNpcAction, (int32_t)STAT_RUN_NPC_ITEMS_SOLD, 1)
    USER_FUNC(evtTot_EnableNpcEffect, (int32_t)SecondaryNpcType::WONKY)
    USER_FUNC(ttyd::evt_window::evt_win_coin_wait, LW(8))
    WAIT_MSEC(200)
    USER_FUNC(ttyd::evt_window::evt_win_coin_off, LW(8))
    USER_FUNC(ttyd::evt_shop::sell_pouchcheck_func)
    IF_EQUAL(LW(0), 0)
        USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("tot_wonky_thankslast"))
        RETURN()
    END_IF()
    USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("tot_wonky_thanksnext"))
    USER_FUNC(ttyd::evt_msg::evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("tot_wonky_exit"))
        RETURN()
    END_IF()
    USER_FUNC(ttyd::evt_msg::evt_msg_continue)
    USER_FUNC(ttyd::evt_window::evt_win_coin_on, 0, LW(8))
    GOTO(0)
    RETURN()
EVT_END()

// Selling badges event.
EVT_BEGIN(TowerNpc_SellBadges)
    USER_FUNC(ttyd::evt_pouch::evt_pouch_get_havebadgecnt, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("tot_wonky_nobadges"))
        RETURN()
    END_IF()
    USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("tot_wonky_whichitem"))
    USER_FUNC(ttyd::evt_window::evt_win_coin_on, 0, LW(8))
LBL(0)
    USER_FUNC(ttyd::evt_window::evt_win_other_select, 12)
    IF_EQUAL(LW(0), 0)
        USER_FUNC(ttyd::evt_window::evt_win_coin_off, LW(8))
        USER_FUNC(ttyd::evt_msg::evt_msg_print,
            0, PTR("tot_wonky_exit"), 0, PTR("npc_wonky"))
        RETURN()
    END_IF()
    USER_FUNC(ttyd::evt_msg::evt_msg_fill_num, 0, LW(14), PTR("tot_wonky_itemok"), LW(3))
    USER_FUNC(ttyd::evt_msg::evt_msg_fill_item, 1, LW(14), LW(14), LW(2))
    USER_FUNC(ttyd::evt_msg::evt_msg_print, 1, LW(14), 0, PTR("npc_wonky"))
    USER_FUNC(ttyd::evt_msg::evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("tot_wonky_itemdifferent"))
        GOTO(0)
    END_IF()
    USER_FUNC(ttyd::evt_pouch::N_evt_pouch_remove_item_index, LW(1), LW(4), LW(0))
    USER_FUNC(ttyd::evt_pouch::evt_pouch_add_coin, LW(3))
    USER_FUNC(evtTot_TrackNpcAction, (int32_t)STAT_RUN_NPC_BADGES_SOLD, 1)
    USER_FUNC(evtTot_EnableNpcEffect, (int32_t)SecondaryNpcType::WONKY)
    USER_FUNC(ttyd::evt_window::evt_win_coin_wait, LW(8))
    WAIT_MSEC(200)
    USER_FUNC(ttyd::evt_window::evt_win_coin_off, LW(8))
    USER_FUNC(ttyd::evt_pouch::evt_pouch_get_havebadgecnt, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("tot_wonky_thankslast"))
        RETURN()
    END_IF()
    USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("tot_wonky_thanksnext"))
    USER_FUNC(ttyd::evt_msg::evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("tot_wonky_exit"))
        RETURN()
    END_IF()
    USER_FUNC(ttyd::evt_msg::evt_msg_continue)
    USER_FUNC(ttyd::evt_window::evt_win_coin_on, 0, LW(8))
    GOTO(0)
    RETURN()
EVT_END()

// Talk script for Wonky.
EVT_BEGIN(TowerNpc_WonkyTalk)
    USER_FUNC(ttyd::evt_mario::evt_mario_key_onoff, 0)
    USER_FUNC(ttyd::evt_win::unitwin_get_work_ptr, LW(10))
    USER_FUNC(ttyd::evt_msg::evt_msg_print,
        0, PTR("tot_wonky_intro"), 0, PTR("npc_wonky"))
    USER_FUNC(ttyd::evt_msg::evt_msg_select, 0, PTR("tot_wonky_topmenu"))
    SWITCH(LW(0))
        CASE_EQUAL(0)
            RUN_CHILD_EVT(TowerNpc_SellItems)
        CASE_EQUAL(1)
            RUN_CHILD_EVT(TowerNpc_SellBadges)
        CASE_EQUAL(2)
            USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("tot_wonky_exit"))
    END_SWITCH()
    USER_FUNC(ttyd::evt_mario::evt_mario_key_onoff, 1)
    RETURN()
EVT_END()

// Talk script for Chet Rippo.
EVT_BEGIN(TowerNpc_ChetRippoTalk)
    USER_FUNC(evt_mario_key_onoff, 0)
    USER_FUNC(evt_msg_print, 0, PTR("tot_chet_intro"), 0, PTR("me"))
    USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_chet_decline"))
        GOTO(99)
    END_IF()

    USER_FUNC(evtTot_CheckAnyStatsDowngradeable, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_chet_nostats"))
        GOTO(99)
    END_IF()

    USER_FUNC(evt_msg_print_add, 0, PTR("tot_chet_whichstat"))
LBL(0)
    SET(LW(1), 0)
    USER_FUNC(evt_win_other_select, window_select::MenuType::TOT_CHET_RIPPO_TRADE)
    IF_EQUAL(LW(1), 0)
        USER_FUNC(evt_msg_print, 0, PTR("tot_chet_decline"), 0, PTR("me"))
        GOTO(99)
    END_IF()

    USER_FUNC(evt_win_coin_on, 0, LW(8))
    USER_FUNC(evtTot_GetChetCost, LW(5))
    USER_FUNC(
        evt_msg_print_insert, 0, PTR("tot_chet_confirm"), 0, PTR("me"), 
        LW(2), LW(3), LW(4), LW(5))

    USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_chet_different"))
        USER_FUNC(evt_win_coin_off, LW(8))
        GOTO(0)
    END_IF()
    USER_FUNC(evt_pouch_get_coin, LW(0))
    IF_SMALL(LW(0), LW(5))
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_chet_nocoins"))
        USER_FUNC(evt_win_coin_off, LW(8))
        GOTO(99)
    END_IF()

    MUL(LW(5), -1)
    // TODO: Play a sound effect or visual effect?
    USER_FUNC(evt_pouch_add_coin, LW(5))
    USER_FUNC(evtTot_TrackNpcAction, (int32_t)STAT_RUN_NPC_LEVELS_SOLD, 1)
    USER_FUNC(evtTot_EnableNpcEffect, (int32_t)SecondaryNpcType::CHET_RIPPO)
    USER_FUNC(evt_win_coin_wait, LW(8))
    WAIT_MSEC(200)
    USER_FUNC(evt_win_coin_off, LW(8))

    USER_FUNC(evtTot_DowngradeStat, LW(1))
    USER_FUNC(evtTot_CheckCompletedAchievement,
        AchievementId::MISC_CHET_RIPPO_SELL_ALL, EVT_NULLPTR)
    USER_FUNC(evt_msg_continue)
    USER_FUNC(evtTot_GetUniqueItemName, LW(0))
    USER_FUNC(evt_item_entry, LW(0), ItemType::SHINE_SPRITE, FLOAT(0.0), FLOAT(-999.0), FLOAT(0.0), 17, -1, 0)
    USER_FUNC(evt_item_get_item, LW(0))

LBL(99)
    USER_FUNC(evt_mario_key_onoff, 1)
    RETURN()
EVT_END()

// Talk script for Dazzle.
EVT_BEGIN(TowerNpc_DazzleTalk)
    USER_FUNC(evt_mario_key_onoff, 0)

    USER_FUNC(evtTot_GetDazzleCost, LW(5))
    IF_EQUAL(LW(5), 0)
        USER_FUNC(evt_msg_print, 0, PTR("tot_dazzle_intro"), 0, PTR("me"))
        USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
        IF_EQUAL(LW(0), 1)
            USER_FUNC(evt_msg_print_add, 0, PTR("tot_dazzle_decline"))
            GOTO(99)
        END_IF()
        GOTO(10)
    END_IF()

    USER_FUNC(evt_win_coin_on, 0, LW(8))
    USER_FUNC(evt_msg_print_insert, 0, PTR("tot_dazzle_offer"), 0, PTR("me"), LW(5))
    USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_dazzle_decline"))
        USER_FUNC(evt_win_coin_off, LW(8))
        GOTO(99)
    END_IF()
    USER_FUNC(evt_pouch_get_coin, LW(0))
    IF_SMALL(LW(0), LW(5))
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_dazzle_nocoins"))
        USER_FUNC(evt_win_coin_off, LW(8))
        GOTO(99)
    END_IF()
    
    IF_LARGE_EQUAL(LW(5), 100)
        USER_FUNC(evtTot_MarkCompletedAchievement, AchievementId::MISC_DAZZLE_100_COINS)
    END_IF()
    MUL(LW(5), -1)
    USER_FUNC(evt_pouch_add_coin, LW(5))
    USER_FUNC(evt_win_coin_wait, LW(8))
    WAIT_MSEC(200)
    USER_FUNC(evt_win_coin_off, LW(8))

LBL(10)
    USER_FUNC(evt_msg_continue)
    USER_FUNC(evtTot_EnableNpcEffect, (int32_t)SecondaryNpcType::DAZZLE)
    IF_NOT_EQUAL(LW(5), 0)
        USER_FUNC(evtTot_TrackNpcAction, (int32_t)STAT_RUN_NPC_SP_PURCHASED, 1)
    END_IF()
    USER_FUNC(evtTot_GetUniqueItemName, LW(0))
    USER_FUNC(evt_item_entry, LW(0), ItemType::STAR_PIECE, FLOAT(0.0), FLOAT(-999.0), FLOAT(0.0), 17, -1, 0)
    USER_FUNC(evt_item_get_item, LW(0))

LBL(99)
    USER_FUNC(evt_mario_key_onoff, 1)
    RETURN()
EVT_END()

// Talk script for Doopliss.
EVT_BEGIN(TowerNpc_DooplissTalk)
    USER_FUNC(evt_mario_key_onoff, 0)
    USER_FUNC(evtTot_CheckNpcEffectEnabled, (int32_t)SecondaryNpcType::DOOPLISS, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_msg_print, 0, PTR("tot_doopliss_active"), 0, PTR("me"))
        GOTO(99)
    END_IF()

    USER_FUNC(evt_msg_print, 0, PTR("tot_doopliss_intro"), 0, PTR("me"))
    USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_doopliss_decline"))
        GOTO(99)
    END_IF()

    USER_FUNC(evt_msg_print_add, 0, PTR("tot_doopliss_accept"))

    USER_FUNC(evt_npc_get_dir, PTR("me"), LW(3))
    IF_SMALL(LW(3), 180)
        SET(LW(3), 1)
    ELSE()
        SET(LW(3), -1)
    END_IF()

    WAIT_MSEC(100)
    USER_FUNC(evt_npc_get_position, PTR("me"), LW(0), LW(1), LW(2))
    USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_BOSS_RNPL_ARM_UP1"), LW(0), LW(1), LW(2), 0)
    USER_FUNC(evt_npc_set_anim, PTR("me"), PTR("A_3A"))
    WAIT_MSEC(600)
    USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_BOSS_RNPL_ARM_UP1"), LW(0), LW(1), LW(2), 0)
    USER_FUNC(evt_npc_set_anim, PTR("me"), PTR("A_3A"))
    WAIT_MSEC(600)
    USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_BOSS_RNPL_ARM_UP1"), LW(0), LW(1), LW(2), 0)
    USER_FUNC(evt_npc_set_anim, PTR("me"), PTR("A_3A"))
    WAIT_MSEC(200)
    USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_BOSS_RNPL_LAUGH1"), LW(0), LW(1), LW(2), 0)
    USER_FUNC(evt_npc_set_anim, PTR("me"), PTR("A_3B"))
    WAIT_MSEC(800)
    USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_BOSS_RNPL_EYE_SHINE1"), LW(0), LW(1), LW(2), 0)
    SET(LW(4), 6)
    MUL(LW(4), LW(3))
    ADD(LW(0), LW(4))
    ADD(LW(1), 33)
    USER_FUNC(evt_eff, PTR(""), PTR("toge_flush"), 3, LW(0), LW(1), LW(2), 60, 0, 0, 0, 0, 0, 0, 0)
    SET(LW(4), -7)
    MUL(LW(4), LW(3))
    ADD(LW(0), LW(4))
    ADD(LW(1), -1)
    USER_FUNC(evt_eff, PTR(""), PTR("toge_flush"), 3, LW(0), LW(1), LW(2), 60, 0, 0, 0, 0, 0, 0, 0)
    WAIT_MSEC(1000)

    USER_FUNC(evtTot_EnableNpcEffect, (int32_t)SecondaryNpcType::DOOPLISS)
    USER_FUNC(evt_msg_print, 0, PTR("tot_doopliss_active"), 0, PTR("me"))

LBL(99)
    USER_FUNC(evt_mario_key_onoff, 1)
    RETURN()
EVT_END()

// Talk script for Grubba.
EVT_BEGIN(TowerNpc_GrubbaTalk)
    USER_FUNC(evt_mario_key_onoff, 0)
    USER_FUNC(evtTot_CheckNpcEffectEnabled, (int32_t)SecondaryNpcType::GRUBBA, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_msg_print, 0, PTR("tot_grubba_active"), 0, PTR("me"))
        GOTO(99)
    END_IF()

    USER_FUNC(evt_msg_print, 0, PTR("tot_grubba_intro"), 0, PTR("me"))
    USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_grubba_decline"))
        GOTO(99)
    END_IF()

    USER_FUNC(evtTot_EnableNpcEffect, (int32_t)SecondaryNpcType::GRUBBA)
    USER_FUNC(evt_msg_print_add, 0, PTR("tot_grubba_accept"))

LBL(99)
    USER_FUNC(evt_mario_key_onoff, 1)
    RETURN()
EVT_END()

// Talk script for Lumpy.
EVT_BEGIN(TowerNpc_LumpyTalk)
    USER_FUNC(evt_mario_key_onoff, 0)

    // LW(5) = investment (0 if invested this floor), LW(6) = can invest.
    USER_FUNC(evtTot_GetLumpyInfo, LW(5), LW(6))
    IF_LARGE(LW(5), 0)
        GOTO(50)
    END_IF()
    IF_EQUAL(LW(6), 0)
        USER_FUNC(evt_msg_print, 0, PTR("tot_lumpy_goodluck"), 0, PTR("me"))
        GOTO(99)
    END_IF()

    // If no prior coins invested, explain but then acknowledge lack of coins.
    USER_FUNC(evt_pouch_get_coin, LW(5))
    IF_EQUAL(LW(5), 0)
        USER_FUNC(evt_msg_print, 0, PTR("tot_lumpy_intronocoin"), 0, PTR("me"))
        GOTO(99)
    END_IF()
    
    // Explain purpose and offer deal.
    USER_FUNC(evt_win_coin_on, 0, LW(8))
    USER_FUNC(evt_msg_print, 0, PTR("tot_lumpy_intro"), 0, PTR("me"))

LBL(10)
    USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_win_coin_off, LW(8))
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_lumpy_decline"))
        GOTO(99)
    END_IF()
    // Enable (takes all coins away).
    USER_FUNC(evtTot_EnableNpcEffect, (int32_t)SecondaryNpcType::LUMPY)
    USER_FUNC(evt_win_coin_wait, LW(8))
    WAIT_MSEC(200)
    USER_FUNC(evt_win_coin_off, LW(8))

    USER_FUNC(evt_msg_print_add, 0, PTR("tot_lumpy_accept"))
    GOTO(99)

LBL(50)
    // Return past investment with interest.
    USER_FUNC(evt_win_coin_on, 0, LW(8))
    USER_FUNC(evt_msg_print, 0, PTR("tot_lumpy_reward"), 0, PTR("me"))
    USER_FUNC(evtTot_ReturnLumpy)
    USER_FUNC(evt_win_coin_wait, LW(8))
    WAIT_MSEC(200)

    // Check whether the player can invest again.
    IF_EQUAL(LW(6), 0)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_lumpy_goodluckadd"))
        USER_FUNC(evt_win_coin_off, LW(8))
        GOTO(99)
    END_IF()
    USER_FUNC(evt_msg_print_add, 0, PTR("tot_lumpy_doubleorno"))
    GOTO(10)

LBL(99)
    USER_FUNC(evt_mario_key_onoff, 1)
    RETURN()
EVT_END()

// Talk script for Mover.
EVT_BEGIN(TowerNpc_MoverTalk)
    USER_FUNC(evtTot_CheckNpcEffectEnabled, (int32_t)SecondaryNpcType::MOVER, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_msg_print, 0, PTR("tot_mover_active"), 0, PTR("me"))
        GOTO(99)
    END_IF()

    USER_FUNC(evt_msg_print, 0, PTR("tot_mover_intro"), 0, PTR("me"))
    USER_FUNC(evt_msg_select, 0, PTR("tot_mover_menu"))
    IF_EQUAL(LW(0), 2)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_mover_decline"))
        GOTO(99)
    END_IF()

    USER_FUNC(evtTot_GetMoverCost, LW(0), LW(5))
    USER_FUNC(evt_win_coin_on, 0, LW(8))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(evt_msg_print_add_insert, 0, PTR("tot_mover_offer0"), LW(5))
        SET(LW(1), (int32_t)ItemType::TOT_TOWER_KEY)
    ELSE()
        USER_FUNC(evt_msg_print_add_insert, 0, PTR("tot_mover_offer1"), LW(5))
        SET(LW(1), (int32_t)ItemType::TOT_MASTER_KEY)
    END_IF()
    USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_mover_decline"))
        USER_FUNC(evt_win_coin_off, LW(8))
        GOTO(99)
    END_IF()
    USER_FUNC(evt_pouch_get_coin, LW(0))
    IF_SMALL(LW(0), LW(5))
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_mover_nocoins"))
        USER_FUNC(evt_win_coin_off, LW(8))
        GOTO(99)
    END_IF()

    USER_FUNC(evt_pouch_add_item, LW(1), LW(0))
    IF_EQUAL(LW(0), -1)
        // Inventory full; prompt if the player wants to buy the item anyway.
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_mover_full_inv"))
        USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
        IF_EQUAL(LW(0), 1)  // Declined.
            USER_FUNC(evt_win_coin_off, LW(8))
            USER_FUNC(evt_msg_print_add, 0, PTR("tot_mover_decline"))
            GOTO(99)
        END_IF()
        SET(LW(6), 0)
    ELSE()
        // If inventory wasn't full, remove item so it can be given back.
        USER_FUNC(evt_pouch_remove_item, LW(1), LW(0))
        SET(LW(6), 1)
    END_IF()
    
    MUL(LW(5), -1)
    USER_FUNC(evt_pouch_add_coin, LW(5))
    USER_FUNC(evt_win_coin_wait, LW(8))
    WAIT_MSEC(200)
    USER_FUNC(evt_win_coin_off, LW(8))
    USER_FUNC(evtTot_EnableNpcEffect, (int32_t)SecondaryNpcType::MOVER)

    USER_FUNC(evt_msg_continue)
    USER_FUNC(evtTot_GetUniqueItemName, LW(0))
    USER_FUNC(evt_item_entry, LW(0), LW(1), FLOAT(0.0), FLOAT(-999.0), FLOAT(0.0), 17, -1, 0)
    USER_FUNC(evt_item_get_item, LW(0))
    
    IF_EQUAL(LW(6), 1)
        // Skip success text if player has to throw an item away.
        USER_FUNC(evt_msg_print, 0, PTR("tot_mover_success"), 0, PTR("me"))
    END_IF()

LBL(99)
    RETURN()
EVT_END()

// Talk script for Zess T.
EVT_BEGIN(TowerNpc_ZessTalk)
    USER_FUNC(evtTot_GetRecipeOptionsString, LW(15), LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(evt_msg_print, 0, PTR("tot_zess_intronoitems"), 0, PTR("me"))
        GOTO(99)
    END_IF()
    
    USER_FUNC(evt_msg_print, 0, PTR("tot_zess_intro"), 0, PTR("me"))
LBL(10)
    USER_FUNC(evtTot_ResetIngredients)
    SET(LW(8), 0)
    SET(LW(9), 0)
    USER_FUNC(evt_msg_select, 1, LW(15))
    IF_EQUAL(LW(0), 3)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_zess_decline"))
        GOTO(99)
    END_IF()

    USER_FUNC(evtTot_GetSelectedRecipeMode, LW(0), LW(14), LW(5))
    SWITCH(LW(14))
        CASE_EQUAL((int32_t)RecipeMode::SINGLE_ITEM)
            GOTO(40)
        CASE_EQUAL((int32_t)RecipeMode::DOUBLE_ITEM)
            GOTO(20)
        CASE_EQUAL((int32_t)RecipeMode::POINT_SWAP)
            USER_FUNC(evtTot_AddIngredient, (int32_t)ItemType::POINT_SWAP)
            GOTO(40)
        CASE_ETC()
            // Should not be possible to reach; bail out.
            USER_FUNC(evt_msg_continue)
            GOTO(99)
    END_SWITCH()

LBL(20)
    USER_FUNC(evt_msg_print_add, 0, PTR("tot_zess_which1st"))
    USER_FUNC(evtTot_MakeIngredientList, LW(14), LW(13), LW(1))
    USER_FUNC(evt_win_item_select, 0, LW(13), LW(0), 0)
    IF_EQUAL(LW(0), -1)
        USER_FUNC(evt_msg_print, 0, PTR("tot_zess_giveup"), 0, PTR("me"))
        USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
        IF_EQUAL(LW(0), 1)
            USER_FUNC(evt_msg_print_add, 0, PTR("tot_zess_decline"))
            GOTO(99)
        END_IF()
        GOTO(20)
    END_IF()
    USER_FUNC(evtTot_AddIngredient, LW(0))
    SET(LW(9), LW(0))
    USER_FUNC(evtTot_GetItemName, LW(9), LW(7))

LBL(30)
    USER_FUNC(evt_msg_print, 0, PTR("tot_zess_which2nd"), 0, PTR("me"))
    GOTO(50)

LBL(40)
    USER_FUNC(evt_msg_print_add, 0, PTR("tot_zess_which1st"))

LBL(50)
    USER_FUNC(evtTot_MakeIngredientList, LW(14), LW(13), LW(1))
    USER_FUNC(evt_win_item_select, 0, LW(13), LW(0), 0)
    IF_EQUAL(LW(0), -1)
        USER_FUNC(evt_msg_print, 0, PTR("tot_zess_giveup"), 0, PTR("me"))
        USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
        IF_EQUAL(LW(0), 1)
            USER_FUNC(evt_msg_print_add, 0, PTR("tot_zess_decline"))
            GOTO(99)
        END_IF()
        GOTO(40)
    END_IF()
    USER_FUNC(evtTot_AddIngredient, LW(0))
    SET(LW(8), LW(0))
    USER_FUNC(evtTot_GetItemName, LW(8), LW(6))

    USER_FUNC(evtTot_GetRecipeResults, LW(10), LW(11), LW(12))
    IF_SMALL(LW(10), 2)
        USER_FUNC(evt_msg_print, 0, PTR("tot_zess_onlyrecipe"), 0, PTR("me"))
    ELSE()
        USER_FUNC(evt_msg_print, 0, PTR("tot_zess_whichrecipe"), 0, PTR("me"))
    END_IF()
    USER_FUNC(evtTot_MakeRecipeList, LW(13), LW(10), LW(11), LW(12))
    USER_FUNC(evt_win_item_select, 0, LW(13), LW(0), 0)
    IF_EQUAL(LW(0), -1)
        USER_FUNC(evt_msg_print, 0, PTR("tot_zess_nochooserecipe"), 0, PTR("me"))
        GOTO(99)
    END_IF()
    SET(LW(11), LW(0))

    IF_LARGE(LW(5), 0)
        // Prompt with coin cost.
        USER_FUNC(evt_win_coin_on, 0, LW(12))
        USER_FUNC(evt_msg_print_insert, 0, PTR("tot_zess_confirmcost"), 0, PTR("me"), LW(5))
        USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
        IF_EQUAL(LW(0), 1)
            USER_FUNC(evt_msg_print_add, 0, PTR("tot_zess_declinecost"))
            USER_FUNC(evt_win_coin_off, LW(12))
            GOTO(99)
        END_IF()
        USER_FUNC(evt_pouch_get_coin, LW(0))
        IF_SMALL(LW(0), LW(5))
            USER_FUNC(evt_msg_print_add, 0, PTR("tot_zess_declinecost"))
            USER_FUNC(evt_win_coin_off, LW(12))
            GOTO(99)
        END_IF()
        
        MUL(LW(5), -1)
        USER_FUNC(evt_pouch_add_coin, LW(5))
        USER_FUNC(evt_win_coin_wait, LW(12))
        WAIT_MSEC(200)
        USER_FUNC(evt_win_coin_off, LW(12))
    ELSE()
        // Prompt without coin cost.
        USER_FUNC(evt_msg_print_insert, 0, PTR("tot_zess_confirmswap"), 0, PTR("me"), LW(6))
        USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
        IF_EQUAL(LW(0), 1)
            USER_FUNC(evt_msg_print_add, 0, PTR("tot_zess_decline"))
            GOTO(99)
        END_IF()
    END_IF()

    USER_FUNC(evt_msg_continue)

    // Remove items LW(8) and LW(9) if set, give item LW(11).
    USER_FUNC(evt_pouch_remove_item, LW(8), LW(0))
    USER_FUNC(evt_pouch_remove_item, LW(9), LW(0))
    USER_FUNC(evtTot_GetUniqueItemName, LW(0))
    USER_FUNC(evt_item_entry, LW(0), LW(11), FLOAT(0.0), FLOAT(-999.0), FLOAT(0.0), 17, -1, 0)
    USER_FUNC(evt_item_get_item, LW(0))
    USER_FUNC(evtTot_EnableNpcEffect, (int32_t)SecondaryNpcType::ZESS_T)

    SWITCH(LW(14))
        CASE_EQUAL((int32_t)RecipeMode::DOUBLE_ITEM)
            USER_FUNC(evtTot_MarkCompletedAchievement, AchievementId::MISC_ZESS_SIGNATURE)
        CASE_EQUAL((int32_t)RecipeMode::POINT_SWAP)
            USER_FUNC(evtTot_MarkCompletedAchievement, AchievementId::MISC_ZESS_POINT_SWAP)
    END_SWITCH()
    
    // Check whether to prompt to cook something else.    
    USER_FUNC(evtTot_GetRecipeOptionsString, LW(15), LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(evt_msg_print, 0, PTR("tot_zess_cookagain"), 0, PTR("me"))
        GOTO(10)
    END_IF()
    USER_FUNC(evt_msg_print, 0, PTR("tot_zess_nomoretocook"), 0, PTR("me"))

LBL(99)
    RETURN()
EVT_END()

int32_t g_SecondaryNpcTribeIndices[SecondaryNpcType::NUM_NPC_TYPES] = {
    NpcTribeType::WONKY,
    NpcTribeType::DAZZLE,
    NpcTribeType::CHET_RIPPO,
    NpcTribeType::LUMPY,
    NpcTribeType::DOOPLISS,
    NpcTribeType::GRUBBA,
    NpcTribeType::MOVER,
    NpcTribeType::ZESS_T,
};

NpcSetupInfo g_SecondaryNpcTemplates[SecondaryNpcType::NUM_NPC_TYPES] = {
    {
        .name = "npc_wonky",
        .flags = 0x1000'0600,
        .initEvtCode = nullptr,
        .regularEvtCode = nullptr,
        .talkEvtCode = (void*)TowerNpc_WonkyTalk,
        .battleInfoId = -1,
    },
    {
        .name = "npc_dazzle",
        .flags = 0x1000'0600,
        .initEvtCode = nullptr,
        .regularEvtCode = nullptr,
        .talkEvtCode = (void*)TowerNpc_DazzleTalk,
        .battleInfoId = -1,
    },
    {
        .name = "npc_chet",
        .flags = 0x1000'0600,
        .initEvtCode = nullptr,
        .regularEvtCode = nullptr,
        .talkEvtCode = (void*)TowerNpc_ChetRippoTalk,
        .battleInfoId = -1,
    },
    {
        .name = "npc_lumpy",
        .flags = 0x1000'0600,
        .initEvtCode = nullptr,
        .regularEvtCode = nullptr,
        .talkEvtCode = (void*)TowerNpc_LumpyTalk,
        .battleInfoId = -1,
    },
    {
        .name = "npc_doopliss",
        .flags = 0x1000'0600,
        .initEvtCode = nullptr,
        .regularEvtCode = nullptr,
        .talkEvtCode = (void*)TowerNpc_DooplissTalk,
        .battleInfoId = -1,
    },
    {
        .name = "npc_grubba",
        .flags = 0x1000'0600,
        .initEvtCode = nullptr,
        .regularEvtCode = nullptr,
        .talkEvtCode = (void*)TowerNpc_GrubbaTalk,
        .battleInfoId = -1,
    },
    {
        .name = "npc_mover",
        .flags = 0x1000'0600,
        .initEvtCode = nullptr,
        .regularEvtCode = nullptr,
        .talkEvtCode = (void*)TowerNpc_MoverTalk,
        .battleInfoId = -1,
    },
    {
        .name = "npc_zess",
        .flags = 0x1000'0600,
        .initEvtCode = nullptr,
        .regularEvtCode = nullptr,
        .talkEvtCode = (void*)TowerNpc_ZessTalk,
        .battleInfoId = -1,
    },
};

NpcSetupInfo g_NpcSetup[3] = {
    {
        .name = "npc_shop",
        .flags = 0x1000'0600,
        .initEvtCode = (void*)TowerNpc_CharlietonInit,
        .regularEvtCode = nullptr,
        .talkEvtCode = (void*)TowerNpc_CharlietonTalk,
        .battleInfoId = -1,
    },
};

EVT_DEFINE_USER_FUNC(evtTot_SelectCharlietonItems) {
    int16_t* inventory = GetCharlietonInventoryPtr();
    
    // Normal stock = 5 common, 2 rare, 5 (+1 unique) badges;
    // Smaller / Limited stock = 3 common, 1 rare, 3 (+1 unique) badges.
    // Tiny stock = 2 common items, 2 badges, 1 rare item or unique badge.
    int32_t common_items, rare_items, badges;
    switch (g_Mod->state_.GetOptionValue(OPT_CHARLIETON_STOCK)) {
        default:
            common_items = 5;
            rare_items = 2;
            badges = 5;
            break;
        case OPTVAL_CHARLIETON_SMALLER:
        case OPTVAL_CHARLIETON_LIMITED:
            common_items = 3;
            rare_items = 1;
            badges = 3;
            break;
        case OPTVAL_CHARLIETON_TINY:
            common_items = 2;
            rare_items = 0;
            badges = 2;
            break;
    }
    int32_t total_items = common_items + rare_items + badges;
    
    for (int32_t i = 0; i < total_items; ++i) {
        bool found = true;
        while (found) {
            found = false;
            int32_t item = PickRandomItem(
                RNG_NPC_OPTIONS,
                i < common_items,
                i >= common_items && i < common_items + rare_items,
                i >= common_items + rare_items,
                0);
            // Make sure no duplicate items exist.
            for (int32_t j = 0; j < i; ++j) {
                if (inventory[j] == item) {
                    found = true;
                    break;
                }
            }
            inventory[i] = item;
        }
    }
    
    // Add a unique badge, and a (one-time purchase) Star Piece to the shop.
    int32_t special_badge = RewardManager::GetUniqueBadgeForShop();
    if (special_badge) {
        inventory[total_items + 0] = special_badge;
        inventory[total_items + 1] = -1;
        ++badges;
    } else {
        inventory[total_items + 0] = -1;
    }

    // For Tiny mode specifically, fill the fifth slot with a rare item
    // 50% of the time, instead of a specialty badge (if one was selected).
    if (g_Mod->state_.CheckOptionValue(OPTVAL_CHARLIETON_TINY)) {
        int32_t item = PickRandomItem(RNG_NPC_OPTIONS, 0, 50, 0, 50);
        if (item) {
            inventory[5] = -1;
            inventory[4] = inventory[3];
            inventory[3] = inventory[2];
            inventory[2] = item;
            rare_items = 1;
            badges = 2;
        }
    }
    
    // Sort each category by ascending price.
    qqsort(
        &inventory[0], common_items, sizeof(int16_t),
        (void*)BuyPriceComparator);
    qqsort(
        &inventory[common_items], rare_items, sizeof(int16_t),
        (void*)BuyPriceComparator);
    qqsort(
        &inventory[common_items + rare_items], badges, sizeof(int16_t),
        (void*)BuyPriceComparator);
    
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_CheckCharlietonSoldOut) {
    int16_t* inventory = GetCharlietonInventoryPtr();
    evtSetValue(evt, evt->evtArguments[0], inventory[0] == -1);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_TrackNpcAction) {
    uint32_t type = evtGetValue(evt, evt->evtArguments[0]);
    uint32_t amount = evtGetValue(evt, evt->evtArguments[1]);
    g_Mod->state_.ChangeOption(type, amount);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_CheckAnyStatsDowngradeable) {
    auto& state = g_Mod->state_;
    int32_t stats_downgradeable = 0;

    // If stats aren't locked to 0 / infinite, and at least one level...
    if (state.hp_level_ > 0 && state.GetOption(OPT_MARIO_HP) > 0)
        ++stats_downgradeable;
    if (state.fp_level_ > 0 && state.GetOption(OPT_MARIO_FP) > 0)
        ++stats_downgradeable;
    if (state.bp_level_ > 0
        && state.GetOption(OPT_MARIO_BP) > 0
        && !state.CheckOptionValue(OPTVAL_INFINITE_BP)) ++stats_downgradeable;
    if (state.hp_p_level_ > 0
        && state.GetOption(OPT_PARTNER_HP) > 0
        && !state.CheckOptionValue(OPTVAL_NO_PARTNERS)) ++stats_downgradeable;

    evtSetValue(evt, evt->evtArguments[0], stats_downgradeable > 0);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_DowngradeStat) {
    switch (evtGetValue(evt, evt->evtArguments[0])) {
        case 1:
            --g_Mod->state_.hp_level_;
            break;
        case 2:
            --g_Mod->state_.hp_p_level_;
            break;
        case 3:
            --g_Mod->state_.fp_level_;
            break;
        case 4:
            --g_Mod->state_.bp_level_;
            break;
    }
    OptionsManager::UpdateLevelupStats();
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_GetChetCost) {
    evtSetValue(evt, evt->evtArguments[0], 50 * GetBuyPriceScale() / 100);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_GetDazzleCost) {
    auto& state = g_Mod->state_;
    int32_t last_floor_taken = state.GetOption(STAT_RUN_NPC_DAZZLE_FLOOR);
    if (last_floor_taken != state.floor_) {
        // First one's on the house
        evtSetValue(evt, evt->evtArguments[0], 0);
    } else {
        int32_t num_sp_bought = state.GetOption(STAT_RUN_NPC_SP_PURCHASED);
        evtSetValue(evt, evt->evtArguments[0], (num_sp_bought + 1) * 10);
    }
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_GetMoverCost) {
    if (evtGetValue(evt, evt->evtArguments[0]) == 0) {
        evtSetValue(evt, evt->evtArguments[1], 100 * GetBuyPriceScale() / 100);
    } else {
        evtSetValue(evt, evt->evtArguments[1], 200 * GetBuyPriceScale() / 100);
    }
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_GetLumpyInfo) {
    int32_t invested_coins = g_Mod->state_.GetOption(STAT_RUN_NPC_LUMPY_COINS);

    int32_t floor = g_Mod->state_.floor_;
    bool can_invest = !g_Mod->state_.IsFinalBossFloor(floor + 1);
    if (floor == g_Mod->state_.GetOption(STAT_RUN_NPC_LUMPY_FLOOR)) {
        invested_coins = 0;
        can_invest = 0;
    }

    evtSetValue(evt, evt->evtArguments[0], invested_coins);
    evtSetValue(evt, evt->evtArguments[1], can_invest);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_ReturnLumpy) {
    int32_t num_coins = g_Mod->state_.GetOption(STAT_RUN_NPC_LUMPY_COINS);
    ttyd::mario_pouch::pouchAddCoin(num_coins * 2);
    g_Mod->state_.SetOption(STAT_RUN_NPC_LUMPY_COINS, 0);

    AchievementsManager::CheckCompleted(AchievementId::MISC_LUMPY_DOUBLE_2);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_GetRecipeOptionsString) {
    static char buf[128];
    char* p_buf = buf;
    p_buf += sprintf(p_buf, "<select 0 3 0 40>");
    int32_t num_options = 0;

    const auto& pouch = *ttyd::mario_pouch::pouchGetPtr();

    // Regular recipe - require one item that has a single-item result.
    for (int32_t i = 0; i < 20; ++i) {
        if (g_RecipeInfo[pouch.items[i] - ItemType::GOLD_BAR].recipe_1) {
            g_RecipeModes[num_options++] = RecipeMode::SINGLE_ITEM;
            p_buf += sprintf(
                p_buf, "\nRegular recipe (%" PRId32 " coins)",
                25 * GetBuyPriceScale() / 100);
            break;
        }
    }

    // Specialty recipe - require two distinct Rare-tier items.
    int32_t rare_item = 0;
    for (int32_t i = 0; i < 20; ++i) {
        int32_t item = pouch.items[i];
        if (g_RecipeInfo[item - ItemType::GOLD_BAR].item_rarity 
            == RecipeItemRarity::RARE) {
            if (rare_item == 0 || rare_item == item) {
                rare_item = item;
                continue;
            }
            g_RecipeModes[num_options++] = RecipeMode::DOUBLE_ITEM;
            p_buf += sprintf(
                p_buf, "\nSpecialty recipe (%" PRId32 " coins)",
                50 * GetBuyPriceScale() / 100);
            break;
        }
    }

    // Point Swap recipe - require a Point Swap in the inventory.
    for (int32_t i = 0; i < 20; ++i) {
        if (pouch.items[i] == ItemType::POINT_SWAP) {
            g_RecipeModes[num_options++] = RecipeMode::POINT_SWAP;
            p_buf += sprintf(p_buf, "\nPoint Swap recipe (free)");
            break;
        }
    }

    evtSetValue(evt, evt->evtArguments[0], PTR(buf));
    evtSetValue(evt, evt->evtArguments[1], num_options);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_GetSelectedRecipeMode) {
    int32_t index = evtGetValue(evt, evt->evtArguments[0]);
    evtSetValue(evt, evt->evtArguments[1], g_RecipeModes[index]);
    switch(g_RecipeModes[index]) {
        case RecipeMode::SINGLE_ITEM:
            evtSetValue(evt, evt->evtArguments[2], 25 * GetBuyPriceScale() / 100);
            break;
        case RecipeMode::DOUBLE_ITEM:
            evtSetValue(evt, evt->evtArguments[2], 50 * GetBuyPriceScale() / 100);
            break;
        default:
            evtSetValue(evt, evt->evtArguments[2], 0);
            break;
    }
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_AddIngredient) {
    int32_t item = evtGetValue(evt, evt->evtArguments[0]);
    if (g_CurrentIngredients[0] == 0) {
        g_CurrentIngredients[0] = item;
    } else {
        g_CurrentIngredients[1] = item;
    }
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_MakeIngredientList) {
    static int32_t g_IngredientList[21];
    int32_t mode = evtGetValue(evt, evt->evtArguments[0]);
    int32_t index = 0;

    const auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
    for (int32_t i = 0; i < 20; ++i) {
        int32_t item = pouch.items[i];

        // Determine whether the item is eligible to be cooked.
        bool add_item = false;
        switch (mode) {
            case RecipeMode::SINGLE_ITEM:
                add_item = g_RecipeInfo[item - ItemType::GOLD_BAR].recipe_1;
                break;
            case RecipeMode::DOUBLE_ITEM:
                add_item =
                    g_RecipeInfo[item - ItemType::GOLD_BAR].item_rarity 
                        == RecipeItemRarity::RARE &&
                    (g_CurrentIngredients[0] == 0 || g_CurrentIngredients[0] != item);
                break;
            case RecipeMode::POINT_SWAP:
                add_item = g_RecipeInfo[item - ItemType::GOLD_BAR].swap_recipe;
                break;
        }

        // Add the item to the list, if not already present.
        if (add_item) {
            for (int32_t i = 0; i < index; ++i) {
                if (g_IngredientList[i] == item) {
                    add_item = false;
                    break;
                }
            }
            if (add_item) g_IngredientList[index++] = item;
        }
    }
    g_IngredientList[index] = -1;
    evtSetValue(evt, evt->evtArguments[1], PTR(&g_IngredientList[0]));
    evtSetValue(evt, evt->evtArguments[2], index);

    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_MakeRecipeList) {
    static int32_t g_IngredientList[3];
    
    int32_t num_recipes = evtGetValue(evt, evt->evtArguments[1]);
    for (int32_t i = 0; i < num_recipes; ++i) {
        g_IngredientList[i] = evtGetValue(evt, evt->evtArguments[i + 2]);
    }
    g_IngredientList[num_recipes] = -1;
    evtSetValue(evt, evt->evtArguments[0], PTR(&g_IngredientList[0]));

    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_ResetIngredients) {
    g_CurrentIngredients[0] = 0;
    g_CurrentIngredients[1] = 0;
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_GetRecipeResults) {
    if (g_CurrentIngredients[0] == ItemType::MYSTERY) {
        // Return a random rare / specialty item.
        int32_t item = ItemType::SHROOM_FRY;
        int32_t val = g_Mod->state_.Rand(33, RNG_MYSTERY_COOK_RESULT);
        int32_t index = 0;
        for (int32_t i = ItemType::GOLD_BAR; i <= ItemType::FRESH_JUICE; ++i) {
            if (g_RecipeInfo[i - ItemType::GOLD_BAR].item_rarity > 
                RecipeItemRarity::COMMON) {
                if (index++ == val) {
                    item = i;
                    break;
                }
            }
        }
        evtSetValue(evt, evt->evtArguments[0], 1);
        evtSetValue(evt, evt->evtArguments[1], item);
    } else if (g_CurrentIngredients[0] == ItemType::POINT_SWAP) {
        // Return the item's opposite.
        int32_t item =
            g_RecipeInfo[g_CurrentIngredients[1] - ItemType::GOLD_BAR].swap_recipe;
        evtSetValue(evt, evt->evtArguments[0], 1);
        evtSetValue(evt, evt->evtArguments[1], item);
    } else if (g_CurrentIngredients[1]) {
        // Return the correct specialty item.
        int32_t type_1 =
            g_RecipeInfo[g_CurrentIngredients[0] - ItemType::GOLD_BAR].item_type;
        int32_t type_2 =
            g_RecipeInfo[g_CurrentIngredients[1] - ItemType::GOLD_BAR].item_type;
        evtSetValue(evt, evt->evtArguments[0], 1);
        if (type_1 == RecipeItemType::DAMAGE && type_2 == RecipeItemType::DAMAGE) {
            evtSetValue(evt, evt->evtArguments[1], ItemType::ZESS_DYNAMITE);
        } else if (
            type_1 == RecipeItemType::DAMAGE || type_1 == RecipeItemType::STATUS ||
            type_2 == RecipeItemType::DAMAGE || type_2 == RecipeItemType::STATUS) {
            evtSetValue(evt, evt->evtArguments[1], ItemType::TRIAL_STEW);
        } else {
            // Returns Zess T. food item based on total stat worth.
            int32_t total_stats = 0;

            auto& item_1 = itemDataTable[g_CurrentIngredients[0]];
            int32_t item_1_stats = item_1.hp_restored + item_1.fp_restored;
            total_stats += item_1_stats > 5 ? item_1_stats : 5;

            auto& item_2 = itemDataTable[g_CurrentIngredients[1]];
            int32_t item_2_stats = item_2.hp_restored + item_2.fp_restored;
            total_stats += item_2_stats > 5 ? item_2_stats : 5;

            if (total_stats < 25) {
                evtSetValue(evt, evt->evtArguments[1], ItemType::ZESS_DINNER);
            } else if (total_stats < 45) {
                evtSetValue(evt, evt->evtArguments[1], ItemType::ZESS_SPECIAL);
            } else {
                evtSetValue(evt, evt->evtArguments[1], ItemType::ZESS_DELUXE);
            }
        }
    } else {
        // Return the possible recipe(s).
        int32_t item_1 =
            g_RecipeInfo[g_CurrentIngredients[0] - ItemType::GOLD_BAR].recipe_1;
        int32_t item_2 =
            g_RecipeInfo[g_CurrentIngredients[0] - ItemType::GOLD_BAR].recipe_2;
        evtSetValue(evt, evt->evtArguments[0], item_2 ? 2 : 1);
        evtSetValue(evt, evt->evtArguments[1], item_1);
        evtSetValue(evt, evt->evtArguments[2], item_2);
    }
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_GetItemName) {
    int32_t item = evtGetValue(evt, evt->evtArguments[0]);
    evtSetValue(
        evt, evt->evtArguments[1],
        PTR(ttyd::msgdrv::msgSearch(itemDataTable[item].name)));
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_CheckNpcEffectEnabled) {
    const int32_t floor = g_Mod->state_.floor_;
    bool effect = false;
    switch (evtGetValue(evt, evt->evtArguments[0])) {
        case SecondaryNpcType::GRUBBA:
            effect = g_Mod->state_.GetOption(STAT_RUN_NPC_GRUBBA_FLOOR) == floor;
            break;
        case SecondaryNpcType::DOOPLISS:
            effect = g_Mod->state_.GetOption(STAT_RUN_NPC_DOOPLISS_FLOOR) == floor;
            break;
        case SecondaryNpcType::MOVER:
            effect = g_Mod->state_.GetOption(STAT_RUN_NPC_MOVER_FLOOR) == floor;
            break;
    }
    evtSetValue(evt, evt->evtArguments[1], effect);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_EnableNpcEffect) {
    auto& state = g_Mod->state_;
    const int32_t floor = state.floor_;
    switch (evtGetValue(evt, evt->evtArguments[0])) {
        case SecondaryNpcType::WONKY: {
            state.ChangeOption(STAT_PERM_NPC_WONKY_TRADES, 1);
            state.ChangeOption(STAT_PERM_NPC_DEALS_TOTAL, 1);
            break;
        }
        case SecondaryNpcType::DAZZLE: {
            state.SetOption(STAT_RUN_NPC_DAZZLE_FLOOR, floor);
            state.ChangeOption(STAT_PERM_NPC_DAZZLE_TRADES, 1);
            state.ChangeOption(STAT_PERM_NPC_DEALS_TOTAL, 1);
            break;
        }
        case SecondaryNpcType::CHET_RIPPO: {
            state.ChangeOption(STAT_PERM_NPC_RIPPO_TRADES, 1);
            state.ChangeOption(STAT_PERM_NPC_DEALS_TOTAL, 1);
            break;
        }
        case SecondaryNpcType::LUMPY: {
            auto& coins = ttyd::mario_pouch::pouchGetPtr()->coins;
            state.SetOption(STAT_RUN_NPC_LUMPY_COINS, coins);
            state.SetOption(STAT_RUN_NPC_LUMPY_FLOOR, floor);
            state.ChangeOption(STAT_RUN_COINS_EARNED, -coins);
            state.ChangeOption(STAT_PERM_COINS_EARNED, -coins);
            state.ChangeOption(STAT_PERM_NPC_LUMPY_TRADES, 1);
            state.ChangeOption(STAT_PERM_NPC_DEALS_TOTAL, 1);
            coins = 0;
            break;
        }
        case SecondaryNpcType::GRUBBA: {
            state.SetOption(STAT_RUN_NPC_GRUBBA_FLOOR, floor);
            state.ChangeOption(STAT_PERM_NPC_GRUBBA_DEAL, 1);
            state.ChangeOption(STAT_PERM_NPC_DEALS_TOTAL, 1);
            break;
        }
        case SecondaryNpcType::DOOPLISS: {
            state.SetOption(STAT_RUN_NPC_DOOPLISS_FLOOR, floor);
            state.ChangeOption(STAT_PERM_NPC_DOOPLISS_DEAL, 1);
            state.ChangeOption(STAT_PERM_NPC_DEALS_TOTAL, 1);
            break;
        }
        case SecondaryNpcType::MOVER: {
            state.SetOption(STAT_RUN_NPC_MOVER_FLOOR, floor);
            state.ChangeOption(STAT_PERM_NPC_MOVER_TRADES, 1);
            state.ChangeOption(STAT_PERM_NPC_DEALS_TOTAL, 1);
            break;
        }
        case SecondaryNpcType::ZESS_T: {
            state.ChangeOption(STAT_PERM_NPC_ZESS_COOKS, 1);
            state.ChangeOption(STAT_PERM_NPC_DEALS_TOTAL, 1);
            break;
        }
    }
    // Set flag for having dealt with an NPC on each floor.
    state.SetOption(STAT_RUN_NPCS_DEALT_WITH, 1, floor / 8);
    if (state.GetOption(STAT_PERM_NPC_DEALS_TOTAL) >= 10) {
        AchievementsManager::MarkCompleted(AchievementId::AGG_NPC_DEALS_10);
    }

    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_GetTowerNpcParams) {
    // Charlieton parameters.
    evtSetValue(evt, evt->evtArguments[0], PTR(g_NpcSetup[0].name));
    evtSetValue(evt, evt->evtArguments[1], PTR(
        npcTribe[NpcTribeType::CHARLIETON].nameJp));
    evtSetValue(evt, evt->evtArguments[2], PTR(
        npcTribe[NpcTribeType::CHARLIETON].modelName));

    // Secondary NPC parameters (if one exists).
    int32_t floor = g_Mod->state_.floor_ / 8;
    int32_t npc_type = g_Mod->state_.GetOption(STAT_RUN_NPCS_SELECTED, floor);
    if (npc_type == SecondaryNpcType::NONE) {
        memset(&g_NpcSetup[1], 0, sizeof(NpcSetupInfo));
        evtSetValue(evt, evt->evtArguments[3], 0);
        evtSetValue(evt, evt->evtArguments[4], 0);
        evtSetValue(evt, evt->evtArguments[5], 0);
    } else {
        memcpy(
            &g_NpcSetup[1], &g_SecondaryNpcTemplates[npc_type],
            sizeof(NpcSetupInfo));
        evtSetValue(evt, evt->evtArguments[3], PTR(g_NpcSetup[1].name));
        evtSetValue(evt, evt->evtArguments[4], PTR(
            npcTribe[g_SecondaryNpcTribeIndices[npc_type]].nameJp));
        evtSetValue(evt, evt->evtArguments[5], PTR(
            npcTribe[g_SecondaryNpcTribeIndices[npc_type]].modelName));
    }

    evtSetValue(evt, evt->evtArguments[6], PTR(g_NpcSetup));
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_SelectSecondaryNpcs) {
    auto& state = g_Mod->state_;

    const int32_t kPoolDistinct =
        state.CheckOptionValue(OPTVAL_DIFFICULTY_HALF) ? 3 : 4;
    const int32_t kPoolMax =
        state.CheckOptionValue(OPTVAL_DIFFICULTY_HALF) ? 5 : 9;

    // Base weights for each NPC, and how much to reduce them per appearance.
    int32_t base_weights[SecondaryNpcType::NUM_NPC_TYPES + 1] = {
        12, 12, 10, 15, 10, 10, 10, 10, 0
    };
    int32_t sub_weights[SecondaryNpcType::NUM_NPC_TYPES + 1] = {
        3, 3, 2, 5, 2, 2, 2, 2, 0 
    };
    // Active weights for each NPC.
    int32_t weights[SecondaryNpcType::NUM_NPC_TYPES + 1] = { 0 };
    
    // Determine which 3-4 NPCs have a chance to appear.
    // Start by shuffling an array of the different types.
    int32_t npcs_selected[SecondaryNpcType::NUM_NPC_TYPES];
    for (int32_t i = 0; i < SecondaryNpcType::NUM_NPC_TYPES; ++i) {
        npcs_selected[i] = i;
    }
    for (int32_t i = 0; i < 100; ++i) {
        int32_t x = g_Mod->state_.Rand(
            SecondaryNpcType::NUM_NPC_TYPES, RNG_SECONDARY_NPC);
        int32_t y = g_Mod->state_.Rand(
            SecondaryNpcType::NUM_NPC_TYPES, RNG_SECONDARY_NPC);
        int32_t tmp = npcs_selected[x];
        npcs_selected[x] = npcs_selected[y];
        npcs_selected[y] = tmp;
    }
    // Incorporate player's choices.
    int32_t npc_player_choices[] = {
        state.GetOption(OPT_NPC_CHOICE_1), state.GetOption(OPT_NPC_CHOICE_2),
        state.GetOption(OPT_NPC_CHOICE_3), state.GetOption(OPT_NPC_CHOICE_4),
    };
    for (int32_t i = 0; i < kPoolDistinct; ++i) {
        // Bubble sort to order by NPC type, then random, then none.
        for (int32_t j = i+1; j < kPoolDistinct; ++j) {
            if (npc_player_choices[i] > npc_player_choices[j]) {
                int32_t tmp = npc_player_choices[i];
                npc_player_choices[i] = npc_player_choices[j];
                npc_player_choices[j] = tmp;
            }
        }

        // If type is a valid NPC, bring that to the front of active types.
        if (npc_player_choices[i] < SecondaryNpcType::NUM_NPC_TYPES) {
            for (int32_t j = i; j < SecondaryNpcType::NUM_NPC_TYPES; ++j) {
                if (npc_player_choices[i] == npcs_selected[j]) {
                    int32_t tmp = npcs_selected[j];
                    npcs_selected[j] = npcs_selected[i];
                    npcs_selected[i] = tmp;
                }
            }
        } else if (npc_player_choices[i] == SecondaryNpcType::CHOICE_NONE) {
            npcs_selected[i] = SecondaryNpcType::NONE;
        }
    }
    // Set up weights array based on selected types.
    for (int32_t i = 0; i < kPoolDistinct; ++i) {
        if (npcs_selected[i] == SecondaryNpcType::NONE) {
            // Slight weight for no NPC.
            weights[SecondaryNpcType::NUM_NPC_TYPES] += 6;
        } else {
            weights[npcs_selected[i]] = base_weights[npcs_selected[i]];
        }
    }

    // Make a pool of NPCs starting with 1 each of the enabled types, and
    // the rest initialized to type NONE.
    int32_t npc_pool[9];
    for (int32_t i = 0; i < kPoolMax; ++i) {
        npc_pool[i] = SecondaryNpcType::NONE;
    }
    int32_t pool_index = 0;
    for (int32_t i = 0; i < SecondaryNpcType::NUM_NPC_TYPES; ++i) {
        if (weights[i]) npc_pool[pool_index++] = i;
    }
    // Select random NPCs for remaining entries.
    for (pool_index = kPoolDistinct; pool_index < kPoolMax; ++pool_index) {
        int32_t weight = 0;
        for (int32_t i = 0; i <= SecondaryNpcType::NUM_NPC_TYPES; ++i) {
            weight += weights[i];
        }
        weight = g_Mod->state_.Rand(weight, RNG_SECONDARY_NPC);
        for (int32_t i = 0; i < SecondaryNpcType::NUM_NPC_TYPES; ++i) {
            weight -= weights[i];
            if (weight < 0) {
                npc_pool[pool_index] = i;
                weights[i] -= sub_weights[i];
                break;
            }
        }
    }
    // Shuffle pool order with random swaps.
    for (int32_t i = 0; i < 100; ++i) {
        int32_t x = g_Mod->state_.Rand(kPoolMax, RNG_SECONDARY_NPC);
        int32_t y = g_Mod->state_.Rand(kPoolMax, RNG_SECONDARY_NPC);
        int32_t temp = npc_pool[x];
        npc_pool[x] = npc_pool[y];
        npc_pool[y] = temp;
    }

    // Disable certain NPCs for last floor, as they can't have any effect.
    int32_t kNumRestFloors = g_Mod->state_.GetNumFloors() / 8;
    int32_t lumpy_appearances = 0;
    for (int32_t i = 0; i < kNumRestFloors; ++i) {
        if (npc_pool[i] == SecondaryNpcType::LUMPY) ++lumpy_appearances;
    }
    switch (npc_pool[kNumRestFloors - 1]) {
        case SecondaryNpcType::DOOPLISS:
        case SecondaryNpcType::GRUBBA:
        case SecondaryNpcType::MOVER:
            npc_pool[kNumRestFloors - 1] = SecondaryNpcType::NONE;
            break;
        case SecondaryNpcType::LUMPY:
            if (lumpy_appearances == 1) {
                npc_pool[kNumRestFloors - 1] = SecondaryNpcType::NONE;
            }
            break;
    }

    // Assign NPCs.
    for (int32_t i = 0; i < kNumRestFloors; ++i) {
        g_Mod->state_.SetOption(STAT_RUN_NPCS_SELECTED, npc_pool[i], i);
    }

    return 2;
}

int32_t GetNumSecondaryNpcTypes() {
    return SecondaryNpcType::NUM_NPC_TYPES;
}

void GetNpcMsgs(int32_t type, const char** out_name, const char** out_help) {
    switch (type) {
        case SecondaryNpcType::WONKY:
            if (out_name) *out_name = "tot_optr_npc_wonky";
            if (out_help) *out_help = "tot_opth_npc_wonky";
            break;
        case SecondaryNpcType::DAZZLE:
            if (out_name) *out_name = "tot_optr_npc_dazzle";
            if (out_help) *out_help = "tot_opth_npc_dazzle";
            break;
        case SecondaryNpcType::CHET_RIPPO:
            if (out_name) *out_name = "tot_optr_npc_chet";
            if (out_help) *out_help = "tot_opth_npc_chet";
            break;
        case SecondaryNpcType::LUMPY:
            if (out_name) *out_name = "tot_optr_npc_lumpy";
            if (out_help) *out_help = "tot_opth_npc_lumpy";
            break;
        case SecondaryNpcType::DOOPLISS:
            if (out_name) *out_name = "tot_optr_npc_doopliss";
            if (out_help) *out_help = "tot_opth_npc_doopliss";
            break;
        case SecondaryNpcType::GRUBBA:
            if (out_name) *out_name = "tot_optr_npc_grubba";
            if (out_help) *out_help = "tot_opth_npc_grubba";
            break;
        case SecondaryNpcType::MOVER:
            if (out_name) *out_name = "tot_optr_npc_mover";
            if (out_help) *out_help = "tot_opth_npc_mover";
            break;
        case SecondaryNpcType::ZESS_T:
            if (out_name) *out_name = "tot_optr_npc_zess";
            if (out_help) *out_help = "tot_opth_npc_zess";
            break;
        case SecondaryNpcType::CHOICE_RANDOM:
            if (out_name) *out_name = "tot_optr_npc_random";
            if (out_help) *out_help = "tot_opth_npc_generic";
            break;
        case SecondaryNpcType::CHOICE_NONE:
            if (out_name) *out_name = "tot_optr_npc_none";
            if (out_help) *out_help = "tot_opth_npc_generic";
            break;
        default:
            // Should not be reachable.
            if (out_name) *out_name = "tot_optr_npc_none";
            if (out_help) *out_help = "tot_optr_npc_none";
            break;
    }
}

}  // namespace mod::tot::gon