#pragma once

#include <cstdint>

namespace ttyd::battle_unit {

struct BattleWorkUnit;

}

namespace ttyd::battle_status_effect {
    
struct BattleStatusChangeMsgData {
    int8_t          status_type;
    int8_t          pad_01[3];
    const char*     status_id;
    const char*     msg_effect;
    const char*     msg_no_effect;
} ;

static_assert(sizeof(BattleStatusChangeMsgData) == 0x10);

extern "C" {

// .text
// _st_chg_msg_disp
// BattleStatusChangeMsgMain
// BattleStatusChangeMsgAdjust
void BattleStatusChangeMsgSetAnnouce(
    battle_unit::BattleWorkUnit* unit, int32_t status_type,
    uint32_t will_have_effect);
// BattleStatusChangeMsgWorkInit
// BattleStatusChangeAnnouceMain_Unit
// BattleStatusChangeAnnouceMain
void BattleStatusChangeInfoSetAnnouce(
    battle_unit::BattleWorkUnit* unit, int32_t status_type, int8_t turn_count, 
    uint32_t will_have_effect);
// _get_pri
// BattleStatusChangeInfoWorkInit
// BSE_TurnFirstProcessEffectMain
// BSE_TurnFirstProcessEffectEntry
// BSE_KagegakureDelete
// BSE_Kagegakure
// BSE_FreezeDelete
// BSE_Freeze
// BSE_FireDelete
// BSE_Fire
// BSE_BiribiriDelete
// BSE_Biribiri
// BSE_SleepDelete
// BSE_Sleep
// BattleStatusEffectDelete
// BattleStatusEffectMain
// BattleStatusEffectInit

// .data
extern BattleStatusChangeMsgData _st_chg_msg_data[25];

}

}