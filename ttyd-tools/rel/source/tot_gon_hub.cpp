#include "tot_gon_hub.h"

#include "evt_cmd.h"
#include "mod.h"
#include "tot_generate_item.h"
#include "tot_gsw.h"
#include "tot_manager_achievements.h"
#include "tot_manager_cosmetics.h"
#include "tot_state.h"
#include "tot_window_select.h"

#include <ttyd/battle_event_cmd.h>
#include <ttyd/database.h>
#include <ttyd/evt_bero.h>
#include <ttyd/evt_cam.h>
#include <ttyd/evt_damage.h>
#include <ttyd/evt_door.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_hit.h>
#include <ttyd/evt_item.h>
#include <ttyd/evt_map.h>
#include <ttyd/evt_mario.h>
#include <ttyd/evt_mobj.h>
#include <ttyd/evt_msg.h>
#include <ttyd/evt_npc.h>
#include <ttyd/evt_pouch.h>
#include <ttyd/evt_shop.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evt_urouro.h>
#include <ttyd/evt_window.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/hitdrv.h>
#include <ttyd/item_data.h>
#include <ttyd/mapdata.h>
#include <ttyd/npc_data.h>
#include <ttyd/npcdrv.h>
#include <ttyd/system.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::tot::gon {

namespace {

// Including entire namespaces for convenience.
using namespace ::ttyd::evt_bero;
using namespace ::ttyd::evt_cam;
using namespace ::ttyd::evt_door;
using namespace ::ttyd::evt_eff;
using namespace ::ttyd::evt_hit;
using namespace ::ttyd::evt_item;
using namespace ::ttyd::evt_map;
using namespace ::ttyd::evt_mario;
using namespace ::ttyd::evt_mobj;
using namespace ::ttyd::evt_msg;
using namespace ::ttyd::evt_npc;
using namespace ::ttyd::evt_pouch;
using namespace ::ttyd::evt_shop;
using namespace ::ttyd::evt_snd;
using namespace ::ttyd::evt_sub;
using namespace ::ttyd::evt_urouro;
using namespace ::ttyd::evt_window;

using ::ttyd::evt_bero::BeroEntry;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::hitdrv::HitReturnPoint;
using ::ttyd::npcdrv::NpcSetupInfo;

namespace BeroAnimType = ::ttyd::evt_bero::BeroAnimType;
namespace BeroDirection = ::ttyd::evt_bero::BeroDirection;
namespace BeroType = ::ttyd::evt_bero::BeroType;
namespace ItemType = ::ttyd::item_data::ItemType;
namespace NpcTribeType = ::ttyd::npc_data::NpcTribeType;

}  // namespace

// gon_10 NPC names.
constexpr char g_NpcNokonokoA[] = "\x83\x6d\x83\x52\x83\x6d\x83\x52" "A";
constexpr char g_NpcNokonokoB[] = "\x83\x6d\x83\x52\x83\x6d\x83\x52" "B";
constexpr char g_NpcNokonokoC[] = "\x83\x6d\x83\x52\x83\x6d\x83\x52" "C";
constexpr char g_NpcNokonokoD[] = "\x83\x6d\x83\x52\x83\x6d\x83\x52" "D";
constexpr char g_NpcNokonokoF[] = "\x83\x6d\x83\x52\x83\x6d\x83\x52" "F";
constexpr char g_NpcInnkeeper[] = "\x8f\x68\x93\x58\x88\xf5";
constexpr char g_NpcShopkeeper[] = "\x93\x58\x88\xf5";
constexpr char g_NpcHooktail[] = "\x83\x6f\x83\x6f";
constexpr char g_NpcGeneralWhite[] = "\x83\x7a\x83\x8f\x83\x43\x83\x67";

// gon_11 NPC names.
constexpr char g_NpcNokonokoG[] = "\x83\x6d\x83\x52\x83\x6d\x83\x52" "G";
constexpr char g_NpcNokonokoH[] = "\x83\x6d\x83\x52\x83\x6d\x83\x52" "H";
constexpr char g_NpcNokonokoI[] = "\x83\x6d\x83\x52\x83\x6d\x83\x52" "I";
constexpr char g_NpcNokonokoK[] = "\x83\x6d\x83\x52\x83\x6d\x83\x52" "K";
constexpr char g_NpcMayorKroop[] = "\x91\xba\x92\xb7";
constexpr char g_NpcGatekeeper[] = "\x96\xe5\x94\xd4\x83\x6d\x83\x52\x83\x6d\x83\x52";
constexpr char g_NpcKoops[] = "\x83\x6d\x83\x52\x83\x5e\x83\x8d\x83\x45";
constexpr char g_NpcKoopley[] = "\x83\x6d\x83\x52\x83\x5e\x83\x8d\x83\x45\x95\x83";
constexpr char g_NpcKoopieKoo[] = "\x83\x6d\x83\x52\x83\x8a\x83\x93";

// Structure forward declarations.
extern const BeroEntry gon_10_entry_data[3];
extern const BeroEntry gon_11_entry_data[3];
extern const NpcSetupInfo gon_10_npc_data[10];
extern const NpcSetupInfo gon_11_npc_data[10];
extern const DoorSubmapInfo gon_10_door_data[3];
extern const DoorSubmapInfo gon_11_door_data[3];
extern HitReturnPoint gon_10_hit_return_points[10];
extern HitReturnPoint gon_11_hit_return_points[13];
extern const char* shop_obj_list[12];
extern ShopItem shop_buy_list[6];
extern ShopItem shop_trade_list[6];
extern ShopkeeperData shopkeeper_data;

// Function declarations.
EVT_DECLARE_USER_FUNC(evtTot_GetCosmeticShopMsg, 3)
EVT_DECLARE_USER_FUNC(evtTot_GetNumCosmeticsUnlockable, 2)
EVT_DECLARE_USER_FUNC(evtTot_UnlockCosmetic, 2)
EVT_DECLARE_USER_FUNC(evtTot_SelectShopItems, 0)
EVT_DECLARE_USER_FUNC(evtTot_DeleteShopItems, 0)

// LW(15) = Cosmetic type (0 ~ 2).
EVT_BEGIN(CosmeticShops_CommonTalkEvt)
    USER_FUNC(evt_mario_normalize)
    USER_FUNC(evt_mario_key_onoff, 0)

    // Set type of sel window / corresponding key item based on cosmetic type.
    SWITCH(LW(15))
        CASE_EQUAL((int32_t)CosmeticType::ATTACK_FX)
            SET(LW(10), (int32_t)window_select::MenuType::COSMETICS_SHOP_ATTACK_FX)
            SET(LW(11), (int32_t)ItemType::TOT_KEY_ATTACK_FX)
        CASE_EQUAL((int32_t)CosmeticType::MARIO_COSTUME)
            SET(LW(10), (int32_t)window_select::MenuType::COSMETICS_SHOP_MARIO_COSTUME)
            SET(LW(11), (int32_t)ItemType::TOT_KEY_MARIO_COSTUME)
        CASE_ETC()
            SET(LW(10), (int32_t)window_select::MenuType::COSMETICS_SHOP_YOSHI_COSTUME)
            SET(LW(11), (int32_t)ItemType::TOT_KEY_YOSHI_COSTUME)
    END_SWITCH()

    USER_FUNC(evtTot_GetNumCosmeticsUnlockable, LW(15), LW(0))
    IF_SMALL_EQUAL(LW(0), 0)
        USER_FUNC(evtTot_GetCosmeticShopMsg, LW(15), 10, LW(13))
        USER_FUNC(evt_msg_print, 0, LW(13), 0, PTR("me"))
        GOTO(99)
    END_IF()

    USER_FUNC(evtTot_GetCosmeticShopMsg, LW(15), 11, LW(13))
    USER_FUNC(evt_msg_print, 0, LW(13), 0, PTR("me"))

LBL(10)
    USER_FUNC(evt_win_coin_on, 2, LW(8))
LBL(11)
    USER_FUNC(evt_win_other_select, LW(10))
    // Cancelled selection.
    IF_EQUAL(LW(0), 0)
        USER_FUNC(evt_win_coin_off, LW(8))
        USER_FUNC(evtTot_GetCosmeticShopMsg, LW(15), 20, LW(13))
        USER_FUNC(evt_msg_print, 0, LW(13), 0, PTR("me"))
        GOTO(99)
    END_IF()
    // Made selection; 1 = subtype, 2 = name string, 3 = price.

    USER_FUNC(evtTot_GetCosmeticShopMsg, LW(15), 12, LW(13))
    USER_FUNC(evt_msg_fill_num, 0, LW(13), LW(13), LW(3))
    USER_FUNC(evt_msg_fill_item, 1, LW(13), LW(13), LW(2))
    USER_FUNC(evt_msg_print, 1, LW(13), 0, PTR("me"))
    USER_FUNC(evt_msg_select, 0, PTR("tot_shopkeep_yesno"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evtTot_GetCosmeticShopMsg, LW(15), 30, LW(13))
        USER_FUNC(evt_msg_print_add, 0, LW(13))
        GOTO(11)
    END_IF()
    USER_FUNC(evt_pouch_get_starpiece, LW(0))
    IF_SMALL(LW(0), LW(3))
        USER_FUNC(evtTot_GetCosmeticShopMsg, LW(15), 13, LW(13))
        USER_FUNC(evt_msg_print_add, 0, LW(13))
        USER_FUNC(evt_win_coin_off, LW(8))
        GOTO(99)
    END_IF()
    USER_FUNC(tot::evtTot_SpendPermanentCurrency, 1, LW(3))
    MUL(LW(3), -1)
    USER_FUNC(evt_pouch_add_starpiece, LW(3))
    USER_FUNC(evt_win_coin_wait, LW(8))
    WAIT_MSEC(200)
    USER_FUNC(evt_win_coin_off, LW(8))

    // Mark cosmetic as collected.
    USER_FUNC(evtTot_UnlockCosmetic, LW(15), LW(1))

    // Check for completing meta cosmetic purchasing achievement.
    USER_FUNC(evtTot_CheckCompletedAchievement,
        AchievementId::META_COSMETICS_5, EVT_NULLPTR)

    // Check for cosmetic's corresponding key item, and give it if missing.
    USER_FUNC(evt_pouch_check_item, LW(11), LW(0))
    IF_LARGE_EQUAL(LW(0), 1)
        GOTO(80)
    END_IF()
    USER_FUNC(evtTot_GetCosmeticShopMsg, LW(15), 40, LW(13))
    USER_FUNC(evt_msg_print_add, 0, LW(13))
    // Give item directly.
    USER_FUNC(evtTot_GetUniqueItemName, LW(0))
    USER_FUNC(evt_item_entry, LW(0), LW(11), FLOAT(0.0), FLOAT(-999.0), FLOAT(0.0), 17, -1, 0)
    USER_FUNC(evt_item_get_item, LW(0))

    // If there are more cosmetics available, prompt for buying more.
    USER_FUNC(evtTot_GetNumCosmeticsUnlockable, LW(15), LW(0))
    IF_LARGE(LW(0), 0)
        USER_FUNC(evtTot_GetCosmeticShopMsg, LW(15), 17, LW(13))
        USER_FUNC(evt_msg_print, 0, LW(13), 0, PTR("me"))
        GOTO(10)
    END_IF()
    USER_FUNC(evtTot_GetCosmeticShopMsg, LW(15), 18, LW(13))
    USER_FUNC(evt_msg_print, 0, LW(13), 0, PTR("me"))

LBL(80)

    // If there are more cosmetics available, prompt for buying more.
    USER_FUNC(evtTot_GetNumCosmeticsUnlockable, LW(15), LW(0))
    IF_LARGE(LW(0), 0)
        USER_FUNC(evtTot_GetCosmeticShopMsg, LW(15), 15, LW(13))
        USER_FUNC(evt_msg_print_add, 0, LW(13))
        GOTO(10)
    END_IF()
    USER_FUNC(evtTot_GetCosmeticShopMsg, LW(15), 16, LW(13))
    USER_FUNC(evt_msg_print_add, 0, LW(13))

LBL(99)
    USER_FUNC(evt_mario_key_onoff, 1)
    RETURN()
EVT_END()

EVT_BEGIN(CosmeticShops_TutorialEvt)
    SWITCH(LW(15))
        CASE_EQUAL(0)
            SET(LW(11), PTR("cshopkeep_A"))
            SET(LW(12), PTR("cshopkeep_M"))
            SET(LW(13), PTR("cshopkeep_Y"))
        CASE_EQUAL(1)
            SET(LW(11), PTR("cshopkeep_M"))
            SET(LW(12), PTR("cshopkeep_A"))
            SET(LW(13), PTR("cshopkeep_Y"))
        CASE_ETC()
            SET(LW(11), PTR("cshopkeep_Y"))
            SET(LW(12), PTR("cshopkeep_M"))
            SET(LW(13), PTR("cshopkeep_A"))
    END_SWITCH()

    USER_FUNC(evt_mario_set_dir_npc, LW(11))
    USER_FUNC(evt_set_dir_to_target, LW(11), PTR("mario"))
    USER_FUNC(evt_msg_print, 0, PTR("tot_cshopkeep_tut1"), 0, LW(11))

    USER_FUNC(evt_mario_set_dir_npc, LW(12))
    USER_FUNC(evt_set_dir_to_target, LW(12), PTR("mario"))
    USER_FUNC(evt_msg_print, 0, PTR("tot_cshopkeep_tut2"), 0, LW(12))

    USER_FUNC(evt_mario_set_dir_npc, LW(13))
    USER_FUNC(evt_set_dir_to_target, LW(13), PTR("mario"))
    USER_FUNC(evt_msg_print, 0, PTR("tot_cshopkeep_tut3"), 0, LW(13))

    // Explanation complete.
    SET((int32_t)tot::GSWF_CosmeticShopTutorial, 1)

    RETURN()
EVT_END()

EVT_BEGIN(CosmeticShopA_TalkEvt)
    SET(LW(15), (int32_t)CosmeticType::ATTACK_FX)
    IF_EQUAL((int32_t)tot::GSWF_CosmeticShopTutorial, 0)
        RUN_CHILD_EVT(CosmeticShops_TutorialEvt)
    ELSE()
        RUN_CHILD_EVT(CosmeticShops_CommonTalkEvt)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(CosmeticShopM_TalkEvt)
    SET(LW(15), (int32_t)CosmeticType::MARIO_COSTUME)
    IF_EQUAL((int32_t)tot::GSWF_CosmeticShopTutorial, 0)
        RUN_CHILD_EVT(CosmeticShops_TutorialEvt)
    ELSE()
        RUN_CHILD_EVT(CosmeticShops_CommonTalkEvt)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(CosmeticShopY_TalkEvt)
    SET(LW(15), (int32_t)CosmeticType::YOSHI_COSTUME)
    IF_EQUAL((int32_t)tot::GSWF_CosmeticShopTutorial, 0)
        RUN_CHILD_EVT(CosmeticShops_TutorialEvt)
    ELSE()
        RUN_CHILD_EVT(CosmeticShops_CommonTalkEvt)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(CosmeticShops_InitEvt)
    // Mario costume shopkeeper (red).
    SET(LW(0), "cshopkeep_M")
    USER_FUNC(evt_npc_entry, LW(0), PTR(
        ttyd::npc_data::npcTribe[NpcTribeType::TOAD_SISTER_R].modelName))
    USER_FUNC(evt_npc_set_tribe, LW(0), PTR(
        ttyd::npc_data::npcTribe[NpcTribeType::TOAD_SISTER_R].nameJp))
    USER_FUNC(evt_npc_change_interrupt, LW(0), 6, PTR(&CosmeticShopM_TalkEvt))
    USER_FUNC(evt_npc_flag_onoff, 1, LW(0), 1536)
    USER_FUNC(evt_npc_set_ry, LW(0), 270)
    USER_FUNC(evt_npc_set_position, LW(0), -60, 0, 340)

    // Attack FX shopkeeper (pink).
    SET(LW(0), "cshopkeep_A")
    USER_FUNC(evt_npc_entry, LW(0), PTR(
        ttyd::npc_data::npcTribe[NpcTribeType::TOAD_SISTER_P].modelName))
    USER_FUNC(evt_npc_set_tribe, LW(0), PTR(
        ttyd::npc_data::npcTribe[NpcTribeType::TOAD_SISTER_P].nameJp))
    USER_FUNC(evt_npc_change_interrupt, LW(0), 6, PTR(&CosmeticShopA_TalkEvt))
    USER_FUNC(evt_npc_flag_onoff, 1, LW(0), 1536)
    USER_FUNC(evt_npc_set_ry, LW(0), 270)
    USER_FUNC(evt_npc_set_position, LW(0), 0, 0, 425)

    // Yoshi costume shopkeeper (green).
    SET(LW(0), "cshopkeep_Y")
    USER_FUNC(evt_npc_entry, LW(0), PTR(
        ttyd::npc_data::npcTribe[NpcTribeType::TOAD_SISTER_G].modelName))
    USER_FUNC(evt_npc_set_tribe, LW(0), PTR(
        ttyd::npc_data::npcTribe[NpcTribeType::TOAD_SISTER_G].nameJp))
    USER_FUNC(evt_npc_change_interrupt, LW(0), 6, PTR(&CosmeticShopY_TalkEvt))
    USER_FUNC(evt_npc_flag_onoff, 1, LW(0), 1536)
    USER_FUNC(evt_npc_set_ry, LW(0), 270)
    USER_FUNC(evt_npc_set_position, LW(0), 60, 0, 340)

    RETURN()
EVT_END()

EVT_BEGIN(Npc_GenericMove)
    USER_FUNC(evt_npc_get_position, PTR("me"), LW(0), LW(1), LW(2))
    USER_FUNC(urouro_init_func, PTR("me"), LW(0), LW(2), FLOAT(100.0), FLOAT(30.0), 0)
    USER_FUNC(urouro_main_func, PTR("me"))
    RETURN()
EVT_END()

EVT_BEGIN(Npc_GenericTalk)
    USER_FUNC(evt_msg_print, 0, PTR("tot_npc_generic"), 0, PTR("me"))
    RETURN()
EVT_END()

EVT_BEGIN(Npc_ShopkeepTalk)
    // Get name of shopkeeper npc.
    USER_FUNC(shopper_name, LW(9))

    // Check for tutorial dialogue.
    IF_EQUAL((int32_t)tot::GSWF_HubShopTutorial, 0)
        // Turn to face each other.
        USER_FUNC(evt_mario_set_dir_npc, LW(9))
        USER_FUNC(evt_set_dir_to_target, LW(9), PTR("mario"))

        USER_FUNC(evt_msg_print, 0, PTR("tot_shopkeep_tut"), 0, LW(9))
LBL(10)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_shopkeep_tut_body"))
        USER_FUNC(evt_msg_select, 0, PTR("tot_shopkeep_tutrepeat"))
        IF_EQUAL(LW(0), 0)
            GOTO(10)
        END_IF()
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_shopkeep_11"))

        // Explanation complete.
        SET((int32_t)tot::GSWF_HubShopTutorial, 1)
        GOTO(99)
    END_IF()

    USER_FUNC(evt_msg_print, 0, PTR("tot_shopkeep_generic"), 0, LW(9))

LBL(99)    
    RETURN()
EVT_END()

EVT_BEGIN(Villager_A_InitEvt)
    USER_FUNC(evt_npc_flag_onoff, 1, PTR("me"), 1536)
    // TODO: Don't run this if in first-time cutscene.
    USER_FUNC(evt_npc_set_position, PTR("me"), -350, 0, 65)
    RETURN()
EVT_END()

EVT_BEGIN(Villager_A_MoveEvt)
    RETURN()
EVT_END()

EVT_BEGIN(Gatekeeper_InitEvt)
    // Location when standing out of the way (should never be the case):
    // USER_FUNC(evt_npc_set_position, PTR("me"), 370, 0, -76)
    RETURN()
EVT_END()

EVT_BEGIN(Gatekeeper_TalkEvt)
    USER_FUNC(evt_msg_print, 0, PTR("tot_npc_generic"), 0, PTR("me"))
    RETURN()
EVT_END()

EVT_BEGIN(gon_10_InitEvt)
    SET(LW(0), PTR(&gon_10_entry_data))
    USER_FUNC(evt_bero_get_info)
    USER_FUNC(evt_snd_bgmon, 512, PTR("BGM_STG1_NOK1"))
    USER_FUNC(evt_snd_envon, 272, PTR("ENV_STG1_NOK1"))
    USER_FUNC(evt_npc_setup, PTR(&gon_10_npc_data))

    // TODO: Add a one-time opening cutscene on first visit.
    IF_SMALL(0, 1)
        RUN_CHILD_EVT(evt_bero_info_run)
    ELSE()
        RUN_EVT(bero_case_entry)
        // RUN_EVT for first time opening cutscene event.
    END_IF()

    USER_FUNC(evt_map_playanim, PTR("S_kawa"), 1, 0)

    // Door setup.
    // TODO: Close shop door and move shopkeeper outside on first visit.
    SET(LW(0), PTR(&gon_10_door_data[0]))
    RUN_CHILD_EVT(evt_door_setup)
    SET(LW(0), PTR(&gon_10_door_data[1]))
    RUN_CHILD_EVT(evt_door_setup)
    SET(LW(0), PTR(&gon_10_door_data[2]))
    RUN_CHILD_EVT(evt_door_setup)

    USER_FUNC(evtTot_SelectShopItems)
    USER_FUNC(evt_shop_setup, PTR(&shop_obj_list), PTR(&shop_buy_list), PTR(&shopkeeper_data), PTR(&shop_trade_list))
    USER_FUNC(evtTot_DeleteShopItems)
    
    USER_FUNC(evt_mobj_save_blk, PTR("mobj_save"), 155, 60, -60, 0, 0)

    DO(5)
        USER_FUNC(evt_sub_random, 10, LW(0))
        USER_FUNC(evt_eff, 0, PTR("butterfly"), LW(0), -395, 20, 250, 0, 0, 0, 0, 0, 0, 0, 0)
    WHILE()
    DO(5)
        USER_FUNC(evt_sub_random, 10, LW(0))
        USER_FUNC(evt_eff, 0, PTR("butterfly"), LW(0), -80, 20, 270, 0, 0, 0, 0, 0, 0, 0, 0)
    WHILE()
    DO(2)
        USER_FUNC(evt_sub_random, 10, LW(0))
        USER_FUNC(evt_eff, 0, PTR("butterfly"), LW(0), -40, 20, 470, 0, 0, 0, 0, 0, 0, 0, 0)
    WHILE()
    DO(2)
        USER_FUNC(evt_sub_random, 10, LW(0))
        USER_FUNC(evt_eff, 0, PTR("butterfly"), LW(0), 375, 20, 250, 0, 0, 0, 0, 0, 0, 0, 0)
    WHILE()

    USER_FUNC(evt_hit_damage_return_set, PTR(&gon_10_hit_return_points))
    RUN_CHILD_EVT(ttyd::evt_damage::evt_gazigazi_entry)
    RETURN()
EVT_END()

EVT_BEGIN(gon_11_InitEvt)
    SET(LW(0), PTR(&gon_11_entry_data))
    USER_FUNC(evt_bero_get_info)
    RUN_CHILD_EVT(PTR(&evt_bero_info_run))

    USER_FUNC(evt_snd_bgmon, 512, PTR("BGM_STG1_NOK1"))
    USER_FUNC(evt_snd_envon, 272, PTR("ENV_STG1_NOK2"))
    USER_FUNC(evt_npc_setup, PTR(&gon_11_npc_data))

    // Turn off fast-travel pipe.
    USER_FUNC(evt_mapobj_flag_onoff, 1, 1, PTR("S_dakan"), 1)
    USER_FUNC(evt_hitobj_onoff, PTR("A_dokan"), 1, 0)

    // TODO: Only init Toad sister NPCs after a couple successful runs.
    RUN_CHILD_EVT(CosmeticShops_InitEvt)

    USER_FUNC(evt_map_playanim, PTR("S_kawa"), 1, 0)

    // Door setup.
    SET(LW(0), PTR(&gon_11_door_data[0]))
    RUN_CHILD_EVT(evt_door_setup)
    SET(LW(0), PTR(&gon_11_door_data[1]))
    RUN_CHILD_EVT(evt_door_setup)
    SET(LW(0), PTR(&gon_11_door_data[2]))
    RUN_CHILD_EVT(evt_door_setup)

    // Run these to open the gate (should never happen):
    // USER_FUNC(evt_map_playanim, PTR("S_mon"), 2, 0)
    // USER_FUNC(evt_hitobj_onoff, PTR("A_mon"), 1, 0)

    DO(2)
        USER_FUNC(evt_sub_random, 10, LW(0))
        USER_FUNC(evt_eff, 0, PTR("butterfly"), LW(0), -395, 20, 225, 0, 0, 0, 0, 0, 0, 0, 0)
    WHILE()
    DO(5)
        USER_FUNC(evt_sub_random, 10, LW(0))
        USER_FUNC(evt_eff, 0, PTR("butterfly"), LW(0), -170, 20, 400, 0, 0, 0, 0, 0, 0, 0, 0)
    WHILE()
    DO(5)
        USER_FUNC(evt_sub_random, 10, LW(0))
        USER_FUNC(evt_eff, 0, PTR("butterfly"), LW(0), 180, 20, 400, 0, 0, 0, 0, 0, 0, 0, 0)
    WHILE()
    DO(2)
        USER_FUNC(evt_sub_random, 10, LW(0))
        USER_FUNC(evt_eff, 0, PTR("butterfly"), LW(0), 400, 20, 200, 0, 0, 0, 0, 0, 0, 0, 0)
    WHILE()

    USER_FUNC(evt_hit_damage_return_set, PTR(&gon_11_hit_return_points))
    RUN_CHILD_EVT(ttyd::evt_damage::evt_gazigazi_entry)
    RETURN()
EVT_END()

const BeroEntry gon_10_entry_data[3] = {
    {
        .name = "w_bero",
        .type = BeroType::ROAD,
        .sfx_id = 0,
        .direction = BeroDirection::AUTO,
        .center_position = { 100000, 0, 0 },
        .length = -1,
        .entry_evt_code = nullptr,
        .case_type = -1,
        .out_evt_code = nullptr,
        .target_map = "gon_00",
        .target_bero = "w_bero",
        .entry_anim_type = BeroAnimType::ANIMATION,
        .out_anim_type = BeroAnimType::ANIMATION,
        .entry_anim_args = nullptr,
        .out_anim_args = nullptr,
    },{
        .name = "e_bero",
        .type = BeroType::ROAD,
        .sfx_id = 0,
        .direction = BeroDirection::AUTO,
        .center_position = { 100000, 0, 0 },
        .length = -1,
        .entry_evt_code = nullptr,
        .case_type = -1,
        .out_evt_code = nullptr,
        .target_map = "gon_11",
        .target_bero = "w_bero",
        .entry_anim_type = BeroAnimType::ANIMATION,
        .out_anim_type = BeroAnimType::ANIMATION,
        .entry_anim_args = nullptr,
        .out_anim_args = nullptr,
    },
    { /* null-terminator */ },
};

const BeroEntry gon_11_entry_data[3] = {
    {
        .name = "w_bero",
        .type = BeroType::ROAD,
        .sfx_id = 0,
        .direction = BeroDirection::AUTO,
        .center_position = { 100000, 0, 0 },
        .length = -1,
        .entry_evt_code = nullptr,
        .case_type = -1,
        .out_evt_code = nullptr,
        .target_map = "gon_10",
        .target_bero = "e_bero",
        .entry_anim_type = BeroAnimType::ANIMATION,
        .out_anim_type = BeroAnimType::ANIMATION,
        .entry_anim_args = nullptr,
        .out_anim_args = nullptr,
    },{
        .name = "e_bero",
        .type = BeroType::ROAD,
        .sfx_id = 0,
        .direction = BeroDirection::AUTO,
        .center_position = { 100000, 0, 0 },
        .length = -1,
        .entry_evt_code = nullptr,
        .case_type = -1,
        .out_evt_code = nullptr,
        .target_map = "gon_00",
        .target_bero = "w_bero",
        .entry_anim_type = BeroAnimType::ANIMATION,
        .out_anim_type = BeroAnimType::ANIMATION,
        .entry_anim_args = nullptr,
        .out_anim_args = nullptr,
    },
    { /* null-terminator */ },
};

const NpcSetupInfo gon_10_npc_data[10] = {
    {
        .name = g_NpcNokonokoA,
        .flags = 0,
        .initEvtCode = (void*)Villager_A_InitEvt,
        .regularEvtCode = (void*)Npc_GenericMove,
        .talkEvtCode = (void*)Npc_GenericTalk,
    },
    {
        .name = g_NpcNokonokoB,
        .flags = 0,
        .initEvtCode = npc_init_evt,
        .regularEvtCode = (void*)Npc_GenericMove,
        .talkEvtCode = (void*)Npc_GenericTalk,
    },
    {
        .name = g_NpcNokonokoC,
        .flags = 0,
        .initEvtCode = npc_init_evt,
        .regularEvtCode = (void*)Npc_GenericMove,
        .talkEvtCode = (void*)Npc_GenericTalk,
    },
    {
        .name = g_NpcNokonokoD,
        .flags = 0,
        .initEvtCode = npc_init_evt,
        .talkEvtCode = (void*)Npc_GenericTalk,
    },
    {
        .name = g_NpcNokonokoF,
        .flags = 0,
        .initEvtCode = npc_init_evt,
        .talkEvtCode = (void*)Npc_GenericTalk,
    },
    {
        .name = g_NpcInnkeeper,
        .flags = 0,
        .initEvtCode = npc_init_evt,
        .talkEvtCode = (void*)Npc_GenericTalk,
    },
    {
        .name = g_NpcShopkeeper,
        .flags = 0,
        .initEvtCode = npc_init_evt,
        .talkEvtCode = (void*)Npc_ShopkeepTalk,
    },
    {
        .name = g_NpcHooktail,
        .flags = 0,
        .initEvtCode = npc_init_evt,
    },
    {
        // Leave disabled for now.
        .name = g_NpcGeneralWhite,
        .flags = 0x4000'0600,
        .initEvtCode = nullptr,     // (void*)init_white
        .talkEvtCode = nullptr,     // (void*)talk_white
    },
};

const NpcSetupInfo gon_11_npc_data[10] = {
    {
        .name = g_NpcNokonokoG,
        .flags = 0,
        .initEvtCode = npc_init_evt,
        .regularEvtCode = (void*)Npc_GenericMove,
        .talkEvtCode = (void*)Npc_GenericTalk,
    },
    {
        .name = g_NpcNokonokoH,
        .flags = 0,
        .initEvtCode = npc_init_evt,
        .regularEvtCode = (void*)Npc_GenericMove,
        .talkEvtCode = (void*)Npc_GenericTalk,
    },
    {
        .name = g_NpcNokonokoI,
        .flags = 0,
        .initEvtCode = npc_init_evt,
        .regularEvtCode = (void*)Npc_GenericMove,
        .talkEvtCode = (void*)Npc_GenericTalk,
    },
    {
        .name = g_NpcNokonokoK,
        .flags = 0,
        .initEvtCode = npc_init_evt,
        .talkEvtCode = (void*)Npc_GenericTalk,
    },
    {
        .name = g_NpcMayorKroop,
        .flags = 0x4000'0600,
        .initEvtCode = npc_init_evt,
        .talkEvtCode = (void*)Npc_GenericTalk,
    },
    {
        .name = g_NpcGatekeeper,
        .flags = 0x4000'0600,
        .initEvtCode = (void*)Gatekeeper_InitEvt,
        .talkEvtCode = (void*)Gatekeeper_TalkEvt,
    },
    {
        // Leave disabled permanently.
        .name = g_NpcKoops,
        .flags = 0x4000'0600,
        .initEvtCode = nullptr,     // (void*)nokotarou_init
        .talkEvtCode = nullptr,     // (void*)nokotarou_talk
    },
    {
        // Leave disabled for now.
        .name = g_NpcKoopley,
        .flags = 0x4000'0600,
        .initEvtCode = nullptr,     // (void*)nokotarou_init
        .talkEvtCode = nullptr,     // (void*)nokotarou_talk
    },
    {
        // Leave disabled for now.
        .name = g_NpcKoopieKoo,
        .flags = 0x4000'0600,
        .initEvtCode = nullptr,     // (void*)nokorin_init
        .talkEvtCode = nullptr,     // (void*)nokorin_talk
    },
};

const char* gon_10_door_1_npcs[] = { g_NpcInnkeeper, nullptr };
const char* gon_10_door_1_map_groups[] = { "S_yad_mae", nullptr };
const char* gon_10_door_2_npcs[] = { g_NpcShopkeeper, nullptr };
const char* gon_10_door_3_npcs[] = { g_NpcNokonokoD, nullptr };
const char* gon_10_door_3_map_groups[] = { "S_ie_mae", nullptr };

const DoorSubmapInfo gon_10_door_data[3] = {
    {
        .type = 1,
        .in_pos = 9,
        .in_hitobj = "A_yad_doa01",
        .out_pos = 9,
        .out_hitobj = "A_yad_in_doa",
        .anim_1 = "S_yado_open",
        .anim_2 = "S_yado_doa_open",
        .anim_3 = "S_yado_close",
        .anim_4 = "S_yado_doa_close",
        .outside_group = "S_yad_mae",
        .door_group = "S_yad_doa01",
        .inside_group_s = "S_yado_in",
        .inside_group_a = "A_yado_in",
        .init_inside_group_s = "S_yado_in",
        .init_inside_group_a = "A_yado_in",
        .npc_list = gon_10_door_1_npcs,
        .map_group_list = gon_10_door_1_map_groups,
    },
    {
        .type = 1,
        .in_pos = 9,
        .in_hitobj = "A_mise_doa",
        .out_pos = 9,
        .out_hitobj = "A_mise_in_doa",
        .anim_1 = "S_mise_open",
        .anim_2 = "S_mise_doa_open",
        .anim_3 = "S_mise_close",
        .anim_4 = "S_mise_doa_close",
        .outside_group = "S_mise_mae",
        .door_group = "S_mise_doa",
        .inside_group_s = "S_mise_in",
        .inside_group_a = "A_mise_in",
        .init_inside_group_s = "S_mise_in",
        .init_inside_group_a = "A_mise_in",
        .npc_list = gon_10_door_2_npcs,
    },
    {
        .type = 1,
        .in_pos = 9,
        .in_hitobj = "A_ie_doa",
        .out_pos = 9,
        .out_hitobj = "A_ie_in_doa",
        .anim_1 = "S_ie_open",
        .anim_2 = "S_ie_doa_open",
        .anim_3 = "S_ie_close",
        .anim_4 = "S_ie_doa_close",
        .outside_group = "S_ie_mae",
        .door_group = "S_ie_doa",
        .inside_group_s = "S_ie_in",
        .inside_group_a = "A_ie_in",
        .init_inside_group_s = "S_ie_in",
        .init_inside_group_a = "A_ie_in",
        .npc_list = gon_10_door_3_npcs,
        .map_group_list = gon_10_door_3_map_groups,
    },
};

const char* gon_11_door_1_npcs[] = { g_NpcMayorKroop, nullptr };
const char* gon_11_door_1_mobjs[] = { "kururin3", nullptr };
const char* gon_11_door_1_items[] = { "iri_08", nullptr };
const char* gon_11_door_2_npcs[] = { g_NpcKoops, nullptr };
const char* gon_11_door_2_map_groups[] = { "S_taro_in", nullptr };
const char* gon_11_door_2_mobjs[] = { "box", nullptr };
const char* gon_11_door_2_items[] = { "item_01", nullptr };
const char* gon_11_door_3_npcs[] = { g_NpcNokonokoK, nullptr };
const char* gon_11_door_3_map_groups[] = { "S_ie_mae", nullptr };

const DoorSubmapInfo gon_11_door_data[3] = {
    {
        .type = 1,
        .in_pos = 9,
        .in_hitobj = "A_son_doa",
        .out_pos = 9,
        .out_hitobj = "A_son_in_doa",
        .anim_1 = "S_son_open",
        .anim_2 = "S_son_doa_open",
        .anim_3 = "S_son_close",
        .anim_4 = "S_son_doa_close",
        .outside_group = "S_son_mae",
        .door_group = "S_son_doa1",
        .inside_group_s = "S_son_in",
        .inside_group_a = "A_son_in",
        .init_inside_group_s = "S_son_in",
        .init_inside_group_a = "A_son_in",
        .npc_list = gon_11_door_1_npcs,
        .mobj_list = gon_11_door_1_mobjs,
        .item_obj_list = gon_11_door_1_items,
    },
    {
        .type = 1,
        .in_pos = 9,
        .in_hitobj = "A_taro_doa",
        .out_pos = 9,
        .out_hitobj = "A_taro_in_doa",
        .anim_1 = "S_taro_open",
        .anim_2 = "S_taro_doa_open",
        .anim_3 = "S_taro_close",
        .anim_4 = "S_taro_doa_close",
        .outside_group = "S_taro_mae",
        .door_group = "S_taro_doa",
        .inside_group_s = "S_taro_in",
        .inside_group_a = "A_taro_in",
        .init_inside_group_s = "S_taro_in",
        .init_inside_group_a = "A_taro_in",
        .npc_list = gon_11_door_2_npcs,
        .map_group_list = gon_11_door_2_map_groups,
        .mobj_list = gon_11_door_2_mobjs,
        .item_obj_list = gon_11_door_2_items,
    },
    {
        .type = 1,
        .in_pos = 9,
        .in_hitobj = "A_ie_doa",
        .out_pos = 9,
        .out_hitobj = "S_ie_in_doa",
        .anim_1 = "S_ie_open",
        .anim_2 = "S_ie_doa_open",
        .anim_3 = "S_ie_close",
        .anim_4 = "S_ie_doa_close",
        .outside_group = "S_ie_mae",
        .door_group = "S_ie_doa",
        .inside_group_s = "S_ie_in",
        .inside_group_a = "A_ie_in",
        .init_inside_group_s = "S_ie_in",
        .init_inside_group_a = "A_ie_in",
        .npc_list = gon_11_door_3_npcs,
        .map_group_list = gon_11_door_3_map_groups,
    },
};

HitReturnPoint gon_10_hit_return_points[10] = {
    { "mod_00", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_01", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_02", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_04", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_05", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_06", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_07", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_08", { 0.0f, -1000.0f, 0.0f }, },
    { "A_hasi04", { 0.0f, -1000.0f, 0.0f }, },
};

HitReturnPoint gon_11_hit_return_points[13] = {
    { "mod_00", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_01", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_02", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_03", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_04", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_05", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_06", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_07", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_08", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_09", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_010", { 0.0f, -1000.0f, 0.0f }, },
    { "A_hasi1", { 0.0f, -1000.0f, 0.0f }, },
};

const char* shop_obj_list[12] = {
    "S_item_01", "A_item_01",
    "S_item_02", "A_item_02",
    "S_item_03", "A_item_03",
    "S_item_04", "A_item_04",
    "S_item_05", "A_item_05",
    "S_item_06", "A_item_06",
};
// Dynamically filled; the last item will always be a Star Piece.
ShopItem shop_buy_list[6] = {
    {}, {}, {}, {}, {}, { .item_id = ItemType::STAR_PIECE, .buy_price = 99, },
};
ShopkeeperData shopkeeper_data = {
    .npc_name = g_NpcShopkeeper,
};
ShopItem shop_trade_list[6] = {};

const int32_t* GetWestSideInitEvt() {
    return gon_10_InitEvt;
}

const int32_t* GetEastSideInitEvt() {
    return gon_11_InitEvt;
}

EVT_DEFINE_USER_FUNC(evtTot_GetCosmeticShopMsg) {
    static char buf[16];
    int32_t cosmetic_type = evtGetValue(evt, evt->evtArguments[0]);
    int32_t msg_num = evtGetValue(evt, evt->evtArguments[1]);
    sprintf(buf, "tot_cshop%" PRId32 "_%02" PRId32, cosmetic_type, msg_num);
    evtSetValue(evt, evt->evtArguments[2], PTR(buf));
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_GetNumCosmeticsUnlockable) {
    int32_t cosmetic_type = evtGetValue(evt, evt->evtArguments[0]);
    int32_t num_cosmetics = 0;
    for (int32_t i = 1; i <= 30; ++i) {
        num_cosmetics += CosmeticsManager::IsPurchaseable(cosmetic_type, i);
    }
    evtSetValue(evt, evt->evtArguments[1], num_cosmetics);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_UnlockCosmetic) {
    int32_t cosmetic_type = evtGetValue(evt, evt->evtArguments[0]);
    int32_t cosmetic_id = evtGetValue(evt, evt->evtArguments[1]);
    CosmeticsManager::MarkAsPurchased(cosmetic_type, cosmetic_id);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_SelectShopItems) {
    auto& state = g_Mod->state_;

    // Pick the items if they haven't already been chosen since the last run.
    if (!state.GetOption(OPT_SHOP_ITEMS_CHOSEN)) GenerateHubShopItems();

    // Read previously selected items.
    for (int32_t i = 0; i < 5; ++i) {
        int32_t id = state.GetOption(STAT_PERM_SHOP_ITEMS, i);
        if (id == 255) {
            // Sentinel.
            shop_buy_list[i].item_id = 1;
        } else {
            id += ItemType::THUNDER_BOLT;
            shop_buy_list[i].item_id = id;
            shop_buy_list[i].buy_price = 
                ttyd::item_data::itemDataTable[id].buy_price * 2;
        }
    }

    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_DeleteShopItems) {
    for (int32_t i = 0; i < 5; ++i) {
        int32_t item = shop_buy_list[i].item_id;
        // Delete sentinel items and ones that are already purchased.
        if (item == 1 ||
            g_Mod->state_.GetOption(
                FLAGS_ITEM_PURCHASED, item - ItemType::THUNDER_BOLT)) {
            ttyd::evt_shop::g_ShopWork->item_flags[i] |= 1;
        }
    }
    return 2;
}

}  // namespace mod::tot::gon