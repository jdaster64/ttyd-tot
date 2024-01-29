#pragma once

#include "evt_cmd.h"

#include <ttyd/evtmgr.h>

#include <cstdint>

namespace ttyd::battle_database_common {
struct BattleWeapon;
}

namespace ttyd::battle_unit {
struct BattleWorkUnit;
struct BattleWorkUnitPart;
}

namespace mod::tot {

namespace MoveType {
    enum e {
        JUMP_BASE = 0,
        JUMP_SPIN,
        JUMP_SPRING,
        JUMP_POWER_JUMP,
        JUMP_MULTIBOUNCE,
        JUMP_POWER_BOUNCE,
        JUMP_SLEEPY_STOMP,
        JUMP_TORNADO_JUMP,
        
        HAMMER_BASE,
        HAMMER_SUPER,
        HAMMER_ULTRA,
        HAMMER_POWER_SMASH,
        HAMMER_SHRINK_SMASH,
        HAMMER_ICE_SMASH,
        HAMMER_QUAKE_HAMMER,
        HAMMER_FIRE_DRIVE,
        
        SP_SWEET_TREAT,
        SP_EARTH_TREMOR,
        SP_CLOCK_OUT,
        SP_POWER_LIFT,
        SP_ART_ATTACK,
        SP_SWEET_FEAST,
        SP_SHOWSTOPPER,
        SP_SUPERNOVA,
        
        GOOMBELLA_BASE,
        GOOMBELLA_TATTLE,
        GOOMBELLA_MULTIBONK,
        GOOMBELLA_RALLY_WINK,
        GOOMBELLA_5,
        GOOMBELLA_6,
        
        KOOPS_BASE,
        KOOPS_POWER_SHELL,
        KOOPS_SHELL_SHIELD,
        KOOPS_SHELL_SLAM,
        KOOPS_5,
        KOOPS_6,
        
        FLURRIE_BASE,
        FLURRIE_GALE_FORCE,
        FLURRIE_LIP_LOCK,
        FLURRIE_DODGY_FOG,
        FLURRIE_5,
        FLURRIE_6,
        
        YOSHI_BASE,
        YOSHI_GULP,
        YOSHI_MINI_EGG,
        YOSHI_STAMPEDE,
        YOSHI_5,
        YOSHI_6,
        
        VIVIAN_BASE,
        VIVIAN_VEIL,
        VIVIAN_FIERY_JINX,
        VIVIAN_INFATUATE,
        VIVIAN_5,
        VIVIAN_6,
        
        BOBBERY_BASE,
        BOBBERY_BOMB_SQUAD,
        BOBBERY_HOLD_FAST,
        BOBBERY_BOBOMBAST,
        BOBBERY_5,
        BOBBERY_6,
        
        MOWZ_BASE,
        MOWZ_KISS_THIEF,
        MOWZ_TEASE,
        MOWZ_SMOOCH,
        MOWZ_5,
        MOWZ_6,
        
        MOVE_TYPE_MAX,
    };
}

class MoveManager {
public:
    static void Init();

    static int32_t GetUnlockedLevel(int32_t move_type);
    static int32_t GetSelectedLevel(int32_t move_type);
    static int32_t GetMoveCost(int32_t move_type);

    static void GetCurrentSelectionString(int32_t move_type, char* out_buf);
    
    static bool ChangeSelectedLevel(int32_t move_type, int32_t change);
    static void ResetSelectedLevels();
    
    static bool IsUnlockable(int32_t move_type);
    static bool IsUpgradable(int32_t move_type);
};

// battle_weapon_power-like interface for scaling ATK based on a move.
uint32_t GetWeaponPowerFromSelectedLevel(
    ttyd::battle_unit::BattleWorkUnit* unit1,
    ttyd::battle_database_common::BattleWeapon* weapon,
    ttyd::battle_unit::BattleWorkUnit* unit2,
    ttyd::battle_unit::BattleWorkUnitPart* part);

uint32_t GetWeaponPowerFromMaxLevel(
    ttyd::battle_unit::BattleWorkUnit* unit1,
    ttyd::battle_database_common::BattleWeapon* weapon,
    ttyd::battle_unit::BattleWorkUnit* unit2,
    ttyd::battle_unit::BattleWorkUnitPart* part);

// Returns the current level of move arg0, storing the result in arg1.
EVT_DECLARE_USER_FUNC(evtTot_GetMoveSelectedLevel, 2)

}