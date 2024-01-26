#include "custom_condition.h"

#include "custom_item.h"
#include "mod.h"
#include "mod_state.h"
#include "patches_options.h"

#include <ttyd/battle.h>
#include <ttyd/battle_actrecord.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/msgdrv.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::infinite_pit {

namespace {

using ::ttyd::mario_pouch::PouchData;

namespace ItemType = ::ttyd::item_data::ItemType;

}

struct BattleCondition {
    const char* description;
    uint8_t     type;
    uint8_t     param_min;
    int8_t      param_max;  // if -1, not a range.
    uint8_t     weight = 10;
};

using namespace ttyd::battle_actrecord::ConditionType;
static constexpr const BattleCondition kBattleConditions[] = {
    { "Don't ever use Jump moves!", JUMP_LESS, 1, -1 },
    { "Use fewer than %" PRId32 " Jump moves!", JUMP_LESS, 2, 3 },
    { "Don't ever use Hammer moves!", HAMMER_LESS, 1, -1 },
    { "Use fewer than %" PRId32 " Hammer moves!", HAMMER_LESS, 2, 3 },
    { "Don't use Special moves!", SPECIAL_MOVES_LESS, 1, -1 },
    { "Use a Special move!", SPECIAL_MOVES_MORE, 1, -1 },
    { "Don't take damage with Mario!", MARIO_TOTAL_DAMAGE_LESS, 1, -1 },
    { "Don't take damage with your partner!", PARTNER_TOTAL_DAMAGE_LESS, 1, -1 },
    { "Take less than %" PRId32 " total damage!", TOTAL_DAMAGE_LESS, 1, 5 },
    { "Take at least %" PRId32 " total damage!", TOTAL_DAMAGE_MORE, 1, 5 },
    { "Take damage at least %" PRId32 " times!", HITS_MORE, 3, 5 },
    { "Win with Mario at %" PRId32 " or more HP!", MARIO_FINAL_HP_MORE, 1, -1 },
    { "Win with Mario in Danger or worse!", MARIO_FINAL_HP_LESS, 6, -1 },
    { "Win with Mario in Peril!", MARIO_FINAL_HP_LESS, 2, -1 },
    { "Don't use any items!", ITEMS_LESS, 1, -1, 30 },
    { "Don't ever swap partners!", SWAP_PARTNERS_LESS, 1, -1 },
    { "Have Mario attack an audience member!", MARIO_ATTACK_AUDIENCE_MORE, 1, -1, 3 },
    { "Appeal to the crowd at least %" PRId32 " times!", APPEAL_MORE, 3, 5 },
    { "Don't use any FP!", FP_LESS, 1, -1 },
    { "Don't use more than %" PRId32 " FP!", FP_LESS, 4, 11 },
    { "Use at least %" PRId32 " FP!", FP_MORE, 1, 5 },
    { "Mario must only Appeal and Defend!", MARIO_INACTIVE_TURNS, 255, -1 },
    { "Your partner must only Appeal and Defend!", PARTNER_INACTIVE_TURNS, 255, -1 },
    { "Appeal/Defend only for %" PRId32 " turns!", INACTIVE_TURNS, 2, 5 },
    { "Never use attacks with Mario!", MARIO_NO_ATTACK_TURNS, 255, -1 },
    { "Never use attacks with your partner!", PARTNER_NO_ATTACK_TURNS, 255, -1 },
    { "Don't use attacks for %" PRId32 " turns!", NO_ATTACK_TURNS, 2, 5 },
    { "Mario can only Defend or use Jump moves!", JUMPMAN, 1, -1, 3 },
    { "Mario can only Defend or use Hammer moves!", HAMMERMAN, 1, -1, 3 },
    { "Finish the fight within %" PRId32 " turns!", TURNS_LESS, 2, 5, 20 },
};
char g_ConditionTextBuf[64];

void SetBattleCondition(ttyd::npcdrv::NpcBattleInfo* npc_info, bool enable) {
    StateManager_v2& state = g_Mod->state_;
    // No conditions on Bonetail fights (or reward floors).
    if (state.floor_ % 10 == 9) return;
    
    // Only have conditions on ~1/4 floors for one/all held drops + bonus modes.
    int32_t reward_mode = state.GetOptionValue(OPT_BATTLE_REWARD_MODE);
    uint32_t condition_chance = 0;
    switch (reward_mode) {
        case OPTVAL_DROP_STANDARD:
        case OPTVAL_DROP_ALL_HELD: {
            condition_chance = 1;
            break;
        }
        case OPTVAL_DROP_HELD_FROM_BONUS:
        case OPTVAL_DROP_NO_HELD_W_BONUS: {
            condition_chance = 4;
            break;
        }
    }
    if (state.Rand(4, RNG_CONDITION) >= condition_chance) return;
        
    const int32_t shine_rate =
        reward_mode == OPTVAL_DROP_NO_HELD_W_BONUS ? 8 : 30;
    
    // Use the unused "random_item_weight" field to store the item reward.
    int32_t* item_reward = &npc_info->pConfiguration->random_item_weight;
    *item_reward = PickRandomItem(
        RNG_CONDITION_ITEM, 10, 10, 40, state.floor_ < 30 ? 0 : shine_rate);
    // If the "none" case was picked, make it a Shine Sprite.
    if (*item_reward <= 0) *item_reward = ItemType::GOLD_BAR_X3;
    
    // Make a copy of the conditions array so the weights can be mutated.
    constexpr const int32_t kNumConditions = 
        sizeof(kBattleConditions) / sizeof(BattleCondition);
    BattleCondition conditions[kNumConditions];
    memcpy(conditions, kBattleConditions, sizeof(kBattleConditions));
    
    const PouchData& pouch = *ttyd::mario_pouch::pouchGetPtr();
    int32_t num_partners = 0;
    for (int32_t i = 0; i < 8; ++i) {
        num_partners += pouch.party_data[i].flags & 1;
    }
    
    // Disable conditions that rely on having partners or FP-using moves.
    for (auto& condition : conditions) {
        switch (condition.type) {
            case PARTNER_TOTAL_DAMAGE_LESS:
            case MARIO_INACTIVE_TURNS:
            case MARIO_NO_ATTACK_TURNS:
            case PARTNER_INACTIVE_TURNS:
            case PARTNER_NO_ATTACK_TURNS:
            case JUMPMAN:
            case HAMMERMAN:
                if (num_partners < 1) condition.weight = 0;
                break;
            case SWAP_PARTNERS_LESS:
                if (num_partners < 2) condition.weight = 0;
                break;
            case MARIO_ATTACK_AUDIENCE_MORE:
                // This condition will be guaranteed a Shine Sprite,
                // so there needs to be a partner (and SP needs to be unlocked).
                if (num_partners < 1) condition.weight = 0;
                break;
            case FP_MORE:
                // "Use FP" shouldn't appear too early in the Pit.
                if (state.floor_ < 30) condition.weight = 0;
                break;
        }
    }
    
    int32_t sum_weights = 0;
    for (int32_t i = 0; i < kNumConditions; ++i) 
        sum_weights += conditions[i].weight;
    
    int32_t weight = state.Rand(sum_weights, RNG_CONDITION);
    int32_t idx = 0;
    for (; (weight -= conditions[idx].weight) >= 0; ++idx);
    
    // Finalize and assign selected condition's parameters.
    int32_t param = conditions[idx].param_min;
    if (conditions[idx].param_max > 0) {
        param += state.Rand(
            conditions[idx].param_max - conditions[idx].param_min + 1,
            RNG_CONDITION);
    }
    switch (conditions[idx].type) {
        case TOTAL_DAMAGE_LESS:
        case TOTAL_DAMAGE_MORE:
        case FP_MORE:
            param *= state.floor_ < 50 ? 2 : 3;
            break;
        case MARIO_FINAL_HP_LESS:
            // Override Danger / Peril thresholds with correct value,
            // based on whether the "%-based" setting is enabled.
            param = 1 + options::GetPinchThresholdForMaxHp(
                ttyd::mario_pouch::pouchGetMaxHP(), /* peril? */ param == 2);
            break;
        case MARIO_FINAL_HP_MORE:
            // Make it based on percentage of max HP.
            param = state.Rand(4, RNG_CONDITION);
            switch (param) {
                case 0:
                    // Half, rounded up.
                    param = (ttyd::mario_pouch::pouchGetMaxHP() + 1) * 50 / 100;
                    break;
                case 1:
                    param = ttyd::mario_pouch::pouchGetMaxHP() * 60 / 100;
                    break;
                case 2:
                    param = ttyd::mario_pouch::pouchGetMaxHP() * 80 / 100;
                    break;
                case 3:
                default:
                    param = ttyd::mario_pouch::pouchGetMaxHP();
                    break;
            }
            break;
        case MARIO_ATTACK_AUDIENCE_MORE:
            // Guarantee a Shine Sprite.
            *item_reward = ItemType::GOLD_BAR_X3;
            break;
    }
    npc_info->ruleCondition = conditions[idx].type;
    npc_info->ruleParameter0 = param;
    npc_info->ruleParameter1 = param;
    
    // Assign the condition text.
    if (conditions[idx].param_max > 0 || 
        conditions[idx].type == MARIO_FINAL_HP_MORE) {
        // FP condition text says "no more than", rather than "less than".
        if (conditions[idx].type == FP_LESS) --param;
        sprintf(g_ConditionTextBuf, conditions[idx].description, param);
    } else {
        sprintf(g_ConditionTextBuf, conditions[idx].description);
    }
    
    // Increment the counter of bonus challenges.
    state.ChangeOption(STAT_CONDITIONS_TOTAL);
}

void GetBattleConditionString(char* out_buf) {
    auto* fbat_info = ttyd::battle::g_BattleWork->fbat_info;
    const int32_t item_reward =
        fbat_info->wBattleInfo->pConfiguration->random_item_weight;
    const char* item_name = ttyd::msgdrv::msgSearch(
        ttyd::item_data::itemDataTable[item_reward].name);
    
    // If held item drop is contingent on condition, don't say which will drop.
    if (g_Mod->state_.CheckOptionValue(OPTVAL_DROP_HELD_FROM_BONUS)) {
        sprintf(out_buf, "Reward challenge:\n%s", g_ConditionTextBuf);
    } else {
        sprintf(out_buf, "Bonus reward (%s):\n%s", item_name, g_ConditionTextBuf);
    }
}

}