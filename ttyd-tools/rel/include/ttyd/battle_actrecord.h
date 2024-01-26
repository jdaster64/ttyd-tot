#pragma once

#include <cstdint>

namespace ttyd::battle_actrecord {
    
namespace ConditionType {
    enum e {
        NO_CONDITION = 0,
        JUMP_LESS,
        JUMP_MORE,
        HAMMER_LESS,
        HAMMER_MORE,
        SPECIAL_MOVES_LESS,
        SPECIAL_MOVES_MORE,
        MARIO_TOTAL_DAMAGE_LESS,
        MARIO_TOTAL_DAMAGE_MORE,
        PARTNER_TOTAL_DAMAGE_LESS,
        PARTNER_TOTAL_DAMAGE_MORE,
        TOTAL_DAMAGE_LESS,
        TOTAL_DAMAGE_MORE,
        MARIO_HITS_LESS,
        MARIO_HITS_MORE,
        PARTNER_HITS_LESS,
        PARTNER_HITS_MORE,
        HITS_LESS,
        HITS_MORE,
        MARIO_FINAL_HP_MORE,
        MARIO_FINAL_HP_LESS,
        POWER_BOUNCE_COMBO,
        MARIO_ITEMS_LESS,
        MARIO_ITEMS_MORE,
        PARTNER_ITEMS_LESS,
        PARTNER_ITEMS_MORE,
        ITEMS_LESS,
        ITEMS_MORE,
        MARIO_SWAP_PARTNERS_LESS,
        MARIO_SWAP_PARTNERS_MORE,
        PARTNER_SWAP_PARTNERS_LESS,
        PARTNER_SWAP_PARTNERS_MORE,
        SWAP_PARTNERS_LESS,
        SWAP_PARTNERS_MORE,
        MARIO_ATTACK_AUDIENCE_LESS,
        MARIO_ATTACK_AUDIENCE_MORE,
        PARTNERS_ATTACK_AUDIENCE_LESS,
        PARTNERS_ATTACK_AUDIENCE_MORE,
        MARIO_APPEAL_LESS,
        MARIO_APPEAL_MORE,
        PARTNERS_APPEAL_LESS,
        PARTNERS_APPEAL_MORE,
        APPEAL_LESS,
        APPEAL_MORE,
        MARIO_FP_LESS,
        MARIO_FP_MORE,
        PARTNERS_FP_LESS,
        PARTNERS_FP_MORE,
        FP_LESS,
        FP_MORE,
        MARIO_MOVES_LESS,       // Jump, Hammer, SP, Charges/Defend/Dip
        MARIO_MOVES_MORE,
        PARTNERS_MOVES_LESS,
        PARTNERS_MOVES_MORE,
        FP_LESS_2,
        FP_MORE_2,
        MOVES_LESS,
        MOVES_MORE,
        TURNS_MORE,
        TURNS_LESS,
        MARIO_INACTIVE_TURNS,   // only Appeal / Defend allowed
        PARTNER_INACTIVE_TURNS,
        INACTIVE_TURNS,
        MARIO_NO_ATTACK_TURNS,  // All Strategies and non-"attacks" allowed
        PARTNER_NO_ATTACK_TURNS,
        NO_ATTACK_TURNS,
        JUMPMAN,                // Mario can only Jump or Defend entire battle
        HAMMERMAN,              // Mario can only Hammer or Defend entire battle
        MAX_CONDITION_TYPE
    };
    static_assert(MAX_CONDITION_TYPE == 0x44);
}

extern "C" {

// _check_turn_count_0_end
// _check_turn_count_0_turn
// _check_no_use
// _check_use
void BtlActRec_JudgeRuleKeep();
void BtlActRec_JudgeTurnRuleKeep();
void BtlActRec_AddPoint(uint8_t* counter, int8_t num_to_add);
void BtlActRec_AddCount(uint8_t* counter);

}

}