#include "patches_item.h"

#include "evt_cmd.h"
#include "mod.h"
#include "mod_achievements.h"
#include "mod_state.h"
#include "patch.h"

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
#include <ttyd/item_data.h>
#include <ttyd/system.h>

#include <cstdint>
#include <cstring>

// Assembly patch functions.
extern "C" {
    // evasion_badge_patches.s
    void StartCheckBadgeEvasion();
    void ConditionalBranchCheckBadgeEvasion();
    void BranchBackCheckBadgeEvasion();
    // rush_badge_patches.s
    void StartGetDangerStrength();
    void BranchBackGetDangerStrength();
    void StartGetPerilStrength();
    void BranchBackGetPerilStrength();
    
    int32_t getDangerStrength(int32_t num_badges) {
        bool weaker_rush_badges =
            mod::infinite_pit::g_Mod->state_.GetOptionNumericValue(
                mod::infinite_pit::OPT_WEAKER_RUSH_BADGES);
        return num_badges * (weaker_rush_badges ? 1 : 2);
    }
    int32_t getPerilStrength(int32_t num_badges) {
        bool weaker_rush_badges =
            mod::infinite_pit::g_Mod->state_.GetOptionNumericValue(
                mod::infinite_pit::OPT_WEAKER_RUSH_BADGES);
        return num_badges * (weaker_rush_badges ? 2 : 5);
    }
    bool checkBadgeEvasion(ttyd::battle_unit::BattleWorkUnit* unit) {
        return mod::infinite_pit::item::CheckEvasionBadges(unit);
    }
}

namespace mod::infinite_pit {

namespace {

using ::ttyd::battle_database_common::BattleWeapon;
using ::ttyd::battle_unit::BattleWorkUnit;
using ::ttyd::evtmgr::EvtEntry;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::item_data::itemDataTable;
using ::ttyd::item_data::ItemData;

namespace ItemType = ::ttyd::item_data::ItemType;
namespace ItemUseLocation = ::ttyd::item_data::ItemUseLocation_Flags;

}

// Function hooks.
extern int32_t (*g__get_flower_suitoru_point_trampoline)(EvtEntry*, bool);
extern int32_t (*g__get_heart_suitoru_point_trampoline)(EvtEntry*, bool);
extern int32_t (*g_btlevtcmd_GetItemRecoverParam_trampoline)(EvtEntry*, bool);
// Patch addresses.
extern const int32_t g_ItemEvent_LastDinner_Weapon;
extern const int32_t g_ItemEvent_Teki_Kyouka_ApplyStatusHook;
extern const int32_t g_ItemEvent_Support_NoEffect_TradeOffJumpPoint;
extern const int32_t g_ItemEvent_Poison_Kinoko_PoisonChance;
extern const int32_t g__getSickStatusParam_Patch_CheckChargeCap;
extern const int32_t g__getSickStatusParam_Patch_SetChargeCap;
extern const int32_t g_fbatBattleMode_Patch_DoubleCoinsBadge1;
extern const int32_t g_fbatBattleMode_Patch_DoubleCoinsBadge2;
extern const int32_t g_btlseqTurn_Patch_HappyFlowerReductionAtMax;
extern const int32_t g_btlseqTurn_Patch_HappyFlowerBaseRate;
extern const int32_t g_btlseqTurn_Patch_HappyHeartReductionAtMax;
extern const int32_t g_btlseqTurn_Patch_HappyHeartBaseRate;
extern const int32_t g_BattleDamageDirect_Patch_PityFlowerChance;
extern const int32_t g_btlevtcmd_ConsumeItem_Patch_RefundPer;
extern const int32_t g_btlevtcmd_ConsumeItemReserve_Patch_RefundPer;
extern const int32_t g_btlevtcmd_ConsumeItem_Patch_RefundBase;
extern const int32_t g_btlevtcmd_ConsumeItemReserve_Patch_RefundBase;
extern const int32_t g_BattleDamageDirect_Patch_AddTotalDamage;
extern const int32_t g_BattleCalculateDamage_MegaRushStrength_BH;
extern const int32_t g_BattleCalculateDamage_PowerRushStrength_BH;
extern const int32_t g_BattlePreCheckDamage_CheckEvasion_BH;
extern const int32_t g_BattlePreCheckDamage_CheckEvasion_EH;
extern const int32_t g_BattlePreCheckDamage_CheckEvasion_CH1;

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
    
    int32_t drain = 0;
    if (unit) {
        int32_t num_badges = 0;
        if (hp_drain) {
            num_badges = unit->badges_equipped.hp_drain;
        } else {
            num_badges = unit->badges_equipped.fp_drain;
        }
        if (g_Mod->state_.GetOptionNumericValue(OPT_64_STYLE_HP_FP_DRAIN)) {
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
        0x1a444662, 0x5a334343, 0xb7321253, 0x34453205, 0x00700665,
        0x00700000, 0x30743250, 0xa7764353, 0x35078644, 0x00842420,
        0x34703543, 0x30040740, 0x54444045, 0x00002045,
        // Badges.
        0xb8a88dbb, 0x009d8a8b, 0xeedd99cc, 0xcceeffff, 0xbbccccdd,
        0x0000beeb, 0x9aaa0dfc, 0xcaaddcc9, 0xc000dd7c, 0x0000000c,
        0x00770000, 0x00000000, 0x00000880
    };
    // Prices corresponding to the price tiers in the above array.
    static const constexpr uint8_t kPrices[] = {
        5, 10, 15, 20, 25, 30, 40, 50, 60, 70, 80, 100, 125, 150, 200, 250
    };
    
    static const constexpr int16_t kSquareDiamondIconId     =  44;
    static const constexpr int16_t kSquareDiamondPartnerId  =  87;
    static const constexpr int16_t kKoopaCurseIconId        = 390;
    
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
            // Fix sorting order.
            if (item.type_sort_order > 0x49) ++item.type_sort_order;
            if (item.type_sort_order > 0x43) ++item.type_sort_order;
            if (item.type_sort_order > 0x3b) ++item.type_sort_order;
            if (item.type_sort_order > 0x24) ++item.type_sort_order;
            if (item.type_sort_order > 0x21) ++item.type_sort_order;
            if (item.type_sort_order > 0x1f) ++item.type_sort_order;
            if (item.type_sort_order > 0x16) item.type_sort_order += 2;
        }
    }
    
    // Fixed sort order for Koopa Curse, new badges, and unused 'P' badges.
    itemDataTable[ItemType::KOOPA_CURSE].type_sort_order        = 0x31 + 1;
    
    itemDataTable[ItemType::SUPER_CHARGE].type_sort_order       = 0x16 + 1;
    itemDataTable[ItemType::SUPER_CHARGE_P].type_sort_order     = 0x16 + 2;
    // Leftover code for Mini HP-/FP-Plus from Shufflizer.
    // itemDataTable[ItemType::SQUARE_DIAMOND_BADGE].type_sort_order = 0x1f + 3;
    // itemDataTable[ItemType::SQUARE_DIAMOND_BADGE_P].type_sort_order = 0x21 + 4;
    itemDataTable[ItemType::ALL_OR_NOTHING_P].type_sort_order   = 0x24 + 5;
    itemDataTable[ItemType::LUCKY_DAY_P].type_sort_order        = 0x3b + 6;
    itemDataTable[ItemType::PITY_FLOWER_P].type_sort_order      = 0x43 + 7;
    itemDataTable[ItemType::FP_DRAIN_P].type_sort_order         = 0x49 + 8;
    
    // Make Peekaboo 0 BP, and sort it last, as it's unlikely to be unequipped.
    itemDataTable[ItemType::PEEKABOO].bp_cost = 0;
    itemDataTable[ItemType::PEEKABOO].type_sort_order = 999;
    
    // Set sort order of unused badges to -1 so they don't show up in log.
    itemDataTable[ItemType::TIMING_TUTOR].type_sort_order = -1;
    itemDataTable[ItemType::MONEY_MONEY].type_sort_order = -1;
    itemDataTable[ItemType::ITEM_HOG].type_sort_order = -1;
    itemDataTable[ItemType::BUMP_ATTACK].type_sort_order = -1;
    itemDataTable[ItemType::FIRST_ATTACK].type_sort_order = -1;
    itemDataTable[ItemType::SLOW_GO].type_sort_order = -1;
    
    // BP cost changes.
    itemDataTable[ItemType::TORNADO_JUMP].bp_cost   = 1;
    itemDataTable[ItemType::FIRE_DRIVE].bp_cost     = 2;
    itemDataTable[ItemType::DEFEND_PLUS].bp_cost    = 4;
    itemDataTable[ItemType::DEFEND_PLUS_P].bp_cost  = 4;
    itemDataTable[ItemType::FEELING_FINE].bp_cost   = 3;
    itemDataTable[ItemType::FEELING_FINE_P].bp_cost = 3;
    itemDataTable[ItemType::FP_DRAIN_P].bp_cost     = 1;
    itemDataTable[ItemType::PITY_FLOWER].bp_cost    = 4;
    itemDataTable[ItemType::PITY_FLOWER_P].bp_cost  = 4;
    itemDataTable[ItemType::RETURN_POSTAGE].bp_cost = 5;
    itemDataTable[ItemType::LUCKY_START].bp_cost    = 3;
    
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
    
    // New badges (Toughen Up, Toughen Up P); a single-turn +DEF buff.
    itemDataTable[ItemType::SUPER_CHARGE].bp_cost = 1;
    itemDataTable[ItemType::SUPER_CHARGE].icon_id = kSquareDiamondIconId;
    itemDataTable[ItemType::SUPER_CHARGE].name = "in_toughen_up";
    itemDataTable[ItemType::SUPER_CHARGE].description = "msg_toughen_up";
    itemDataTable[ItemType::SUPER_CHARGE].menu_description = "msg_toughen_up_menu";
    itemDataTable[ItemType::SUPER_CHARGE_P].bp_cost = 1;
    itemDataTable[ItemType::SUPER_CHARGE_P].icon_id = kSquareDiamondPartnerId;
    itemDataTable[ItemType::SUPER_CHARGE_P].name = "in_toughen_up_p";
    itemDataTable[ItemType::SUPER_CHARGE_P].description = "msg_toughen_up_p";
    itemDataTable[ItemType::SUPER_CHARGE_P].menu_description = "msg_toughen_up_p_menu";
        
    // Change Super Charge (P) weapons into Toughen Up (P).
    ttyd::battle_mario::badgeWeapon_SuperCharge.base_fp_cost = 1;
    ttyd::battle_mario::badgeWeapon_SuperCharge.charge_strength = 0;
    ttyd::battle_mario::badgeWeapon_SuperCharge.def_change_chance = 100;
    ttyd::battle_mario::badgeWeapon_SuperCharge.def_change_time = 1;
    ttyd::battle_mario::badgeWeapon_SuperCharge.def_change_strength = 2;
    ttyd::battle_mario::badgeWeapon_SuperCharge.icon = kSquareDiamondIconId;
    ttyd::battle_mario::badgeWeapon_SuperCharge.name = "in_toughen_up";
    
    ttyd::battle_mario::badgeWeapon_SuperChargeP.base_fp_cost = 1;
    ttyd::battle_mario::badgeWeapon_SuperChargeP.charge_strength = 0;
    ttyd::battle_mario::badgeWeapon_SuperChargeP.def_change_chance = 100;
    ttyd::battle_mario::badgeWeapon_SuperChargeP.def_change_time = 1;
    ttyd::battle_mario::badgeWeapon_SuperChargeP.def_change_strength = 2;
    ttyd::battle_mario::badgeWeapon_SuperChargeP.icon = kSquareDiamondIconId;
    ttyd::battle_mario::badgeWeapon_SuperChargeP.name = "in_toughen_up";
    
    // Turn Gold Bars x3 into "Shine Sprites" that can be used from the menu.
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
    // Make it inflict +ATK for 9 turns (and increase level by 5, as usual).
    ttyd::battle_item_data::ItemWeaponData_Teki_Kyouka.atk_change_chance = 100;
    ttyd::battle_item_data::ItemWeaponData_Teki_Kyouka.atk_change_time     = 9;
    ttyd::battle_item_data::ItemWeaponData_Teki_Kyouka.atk_change_strength = 3;
    // Patch in evt code to actually apply the item's newly granted status.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_ItemEvent_Teki_Kyouka_ApplyStatusHook),
        TradeOffPatch, sizeof(TradeOffPatch));
        
    // Make Koopa Curse multi-target.
    ttyd::battle_item_data::ItemWeaponData_Kameno_Noroi.target_class_flags =
        0x02101260;
    // Give it its correct icon.
    itemDataTable[ItemType::KOOPA_CURSE].icon_id = kKoopaCurseIconId;
        
    // Make Hot Sauce charge by +3.
    ttyd::battle_item_data::ItemWeaponData_RedKararing.charge_strength = 3;
        
    // Add 75%-rate Dizzy status to Tornado Jump's tornadoes.
    ttyd::battle_mario::badgeWeapon_TatsumakiJumpInvolved.dizzy_time = 3;
    ttyd::battle_mario::badgeWeapon_TatsumakiJumpInvolved.dizzy_chance = 75;
        
    // Make Piercing Blow stackable (copy Hammer Throw damage function & params)
    memcpy(
        &ttyd::battle_mario::badgeWeapon_TsuranukiNaguri.damage_function,
        &ttyd::battle_mario::badgeWeapon_HammerNageru.damage_function,
        9 * sizeof(uint32_t));
    // Determines which badge type to count to determine the power level.
    ttyd::battle_mario::badgeWeapon_TsuranukiNaguri.damage_function_params[6] =
        ItemType::PIERCING_BLOW;
        
    // Make Head Rattle have a higher rate of success and base turn count.
    ttyd::battle_mario::badgeWeapon_ConfuseHammer.confuse_chance = 127;
    ttyd::battle_mario::badgeWeapon_ConfuseHammer.confuse_time = 4;

    // Make Fire Drive cheaper to use, but deal only 4 damage at base power.
    ttyd::battle_mario::badgeWeapon_FireNaguri.damage_function_params[1] = 2;
    ttyd::battle_mario::badgeWeapon_FireNaguri.damage_function_params[3] = 2;
    ttyd::battle_mario::badgeWeapon_FireNaguri.damage_function_params[5] = 2;
    ttyd::battle_mario::badgeWeapon_FireNaguri.base_fp_cost = 3;
        
    // Change base FP cost of some moves.
    ttyd::battle_mario::badgeWeapon_Charge.base_fp_cost = 2;
    ttyd::battle_mario::badgeWeapon_ChargeP.base_fp_cost = 2;
    ttyd::battle_mario::badgeWeapon_IceNaguri.base_fp_cost = 2;
    ttyd::battle_mario::badgeWeapon_TatsumakiJump.base_fp_cost = 2;
    
    // Make per-turn Charge / Toughen Up cap at 99 instead of 9.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g__getSickStatusParam_Patch_CheckChargeCap),
        0x2c1e0064U /* cmpwi r30, 100 */);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g__getSickStatusParam_Patch_SetChargeCap),
        0x3bc00063U /* li r30, 99 */);
    
    // Double Pain doubles coin drops instead of Money Money.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_fbatBattleMode_Patch_DoubleCoinsBadge1),
        0x38600120U /* li r3, 0x120 (Double Pain) */);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_fbatBattleMode_Patch_DoubleCoinsBadge2),
        0x38600120U /* li r3, 0x120 (Double Pain) */);
        
    // Happy badges have 50% chance of restoring HP / FP instead of 33%.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_btlseqTurn_Patch_HappyHeartBaseRate),
        0x23400032 /* subfic r26, r0, 50 */);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_btlseqTurn_Patch_HappyFlowerBaseRate),
        0x23800032 /* subfic r28, r0, 50 */);
    // For some reason they also were slightly less likely to restore if already
    // at max HP/FP in vanilla!?  Remove that.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_btlseqTurn_Patch_HappyHeartReductionAtMax),
        0x1c000000U /* mulli r0, r0, 0 */);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_btlseqTurn_Patch_HappyFlowerReductionAtMax),
        0x1c000000U /* mulli r0, r0, 0 */);
        
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
        
    // Add code that weakens Power / Mega Rush badges if the option is set.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_BattleCalculateDamage_PowerRushStrength_BH),
        reinterpret_cast<void*>(StartGetDangerStrength),
        reinterpret_cast<void*>(BranchBackGetDangerStrength));
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_BattleCalculateDamage_MegaRushStrength_BH),
        reinterpret_cast<void*>(StartGetPerilStrength),
        reinterpret_cast<void*>(BranchBackGetPerilStrength));
        
    // Add code that puts a cap on the evasion from badges if the option is set.
    mod::patch::writeBranch(
        reinterpret_cast<void*>(g_BattlePreCheckDamage_CheckEvasion_BH),
        reinterpret_cast<void*>(StartCheckBadgeEvasion));
    mod::patch::writeBranch(
        reinterpret_cast<void*>(BranchBackCheckBadgeEvasion),
        reinterpret_cast<void*>(g_BattlePreCheckDamage_CheckEvasion_EH));
    mod::patch::writeBranch(
        reinterpret_cast<void*>(ConditionalBranchCheckBadgeEvasion),
        reinterpret_cast<void*>(g_BattlePreCheckDamage_CheckEvasion_CH1));
            
    g_btlevtcmd_GetItemRecoverParam_trampoline = patch::hookFunction(
        ttyd::battle_event_cmd::btlevtcmd_GetItemRecoverParam,
        [](EvtEntry* evt, bool isFirstCall) {
            g_btlevtcmd_GetItemRecoverParam_trampoline(evt, isFirstCall);
            // Run custom behavior to replace the recovery params in some cases.
            return GetAlteredItemRestorationParams(evt, isFirstCall);
        });
}

bool CheckEvasionBadges(BattleWorkUnit* unit) {
    if (g_Mod->state_.GetOptionNumericValue(OPT_EVASION_BADGES_CAP)) {
        float hit_chance = 100.f;
        for (int32_t i = 0; i < unit->badges_equipped.pretty_lucky; ++i) {
            hit_chance *= 0.90f;
        }
        for (int32_t i = 0; i < unit->badges_equipped.lucky_day; ++i) {
            hit_chance *= 0.75f;
        }
        if (unit->current_hp <= unit->unit_kind_params->danger_hp) {
            for (int32_t i = 0; i < unit->badges_equipped.close_call; ++i) {
                hit_chance *= 0.67f;
            }
        }
        if (hit_chance < 20.f) hit_chance = 20.f;
        return ttyd::system::irand(100) >= hit_chance;
    } else {
        for (int32_t i = 0; i < unit->badges_equipped.pretty_lucky; ++i) {
            if (ttyd::system::irand(100) >= 90) return true;
        }
        for (int32_t i = 0; i < unit->badges_equipped.lucky_day; ++i) {
            if (ttyd::system::irand(100) >= 75) return true;
        }
        if (unit->current_hp <= unit->unit_kind_params->danger_hp) {
            for (int32_t i = 0; i < unit->badges_equipped.close_call; ++i) {
                if (ttyd::system::irand(100) >= 67) return true;
            }
        }
    }
    return false;
}

int32_t GetBonusCakeRestoration() {
    // Returns one of 0, 5, 10, ..., 25 at random (to be added to the base 5).
    return ttyd::system::irand(6) * 5;
}

}  // namespace item
}  // namespace mod::infinite_pit