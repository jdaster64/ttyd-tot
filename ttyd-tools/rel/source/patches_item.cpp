#include "patches_item.h"

#include "evt_cmd.h"
#include "mod.h"
#include "mod_achievements.h"
#include "mod_state.h"
#include "patch.h"
#include "tot_generate_reward.h"
#include "tot_state.h"

#include <ttyd/battle.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_event_default.h>
#include <ttyd/battle_item_data.h>
#include <ttyd/battle_mario.h>
#include <ttyd/battle_sub.h>
#include <ttyd/battle_unit.h>
#include <ttyd/battle_weapon_power.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/icondrv.h>
#include <ttyd/item_data.h>
#include <ttyd/itemdrv.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/swdrv.h>
#include <ttyd/system.h>

#include <cstdint>
#include <cstring>

namespace mod::infinite_pit {

namespace {

using ::ttyd::battle_database_common::BattleWeapon;
using ::ttyd::battle_unit::BattleWorkUnit;
using ::ttyd::evtmgr::EvtEntry;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::item_data::itemDataTable;
using ::ttyd::item_data::ItemData;

namespace IconType = ::ttyd::icondrv::IconType;
namespace ItemType = ::ttyd::item_data::ItemType;
namespace ItemUseLocation = ::ttyd::item_data::ItemUseLocation_Flags;

bool g_FreezeFieldItems = false;

}

// Function hooks.
extern void* (*g_itemEntry_trampoline)(
    const char*, int32_t, float, float, float, uint32_t, int32_t, void*);
extern uint32_t (*g_pouchGetItem_trampoline)(int32_t);
extern int32_t (*g_btlevtcmd_GetItemRecoverParam_trampoline)(EvtEntry*, bool);
extern int32_t (*g__get_flower_suitoru_point_trampoline)(EvtEntry*, bool);
extern int32_t (*g__get_heart_suitoru_point_trampoline)(EvtEntry*, bool);
extern int32_t (*g_BattleItemData_rank_up_trampoline)(EvtEntry*, bool);
// Patch addresses.
extern const int32_t g_itemMain_CheckItemFreeze_BH;
extern const int32_t g_pouchRemoveItemIndex_CheckMaxInv_BH;
extern const int32_t g_pouchRemoveItem_CheckMaxInv_BH;
extern const int32_t g_pouchGetItem_CheckMaxInv_BH;
extern const int32_t g_pouchGetEmptyHaveItemCnt_CheckMaxInv_BH;
extern const int32_t g_ItemEvent_LastDinner_Weapon;
extern const int32_t g_ItemEvent_Teki_Kyouka_ApplyStatusHook;
extern const int32_t g_ItemEvent_Support_NoEffect_TradeOffJumpPoint;
extern const int32_t g_ItemEvent_Poison_Kinoko_PoisonChance;
extern const int32_t g_fbatBattleMode_Patch_BumpAttackLevel;
extern const int32_t g_btlseqTurn_HappyHeartProc_BH;
extern const int32_t g_btlseqTurn_HappyHeartProc_EH;
extern const int32_t g_btlseqTurn_HappyFlowerProc_BH;
extern const int32_t g_btlseqTurn_HappyFlowerProc_EH;
extern const int32_t g_btlevtcmd_ConsumeItem_Patch_RefundPer;
extern const int32_t g_btlevtcmd_ConsumeItemReserve_Patch_RefundPer;
extern const int32_t g_btlevtcmd_ConsumeItem_Patch_RefundBase;
extern const int32_t g_btlevtcmd_ConsumeItemReserve_Patch_RefundBase;
extern const int32_t g_BattleDamageDirect_Patch_AddTotalDamage;
extern const int32_t g_BattleDamageDirect_Patch_PityFlowerChance;
extern const int32_t g_BattleAudience_Case_Appeal_Patch_AppealSp;

// Assembly patch functions.
extern "C" {
    // field_item_patches.s
    void StartCheckItemFreeze();
    void BranchBackCheckItemFreeze();
    // happy_badge_patches.s
    void StartHappyHeartProc();
    void BranchBackHappyHeartProc();
    void StartHappyFlowerProc();
    void BranchBackHappyFlowerProc();
    // item_inventory_patches.s
    void StartGetItemMax();
    void BranchBackGetItemMax();
    void StartRemoveItemMax();
    void BranchBackRemoveItemMax();
    void StartRemoveItemIndexMax();
    void BranchBackRemoveItemIndexMax();
    void StartGetEmptyItemSlotsMax();
    void BranchBackGetEmptyItemSlotsMax();

    bool checkItemFreeze() {
        return g_FreezeFieldItems;
    }
    
    int32_t getTotItemInventorySize() {
        return mod::infinite_pit::item::GetItemInventorySize();
    }
}

namespace item {
    
namespace {

// Forward declarations.
EVT_DECLARE_USER_FUNC(ToggleCookingItemTypeToHeldItem, 1)

// Custom attack event for CookingItems that temporarily sets CookingItem's type
// to the attacker's held item, so it can be used for target weighting.
EVT_BEGIN(CookingItemAttackEvent)
USER_FUNC(ToggleCookingItemTypeToHeldItem, 1)
RUN_CHILD_EVT(PTR(ttyd::battle_item_data::ItemEvent_Recovery))
USER_FUNC(ToggleCookingItemTypeToHeldItem, 0)
RETURN()
EVT_END()

// Patch over the end of the existing Trade Off item script so it actually
// calls the part of the code associated with applying its status.
EVT_BEGIN(TradeOffPatch)
SET(LW(12), PTR(&ttyd::battle_item_data::ItemWeaponData_Teki_Kyouka))
USER_FUNC(ttyd::battle_event_cmd::btlevtcmd_WeaponAftereffect, LW(12))
// Run the end of ItemEvent_Support_NoEffect's evt.
RUN_CHILD_EVT(static_cast<int32_t>(g_ItemEvent_Support_NoEffect_TradeOffJumpPoint))
RETURN()
EVT_END()

// Toggles on/off setting CookingItem's item type to the attacker's held item.
// This allows, e.g., target weighting for pure-FP items to be properly used.
EVT_DEFINE_USER_FUNC(ToggleCookingItemTypeToHeldItem) {
    BattleWeapon& weapon = ttyd::battle_item_data::ItemWeaponData_CookingItem;
    if (evtGetValue(evt, evt->evtArguments[0])) {
        // Get the held item from the current attacker.
        if (evt->wActorThisPtr) {
            BattleWorkUnit* unit = ttyd::battle::BattleGetUnitPtr(
                ttyd::battle::g_BattleWork,
                reinterpret_cast<uint32_t>(evt->wActorThisPtr));
            if (unit && unit->held_item > 0) {
                weapon.item_id = unit->held_item;
            }
        }
    } else {
        weapon.item_id = 0;
    }
    return 2;
}

// Returns altered item restoration parameters.
EVT_DEFINE_USER_FUNC(GetAlteredItemRestorationParams) {
    int32_t item_id = evtGetValue(evt, evt->evtArguments[0]);
    if (item_id == ItemType::CAKE) {
        int32_t hp = 
            evtGetValue(evt, evt->evtArguments[1]) + GetBonusCakeRestoration();
        int32_t fp =
            evtGetValue(evt, evt->evtArguments[2]) + GetBonusCakeRestoration();
        evtSetValue(evt, evt->evtArguments[1], hp);
        evtSetValue(evt, evt->evtArguments[2], fp);
    }
    return 2;
}

// Replaces the vanilla logic for HP or FP Drain restoration.
int32_t GetDrainRestoration(EvtEntry* evt, bool hp_drain) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, id);
    
    bool use_64_style_drain = false;
    int32_t drain = 0;
    if (unit) {
        int32_t num_badges = 0;
        if (hp_drain) {
            num_badges = unit->badges_equipped.hp_drain;
        } else {
            num_badges = unit->badges_equipped.fp_drain;
        }
        if (use_64_style_drain) {
            // 1 point per damaging hit x num badges, max of 5.
            drain = unit->total_damage_dealt_this_attack * num_badges;
            if (drain > 5) drain = 5;
        } else {
            // 1 per badge if any damaging hits were dealt.
            drain = !!unit->total_damage_dealt_this_attack * num_badges;
        }
    }
    evtSetValue(evt, evt->evtArguments[1], drain);
    return 2;
}

}
    
void ApplyFixedPatches() {
    // Rebalanced price tiers for items & badges (non-pool items may have 0s).
    static const constexpr uint32_t kPriceTiers[] = {
        // Items / recipes.
        0x1a444662, 0x5b334333, 0xb7321243, 0x34473205, 0x00800666,
        0x00700000, 0x30753250, 0xa8785474, 0x3407a753, 0x00852420,
        0x35703743, 0x30060740, 0x84544045, 0x00002046,
        // Badges.
        0xb8a88dbb, 0xdd9d8a8b, 0xeeddcccc, 0xcceeffff, 0xbbccccdd,
        0xbbbebeeb, 0xaaaacdfc, 0xcaacccca, 0xd00edd7c, 0x0000000d,
        0x0a770007, 0xbdee0000, 0x00000bbb,
    };
    
    // Prices corresponding to the price tiers in the above array.
    static const constexpr uint8_t kPrices[] = {
        5, 10, 15, 20, 25, 30, 40, 50, 60, 70, 80, 100, 125, 150, 200, 250
    };
    static const constexpr uint32_t kBpCost[] = {
        0x11111111, 0x44111111, 0x22662211, 0x22111144, 0x11224411,
        0x33331441, 0x12226220, 0x62211111, 0x40042216, 0x00000004,
        0x03110001, 0x24220100, 0x00000112,
    };
    static const constexpr int8_t kBadgeSortOrder[] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 33, 34,
        29, 30, 27, 28,  1,  2,  9, 10,  3,  4, 20, 21, 18, 19, 11, 12,
        22, 23, 13, 14, 15, 16, -1, -1, -1, 43, 44, 57, 58, 61, 62, 59,
        17,  7,  8, 60, 40, 41, 42, 50, 51, 52, 53, 24, 25, 35, 36, 37,
        38, 56, 45, 46, 26, 98, 99, 54, 55, -1, -1, 80, 81, 82, 83, 84,
        63, -1, -1, -1, -1, -1, 39, 85, 86, -1, -1, -1,  5,  6, 49, 47,
        48, 31, 32,
    };
    
    // - Set coin buy & sell (for Refund) prices based on above tiers.
    // - Set healing items' weapons to CookingItem if they don't have one.
    // - Fix unused items' and badges' sort order.
    for (int32_t i = ItemType::GOLD_BAR; i < ItemType::MAX_ITEM_TYPE; ++i) {
        ItemData& item = itemDataTable[i];
        
        // Assign new buy / sell prices.
        if (i >= ItemType::THUNDER_BOLT) {
            const int32_t word_index = (i - ItemType::THUNDER_BOLT) >> 3;
            const int32_t nybble_index = (i - ItemType::THUNDER_BOLT) & 7;
            const int32_t tier =
                (kPriceTiers[word_index] >> (nybble_index << 2)) & 15;
            item.buy_price = kPrices[tier];
            if (i >= ItemType::POWER_JUMP) {
                item.sell_price = kPrices[tier] / 10;
            } else {
                item.sell_price = kPrices[tier] / 5;
            }
            if (item.sell_price < 1) item.sell_price = 1;
        }
        
        if (i < ItemType::POWER_JUMP) {
            // For all items that restore HP or FP, assign the "cooked item"
            // weapon struct if they don't already have a weapon assigned.
            if (!item.weapon_params && (item.hp_restored || item.fp_restored)) {
                item.weapon_params =
                    &ttyd::battle_item_data::ItemWeaponData_CookingItem;
            } else if (item.weapon_params && item.hp_restored) {
                // For HP restoration items with weapon structs, give them
                // Mushroom-like target weighting (heal the least healthy).
                item.weapon_params->target_weighting_flags =
                    ttyd::battle_item_data::ItemWeaponData_Kinoko.
                        target_weighting_flags;
            }

            // Fix sorting order.
            if (item.type_sort_order > 0x31) {
                item.type_sort_order += 1;
            }
        } else {
            // Assign new sort order and BP cost.
            const int32_t word_index = (i - ItemType::POWER_JUMP) >> 3;
            const int32_t nybble_index = (i - ItemType::POWER_JUMP) & 7;
            item.bp_cost = (kBpCost[word_index] >> (nybble_index << 2)) & 15;
            item.type_sort_order = kBadgeSortOrder[i - ItemType::POWER_JUMP];
        }
    }
    
    // Set Star Piece buy price (you can buy only 1 per shop).
    itemDataTable[ItemType::STAR_PIECE].buy_price = 50;
    
    // Changed pickup messages for Super / Ultra boots and hammer.
    itemDataTable[ItemType::SUPER_BOOTS].description = "msg_custom_super_boots";
    itemDataTable[ItemType::ULTRA_BOOTS].description = "msg_custom_ultra_boots";
    itemDataTable[ItemType::SUPER_HAMMER].description = "msg_custom_super_hammer";
    itemDataTable[ItemType::ULTRA_HAMMER].description = "msg_custom_ultra_hammer";
    
    // Change item name / description lookup keys for achievement rewards.
    itemDataTable[AchievementsManager::kChestRewardItem].name = "in_ach_1";
    itemDataTable[AchievementsManager::kChestRewardItem].description = "msg_ach_1";
    itemDataTable[AchievementsManager::kChestRewardItem].menu_description = "msg_ach_1";
    itemDataTable[AchievementsManager::kBadgeLogItem].name = "in_ach_2";
    itemDataTable[AchievementsManager::kBadgeLogItem].description = "msg_ach_2";
    itemDataTable[AchievementsManager::kBadgeLogItem].menu_description = "msg_ach_2";
    itemDataTable[AchievementsManager::kTattleLogItem].name = "in_ach_3";
    itemDataTable[AchievementsManager::kTattleLogItem].description = "msg_ach_3";
    itemDataTable[AchievementsManager::kTattleLogItem].menu_description = "msg_ach_3";
    
    // New badges:
    // Toughen Up (P): a move that grants a single-turn +DEF buff.
    itemDataTable[ItemType::TOT_TOUGHEN_UP].icon_id = IconType::DEFEND_BADGE;
    itemDataTable[ItemType::TOT_TOUGHEN_UP].name = "in_toughen_up";
    itemDataTable[ItemType::TOT_TOUGHEN_UP].description = "msg_toughen_up";
    itemDataTable[ItemType::TOT_TOUGHEN_UP].menu_description = "msg_toughen_up_menu";
    itemDataTable[ItemType::TOT_TOUGHEN_UP_P].icon_id = IconType::DEFEND_BADGE_P;
    itemDataTable[ItemType::TOT_TOUGHEN_UP_P].name = "in_toughen_up_p";
    itemDataTable[ItemType::TOT_TOUGHEN_UP_P].description = "msg_toughen_up_p";
    itemDataTable[ItemType::TOT_TOUGHEN_UP_P].menu_description = "msg_toughen_up_p_menu";
    // Perfect Power (P): gives +1 power for being at full health.
    itemDataTable[ItemType::TOT_PERFECT_POWER].icon_id = IconType::PERFECT_POWER;
    itemDataTable[ItemType::TOT_PERFECT_POWER].name = "in_perfect_power";
    itemDataTable[ItemType::TOT_PERFECT_POWER].description = "msg_perfect_power";
    itemDataTable[ItemType::TOT_PERFECT_POWER].menu_description = "msg_perfect_power";
    itemDataTable[ItemType::TOT_PERFECT_POWER_P].icon_id = IconType::PERFECT_POWER_P;
    itemDataTable[ItemType::TOT_PERFECT_POWER_P].name = "in_perfect_power_p";
    itemDataTable[ItemType::TOT_PERFECT_POWER_P].description = "msg_perfect_power_p";
    itemDataTable[ItemType::TOT_PERFECT_POWER_P].menu_description = "msg_perfect_power_p";
    // Pity Star (P): gives +50% extra Star Power from taking hits from enemies.
    itemDataTable[ItemType::TOT_PITY_STAR].icon_id = IconType::PITY_STAR;
    itemDataTable[ItemType::TOT_PITY_STAR].name = "in_pity_star";
    itemDataTable[ItemType::TOT_PITY_STAR].description = "msg_pity_star";
    itemDataTable[ItemType::TOT_PITY_STAR].menu_description = "msg_pity_star";
    itemDataTable[ItemType::TOT_PITY_STAR_P].icon_id = IconType::PITY_STAR_P;
    itemDataTable[ItemType::TOT_PITY_STAR_P].name = "in_pity_star_p";
    itemDataTable[ItemType::TOT_PITY_STAR_P].description = "msg_pity_star_p";
    itemDataTable[ItemType::TOT_PITY_STAR_P].menu_description = "msg_pity_star_p";
    // Super Start: gives +0.50 SP at the start of a battle.
    itemDataTable[ItemType::TOT_SUPER_START].icon_id = IconType::SUPER_START;
    itemDataTable[ItemType::TOT_SUPER_START].name = "in_super_start";
    itemDataTable[ItemType::TOT_SUPER_START].description = "msg_super_start";
    itemDataTable[ItemType::TOT_SUPER_START].menu_description = "msg_super_start";
        
    // Change Super Charge (P) weapons into Toughen Up (P).
    ttyd::battle_mario::badgeWeapon_SuperCharge.base_fp_cost = 1;
    ttyd::battle_mario::badgeWeapon_SuperCharge.charge_strength = 0;
    ttyd::battle_mario::badgeWeapon_SuperCharge.def_change_chance = 100;
    ttyd::battle_mario::badgeWeapon_SuperCharge.def_change_time = 1;
    ttyd::battle_mario::badgeWeapon_SuperCharge.def_change_strength = 2;
    ttyd::battle_mario::badgeWeapon_SuperCharge.icon = IconType::DEFEND_BADGE;
    ttyd::battle_mario::badgeWeapon_SuperCharge.name = "in_toughen_up";
    
    ttyd::battle_mario::badgeWeapon_SuperChargeP.base_fp_cost = 1;
    ttyd::battle_mario::badgeWeapon_SuperChargeP.charge_strength = 0;
    ttyd::battle_mario::badgeWeapon_SuperChargeP.def_change_chance = 100;
    ttyd::battle_mario::badgeWeapon_SuperChargeP.def_change_time = 1;
    ttyd::battle_mario::badgeWeapon_SuperChargeP.def_change_strength = 2;
    ttyd::battle_mario::badgeWeapon_SuperChargeP.icon = IconType::DEFEND_BADGE_P;
    ttyd::battle_mario::badgeWeapon_SuperChargeP.name = "in_toughen_up";
    
    // Turn Gold Bars x3 into "Shine Sprites" that can be used from the menu.
    // TODO: Remove this code as it's not necessary in ToT.
    memcpy(&itemDataTable[ItemType::GOLD_BAR_X3], 
           &itemDataTable[ItemType::SHINE_SPRITE], sizeof(ItemData));
    itemDataTable[ItemType::GOLD_BAR_X3].usable_locations 
        |= ItemUseLocation::kField;
    // Set Shine Sprite sell price.
    itemDataTable[ItemType::GOLD_BAR_X3].sell_price = 25;
    
    // Base HP and FP restored by Strawberry Cake; extra logic is run
    // in the menu / in battle to make it restore random extra HP / FP.
    itemDataTable[ItemType::CAKE].hp_restored = 5;
    itemDataTable[ItemType::CAKE].fp_restored = 5;
    
    // Reinstate Fire Pop's fire damage (base it off of Electro Pop's params).
    static BattleWeapon kFirePopParams;
    memcpy(&kFirePopParams,
           &ttyd::battle_item_data::ItemWeaponData_BiribiriCandy,
           sizeof(BattleWeapon));
    kFirePopParams.item_id = ItemType::FIRE_POP;
    kFirePopParams.damage_function =
        reinterpret_cast<void*>(
            &ttyd::battle_weapon_power::weaponGetPowerDefault);
    kFirePopParams.damage_function_params[0] = 1;
    kFirePopParams.element = 1;  // fire (naturally)
    kFirePopParams.special_property_flags = 0x00030048;  // pierce defense
    kFirePopParams.electric_chance = 0;
    kFirePopParams.electric_time = 0;
    itemDataTable[ItemType::FIRE_POP].weapon_params = &kFirePopParams;
    
    // Make enemies prefer to use CookingItems like standard healing items.
    // (i.e. they use them on characters with less HP)
    ttyd::battle_item_data::ItemWeaponData_CookingItem.target_weighting_flags =
        ttyd::battle_item_data::ItemWeaponData_Kinoko.target_weighting_flags;
    // Change CookingItem's attack event to CookingItemAttack (wrapper to
    // ItemEvent_Recovery that passes in the item type being used for enemies).
    ttyd::battle_item_data::ItemWeaponData_CookingItem.attack_evt_code =
        const_cast<int32_t*>(CookingItemAttackEvent);
        
    // Make Point Swap and Trial Stew only target Mario or his partner.
    ttyd::battle_item_data::ItemWeaponData_Irekaeeru.target_class_flags = 
        0x01100070;
    ttyd::battle_item_data::ItemWeaponData_LastDinner.target_class_flags = 
        0x01100070;
        
    // Make Trial Stew's event use the correct weapon params.
    BattleWeapon* kLastDinnerWeaponAddr = 
        &ttyd::battle_item_data::ItemWeaponData_LastDinner;
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_ItemEvent_LastDinner_Weapon),
        &kLastDinnerWeaponAddr, sizeof(BattleWeapon*));

    // Make Poison Mushrooms able to target anyone, and make enemies prefer
    // to target Mario's team or characters that are in Peril.
    ttyd::battle_item_data::ItemWeaponData_PoisonKinoko.target_class_flags = 
        0x01100060;
    ttyd::battle_item_data::ItemWeaponData_PoisonKinoko.target_weighting_flags =
        0x80001023;
    // Make Poison Mushrooms poison & halve HP 67% of the time instead of 80%.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_ItemEvent_Poison_Kinoko_PoisonChance), 67);
    
    // Make Slow Shroom and Gradual Syrup stronger, but only last 3 turns.
    ttyd::battle_item_data::ItemWeaponData_Jiwajiwa_Kinoko.hp_regen_time = 3;
    ttyd::battle_item_data::ItemWeaponData_Jiwajiwa_Kinoko.hp_regen_strength = 5;
    ttyd::battle_item_data::ItemWeaponData_Jiwajiwa_Syrup.fp_regen_time = 3;
    ttyd::battle_item_data::ItemWeaponData_Jiwajiwa_Syrup.fp_regen_strength = 5;
        
    // Make Space Food guarantee Allergic status.
    ttyd::battle_item_data::ItemWeaponData_SpaceFood.allergic_chance = 100;
        
    // Make Trade Off usable only on the enemy party.
    ttyd::battle_item_data::ItemWeaponData_Teki_Kyouka.target_class_flags =
        0x02100063;
    // Make it inflict +ATK for 9 turns.
    ttyd::battle_item_data::ItemWeaponData_Teki_Kyouka.atk_change_chance = 100;
    ttyd::battle_item_data::ItemWeaponData_Teki_Kyouka.atk_change_time     = 9;
    ttyd::battle_item_data::ItemWeaponData_Teki_Kyouka.atk_change_strength = 3;
    // Patch in evt code to actually apply the item's newly granted status.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_ItemEvent_Teki_Kyouka_ApplyStatusHook),
        TradeOffPatch, sizeof(TradeOffPatch));
    // Change "rank up" code to increase coins instead of level.
    g_BattleItemData_rank_up_trampoline = patch::hookFunction(
        ttyd::battle_item_data::BattleItemData_rank_up,
        [](EvtEntry* evt, bool isFirstCall) {
            //  Replace original logic.
            int32_t unit_id = evtGetValue(evt, evt->evtArguments[0]);
            unit_id = ttyd::battle_sub::BattleTransID(evt, unit_id);
            auto* unit = ttyd::battle::BattleGetUnitPtr(
                ttyd::battle::g_BattleWork, unit_id);
            // Use padding byte as storage for extra coins.
            unit->pad_00f += 10;
            if (unit->pad_00f > 50) unit->pad_00f = 50;
            return 2;
        });
        
    // Make Koopa Curse multi-target.
    ttyd::battle_item_data::ItemWeaponData_Kameno_Noroi.target_class_flags =
        0x02101260;
    // Give it its correct icon and non-default sort order.
    itemDataTable[ItemType::KOOPA_CURSE].icon_id = IconType::KOOPA_CURSE;
    itemDataTable[ItemType::KOOPA_CURSE].type_sort_order = 0x31 + 1;
        
    // Make Hot Sauce charge by +3.
    ttyd::battle_item_data::ItemWeaponData_RedKararing.charge_strength = 3;
        
    // Change base FP cost of Charge badges.
    ttyd::battle_mario::badgeWeapon_Charge.base_fp_cost = 2;
    ttyd::battle_mario::badgeWeapon_ChargeP.base_fp_cost = 2;
    
    // Patch data for TOT reward items.
    tot::RewardManager::PatchRewardItemData();
    
    // Change icon for blue coins.
    itemDataTable[ItemType::PIANTA].icon_id = IconType::TOT_COIN_BLUE;
    
    // For testing purposes, Bump Attack ignores level check.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_fbatBattleMode_Patch_BumpAttackLevel),
        0x60000000U /* nop */);
        
    // Super Appeal (P) give +0.50 SP per copy instead of +0.25.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_BattleAudience_Case_Appeal_Patch_AppealSp),
        0x1c000032U /* mulli r0, r0, 50 */);
        
    // Happy badges are guaranteed to proc on even turns only.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_btlseqTurn_HappyHeartProc_BH),
        reinterpret_cast<void*>(g_btlseqTurn_HappyHeartProc_EH),
        reinterpret_cast<void*>(StartHappyHeartProc),
        reinterpret_cast<void*>(BranchBackHappyHeartProc));
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_btlseqTurn_HappyFlowerProc_BH),
        reinterpret_cast<void*>(g_btlseqTurn_HappyFlowerProc_EH),
        reinterpret_cast<void*>(StartHappyFlowerProc),
        reinterpret_cast<void*>(BranchBackHappyFlowerProc));
        
    // Pity Flower (P) guarantees 1 FP recovery on each damaging hit.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_BattleDamageDirect_Patch_PityFlowerChance),
        0x2c030064U /* cmpwi r3, 100 */);
        
    // Refund grants 100% of sell price, plus 20% per additional badge.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_btlevtcmd_ConsumeItem_Patch_RefundPer),
        0x1ca00014U /* mulli r5, r0, 20 */);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_btlevtcmd_ConsumeItemReserve_Patch_RefundPer),
        0x1ca00014U /* mulli r5, r0, 20 */);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_btlevtcmd_ConsumeItem_Patch_RefundBase),
        0x38a50050U /* addi r5, r5, 80 */);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_btlevtcmd_ConsumeItemReserve_Patch_RefundBase),
        0x38a50050U /* addi r5, r5, 80 */);
    
    // Replace HP/FP Drain logic; counts the number of intended damaging hits
    // and restores 1 HP per badge if there were any (or 1 per hit, to a max
    // of 5, if the PM64-style option is enabled).
    g__get_heart_suitoru_point_trampoline = patch::hookFunction(
        ttyd::battle_event_default::_get_heart_suitoru_point,
        [](EvtEntry* evt, bool isFirstCall) {
            return GetDrainRestoration(evt, /* hp_drain = */ true);
        });
    g__get_flower_suitoru_point_trampoline = patch::hookFunction(
        ttyd::battle_event_default::_get_flower_suitoru_point,
        [](EvtEntry* evt, bool isFirstCall) {
            return GetDrainRestoration(evt, /* hp_drain = */ false);
        });
    // Disable the instruction that normally adds to damage dealt, since that
    // field is now used as a boolean for "has attacked with a damaging move".
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_BattleDamageDirect_Patch_AddTotalDamage),
        0x60000000U /* nop */);
            
    g_btlevtcmd_GetItemRecoverParam_trampoline = patch::hookFunction(
        ttyd::battle_event_cmd::btlevtcmd_GetItemRecoverParam,
        [](EvtEntry* evt, bool isFirstCall) {
            g_btlevtcmd_GetItemRecoverParam_trampoline(evt, isFirstCall);
            // Run custom behavior to replace the recovery params in some cases.
            return GetAlteredItemRestorationParams(evt, isFirstCall);
        });
        
    // Set max inventory size based on number of sack upgrades claimed in ToT.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_pouchGetItem_CheckMaxInv_BH),
        reinterpret_cast<void*>(StartGetItemMax),
        reinterpret_cast<void*>(BranchBackGetItemMax));
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_pouchRemoveItem_CheckMaxInv_BH),
        reinterpret_cast<void*>(StartRemoveItemMax),
        reinterpret_cast<void*>(BranchBackRemoveItemMax));
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_pouchRemoveItemIndex_CheckMaxInv_BH),
        reinterpret_cast<void*>(StartRemoveItemIndexMax),
        reinterpret_cast<void*>(BranchBackRemoveItemIndexMax));
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_pouchGetEmptyHaveItemCnt_CheckMaxInv_BH),
        reinterpret_cast<void*>(StartGetEmptyItemSlotsMax),
        reinterpret_cast<void*>(BranchBackGetEmptyItemSlotsMax));
        
    // Override item pickup script for special items that spawn as item drops.
    g_itemEntry_trampoline = mod::patch::hookFunction(
        ttyd::itemdrv::itemEntry,
        [](const char* name, int32_t item, float x, float y, float z,
           uint32_t mode, int32_t collection_gswf, void* pickup_script) {
            if (item == ItemType::STAR_PIECE && !pickup_script) {
                pickup_script = tot::RewardManager::GetStarPieceItemDropEvt();
            }
            if (item == ItemType::SHINE_SPRITE && !pickup_script) {
                pickup_script = tot::RewardManager::GetShineSpriteItemDropEvt();
            }
            return g_itemEntry_trampoline(
                name, item, x, y, z, mode, collection_gswf, pickup_script);
        });
    
    // Override item-get logic for special items.
    g_pouchGetItem_trampoline = mod::patch::hookFunction(
        ttyd::mario_pouch::pouchGetItem, [](int32_t item_type) {
            // Track coins, Star Pieces, and Shine Sprites gained.
            if (item_type == ItemType::COIN) {
                g_Mod->state_.ChangeOption(tot::STAT_RUN_COINS_EARNED);
            }
            if (item_type == ItemType::STAR_PIECE) {
                g_Mod->state_.ChangeOption(tot::STAT_RUN_STAR_PIECES);
            }
            if (item_type == ItemType::SHINE_SPRITE) {
                g_Mod->state_.ChangeOption(tot::STAT_RUN_SHINE_SPRITES);
            }
            
            // Handle items with special effects in ToT.
            if (tot::RewardManager::HandleRewardItemPickup(item_type)) 
                return 1U;
            
            uint32_t return_value = g_pouchGetItem_trampoline(item_type);

            // Mark unique items as being collected.
            if (return_value) {
                tot::RewardManager::MarkUniqueItemCollected(item_type);
            }
            
            return return_value;
        });

    // Add additional check for items not despawning while a Star Piece
    // or Shine Sprite is being collected on the field.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_itemMain_CheckItemFreeze_BH),
        reinterpret_cast<void*>(StartCheckItemFreeze),
        reinterpret_cast<void*>(BranchBackCheckItemFreeze));
}

int32_t GetBonusCakeRestoration() {
    // Returns one of 0, 5, 10, ..., 25 at random (to be added to the base 5).
    return ttyd::system::irand(6) * 5;
}

int32_t GetItemInventorySize() {
    // Returns the current inventory size based on Strange Sack upgrades.
    int32_t items = g_Mod->state_.num_sack_upgrades_ * 2 + 6;
    return items < 20 ? items : 20;
}

EVT_DEFINE_USER_FUNC(evtTot_FreezeFieldItemTimers) {
    g_FreezeFieldItems = evtGetValue(evt, evt->evtArguments[0]);
    return 2;
}

}  // namespace item
}  // namespace mod::infinite_pit