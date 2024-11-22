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
        GOOMBELLA_IRONBONK,
        GOOMBELLA_SCOPE_OUT,
        GOOMBELLA_MULTIBONK,
        GOOMBELLA_RALLY_WINK,
        
        KOOPS_BASE,
        KOOPS_POWER_SHELL,
        KOOPS_WITHDRAW,
        KOOPS_SHELL_SHIELD,
        KOOPS_BULK_UP,
        KOOPS_SHELL_SLAM,
        
        FLURRIE_BASE,
        FLURRIE_GALE_FORCE,
        FLURRIE_LIP_LOCK,
        FLURRIE_DODGY_FOG,
        FLURRIE_BLIZZARD,
        FLURRIE_THUNDER_STORM,
        
        YOSHI_BASE,
        YOSHI_GULP,
        YOSHI_EGG_BARRAGE,
        YOSHI_SWALLOW,
        YOSHI_MINI_EGG,
        YOSHI_STAMPEDE,
        
        VIVIAN_BASE,
        VIVIAN_CURSE,
        VIVIAN_NEUTRALIZE,
        VIVIAN_VEIL,
        VIVIAN_FIERY_JINX,
        VIVIAN_INFATUATE,
        
        BOBBERY_BASE,
        BOBBERY_BOMB_SQUAD,
        BOBBERY_HOLD_FAST,
        BOBBERY_POISON_BOMB,
        BOBBERY_BOBOMBAST,
        BOBBERY_MEGATON_BOMB,
        
        MOWZ_BASE,
        MOWZ_KISS_THIEF,
        MOWZ_TEASE,
        MOWZ_EMBARGO,
        MOWZ_SMOKE_BOMB,
        MOWZ_SMOOCH,
        
        // For standard moves.
        MOVE_TYPE_MAX,

        // Used only for badge-related moves.
        BADGE_MOVE_BASE = MOVE_TYPE_MAX,
        BADGE_MOVE_CHARGE = BADGE_MOVE_BASE,
        BADGE_MOVE_CHARGE_P,
        BADGE_MOVE_SUPER_CHARGE,
        BADGE_MOVE_SUPER_CHARGE_P,
        BADGE_MOVE_MAX,
    };
}

namespace MoveLogFlags {
    enum e {
        UNLOCKED_LV_1   = 0x1,
        UNLOCKED_LV_2   = 0x2,
        UNLOCKED_LV_3   = 0x4,
        UNLOCKED_ALL    = UNLOCKED_LV_1 | UNLOCKED_LV_2 | UNLOCKED_LV_3,

        USED_LV_1       = 0x8,
        USED_LV_2       = 0x10,
        USED_LV_3       = 0x20,
        USED_ALL        = USED_LV_1 | USED_LV_2 | USED_LV_3,

        STYLISH_1       = 0x40,
        STYLISH_2       = 0x80,
        STYLISH_ALL     = STYLISH_1 | STYLISH_2,
    };
}

// Metadata for every move.
struct MoveData {
    int8_t      move_cost[3];
    int8_t      max_level;
    int8_t      move_tier;
    int8_t      partner_id;     // Using TTYD's internal pouch order; 0 = Mario.
    int16_t     icon_id;    
    const char* name_msg;
    const char* abbrev_msg;
    const char* desc_msg;
    const char* ac_msg;
    const char* upgrade_msg;
};

class MoveManager {
public:
    // Resets available moves to the default (base moves only, level 1).
    static void Init();
    
    // Returns a pointer to the move data array starting at starting_move.
    static const MoveData* GetMoveData(int32_t starting_move = MoveType::JUMP_BASE);
    // Returns the move type corresponding to a particular badge.
    static int32_t GetMoveTypeFromBadge(int32_t badge_id);

    // Gets the currently unlocked / selected move level.
    static int32_t GetUnlockedLevel(int32_t move_type);
    static int32_t GetSelectedLevel(int32_t move_type);
    
    // Gets the FP/SP cost for the currently selected level of the move.
    static int32_t GetMoveCost(int32_t move_type);
    
    // Upgrades the move by a level (Returns false if the move was at max).
    static bool UpgradeMove(int32_t move_type);
    // Changes the currently selected move level.
    static bool ChangeSelectedLevel(int32_t move_type, int32_t change);
    // Resets all move selected levels to the minimum (between battles).
    static void ResetSelectedLevels();
    
    // Returns whether the given move is unlockable.
    static bool IsUnlockable(int32_t move_type);
    // Returns whether the given move is upgradable.
    static bool IsUpgradable(int32_t move_type);

    // Returns whether or not the move name should be overridden.
    static bool GetCurrentSelectionString(int32_t move_type, char* out_buf);

    // For badge-based moves only, sets unlock levels based on badges equipped.
    static void InitBadgeMoveLevels();

    // Tracks unlocking, using and performing Stylish commands for moves.
    static void LogMoveUnlock(int32_t move_type, int32_t level);
    static void LogMoveUse(int32_t move_type);
    static void LogMoveStylish(int32_t move_type, uint32_t stylish_flags = 0);
    // Returns the description of a move in the Move Log, including level info.
    static const char* GetLogDescription(int32_t move_type);
    // Returns true if the partner for the given move has never been unlocked.
    static bool PartnerNeverObtained(int32_t move_type);

    // Checks whether moves of a particular type have reached their limit.
    static bool HasReachedLimit(int32_t move_base);
};

// battle_weapon_power-like interface for scaling ATK based on a move.
uint32_t GetWeaponPowerFromSelectedLevel(
    ttyd::battle_unit::BattleWorkUnit* unit1,
    ttyd::battle_database_common::BattleWeapon* weapon,
    ttyd::battle_unit::BattleWorkUnit* unit2,
    ttyd::battle_unit::BattleWorkUnitPart* part);

// Same, but using the maximum unlocked level; used for partner first strikes.
uint32_t GetWeaponPowerFromMaxLevel(
    ttyd::battle_unit::BattleWorkUnit* unit1,
    ttyd::battle_database_common::BattleWeapon* weapon,
    ttyd::battle_unit::BattleWorkUnit* unit2,
    ttyd::battle_unit::BattleWorkUnitPart* part);

// Uses unit work variable #arg0 for damage.
uint32_t GetWeaponPowerFromUnitWorkVariable(
    ttyd::battle_unit::BattleWorkUnit* unit1,
    ttyd::battle_database_common::BattleWeapon* weapon,
    ttyd::battle_unit::BattleWorkUnit* unit2,
    ttyd::battle_unit::BattleWorkUnitPart* part);

// Returns the current level of move arg0, storing the result in arg1.
EVT_DECLARE_USER_FUNC(evtTot_GetMoveSelectedLevel, 2)

// Unlocks or upgrades the level of move arg0.
EVT_DECLARE_USER_FUNC(evtTot_UpgradeMove, 1)

// Logs a successful Stylish command for the currently executed move.
// arg0 = 0 for all Stylishes, 1/2 for first or second of independent Stylishes.
EVT_DECLARE_USER_FUNC(evtTot_LogActiveMoveStylish, 1)

}