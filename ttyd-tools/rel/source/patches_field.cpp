#include "patches_field.h"

#include "evt_cmd.h"
#include "mod.h"
#include "patch.h"
#include "tot_generate_item.h"
#include "tot_gsw.h"
#include "tot_manager_achievements.h"
#include "tot_manager_cosmetics.h"
#include "tot_state.h"
#include "tot_window_select.h"

#include <gc/types.h>
#include <ttyd/battle_monosiri.h>
#include <ttyd/evt_item.h>
#include <ttyd/evt_mario.h>
#include <ttyd/evt_msg.h>
#include <ttyd/evt_npc.h>
#include <ttyd/evt_pouch.h>
#include <ttyd/evt_shop.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evt_window.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_party.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/npc_data.h>
#include <ttyd/npc_event.h>
#include <ttyd/npcdrv.h>
#include <ttyd/pmario_sound.h>
#include <ttyd/system.h>

#include <cstring>

// Assembly patch functions.
extern "C" {
    // attack_fx_patches.s
    void StartPlayFieldHammerFX();
    void BranchBackPlayFieldHammerFX();
    // danger_threshold_patches.s
    void StartFieldDangerIdleCheck1();
    void BranchBackFieldDangerIdleCheck1();
    void StartFieldDangerIdleCheck2();
    void BranchBackFieldDangerIdleCheck2();
    void StartFieldDangerIdleCheck3();
    void BranchBackFieldDangerIdleCheck3();
    void StartFieldDangerIdleCheck4();
    void BranchBackFieldDangerIdleCheck4();
    
    void playFieldHammerFX(gc::vec3* position) {
        int32_t id = mod::tot::CosmeticsManager::PickActiveFX();
        if (id) {
            const char* fx_name = mod::tot::CosmeticsManager::GetSoundFromFXGroup(id);
            const auto* data = mod::tot::CosmeticsManager::GetAttackFxData(id);

            int32_t sfx_id = ttyd::pmario_sound::psndSFXOn_3D(fx_name, position);
            if (data->randomize_pitch) {
                // Play at one of a few random pitches.
                int16_t pitch = 0x400 * (ttyd::system::irand(3) - 1);
                ttyd::pmario_sound::psndSFX_pit(sfx_id, pitch);
            }
        } else {
            // Play standard hammer impact sound.
            ttyd::pmario_sound::psndSFXOn_3D("SFX_MARIO_HAMMER_WOOD_DON1", position);
        }
    }

    int32_t getMarioHpForFieldAnim() {
        // Return a fake value for the purposes of setting Mario's field anim.
        const auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
        int32_t fake_hp = 10;

        const int32_t danger_threshold = (pouch.max_hp * 3 + 5) / 10;
        if (pouch.current_hp <= 0) {
            fake_hp = 0;
        } else if (pouch.current_hp <= danger_threshold) {
            fake_hp = 5;
        }

        return fake_hp;
    }
}

namespace mod::tot::patch {

// Function hooks.
extern int32_t (*g_partyGetHp_trampoline)(int32_t);
// Patch addresses.
extern const int32_t g_mot_hammer_PickHammerFieldSfx_BH;
extern const int32_t g_mot_hammer_PickHammerFieldSfx_EH;
extern const int32_t g_mot_stay_MarioCheckDangerIdleAnim1_BH;
extern const int32_t g_mot_stay_MarioCheckDangerIdleAnim2_BH;
extern const int32_t g_mot_stay_MarioCheckDangerIdleAnim3_BH;
extern const int32_t g_mot_stay_MarioCheckDangerIdleAnim4_BH;
extern const int32_t g_evt_shop_setup_Patch_DisableShopperTalkEvt;
extern const int32_t g_mot_damage_Patch_DisableFallDamage;
extern const int32_t g_chorobon_move_event_Patch_SetScale;
extern const int32_t g_chorobon_find_event_Patch_SetScale;
extern const int32_t g_chorobon_lost_event_Patch_SetScale;
extern const int32_t g_chorobon_return_event_Patch_SetScale;
extern const int32_t g_zakowiz_find_event_Patch_ProjectileScaleHook;

namespace field {

namespace {

// For convenience.
using namespace ::ttyd::evt_item;
using namespace ::ttyd::evt_mario;
using namespace ::ttyd::evt_msg;
using namespace ::ttyd::evt_npc;
using namespace ::ttyd::evt_pouch;
using namespace ::ttyd::evt_shop;
using namespace ::ttyd::evt_sub;
using namespace ::ttyd::evt_window;

using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::npcdrv::NpcTribeDescription;

namespace BattleUnitType = ::ttyd::battle_database_common::BattleUnitType;
namespace ItemType = ::ttyd::item_data::ItemType;
namespace NpcAiType = ::ttyd::npc_data::NpcAiType;

int16_t g_ShopSignItems[256] = { -1 };
int32_t g_NumShopSignItems = 0;

}

// Populates the list of items that can be bought from the shop sign.
// out arg0 = the number of available items.
EVT_DECLARE_USER_FUNC(evtTot_InitializeShopSign, 1)
EVT_DEFINE_USER_FUNC(evtTot_InitializeShopSign) {
    const auto& state = g_Mod->state_;

    int32_t shelf_items[5];
    for (int32_t i = 0; i < 5; ++i) {
        shelf_items[i] = state.GetOption(STAT_PERM_SHOP_ITEMS, i);
    }

    g_NumShopSignItems = 0;
    for (int32_t id = 0; id < ItemType::MAX_ITEM_TYPE - ItemType::THUNDER_BOLT; ++id) {
        // Skip invalid, non-encountered, or already purchased items.
        if (ttyd::item_data::itemDataTable[id + ItemType::THUNDER_BOLT]
                .type_sort_order < 0 ||
            !state.GetOption(FLAGS_ITEM_ENCOUNTERED, id) ||
            state.GetOption(FLAGS_ITEM_PURCHASED, id)) {
            continue;
        }
        // Skip items that are already on display on the shelf.
        bool found = false;
        for (int32_t j = 0; j < 5; ++j) {
            if (shelf_items[j] == id) found = true;
        }
        if (!found) {
            g_ShopSignItems[g_NumShopSignItems++] = id + ItemType::THUNDER_BOLT;
        }
    }
    g_ShopSignItems[g_NumShopSignItems] = -1;
    
    // Sort items + badges by combined type sort order.
    ttyd::system::qqsort(
        g_ShopSignItems, g_NumShopSignItems, sizeof(int16_t),
        (void*)TypeSortOrderComparator);

    evtSetValue(evt, evt->evtArguments[0], g_NumShopSignItems);

    return 2;
}

// Marks item as collected and removes it from display, if applicable.
// arg0 = shop work, arg1 = item id (only set if item was back-order).
EVT_DECLARE_USER_FUNC(evtTot_AfterBuyingShopItem, 2)
EVT_DEFINE_USER_FUNC(evtTot_AfterBuyingShopItem) {
    auto* work = (ShopWork*)evtGetValue(evt, evt->evtArguments[0]);
    int32_t item_id = evtGetValue(evt, evt->evtArguments[1]);
    bool on_shelf = false;

    // If no item id provided, must be from shelf.
    if (item_id < 0) {
        item_id = work->purchased_item_id;
        on_shelf = true;
    }

    // Mark normal items as permanently purchased.
    if (item_id >= ItemType::THUNDER_BOLT) {
        g_Mod->state_.SetOption(FLAGS_ITEM_PURCHASED, item_id - 0x80);

    }

    // Remove non-currency items from shelf.
    if (item_id != ItemType::STAR_PIECE) {
        if (on_shelf) work->item_flags[work->buy_item_idx] |= 1;
    } else {
        ttyd::mario_pouch::pouchAddStarPiece(1);
        g_Mod->state_.ChangeOption(STAT_PERM_CURRENT_SP, 1);
    }
    
    return 2;
}

EVT_BEGIN(HammerBroInit_WrapperEvt)
    RUN_CHILD_EVT(ttyd::npc_event::hbross_init_event)
    USER_FUNC(evtTot_IsMidbossFloor, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_npc_set_scale, PTR("slave_0"), 2, 2, 2)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(DryBonesInit_WrapperEvt)
    RUN_CHILD_EVT(ttyd::npc_event::karon_init_event)
    USER_FUNC(evtTot_IsMidbossFloor, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_npc_set_scale, PTR("slave_0"), 2, 2, 2)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(WizzerdInit_WrapperEvt)
    RUN_CHILD_EVT(ttyd::npc_event::mahoon_init_event)
    USER_FUNC(evtTot_IsMidbossFloor, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_npc_set_scale, PTR("slave_0"), 2, 2, 2)
        USER_FUNC(evt_npc_set_scale, PTR("slave_1"), 2, 2, 2)
        USER_FUNC(evt_npc_set_scale, PTR("slave_2"), 2, 2, 2)
        USER_FUNC(evt_npc_set_scale, PTR("slave_3"), 2, 2, 2)
    END_IF()
    RETURN()
EVT_END()

// Runs passively when Wizzerd on field is in most movement states.
EVT_BEGIN(WizzerdHandsIdle_Evt)
    USER_FUNC(evt_npc_get_dir, PTR("me"), LW(12))

    // Scale to 1x if non-midboss, or 2x if midboss.
    USER_FUNC(evtTot_IsMidbossFloor, LW(13))
    ADD(LW(13), 1)

    SET(LW(14), 0)
    DO(4)
        SWITCH(LW(14))
            CASE_EQUAL(0)
                SET(LW(5), PTR("slave_0"))
                SET(LW(6), 25)
                SET(LW(7), 5)
            CASE_EQUAL(1)
                SET(LW(5), PTR("slave_1"))
                SET(LW(6), -25)
                SET(LW(7), 5)
            CASE_EQUAL(2)
                SET(LW(5), PTR("slave_2"))
                SET(LW(6), 25)
                SET(LW(7), 30)
            CASE_EQUAL(3)
                SET(LW(5), PTR("slave_3"))
                SET(LW(6), -25)
                SET(LW(7), 30)
        END_SWITCH()
        MUL(LW(6), LW(13))
        MUL(LW(7), LW(13))

        USER_FUNC(evt_npc_set_ry, LW(5), LW(12))
        USER_FUNC(evt_npc_get_position, PTR("me"), LW(0), LW(1), LW(2))
        USER_FUNC(evt_npc_add_dirdist, LW(0), LW(2), LW(12), LW(6))
        ADD(LW(1), LW(7))
        USER_FUNC(evt_npc_set_position, LW(5), LW(0), LW(1), LW(2))

        ADD(LW(14), 1)
    WHILE()

    // Pad out to length of original event section.
    0, 0, 0, 0, 0, 0, 0,

    // No return; patched over part of existing events.
EVT_PATCH_END()
static_assert(sizeof(WizzerdHandsIdle_Evt) == 0x1a0);

EVT_BEGIN(XNautPhdProjectilePosition_Evt)
    USER_FUNC(evtTot_IsMidbossFloor, LW(4))
    ADD(LW(4), 1)
    USER_FUNC(evt_npc_set_scale, PTR("slave_0"), LW(4), LW(4), LW(4))
    MUL(LW(4), 25)
    USER_FUNC(evt_npc_add_dirdist, LW(0), LW(2), LW(3), LW(4))
    ADD(LW(1), LW(4))
    USER_FUNC(evt_npc_set_position, PTR("slave_0"), LW(0), LW(1), LW(2))
    RETURN()
EVT_END()

EVT_BEGIN(XNautPhdProjectilePosition_Hook)
    RUN_CHILD_EVT(XNautPhdProjectilePosition_Evt)
    // Pad out to length of original event section.
    0, 0, 0, 0, 0, 0, 0,
    // No return; patched over part of existing event.
EVT_PATCH_END()
static_assert(sizeof(XNautPhdProjectilePosition_Hook) == 0x24);

EVT_BEGIN(ItemShop_GoodbyeEvt)
    USER_FUNC(evtTot_CheckCompletedAchievement,
        AchievementId::META_ITEMS_BADGES_ALL, EVT_NULLPTR, EVT_NULLPTR)

    USER_FUNC(evtTot_CheckCompletedAchievement,
        AchievementId::META_ITEMS_BADGES_5, LW(1), EVT_NULLPTR)

    // After meeting the 5 items + badges achievement, give selector items.
    IF_SMALL(LW(1), 1)
        GOTO(80)
    END_IF()
    // Skip cutscene if key items were already obtained.
    USER_FUNC(evt_pouch_check_item, (int32_t)ItemType::TOT_KEY_ITEM_SELECTOR, LW(1))
    USER_FUNC(evt_pouch_check_item, (int32_t)ItemType::TOT_KEY_BADGE_SELECTOR, LW(2))
    IF_EQUAL(LW(1), 1)
        IF_EQUAL(LW(2), 1)
            GOTO(80)
        END_IF()
    END_IF()

    USER_FUNC(evt_msg_print_add, 0, PTR("tot_shopkeep_30"))

    IF_EQUAL(LW(1), 0)
        USER_FUNC(evtTot_GetUniqueItemName, LW(0))
        USER_FUNC(
            evt_item_entry, LW(0), (int32_t)ItemType::TOT_KEY_ITEM_SELECTOR,
            FLOAT(0.0), FLOAT(-999.0), FLOAT(0.0), 17, -1, 0)
        USER_FUNC(evt_item_get_item, LW(0))
    END_IF()
    IF_EQUAL(LW(2), 0)
        USER_FUNC(evtTot_GetUniqueItemName, LW(0))
        USER_FUNC(
            evt_item_entry, LW(0), (int32_t)ItemType::TOT_KEY_BADGE_SELECTOR,
            FLOAT(0.0), FLOAT(-999.0), FLOAT(0.0), 17, -1, 0)
        USER_FUNC(evt_item_get_item, LW(0))
    END_IF()
    
    USER_FUNC(evt_msg_print, 0, PTR("tot_shopkeep_31"), 0, LW(9))
    GOTO(99)

LBL(80)
    USER_FUNC(evt_msg_print_add, 0, PTR("tot_shopkeep_11"))
    
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(ShopBuyEvt)
    USER_FUNC(evt_mario_normalize)
    SET(LW(10), LW(0))
    USER_FUNC(disp_off, LW(10))
    USER_FUNC(shopper_name, LW(9))
    USER_FUNC(evt_set_dir_to_target, LW(9), PTR("mario"))
    USER_FUNC(evt_win_coin_on, 0, LW(8))
    USER_FUNC(_evt_shop_get_value, LW(10), LW(11), LW(12), LW(13))

    SET(LW(14), PTR("tot_shopkeep_00"))
    USER_FUNC(evt_msg_fill_num, 0, LW(14), LW(14), LW(13))
    USER_FUNC(evt_msg_fill_item, 1, LW(14), LW(14), LW(11))
    USER_FUNC(evt_msg_print, 1, LW(14), 0, LW(9))
    USER_FUNC(evt_msg_select, 0, PTR("tot_shopkeep_yesno"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_shopkeep_22"))
        USER_FUNC(evt_win_coin_off, LW(8))
        RETURN()
    END_IF()
    USER_FUNC(evt_pouch_get_coin, LW(0))
    IF_SMALL(LW(0), LW(13))
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_shopkeep_01"))
        USER_FUNC(evt_win_coin_off, LW(8))
        RETURN()
    END_IF()
    USER_FUNC(set_buy_item_id, LW(12))
    USER_FUNC(evtTot_SpendPermanentCurrency, 0, LW(13))
    MUL(LW(13), -1)
    USER_FUNC(evt_pouch_add_coin, LW(13))
    USER_FUNC(evt_win_coin_wait, LW(8))
    WAIT_MSEC(200)
    USER_FUNC(evt_win_coin_off, LW(8))

    // Mark item as collected, remove from shelf if appropriate.
    USER_FUNC(evtTot_AfterBuyingShopItem, LW(10), -1)

    RUN_CHILD_EVT(ItemShop_GoodbyeEvt)

    RETURN()
EVT_END()

EVT_BEGIN(ShopBuyEvt_Hook)
    RUN_CHILD_EVT(ShopBuyEvt)
    RETURN()
EVT_END()

EVT_BEGIN(ShopSignEvt)
    USER_FUNC(evt_mario_normalize)
    USER_FUNC(evt_mario_key_onoff, 0)
    // Get name of shopkeeper npc.
    USER_FUNC(shopper_name, LW(9))

    // Check for tutorial dialogue.
    IF_EQUAL((int32_t)GSWF_HubShopTutorial, 0)
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
        SET((int32_t)GSWF_HubShopTutorial, 1)
        GOTO(99)
    END_IF()

    // Initialize the sign with all remaining items.
    USER_FUNC(evtTot_InitializeShopSign, LW(15))
    IF_SMALL(LW(15), 1)
        // Turn to face each other (only in this case, for Mario).
        USER_FUNC(evt_mario_set_dir_npc, LW(9))
        USER_FUNC(evt_set_dir_to_target, LW(9), PTR("mario"))
        // Dialogue for no items available on back order.
        USER_FUNC(evt_msg_print, 0, PTR("tot_shopkeep_nobackorder"), 0, LW(9))
        GOTO(99)
    END_IF()

    USER_FUNC(_evt_shop_get_ptr, LW(10))
    USER_FUNC(disp_off, LW(10))

    USER_FUNC(evt_win_coin_on, 0, LW(8))
    USER_FUNC(evt_win_other_select,
        (uint32_t)tot::window_select::MenuType::HUB_ITEM_SHOP)
    // Cancelled selection.
    IF_EQUAL(LW(0), 0)
        USER_FUNC(evt_win_coin_off, LW(8))
        GOTO(99)
    END_IF()

    USER_FUNC(evt_set_dir_to_target, LW(9), PTR("mario"))
    SET(LW(14), PTR("tot_shopkeep_00"))
    USER_FUNC(evt_msg_fill_num, 0, LW(14), LW(14), LW(3))
    USER_FUNC(evt_msg_fill_item, 1, LW(14), LW(14), LW(2))
    USER_FUNC(evt_msg_print, 1, LW(14), 0, LW(9))
    USER_FUNC(evt_msg_select, 0, PTR("tot_shopkeep_yesno"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_shopkeep_22"))
        USER_FUNC(evt_win_coin_off, LW(8))
        GOTO(99)
    END_IF()
    USER_FUNC(evt_pouch_get_coin, LW(0))
    IF_SMALL(LW(0), LW(3))
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_shopkeep_01"))
        USER_FUNC(evt_win_coin_off, LW(8))
        GOTO(99)
    END_IF()
    USER_FUNC(evtTot_SpendPermanentCurrency, 0, LW(3))
    MUL(LW(3), -1)
    USER_FUNC(evt_pouch_add_coin, LW(3))
    USER_FUNC(evt_win_coin_wait, LW(8))
    WAIT_MSEC(200)
    USER_FUNC(evt_win_coin_off, LW(8))

    // Mark item as collected.
    USER_FUNC(evtTot_AfterBuyingShopItem, LW(10), LW(1))

    RUN_CHILD_EVT(ItemShop_GoodbyeEvt)

LBL(99)
    USER_FUNC(evt_mario_key_onoff, 1)
    RETURN()
EVT_END()

EVT_BEGIN(ShopSignEvt_Hook)
    RUN_CHILD_EVT(ShopSignEvt)
    RETURN()
EVT_END()

void ApplyFixedPatches() {
    // Apply patches to make Mario's idle animation match Danger threshold.
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_mot_stay_MarioCheckDangerIdleAnim1_BH),
        reinterpret_cast<void*>(StartFieldDangerIdleCheck1),
        reinterpret_cast<void*>(BranchBackFieldDangerIdleCheck1));
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_mot_stay_MarioCheckDangerIdleAnim2_BH),
        reinterpret_cast<void*>(StartFieldDangerIdleCheck2),
        reinterpret_cast<void*>(BranchBackFieldDangerIdleCheck2));
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_mot_stay_MarioCheckDangerIdleAnim3_BH),
        reinterpret_cast<void*>(StartFieldDangerIdleCheck3),
        reinterpret_cast<void*>(BranchBackFieldDangerIdleCheck3));
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_mot_stay_MarioCheckDangerIdleAnim4_BH),
        reinterpret_cast<void*>(StartFieldDangerIdleCheck4),
        reinterpret_cast<void*>(BranchBackFieldDangerIdleCheck4));
    
    // Override the function that checks for party Danger animation on field.
    g_partyGetHp_trampoline = mod::hookFunction(
        ttyd::mario_party::partyGetHp, [](int32_t party_idx) {
            // Replace logic; this is only ever used in field animation context,
            // so we can just return what the game 'thinks' are Danger values.
            int32_t fake_hp = 10;
            if (party_idx > 0 && party_idx < 8) {
                const auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
                const int32_t max_hp = pouch.party_data[party_idx].max_hp;
                const int32_t danger_threshold = (max_hp * 3 + 5) / 10;
                const int32_t actual_hp = pouch.party_data[party_idx].current_hp;
                if (actual_hp <= 0) {
                    fake_hp = 0;
                } else if (actual_hp <= danger_threshold) {
                    fake_hp = 5;
                }
            }
            return fake_hp;
        });

    // Replaces logic for picking a FX to play on hammering in the field.
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_mot_hammer_PickHammerFieldSfx_BH),
        reinterpret_cast<void*>(g_mot_hammer_PickHammerFieldSfx_EH),
        reinterpret_cast<void*>(StartPlayFieldHammerFX),
        reinterpret_cast<void*>(BranchBackPlayFieldHammerFX));

    // Disable damage after falling into water.
    mod::writePatch(
        reinterpret_cast<void*>(g_mot_damage_Patch_DisableFallDamage),
        0x38600000U /* li r3, 0 */);

    // Add support for midboss scale to Wizzerd field AI.
    ttyd::npc_data::npc_ai_type_table[NpcAiType::WIZZERD].initEvtCode = 
        const_cast<int32_t*>(WizzerdInit_WrapperEvt);
    mod::writePatch(
        reinterpret_cast<void*>(
            reinterpret_cast<uintptr_t>(ttyd::npc_event::mahoon_move_event) + 0x28),
        WizzerdHandsIdle_Evt, sizeof(WizzerdHandsIdle_Evt));
    mod::writePatch(
        reinterpret_cast<void*>(
            reinterpret_cast<uintptr_t>(ttyd::npc_event::mahoon_find_event) + 0x14),
        WizzerdHandsIdle_Evt, sizeof(WizzerdHandsIdle_Evt));
    mod::writePatch(
        reinterpret_cast<void*>(
            reinterpret_cast<uintptr_t>(ttyd::npc_event::mahoon_lost_event) + 0x14),
        WizzerdHandsIdle_Evt, sizeof(WizzerdHandsIdle_Evt));
    mod::writePatch(
        reinterpret_cast<void*>(
            reinterpret_cast<uintptr_t>(ttyd::npc_event::mahoon_return_event) + 0x14),
        WizzerdHandsIdle_Evt, sizeof(WizzerdHandsIdle_Evt));

    // Add support for midboss scale to X-Naut PhD's projectiles.
    mod::writePatch(
        reinterpret_cast<void*>(g_zakowiz_find_event_Patch_ProjectileScaleHook),
        XNautPhdProjectilePosition_Hook,
        sizeof(XNautPhdProjectilePosition_Hook));

    // Add support for midboss scale to Hammer Bros., Dry Bones' projectiles.
    ttyd::npc_data::npc_ai_type_table[NpcAiType::DRY_BONES].initEvtCode = 
        const_cast<int32_t*>(DryBonesInit_WrapperEvt);
    ttyd::npc_data::npc_ai_type_table[NpcAiType::HAMMER_BRO].initEvtCode = 
        const_cast<int32_t*>(HammerBroInit_WrapperEvt);

    // Null out set scale events for Fuzzies' field AI, since they're unused.
    memset((void*)g_chorobon_move_event_Patch_SetScale, 0, 0x18);
    memset((void*)g_chorobon_find_event_Patch_SetScale, 0, 0x18);
    memset((void*)g_chorobon_lost_event_Patch_SetScale, 0, 0x18);
    memset((void*)g_chorobon_return_event_Patch_SetScale, 0, 0x18);

    // Correcting heights in NPC tribe description data.
    NpcTribeDescription* tribe_descs = ttyd::npc_data::npcTribe;
    // Shady Paratroopa
    tribe_descs[291].height = 30;
    // Fire Bro
    tribe_descs[293].height = 40;
    // Boomerang Bro
    tribe_descs[294].height = 40;
    // Craw
    tribe_descs[298].height = 40;
    // Atomic Boo
    tribe_descs[148].height = 100;
    
    // Copying tribe description data for Bob-omb, Atomic Boo over slots for
    // Bald + Hyper Bald Clefts, so they can be used for variants.
    memcpy(&tribe_descs[238], &tribe_descs[283], sizeof(NpcTribeDescription));
    memcpy(&tribe_descs[288], &tribe_descs[148], sizeof(NpcTribeDescription));
    // Set unique names + model filenames.
    tribe_descs[238].nameJp = "hyper_bomb";
    tribe_descs[238].modelName = "c_bomhey_h";
    tribe_descs[288].nameJp = "cosmic_boo";
    tribe_descs[288].modelName = "c_atmic_trs_p";

    // Replace shop buy_evt and evt_shoplist.
    mod::writePatch(
        reinterpret_cast<void*>(ttyd::evt_shop::buy_evt),
        ShopBuyEvt_Hook, sizeof(ShopBuyEvt_Hook));
    mod::writePatch(
        reinterpret_cast<void*>(ttyd::evt_shop::evt_shoplist),
        ShopSignEvt_Hook, sizeof(ShopSignEvt_Hook));
    
    // Disable special shopkeeper talk event, since shop only allows buying.
    mod::writePatch(
        reinterpret_cast<void*>(g_evt_shop_setup_Patch_DisableShopperTalkEvt),
        0x60000000 /* nop */);
}

int16_t* GetShopBackOrderItems() {
    return g_ShopSignItems;
}

}  // namespace field
}  // namespace mod::tot::patch