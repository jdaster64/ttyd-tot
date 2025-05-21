#include "tot_gon_tower.h"

#include "common_types.h"
#include "evt_cmd.h"
#include "mod.h"
#include "tot_generate_enemy.h"
#include "tot_generate_item.h"
#include "tot_gon.h"
#include "tot_gon_tower_npcs.h"
#include "tot_gsw.h"
#include "tot_manager_achievements.h"
#include "tot_manager_dialogue.h"
#include "tot_manager_reward.h"
#include "tot_manager_timer.h"
#include "tot_state.h"
#include "tot_window_select.h"

#include <gc/types.h>
#include <ttyd/animdrv.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/database.h>
#include <ttyd/dispdrv.h>
#include <ttyd/evt_bero.h>
#include <ttyd/evt_cam.h>
#include <ttyd/evt_case.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_fade.h>
#include <ttyd/evt_hit.h>
#include <ttyd/evt_item.h>
#include <ttyd/evt_map.h>
#include <ttyd/evt_mario.h>
#include <ttyd/evt_mobj.h>
#include <ttyd/evt_msg.h>
#include <ttyd/evt_npc.h>
#include <ttyd/evt_party.h>
#include <ttyd/evt_pouch.h>
#include <ttyd/evt_seq.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evt_window.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/icondrv.h>
#include <ttyd/item_data.h>
#include <ttyd/mapdata.h>
#include <ttyd/mario.h>
#include <ttyd/mobjdrv.h>
#include <ttyd/npc_data.h>
#include <ttyd/npcdrv.h>
#include <ttyd/swdrv.h>
#include <ttyd/system.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::tot::gon {

namespace {

// Including entire namespaces for convenience.
using namespace ::ttyd::evt_bero;
using namespace ::ttyd::evt_cam;
using namespace ::ttyd::evt_case;
using namespace ::ttyd::evt_eff;
using namespace ::ttyd::evt_fade;
using namespace ::ttyd::evt_hit;
using namespace ::ttyd::evt_item;
using namespace ::ttyd::evt_map;
using namespace ::ttyd::evt_mario;
using namespace ::ttyd::evt_mobj;
using namespace ::ttyd::evt_msg;
using namespace ::ttyd::evt_npc;
using namespace ::ttyd::evt_party;
using namespace ::ttyd::evt_pouch;
using namespace ::ttyd::evt_seq;
using namespace ::ttyd::evt_snd;
using namespace ::ttyd::evt_sub;
using namespace ::ttyd::evt_window;

using ::ttyd::dispdrv::CameraId;
using ::ttyd::evt_bero::BeroEntry;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::item_data::itemDataTable;
using ::ttyd::npc_data::npcTribe;
using ::ttyd::npcdrv::NpcSetupInfo;
using ::ttyd::npcdrv::NpcTribeDescription;

namespace BeroAnimType = ::ttyd::evt_bero::BeroAnimType;
namespace BeroDirection = ::ttyd::evt_bero::BeroDirection;
namespace BeroType = ::ttyd::evt_bero::BeroType;
namespace ItemType = ::ttyd::item_data::ItemType;
namespace NpcTribeType = ::ttyd::npc_data::NpcTribeType;

const char kPitNpcName[] = "npc_enemy";
const char kPitBossNpc2Name[] = "npc_enemy_2";

// Info for custom NPCs.
NpcSetupInfo g_EnemyNpcSetup[3];

}  // namespace

// USER_FUNC Declarations.
EVT_DECLARE_USER_FUNC(evtTot_AwardWinnings, 0)
EVT_DECLARE_USER_FUNC(evtTot_CheckReroll, 1)
EVT_DECLARE_USER_FUNC(evtTot_ClearBattleResult, 0)
EVT_DECLARE_USER_FUNC(evtTot_GetContinueDestinationMap, 1)
EVT_DECLARE_USER_FUNC(evtTot_HasBackupSave, 1)
EVT_DECLARE_USER_FUNC(evtTot_LoadBackupSaveData, 0)
EVT_DECLARE_USER_FUNC(evtTot_MakeKeyTable, 3)
EVT_DECLARE_USER_FUNC(evtTot_OverrideLockKey, 1)
EVT_DECLARE_USER_FUNC(evtTot_SetPreviousPartner, 1)
EVT_DECLARE_USER_FUNC(evtTot_UpdateDestinationMap, 0)
EVT_DECLARE_USER_FUNC(evtTot_WaitForDragonLanding, 1)
EVT_DECLARE_USER_FUNC(evtTot_WaitForPlayerActionable, 0)
EVT_DECLARE_USER_FUNC(evtTot_WaitNpcAnimFrame, 2)

// Other declarations.
extern BeroEntry normal_room_entry_data[3];
extern BeroEntry boss_room_entry_data[2];

// Scripts for left / right door animations.
EVT_BEGIN(Tower_LeftExit_Close)
    MULF(LW(0), FLOAT(0.44))
    USER_FUNC(evt_mapobj_rotate, 1, PTR("door01"), 0, 0, LW(0))
    RETURN()
EVT_END()
EVT_BEGIN(Tower_LeftExit_Open)
    MULF(LW(0), FLOAT(-0.44))
    USER_FUNC(evt_mapobj_rotate, 1, PTR("door01"), 0, 0, LW(0))
    RETURN()
EVT_END()
EVT_BEGIN(Tower_RightExit_Close)
    MULF(LW(0), FLOAT(-0.44))
    USER_FUNC(evt_mapobj_rotate, 1, PTR("door02"), 0, 0, LW(0))
    RETURN()
EVT_END()
EVT_BEGIN(Tower_RightExit_Open)
    MULF(LW(0), FLOAT(0.44))
    USER_FUNC(evt_mapobj_rotate, 1, PTR("door02"), 0, 0, LW(0))
    RETURN()
EVT_END()
// Boss room
EVT_BEGIN(Tower_BossDoor_Close)
    MULF(LW(0), FLOAT(0.39))
    USER_FUNC(evt_mapobj_rotate, 1, PTR("door03"), 0, LW(0), 0)
    MULF(LW(0), FLOAT(-1.00))
    USER_FUNC(evt_mapobj_rotate, 1, PTR("door04"), 0, LW(0), 0)
    RETURN()
EVT_END()
EVT_BEGIN(Tower_BossDoor_Open)
    MULF(LW(0), FLOAT(-0.39))
    USER_FUNC(evt_mapobj_rotate, 1, PTR("door03"), 0, LW(0), 0)
    MULF(LW(0), FLOAT(-1.00))
    USER_FUNC(evt_mapobj_rotate, 1, PTR("door04"), 0, LW(0), 0)
    RETURN()
EVT_END()

// Checks whether the player has a Tower / Master key to open the lock early.
EVT_BEGIN(Tower_Lock_Check)
    USER_FUNC(evt_pouch_check_item, (int32_t)ItemType::TOT_MASTER_KEY, LW(2))
    SET(LW(0), LW(2))
    SET(LW(1), 0)
    USER_FUNC(evtTot_IsMidbossFloor, LW(8))
    IF_EQUAL(LW(8), 0)
        USER_FUNC(evt_pouch_check_item, (int32_t)ItemType::TOT_TOWER_KEY, LW(1))
        ADD(LW(0), LW(1))
    END_IF()
    IF_LARGE(LW(0), 0)
        USER_FUNC(evtTot_MakeKeyTable, LW(1), LW(2), LW(3))
        USER_FUNC(evt_win_item_select, 0, LW(3), LW(0), 0)
        IF_NOT_EQUAL(LW(0), -1)
            // Change the key type to match the item selected.
            USER_FUNC(evtTot_OverrideLockKey, LW(0))
        ELSE()
            USER_FUNC(evt_mobj_exec_cancel, PTR("me"))
        END_IF()
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(Tower_Lock_Unlock)
    SET(LW(0), PTR("e_bero"))
    RUN_CHILD_EVT(bero_case_switch_on)
    RETURN()
EVT_END()

// Script for changing room destinations dynamically.
EVT_BEGIN(Tower_UpdateDestinationEvt)
    SET(LW(0), PTR("e_bero"))
    RUN_CHILD_EVT(bero_case_switch_off)
    USER_FUNC(evtTot_UpdateDestinationMap)
    IF_EQUAL((int32_t)GSW_Tower_DisplayChestIcons, 2)
        SET(LW(0), PTR("e_bero"))
        RUN_CHILD_EVT(bero_case_switch_on)
    END_IF()
    RETURN()
EVT_END()

// Script for sign that shows current floor.
EVT_BEGIN(Tower_SignEvt)
    USER_FUNC(evt_npc_stop_for_event)
    USER_FUNC(evt_mario_key_onoff, 0)

    // Sign text (including floor number, seed, and options) is set in code.
    USER_FUNC(evt_msg_print, 0, PTR("tot_floor_sign"), 0, 0)

    USER_FUNC(evt_mario_key_onoff, 1)
    USER_FUNC(evt_npc_start_for_event)
    RETURN()
EVT_END() 

// Script that runs when opening a chest.
EVT_BEGIN(Tower_ChestEvt_Core)
    // Don't allow getting a reward from more than one chest.
    IF_EQUAL((int32_t)GSW_Tower_ChestClaimed, 1)
        RETURN()
    END_IF()
    // Disable drawing item icons above chests.
    SET((int32_t)GSW_Tower_DisplayChestIcons, 2)
    USER_FUNC(evt_mario_key_onoff, 0)
    USER_FUNC(evtTot_GetChestData, LW(9), LW(10), LW(11), LW(12), LW(13), LW(14))
    USER_FUNC(evtTot_MarkChestCollected, LW(9))
    SWITCH(LW(13))
        CASE_SMALL_EQUAL(-1)    // move, partner, coin rewards
        CASE_BETWEEN((int32_t)ItemType::DIAMOND_STAR, (int32_t)ItemType::CRYSTAL_STAR)
            // Spawn NPC and have the star track it.
            USER_FUNC(evt_npc_entry, PTR("cstar"), PTR("c_mario"))
            USER_FUNC(evt_npc_set_position, PTR("cstar"), LW(10), LW(11), LW(12))
            USER_FUNC(evt_npc_flag_onoff, 1, PTR("cstar"), 0x4000'00a0)
            USER_FUNC(
                evt_item_entry, PTR("istar"), LW(13), LW(10), LW(11), LW(12),
                17, -1, LW(14))
            
            INLINE_EVT_ID(LW(9))
                DO(0)
                    USER_FUNC(evt_npc_get_position, PTR("cstar"), LW(0), LW(1), LW(2))
                    USER_FUNC(evt_item_set_position, PTR("istar"), LW(0), LW(1), LW(2))
                    WAIT_FRM(1)
                WHILE()
            END_INLINE()
            
            WAIT_MSEC(900)
            USER_FUNC(evt_mario_get_pos, 0, LW(10), LW(11), LW(12))
            ADD(LW(11), 50)
            ADD(LW(12), 10)
            USER_FUNC(evt_npc_jump_position_nohit, 
                PTR("cstar"), LW(10), LW(11), LW(12), 1000, FLOAT(30.0))
        CASE_ETC()
            USER_FUNC(evtTot_GetUniqueItemName, LW(15))
            USER_FUNC(
                evt_item_entry, LW(15), LW(13), LW(10), LW(11), LW(12),
                17, -1, LW(14))
    END_SWITCH()
    USER_FUNC(evt_mobj_wait_animation_end, LW(8))
    USER_FUNC(evtTot_ToggleIGT, 0)
    SWITCH(LW(13))
        CASE_SMALL_EQUAL(-1)
            // Run special event assigned in RewardManager, then null it out
            // to have this function return control of Mario.
            RUN_CHILD_EVT(LW(14))
            SET(LW(14), 0)
        CASE_BETWEEN((int32_t)ItemType::DIAMOND_STAR, (int32_t)ItemType::CRYSTAL_STAR)
            DELETE_EVT(LW(9))
            USER_FUNC(evt_item_get_item, PTR("istar"))
        CASE_ETC()
            USER_FUNC(evt_item_get_item, LW(15))
    END_SWITCH()
    // If no special pickup script, give Mario back control and spawn pipe.
    IF_EQUAL(LW(14), 0)
        USER_FUNC(evtTot_ToggleIGT, 1)
        USER_FUNC(evt_mario_key_onoff, 1)
        SET((int32_t)GSW_Tower_ChestClaimed, 1)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(Tower_ChestEvt_0)
    SET(LW(8), PTR("box_0"))
    SET(LW(9), 0)
    RUN_CHILD_EVT(PTR(&Tower_ChestEvt_Core))
    RETURN()
EVT_END()

EVT_BEGIN(Tower_ChestEvt_1)
    SET(LW(8), PTR("box_1"))
    SET(LW(9), 1)
    RUN_CHILD_EVT(PTR(&Tower_ChestEvt_Core))
    RETURN()
EVT_END()

EVT_BEGIN(Tower_ChestEvt_2)
    SET(LW(8), PTR("box_2"))
    SET(LW(9), 2)
    RUN_CHILD_EVT(PTR(&Tower_ChestEvt_Core))
    RETURN()
EVT_END()

EVT_BEGIN(Tower_ChestEvt_3)
    SET(LW(8), PTR("box_3"))
    SET(LW(9), 3)
    RUN_CHILD_EVT(PTR(&Tower_ChestEvt_Core))
    RETURN()
EVT_END()

EVT_BEGIN(Tower_HandleRerolls)
    DO(0)
        IF_NOT_EQUAL((int32_t)GSW_Tower_DisplayChestIcons, 1)
            DO_BREAK()
        END_IF()
        
        USER_FUNC(evtTot_CheckReroll, LW(15))
        IF_LARGE(LW(15), 0)
            USER_FUNC(evt_mario_key_onoff, 0)
            USER_FUNC(evt_mario_get_pos, 0, LW(0), LW(1), LW(2))
            USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_MOBJ_ITEM_APPEAR1"), LW(0), LW(1), LW(2), 0)

            // Spawn explosions above chests.
            SET(LW(10), 4)
            DO(LW(10))
                SET(LW(11), 4)
                SUB(LW(11), LW(10))
                USER_FUNC(evtTot_GetChestData, LW(11), LW(0), LW(1), LW(2), LW(3), EVT_NULLPTR)
                IF_EQUAL(LW(3), 0)
                    DO_CONTINUE()
                END_IF()
                ADD(LW(1), 80)
                ADD(LW(2), 5)
                USER_FUNC(
                    evt_eff, PTR(""), PTR("bomb"), 0, LW(0), LW(1), LW(2), FLOAT(1.0),
                    0, 0, 0, 0, 0, 0, 0)
            WHILE()

            WAIT_MSEC(50)
            IF_EQUAL(LW(15), 1)
                // Reroll only if reroll option is enabled.
                USER_FUNC(evtTot_GenerateChestContents)
            END_IF()

            WAIT_MSEC(700)

            USER_FUNC(evt_mario_key_onoff, 1)
        END_IF()

        WAIT_FRM(1)
    WHILE()

    RETURN()
EVT_END()

// Runs when exiting a regular tower floor.
EVT_BEGIN(Tower_IncrementFloor)
    // Check for whether the player left the room with chests accessible.
    USER_FUNC(evt_mobj_check, PTR("box_0"), LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        IF_EQUAL((int32_t)GSW_Tower_DisplayChestIcons, 1)
            USER_FUNC(
                evtTot_MarkCompletedAchievement, AchievementId::MISC_KEY_SKIP_CHEST)
        END_IF()
    END_IF()

    // Increment floor number and run end-of-floor logic.
    USER_FUNC(evtTot_ToggleIGT, 0)
    USER_FUNC(evtTot_IncrementFloor, 1)
    SET(LW(0), 0)
    RETURN()
EVT_END()

// Spawn heart block. LW(15) = Whether to spawn it statically on load.
EVT_BEGIN(Tower_SpawnHeartSaveBlock)
    // Heart block position; leftmost cell for rest floors, rightmost for boss.
    USER_FUNC(evtTot_IsRestFloor, LW(4))
    IF_EQUAL(LW(4), 1)
        SET(LW(0), -150)
    ELSE()
        SET(LW(0), 150)
    END_IF()
    SET(LW(1), 60)
    SET(LW(2), -200)

    // Save block position; on right side of room, just below the door.
    SET(LW(6), 160)
    SET(LW(7), 60)
    SET(LW(8), 50)

    IF_EQUAL(LW(15), 0)
        // Wait and play bomb effect if spawning dynamically.
        WAIT_MSEC(100)
        USER_FUNC(
            evt_eff, PTR(""), PTR("bomb"), 0, LW(0), LW(1), LW(2), FLOAT(1.0),
            0, 0, 0, 0, 0, 0, 0)
        USER_FUNC(
            evt_snd_sfxon_3d, PTR("SFX_BOSS_RNPL_TRANSFORM4"), 
            -100, 60, -80, 0)

        // Play bomb effect for save block as well, if not on countdown timer.
        IF_EQUAL((int32_t)GSW_CountdownTimerState, 0)
            USER_FUNC(
                evt_eff, PTR(""), PTR("bomb"), 0, LW(6), LW(7), LW(8), FLOAT(1.0),
                0, 0, 0, 0, 0, 0, 0)
        END_IF()
    END_IF()

    // Spawn save block, but only on boss floors, and not with countdown timer.
    USER_FUNC(evtTot_GetFloor, LW(3))
    IF_LARGE(LW(3), 0)
        IF_EQUAL(LW(4), 0)
            IF_EQUAL((int32_t)GSW_CountdownTimerState, 0)
                USER_FUNC(evt_mobj_save_blk, PTR("sbox"), LW(6), LW(7), LW(8), 0, 0)
            END_IF()
        END_IF()
    END_IF()
    
    // Set heart block coin price = 10 + 5 for each boss floor after the first.
    SUB(LW(3), 1)
    DIV(LW(3), 8)
    MUL(LW(3), 5)
    ADD(LW(3), 10)
    // Double cost before the boss.
    IF_EQUAL(LW(4), 1)
        MUL(LW(3), 2)
    END_IF()
    // Spawn heart block.
    USER_FUNC(evt_mobj_recovery_blk, PTR("hbox"), LW(3), LW(0), LW(1), LW(2), 0, 0)

    RETURN()
EVT_END()

// Open the grate blocking off access to the chest area.
// LW(15) = Whether to open the grate statically on load.
EVT_BEGIN(Tower_OpenGrate)
    IF_EQUAL(LW(15), 0)
        USER_FUNC(evt_mario_key_onoff, 0)
        WAIT_MSEC(1750)
    END_IF()

    SET(LW(0), 0)
    IF_EQUAL(LW(15), 0)
        SET(LW(1), 3)
    ELSE()
        SET(LW(1), 111)
    END_IF()
    USER_FUNC(evt_hit_bind_mapobj, PTR("a_kabeoku_01"), PTR("s_grate"))
    USER_FUNC(evt_hit_bind_mapobj, PTR("a_kabeoku_02"), PTR("s_grate"))
    USER_FUNC(evt_mapobj_get_position, PTR("s_grate"), LW(10), LW(11), LW(12))
    IF_EQUAL(LW(15), 0)
        USER_FUNC(
            evt_snd_sfxon_3d, PTR("SFX_STG1_DOOR_GRID_OPEN1"), LW(10), LW(11), LW(12), 0)
    END_IF()
    DO(0)
        ADD(LW(0), LW(1))
        USER_FUNC(evt_mapobj_trans, 1, PTR("s_grate"), 0, 0, LW(0))
        USER_FUNC(evt_hit_bind_update, PTR("a_kabeoku_01"))
        USER_FUNC(evt_hit_bind_update, PTR("a_kabeoku_02"))
        IF_LARGE_EQUAL(LW(0), 111)
            DO_BREAK()
        END_IF()
        WAIT_FRM(1)
    WHILE()

    IF_EQUAL(LW(15), 0)
        WAIT_MSEC(500)
        USER_FUNC(evt_mario_key_onoff, 1)
    END_IF()
    SET((int32_t)GSW_Tower_DisplayChestIcons, 1)

    RUN_EVT(Tower_HandleRerolls)

    RETURN()
EVT_END()

// Spawn chests.
EVT_BEGIN(Tower_SpawnChests)
    USER_FUNC(evtTot_GenerateChestContents)

    SET(LW(10), 4)
    DO(LW(10))
        // Get contents of next chest, and break from loop if empty.
        SET(LW(11), 4)
        SUB(LW(11), LW(10))
        USER_FUNC(evtTot_GetChestData, LW(11), LW(0), LW(1), LW(2), LW(3), EVT_NULLPTR)
        IF_EQUAL(LW(3), 0)
            DO_BREAK()
        END_IF()
        
        SWITCH(LW(11))
            CASE_EQUAL(0)
                USER_FUNC(
                    evt_mobj_itembox, PTR("box_0"), LW(0), LW(1), LW(2), 1, 0, 
                    PTR(&Tower_ChestEvt_0), (int32_t)GSWF_Chest_0)
            CASE_EQUAL(1)
                USER_FUNC(
                    evt_mobj_itembox, PTR("box_1"), LW(0), LW(1), LW(2), 1, 0, 
                    PTR(&Tower_ChestEvt_1), (int32_t)GSWF_Chest_1)
            CASE_EQUAL(2)
                USER_FUNC(
                    evt_mobj_itembox, PTR("box_2"), LW(0), LW(1), LW(2), 1, 0, 
                    PTR(&Tower_ChestEvt_2), (int32_t)GSWF_Chest_2)
            CASE_ETC()
                USER_FUNC(
                    evt_mobj_itembox, PTR("box_3"), LW(0), LW(1), LW(2), 1, 0, 
                    PTR(&Tower_ChestEvt_3), (int32_t)GSWF_Chest_3)
        END_SWITCH()
    WHILE()
    RETURN()
EVT_END()

// Spawn the exit to the floor.
EVT_BEGIN(Tower_OpenExit)
    USER_FUNC(evt_mario_key_onoff, 0)
    
    // Open locked door.
    WAIT_MSEC(100)
    USER_FUNC(evt_mobj_check, PTR("mobj_lock_00"), LW(10))
    IF_NOT_EQUAL(LW(10), 0)
        USER_FUNC(evt_mobj_lock_unlock, PTR("mobj_lock_00"))
        WAIT_FRM(2)
    END_IF()

    // Spawn Heart Block + Save block on boss floors.
    USER_FUNC(evtTot_GetFloor, LW(10))
    IF_LARGE(LW(10), 0)
        MOD(LW(10), 8)
        IF_EQUAL(LW(10), 0)
            RUN_EVT(Tower_SpawnHeartSaveBlock)
        END_IF()
    END_IF()
    
    // Delete chest objects.
    SET(LW(10), 4)
    DO(LW(10))
        SET(LW(11), 4)
        SUB(LW(11), LW(10))
        USER_FUNC(evtTot_GetChestData, LW(11), LW(0), LW(1), LW(2), LW(3), EVT_NULLPTR)
        IF_EQUAL(LW(3), 0)
            DO_CONTINUE()
        END_IF()
        ADD(LW(1), 25)
        USER_FUNC(
            evt_eff, PTR(""), PTR("bomb"), 0, LW(0), LW(1), LW(2), FLOAT(1.0),
            0, 0, 0, 0, 0, 0, 0)
    WHILE()
    USER_FUNC(evt_mobj_delete, PTR("box_0"))
    USER_FUNC(evt_mobj_delete, PTR("box_1"))
    USER_FUNC(evt_mobj_delete, PTR("box_2"))
    USER_FUNC(evt_mobj_delete, PTR("box_3"))
    
    USER_FUNC(evt_mario_key_onoff, 1)
    RETURN()
EVT_END()

// LW(15) input:
// - Died to timeout on field - 0
// - Died to regular battle loss - 1
// - Died to battle loss due to timeout - 3
EVT_BEGIN(Tower_TimeUpScript)
    USER_FUNC(evtTot_ToggleIGT, 0)
    USER_FUNC(evt_npc_set_position, PTR(kPitNpcName), 0, -1000, 0)
    USER_FUNC(evt_mario_dispflag_onoff, 1, 2)
    USER_FUNC(evt_mario_key_onoff, 0)

    // Start camera shake.
    INLINE_EVT()
        USER_FUNC(evt_snd_sfxon, PTR("SFX_STG2_QUAKE2"), EVT_NULLPTR)
        USER_FUNC(evt_cam_shake, 4, FLOAT(0.00), FLOAT(0.01), 10000)
    END_INLINE()

    // Party reactions.
    INLINE_EVT()
        IF_EQUAL(LW(15), 0)
            WAIT_MSEC(100)
            USER_FUNC(evt_eff_fukidashi, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 60)
            WAIT_MSEC(1500)
            USER_FUNC(evt_mario_set_pose, PTR("M_I_O"))
            WAIT_FRM(3)
            USER_FUNC(evt_party_cont_onoff, 0, 0)
            USER_FUNC(evt_mario_get_party, LW(0))
            SWITCH(LW(0))
                CASE_EQUAL(1)
                    USER_FUNC(evt_party_set_pose, 0, PTR("PKR_D_1"))
                CASE_EQUAL(2)
                    USER_FUNC(evt_party_set_pose, 0, PTR("PNK_D_1"))
                CASE_EQUAL(3)
                    USER_FUNC(evt_party_set_pose, 0, PTR("D_1"))
                CASE_EQUAL(4)
                    USER_FUNC(evt_party_set_pose, 0, PTR("PYS_D_1"))
                CASE_EQUAL(5)
                    USER_FUNC(evt_party_set_pose, 0, PTR("PWD_D_1"))
                CASE_EQUAL(6)
                    USER_FUNC(evt_party_set_pose, 0, PTR("PTR_D_1"))
                CASE_EQUAL(7)
                    USER_FUNC(evt_party_set_pose, 0, PTR("PCH_D_1"))
            END_SWITCH()
        ELSE()
            USER_FUNC(evt_mario_set_pose, PTR("M_D_2"))
            WAIT_FRM(3)
            USER_FUNC(evt_party_cont_onoff, 0, 0)
            USER_FUNC(evt_mario_get_party, LW(0))
            SWITCH(LW(0))
                CASE_EQUAL(1)
                    USER_FUNC(evt_party_set_pose, 0, PTR("PKR_D_3"))
                CASE_EQUAL(2)
                    USER_FUNC(evt_party_set_pose, 0, PTR("PNK_D_3"))
                CASE_EQUAL(3)
                    USER_FUNC(evt_party_set_pose, 0, PTR("D_3"))
                CASE_EQUAL(4)
                    USER_FUNC(evt_party_set_pose, 0, PTR("PYS_D_3"))
                CASE_EQUAL(5)
                    USER_FUNC(evt_party_set_pose, 0, PTR("PWD_D_3"))
                CASE_EQUAL(6)
                    USER_FUNC(evt_party_set_pose, 0, PTR("PTR_D_3"))
                CASE_EQUAL(7)
                    USER_FUNC(evt_party_set_pose, 0, PTR("PCH_D_3"))
            END_SWITCH()
        END_IF()
    END_INLINE()

    // Wait for explosions only if event started on the field.
    IF_EQUAL(LW(15), 0)
        WAIT_MSEC(1000)
    END_IF()

    // Initial, small explosions.
    DO(10)
        WAIT_MSEC(350)
        USER_FUNC(evt_sub_random, 350, LW(0))
        USER_FUNC(evt_sub_random, 150, LW(1))
        USER_FUNC(evt_sub_random, 300, LW(2))
        SUB(LW(0), 175)
        SUB(LW(2), 150)
        USER_FUNC(
            evt_eff, PTR(""), PTR("bomb"), 0, LW(0), LW(1), LW(2), FLOAT(1.0),
            0, 0, 0, 0, 0, 0, 0)
        USER_FUNC(evt_snd_sfxon, PTR("SFX_EVT_LOTTERY_BOMB_BURST1"), EVT_NULLPTR)
    WHILE()

    WAIT_MSEC(1000)

    // Big series of explosions all at once.
    INLINE_EVT()
        USER_FUNC(evt_snd_sfxon, PTR("SFX_EVT_LOTTERY_BOMB_BURST1"), EVT_NULLPTR)
        DO(30)
            USER_FUNC(evt_sub_random, 200, LW(0))
            USER_FUNC(evt_sub_random, 75, LW(1))
            USER_FUNC(evt_sub_random, 75, LW(2))
            SUB(LW(0), 100)
            ADD(LW(2), 150)
            USER_FUNC(
                evt_eff, PTR(""), PTR("bomb"), 0, LW(0), LW(1), LW(2),
                FLOAT(5.0), 0, 0, 0, 0, 0, 0, 0)
            SUB(LW(2), 5)
            USER_FUNC(
                evt_eff, PTR(""), PTR("kemuri_test"), 6, LW(0), LW(1), LW(2),
                FLOAT(5.0), 0, 0, 0, 0, 0, 0, 0)
            WAIT_MSEC(50)
        WHILE()
    END_INLINE()
    
    // Set Koopa's "run results" conversation to appropriate value.
    IF_EQUAL(LW(15), 1)
        // Died to battle normally.
        SET((int32_t)GSW_NpcA_SpecialConversation, 20)
    ELSE()
        // Died to timeup, on field or in battle.
        SET((int32_t)GSW_NpcA_SpecialConversation, 40)
    END_IF()
    
    USER_FUNC(evtTot_ClearBattleResult)
    WAIT_MSEC(100)

    // Despawn partner, reload into lobby.
    USER_FUNC(evt_mario_goodbye_party, 0)
    USER_FUNC(evtTot_SetPreviousPartner, 0)
    USER_FUNC(evt_fade_set_mapchange_type, 0, 2, 300, 1, 300)
    USER_FUNC(evt_bero_mapchange, PTR("gon_00"), PTR("w_bero"))

    RETURN()
EVT_END()

EVT_BEGIN(Tower_CheckTimeUp)
    DO(0)
        IF_EQUAL((int32_t)GSW_CountdownTimerState, 2)
            USER_FUNC(evtTot_WaitForPlayerActionable)
            SET(LW(15), 0)
            RUN_CHILD_EVT(&Tower_TimeUpScript)
            RETURN()
        END_IF()
        WAIT_FRM(1)
    WHILE()
    RETURN()
EVT_END()

EVT_BEGIN(Tower_GameOverScript)
    // Call time-up script instead.
    IF_LARGE_EQUAL((int32_t)GSW_CountdownTimerState, 1)
        SET(LW(15), (int32_t)GSW_CountdownTimerState)
        SET((int32_t)GSW_CountdownTimerState, 3)
        RUN_CHILD_EVT(&Tower_TimeUpScript)
        RETURN()
    END_IF()

    USER_FUNC(evtTot_ToggleIGT, 0)
    USER_FUNC(evt_npc_set_position, PTR(kPitNpcName), 0, -1000, 0)
    USER_FUNC(evt_mario_dispflag_onoff, 1, 2)
    USER_FUNC(evt_mario_key_onoff, 0)
    USER_FUNC(evt_mario_set_pose, PTR("M_D_2"))
    WAIT_FRM(3)
    USER_FUNC(evt_party_cont_onoff, 0, 0)
    USER_FUNC(evt_mario_get_party, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL(1)
            USER_FUNC(evt_party_set_pose, 0, PTR("PKR_D_3"))
        CASE_EQUAL(2)
            USER_FUNC(evt_party_set_pose, 0, PTR("PNK_D_3"))
        CASE_EQUAL(3)
            USER_FUNC(evt_party_set_pose, 0, PTR("D_3"))
        CASE_EQUAL(4)
            USER_FUNC(evt_party_set_pose, 0, PTR("PYS_D_3"))
        CASE_EQUAL(5)
            USER_FUNC(evt_party_set_pose, 0, PTR("PWD_D_3"))
        CASE_EQUAL(6)
            USER_FUNC(evt_party_set_pose, 0, PTR("PTR_D_3"))
        CASE_EQUAL(7)
            USER_FUNC(evt_party_set_pose, 0, PTR("PCH_D_3"))
    END_SWITCH()
    
    USER_FUNC(evtTot_ClearBattleResult)
    USER_FUNC(evt_msg_print, 0, PTR("tot_gameover"), 0, PTR("me"))
LBL(10)
    USER_FUNC(evt_msg_select, 0, PTR("tot_gameover_opt"))
    SET(LW(1), LW(0))
    // Confirm that the player is okay with losing progress either way.
    IF_EQUAL(LW(0), 0)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_gameover_continueconf"))
    ELSE()
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_gameover_giveupconf"))
    END_IF()
    USER_FUNC(evt_msg_select, 0, PTR("tot_gameover_conf"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_gameover_repeat"))
        GOTO(10)
    END_IF()

    // Despawn partner.
    USER_FUNC(evt_mario_goodbye_party, 0)
    // Chose "continue" option.
    IF_EQUAL(LW(1), 0)
        USER_FUNC(evtTot_LoadBackupSaveData)
        USER_FUNC(evtTot_GetContinueDestinationMap, LW(1))
        // Resets floor-based stats without making saves, etc.
        USER_FUNC(evtTot_IncrementFloor, 0)
        // Set flag to spawn player in room ready to go to the next floor.
        // Setting the "chest claimed" flag to 0 prevents lock in the current
        // room from opening when the old floor is trying to be loaded.
        SET((int32_t)GSW_Tower_ContinuingFromGameOver, 1)
        SET((int32_t)GSW_Tower_ChestClaimed, 0)
        USER_FUNC(evt_fade_set_mapchange_type, 0, 2, 300, 1, 300)
        USER_FUNC(evt_bero_mapchange, LW(1), PTR("w_bero"))
    ELSE()
        // Clear Koopa's "run results" conversation to generic loss.
        SET((int32_t)GSW_NpcA_SpecialConversation, 20)
        // Reload into lobby.
        USER_FUNC(evtTot_SetPreviousPartner, 0)
        USER_FUNC(evt_fade_set_mapchange_type, 0, 2, 300, 1, 300)
        USER_FUNC(evt_bero_mapchange, PTR("gon_00"), PTR("w_bero"))
    END_IF()
    RETURN()
EVT_END()

// Checks for Game Over state on standard floors.
EVT_BEGIN(Tower_CheckGameOver)
    DO(0)
        USER_FUNC(evt_npc_get_battle_result, LW(0))
        IF_EQUAL(LW(0), 3)
            RUN_CHILD_EVT(&Tower_GameOverScript)
            RETURN()
        END_IF()
        WAIT_FRM(1)
    WHILE()
    RETURN()
EVT_END()

// Victory sequence at the end of a run.
// LW(15) should be set to how long to delay the ending stats from popping up.
EVT_BEGIN(Tower_VictorySequence)
    // Increment floor to lock timer.
    USER_FUNC(evtTot_ToggleIGT, 0)
    USER_FUNC(evtTot_IncrementFloor, 1)
    USER_FUNC(evtTot_TrackCompletedRun)
    WAIT_MSEC(LW(15))

    // Skip run results at the end of tutorial runs.
    IF_SMALL((int32_t)GSW_Tower_TutorialClearAttempts, 3)
        GOTO(99)
    END_IF()

    // Show run stats + timer splits dialog.
LBL(10)
    USER_FUNC(evt_win_other_select,
        (uint32_t)window_select::MenuType::RUN_RESULTS_STATS)
LBL(20)
    WAIT_MSEC(200)
    USER_FUNC(evt_win_other_select,
        (uint32_t)window_select::MenuType::RUN_RESULTS_SPLITS)
    IF_EQUAL(LW(0), 0)
        // If "cancelled", go back to regular stats window.
        WAIT_MSEC(200)
        GOTO(10)
    END_IF()

    // Show reward summary dialog.
    WAIT_MSEC(200)
    USER_FUNC(evt_win_other_select,
        (uint32_t)window_select::MenuType::RUN_RESULTS_REWARD)

LBL(99)
    // Add the awarded coins and Star Pieces to the hub currency totals. 
    USER_FUNC(evtTot_AwardWinnings)

    WAIT_MSEC(500)
    // Despawn partner.
    USER_FUNC(evt_mario_goodbye_party, 0)
    // Reload into west Petalburg.
    USER_FUNC(evtTot_SetPreviousPartner, 0)
    USER_FUNC(evt_fade_set_mapchange_type, 0, 2, 300, 1, 300)
    USER_FUNC(evt_bero_mapchange, PTR("gon_10"), PTR("w_bero"))

    RETURN()
EVT_END()

EVT_BEGIN(Tower_DragonFelledShake)
    USER_FUNC(evt_npc_get_position, LW(13), LW(0), LW(1), LW(2))
    USER_FUNC(evt_snd_sfxon, PTR("SFX_STG1_GNB_DOWN1"), 0)
    USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_STG1_GNB_DOWN2"), LW(0), LW(1), LW(2), LW(8))
    INLINE_EVT()
        USER_FUNC(evt_sub_intpl_msec_init, 0, 90, 0, 10000)
        LBL(0)
        USER_FUNC(evt_sub_intpl_msec_get_value)
        USER_FUNC(evt_snd_sfx_vol, LW(8), LW(0))
        WAIT_FRM(1)
        IF_EQUAL(LW(1), 1)
            GOTO(0)
        END_IF()
        USER_FUNC(evt_snd_sfxoff, LW(8))
    END_INLINE()
    USER_FUNC(evt_cam_shake, 4, FLOAT(0.05), FLOAT(0.0), 500)
    USER_FUNC(evt_sub_intpl_init, 11, 50, 5, 50)
    DO(0)
        USER_FUNC(evt_sub_intpl_get_value)
        DIVF(LW(0), FLOAT(1000.0))
        USER_FUNC(evt_cam_shake, 4, LW(0), FLOAT(0.0), 50)
        IF_EQUAL(LW(1), 0)
            DO_BREAK()
        END_IF()
    WHILE()
    USER_FUNC(evt_cam_shake, 4, FLOAT(0.005), FLOAT(0.0), 2000)
    RETURN()
EVT_END()

EVT_BEGIN(Tower_DragonFogChange)
    USER_FUNC(evt_map_get_fog, LW(4), LW(5), LW(6), LW(7), LW(8), LW(9))
    SET(GW(0), LW(5))
    SET(GW(1), LW(6))
    SET_FRAME_FROM_MSEC(LW(10), 8000)
    ADD(LW(10), 1)
    INLINE_EVT()
        DO(LW(10))
            USER_FUNC(evt_map_set_fog, LW(4), GW(0), GW(1), LW(7), LW(8), LW(9))
            WAIT_FRM(1)
        WHILE()
    END_INLINE()
    INLINE_EVT()
        USER_FUNC(evt_sub_intpl_init, 11, 0, 400, LW(10))
        DO(LW(10))
            USER_FUNC(evt_sub_intpl_get_value)
            SET(GW(0), LW(0))
            ADD(GW(0), LW(5))
            IF_EQUAL(LW(1), 0)
                DO_BREAK()
            END_IF()
            WAIT_FRM(1)
        WHILE()
    END_INLINE()
    USER_FUNC(evt_sub_intpl_init, 11, 0, 1600, LW(10))
    DO(LW(10))
        USER_FUNC(evt_sub_intpl_get_value)
        SET(GW(1), LW(0))
        ADD(GW(1), LW(6))
        IF_EQUAL(LW(1), 0)
            DO_BREAK()
        END_IF()
        WAIT_FRM(1)
    WHILE()
    RETURN()
EVT_END()

EVT_BEGIN(Tower_DragonStandupShake)
    WAIT_MSEC(3700)
    USER_FUNC(evt_npc_get_position, PTR(kPitNpcName), LW(0), LW(1), LW(2))
    USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_STG1_GNB_STEP1"), LW(0), LW(1), LW(2), 0)
    USER_FUNC(evt_cam_shake, 4, FLOAT(0.01), FLOAT(0.0), 800)
    RETURN()
EVT_END()

// Runs event for encountering final boss.
EVT_BEGIN(Tower_FinalBossEvent)
    // Entry animation.
    USER_FUNC(evt_hitobj_onoff, PTR("baba"), 1, 0)
    USER_FUNC(evt_map_set_fog, 2, 400, 1000, 0, 10, 40)
    USER_FUNC(evt_map_fog_onoff, 1)
    USER_FUNC(evt_mario_key_onoff, 0)
    USER_FUNC(evt_mario_normalize)
    USER_FUNC(evt_npc_set_position, PTR(kPitNpcName), 60, 10, 45)
    USER_FUNC(evt_npc_set_anim, PTR(kPitNpcName), PTR("GNB_S_1"))
    USER_FUNC(evt_cam3d_evt_set, -480, 150, 459, -480, 67, -13, 0, 11)
    USER_FUNC(evt_mario_set_pos, -560, 10, 0)
    USER_FUNC(evt_party_set_pos, 0, -600, 10, 0)
    USER_FUNC(evt_seq_wait, 2)

    INLINE_EVT()
        SET(LW(1), 0)
        RUN_EVT(bero_close_door_play_se)
        USER_FUNC(evt_sub_intpl_msec_init, 11, 180, 0, 500)
        DO(0)
            USER_FUNC(evt_sub_intpl_msec_get_value)
            RUN_CHILD_EVT(Tower_BossDoor_Close)
            IF_EQUAL(LW(1), 0)
                DO_BREAK()
            END_IF()
        WHILE()
        USER_FUNC(evt_cam_shake, 4, FLOAT(0.01), FLOAT(0.00), 200)
        USER_FUNC(evt_cam_shake, 4, FLOAT(0.01), FLOAT(0.00), 100)
    END_INLINE()
    INLINE_EVT()
        USER_FUNC(evt_party_move_pos2, 0, -490, 0, FLOAT(120.0))
    END_INLINE()
    USER_FUNC(evt_mario_mov_pos2, -450, 0, FLOAT(120.0))
    RUN_EVT(bero_case_entry)
    WAIT_MSEC(1100)

    // Print encounter message.
    USER_FUNC(evtTot_GetDifficulty, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL((int32_t)OPTVAL_DIFFICULTY_HALF)
            USER_FUNC(evtTot_SetConversation, (int32_t)ConversationId::HOOK_F1)
        CASE_ETC()
            USER_FUNC(evtTot_SetConversation, (int32_t)ConversationId::GLOOM_F1)
    END_SWITCH()
    USER_FUNC(evtTot_GetNextMessage, LW(0), LW(1))
    USER_FUNC(evt_msg_print, 0, LW(0), 0, 0)

    USER_FUNC(evt_mario_get_pos, 0, LW(0), LW(1), LW(2))
    USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_VOICE_MARIO_SURPRISED2_3"), LW(0), LW(1), LW(2), 0)
    USER_FUNC(evt_eff_fukidashi, 0, PTR(""), 0, 0, 0, 0, 0, 0, 0, 0, 96)
    USER_FUNC(evt_eff_fukidashi, 3, PTR(""), 0, 0, 0, 0, 0, 0, 0, 0, 96)
    USER_FUNC(evt_mario_set_pose, PTR("M_N_5B"))
    WAIT_MSEC(2000)
    USER_FUNC(evt_mario_set_pose, PTR("M_S_1"))
    USER_FUNC(evt_mario_set_dir_npc, PTR(kPitNpcName))
    USER_FUNC(evt_party_set_dir_npc, 0, PTR(kPitNpcName))
    USER_FUNC(evt_snd_bgmon, 512, PTR("BGM_EVT_GONBABA_FLY2"))
    INLINE_EVT()
        USER_FUNC(evt_snd_sfxon_3d_ex, PTR("SFX_STG1_GNB_ROAR2"), -900, 30, 490, 255, 255, 4300, 16, 0)
        USER_FUNC(evt_npc_set_anim, PTR(kPitNpcName), PTR("GNB_U_1"))
        USER_FUNC(evt_npc_wait_anim, PTR(kPitNpcName), FLOAT(1.0))
        USER_FUNC(evt_npc_set_anim, PTR(kPitNpcName), PTR("GNB_S_1"))
    END_INLINE()
    RUN_EVT(PTR(&Tower_DragonStandupShake))
    RUN_EVT(PTR(&Tower_DragonFogChange))
    USER_FUNC(evt_cam3d_evt_set, -900, 31, 490, -205, 175, 20, 4500, 11)
    INLINE_EVT()
        WAIT_MSEC(1800)
        USER_FUNC(evt_mapobj_flag_onoff, 1, 1, PTR("door10"), 1)
        USER_FUNC(evt_mapobj_flag_onoff, 1, 1, PTR("kesu"), 1)
    END_INLINE()
    WAIT_MSEC(4000)
    DO(30)
        USER_FUNC(evt_cam3d_evt_set, -890, 33, 483, -205, 175, 20, 15, 11)
        WAIT_MSEC(15)
        USER_FUNC(evt_cam3d_evt_set, -900, 31, 490, -205, 175, 20, 15, 11)
        WAIT_MSEC(15)
    WHILE()
    WAIT_MSEC(3000)
    USER_FUNC(evt_npc_battle_start, PTR(kPitNpcName))
    
    // Handle game over sequence.
    USER_FUNC(evt_npc_wait_battle_end)
    USER_FUNC(evt_npc_get_battle_result, LW(0))
    IF_EQUAL(LW(0), 3)
        // Turn on red fog, turn door model back on and point at player.
        USER_FUNC(evt_map_set_fog, 2, 400, 1000, 40, 0, 20)
        USER_FUNC(evt_map_fog_onoff, 1)
        USER_FUNC(evt_mapobj_flag_onoff, 1, 0, PTR("door10"), 1)
        USER_FUNC(evt_mapobj_flag_onoff, 1, 0, PTR("kesu"), 1)
        USER_FUNC(evt_cam3d_evt_off, 0, 11)
        RUN_CHILD_EVT(&Tower_GameOverScript)
        RETURN()
    END_IF()
    
    // Otherwise, play dragon death animation.
    
    SET(LW(13), PTR(kPitNpcName))
    // On FULL_EX difficulty only, swap main and Bonetail NPCs.
    USER_FUNC(evtTot_GetDifficulty, LW(1))
    IF_EQUAL(LW(1), (int32_t)OPTVAL_DIFFICULTY_FULL_EX)
        USER_FUNC(evt_npc_set_position, LW(13), 0, -1000, 0)
        SET(LW(13), PTR(kPitBossNpc2Name))
    END_IF()

    USER_FUNC(evt_cam3d_evt_set, 0, 30, 842, 0, 135, 150, 0, 11)
    USER_FUNC(evt_npc_set_anim, LW(13), PTR("GNB_H_1"))
    USER_FUNC(evt_map_fog_onoff, 0)
    USER_FUNC(evt_npc_set_position, LW(13), 0, 70, -100)
    USER_FUNC(evt_mario_set_pos, 25, 10, 150)
    USER_FUNC(evt_mario_set_dir, 270, 0, 0)
    USER_FUNC(evt_party_set_pos, 0, -25, 10, 150)
    USER_FUNC(evt_party_set_dir, 0, 90, -1)
    RUN_EVT(PTR(&Tower_DragonFelledShake))
    INLINE_EVT()
        WAIT_MSEC(300)
        USER_FUNC(evt_npc_flag_onoff, 1, LW(13), 131088)
        USER_FUNC(evtTot_WaitForDragonLanding, LW(13))
        USER_FUNC(evt_npc_set_anim, LW(13), PTR("GNB_H_2"))
    END_INLINE()
    USER_FUNC(evt_npc_get_position, LW(13), LW(0), LW(1), LW(2))
    INLINE_EVT()
        USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_STG8_GNB_DOWN2"), LW(0), LW(1), LW(2), LW(3))
        USER_FUNC(evt_cam_shake, 4, FLOAT(0.05), FLOAT(0.0), 500)
        USER_FUNC(evt_sub_intpl_init, 11, 50, 5, 50)
        DO(0)
            USER_FUNC(evt_sub_intpl_get_value)
            DIVF(LW(0), FLOAT(1000.0))
            USER_FUNC(evt_cam_shake, 4, LW(0), FLOAT(0.0), 50)
            IF_EQUAL(LW(1), 0)
                DO_BREAK()
            END_IF()
        WHILE()
        USER_FUNC(evt_cam_shake, 4, FLOAT(0.005), FLOAT(0.0), 2000)
        USER_FUNC(evt_snd_sfxoff, LW(3))
    END_INLINE()

    // Handle stopping timer, triggering achievements, results screens, etc.
    SET(LW(15), 5000)
    RUN_CHILD_EVT(Tower_VictorySequence)

    RETURN()
EVT_END()

EVT_BEGIN(Tower_GoldFuzzyIdleSfx)
    SET(LW(13), PTR(kPitNpcName))
    DO(0)
        USER_FUNC(evtTot_WaitNpcAnimFrame, LW(13), 35)
        USER_FUNC(evt_npc_get_position, LW(13), LW(0), LW(1), LW(2))
        USER_FUNC(evt_snd_sfxon_3d, 
            PTR("SFX_STG1_CHORO_MOVE1"), LW(0), LW(1), LW(2), 0)
        USER_FUNC(evtTot_WaitNpcAnimFrame, LW(13), 50)
        USER_FUNC(evt_npc_get_position, LW(13), LW(0), LW(1), LW(2))
        USER_FUNC(evt_snd_sfxon_3d,
            PTR("SFX_STG1_CHORO_G_WAIT2"), LW(0), LW(1), LW(2), 0)
        USER_FUNC(evtTot_WaitNpcAnimFrame, LW(13), 95)
        USER_FUNC(evt_npc_get_position, LW(13), LW(0), LW(1), LW(2))
        USER_FUNC(evt_snd_sfxon_3d,
            PTR("SFX_STG1_CHORO_G_WAIT1"), LW(0), LW(1), LW(2), 0)
        USER_FUNC(evtTot_WaitNpcAnimFrame, LW(13), -1)
    WHILE()
    RETURN()
EVT_END()

// Runs event for encountering final boss.
EVT_BEGIN(Tower_GoldFuzzyBossEvent)
    // Entry animation.
    USER_FUNC(evt_hitobj_onoff, PTR("baba"), 1, 0)
    USER_FUNC(evt_map_set_fog, 2, 400, 1000, 0, 10, 40)
    USER_FUNC(evt_map_fog_onoff, 1)
    USER_FUNC(evt_mario_key_onoff, 0)
    USER_FUNC(evt_mario_normalize)
    USER_FUNC(evt_npc_set_position, PTR(kPitNpcName), 0, 10, 0)
    USER_FUNC(evt_npc_set_anim, PTR(kPitNpcName), PTR("CBN_S_1"))
    USER_FUNC(evt_npc_set_position, PTR(kPitBossNpc2Name), 150, 0, -150)
    USER_FUNC(evt_npc_set_anim, PTR(kPitBossNpc2Name), PTR("GNB_H_3"))
    USER_FUNC(evt_cam3d_evt_set, -480, 150, 459, -480, 67, -13, 0, 11)
    USER_FUNC(evt_mario_set_pos, -560, 10, 0)
    USER_FUNC(evt_party_set_pos, 0, -600, 10, 0)
    USER_FUNC(evt_seq_wait, 2)

    INLINE_EVT()
        SET(LW(1), 0)
        RUN_EVT(bero_close_door_play_se)
        USER_FUNC(evt_sub_intpl_msec_init, 11, 180, 0, 500)
        DO(0)
            USER_FUNC(evt_sub_intpl_msec_get_value)
            RUN_CHILD_EVT(Tower_BossDoor_Close)
            IF_EQUAL(LW(1), 0)
                DO_BREAK()
            END_IF()
        WHILE()
        USER_FUNC(evt_cam_shake, 4, FLOAT(0.01), FLOAT(0.00), 200)
        USER_FUNC(evt_cam_shake, 4, FLOAT(0.01), FLOAT(0.00), 100)
    END_INLINE()
    INLINE_EVT()
        USER_FUNC(evt_party_move_pos2, 0, -490, 0, FLOAT(120.0))
    END_INLINE()
    USER_FUNC(evt_mario_mov_pos2, -450, 0, FLOAT(120.0))
    RUN_EVT(bero_case_entry)
    WAIT_MSEC(1300)

    // Print dragon already-felled message.
    USER_FUNC(evtTot_SetConversation, (int32_t)ConversationId::HOOK_F3)
    USER_FUNC(evtTot_GetNextMessage, LW(0), LW(1))
    USER_FUNC(evt_msg_print, 0, LW(0), 0, 0)

    USER_FUNC(evt_mario_get_pos, 0, LW(0), LW(1), LW(2))
    USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_VOICE_MARIO_SURPRISED2_3"), LW(0), LW(1), LW(2), 0)
    USER_FUNC(evt_eff_fukidashi, 0, PTR(""), 0, 0, 0, 0, 0, 0, 0, 0, 96)
    USER_FUNC(evt_eff_fukidashi, 3, PTR(""), 0, 0, 0, 0, 0, 0, 0, 0, 96)
    USER_FUNC(evt_mario_set_pose, PTR("M_N_5B"))
    WAIT_MSEC(2000)
    USER_FUNC(evt_mario_set_pose, PTR("M_S_1"))
    USER_FUNC(evt_mario_set_dir_npc, PTR(kPitNpcName))
    USER_FUNC(evt_party_set_dir_npc, 0, PTR(kPitNpcName))

    // Pan camera over.
    RUN_EVT_ID(Tower_GoldFuzzyIdleSfx, LW(15))
    USER_FUNC(evt_cam3d_evt_set, 0, 75, 230, 0, 25, -70, 250, 11)
    WAIT_MSEC(500)
    // Print Gold Fuzzy message.
    USER_FUNC(evtTot_SetConversation, (int32_t)ConversationId::SBOSS_F1)
    USER_FUNC(evtTot_GetNextMessage, LW(0), LW(1))
    USER_FUNC(evt_msg_print, 0, LW(0), 0, PTR(kPitNpcName))

    DELETE_EVT(LW(15))
    BROTHER_EVT()
        // Slow camera pan.
        USER_FUNC(evt_cam3d_evt_set, -450, 75, 230, -450, 25, -70, 2000, 11)
    END_BROTHER()

    // Should be three quick, low hops and a higher jump.
    USER_FUNC(evt_npc_set_anim, PTR(kPitNpcName), PTR("CBN_W_1"))
    USER_FUNC(evt_npc_get_position, PTR(kPitNpcName), LW(0), LW(1), LW(2))
    DO(3)
        USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_STG1_CHORO_MOVE1"), LW(0), LW(1), LW(2), 0)
        SUB(LW(0), 115)
        USER_FUNC(evt_npc_jump_position_nohit, PTR(kPitNpcName), LW(0), LW(1), LW(2), 500, FLOAT(35.0))
    WHILE()
    BROTHER_EVT()
        WAIT_MSEC(160)
        USER_FUNC(evt_mario_get_pos, 0, LW(0), LW(1), LW(2))
        USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_VOICE_MARIO_SURPRISED2_2"), LW(0), LW(1), LW(2), 0)
        USER_FUNC(evt_mario_set_pose, PTR("M_N_7"))
    END_BROTHER()
    USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_STG1_CHORO_MOVE1"), LW(0), LW(1), LW(2), 0)
    SET(LW(0), -445)
    ADD(LW(1), 5)
    USER_FUNC(evt_npc_jump_position_nohit, PTR(kPitNpcName), LW(0), LW(1), LW(2), 950, FLOAT(75.0))

    USER_FUNC(evt_npc_battle_start, PTR(kPitNpcName))
    
    // Handle game over sequence.
    USER_FUNC(evt_npc_wait_battle_end)
    USER_FUNC(evt_npc_get_battle_result, LW(0))
    IF_EQUAL(LW(0), 3)
        // Turn on red fog, despawn NPCs, and point at player.
        USER_FUNC(evt_map_set_fog, 2, 400, 1000, 40, 0, 20)
        USER_FUNC(evt_map_fog_onoff, 1)
        USER_FUNC(evt_npc_set_position, PTR(kPitNpcName), 0, -1000, 0)
        USER_FUNC(evt_npc_set_position, PTR(kPitBossNpc2Name), 0, -1000, 0)
        USER_FUNC(evt_cam3d_evt_off, 0, 11)
        RUN_CHILD_EVT(&Tower_GameOverScript)
        RETURN()
    END_IF()
    
    // Otherwise, play boss death animation.

    // Spawn Mario + partner in room, dragon already felled in background.
    USER_FUNC(evt_mario_set_pose, PTR("M_S_1"))
    SET(LW(13), PTR(kPitBossNpc2Name))
    USER_FUNC(evt_cam3d_evt_set, 0, 30, 842, 0, 135, 150, 0, 11)
    USER_FUNC(evt_npc_set_position, LW(13), 0, 0, -140)
    USER_FUNC(evt_npc_set_anim, LW(13), PTR("GNB_H_3"))
    USER_FUNC(evt_map_fog_onoff, 0)
    USER_FUNC(evt_mario_set_pos, 25, 10, 150)
    USER_FUNC(evt_mario_set_dir, 270, 0, 0)
    USER_FUNC(evt_party_set_pos, 0, -25, 10, 150)
    USER_FUNC(evt_party_set_dir, 0, 90, -1)

    // Make Fuzzy do regular death animation.
    SET(LW(13), PTR(kPitNpcName))
    USER_FUNC(evt_npc_set_position, LW(13), 0, 5, 0)
    USER_FUNC(evt_npc_set_anim, LW(13), PTR("CBN_Y_1"))
    WAIT_MSEC(550)
    // Print Gold Fuzzy death message.
    USER_FUNC(evtTot_SetConversation, (int32_t)ConversationId::SBOSS_F3)
    USER_FUNC(evtTot_GetNextMessage, LW(0), LW(1))
    USER_FUNC(evt_msg_print, 0, LW(0), 0, PTR(kPitNpcName))
    WAIT_MSEC(300)
    USER_FUNC(evt_npc_get_position, LW(13), LW(0), LW(1), LW(2))
    USER_FUNC(evt_eff, 
        0, PTR("kemuri_test"), 4, LW(0), LW(1), LW(2), 
        FLOAT(0.80), 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(evt_snd_sfxon, PTR("SFX_ENEMY_DIE1"), 0)
    USER_FUNC(evt_npc_delete, LW(13))
    USER_FUNC(evt_npc_return_interrupt, LW(13))

    // Handle stopping timer, triggering achievements, results screens, etc.
    SET(LW(15), 2000)
    RUN_CHILD_EVT(Tower_VictorySequence)

    RETURN()
EVT_END()

// Set up NPC and run async script for final boss floor.
EVT_BEGIN(Tower_FinalBossSetup)
    // Get battle / NPC info.
    USER_FUNC(evtTot_GetGonBattleDatabasePtr, LW(0))
    SET(LW(1), PTR(g_EnemyNpcSetup))
    USER_FUNC(evtTot_GetEnemyNpcInfo,
        LW(0), LW(1), LW(2), LW(3), LW(4), LW(5), LW(6), LW(7))

    // If in EX mode, a second NPC is needed for swapping to Bonetail.
    SET(LW(9), PTR(npcTribe[NpcTribeType::BONETAIL].modelName))
    SET(LW(10), PTR(npcTribe[NpcTribeType::BONETAIL].nameJp))
    // If using alternate boss, second NPC should be the regular boss instead.
    IF_EQUAL(LW(7), 1)
        USER_FUNC(evtTot_GetDifficulty, LW(8))
        SWITCH(LW(8))
            CASE_EQUAL((int32_t)OPTVAL_DIFFICULTY_HALF)
                SET(LW(9), PTR(npcTribe[NpcTribeType::HOOKTAIL].modelName))
                SET(LW(10), PTR(npcTribe[NpcTribeType::HOOKTAIL].nameJp))
            CASE_ETC()
                SET(LW(9), PTR(npcTribe[NpcTribeType::GLOOMTAIL].modelName))
                SET(LW(10), PTR(npcTribe[NpcTribeType::GLOOMTAIL].nameJp))
        END_SWITCH()
    END_IF()
    
    // Set up main NPC, and Bonetail NPC to swap to for EX difficulty.
    USER_FUNC(evt_npc_entry, PTR(kPitNpcName), LW(2))
    USER_FUNC(evt_npc_set_tribe, PTR(kPitNpcName), LW(3))
    USER_FUNC(evt_npc_entry, PTR(kPitBossNpc2Name), LW(9))
    USER_FUNC(evt_npc_set_tribe, PTR(kPitBossNpc2Name), LW(10))
    USER_FUNC(evt_npc_setup, LW(1))

    USER_FUNC(evt_npc_set_position, PTR(kPitNpcName), 0, 0, 0)
    USER_FUNC(evtTot_SetEnemyNpcBattleInfo, PTR(kPitNpcName), /* battle id */ 1)
    USER_FUNC(evt_npc_set_anim, PTR(kPitNpcName), PTR("GNB_H_3"))
    USER_FUNC(evt_npc_flag_onoff, 1, PTR(kPitNpcName), 33554496)
    USER_FUNC(evt_npc_pera_onoff, PTR(kPitNpcName), 0)
    USER_FUNC(evt_npc_set_ry, PTR(kPitNpcName), 0)
    IF_EQUAL(LW(7), 0)
        USER_FUNC(
            evt_npc_set_scale, PTR(kPitNpcName), FLOAT(1.3), FLOAT(1.3), FLOAT(1.3))
    END_IF()

    USER_FUNC(evt_npc_set_position, PTR(kPitBossNpc2Name), 0, -1000, 0)
    USER_FUNC(evt_npc_set_anim, PTR(kPitBossNpc2Name), PTR("GNB_H_3"))
    USER_FUNC(evt_npc_flag_onoff, 1, PTR(kPitBossNpc2Name), 33554496)
    USER_FUNC(evt_npc_pera_onoff, PTR(kPitBossNpc2Name), 0)
    USER_FUNC(evt_npc_set_ry, PTR(kPitBossNpc2Name), 0)
    USER_FUNC(evt_npc_set_scale, PTR(kPitBossNpc2Name), FLOAT(1.3), FLOAT(1.3), FLOAT(1.3))
    
    IF_EQUAL(LW(7), 0)
        RUN_EVT(&Tower_FinalBossEvent)
    ELSE()
        RUN_EVT(&Tower_GoldFuzzyBossEvent)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(Tower_WaitForChestOpen)
    INLINE_EVT()
        LBL(2)
        WAIT_FRM(1)

        // Quit early if countdown timer is elapsed.
        IF_LARGE_EQUAL((int32_t)GSW_CountdownTimerState, 2)
            RETURN()
        END_IF()

        IF_EQUAL((int32_t)GSW_Tower_ChestClaimed, 0)
            IF_LARGE_EQUAL((int32_t)GSW_Tower_DisplayChestIcons, 1)
                USER_FUNC(evtTot_DisplayChestIcons)
            END_IF()
            GOTO(2)
        END_IF()
        RUN_EVT(Tower_OpenExit)
    END_INLINE()

    RETURN()
EVT_END()

// Set up NPCs (only runs on non-rest floors).
EVT_BEGIN(Tower_EnemySetup)
    USER_FUNC(evtTot_IsRestFloor, LW(0))
    IF_EQUAL(LW(0), 0)
        // Regular enemy floor; spawn enemies.
        USER_FUNC(evtTot_GetGonBattleDatabasePtr, LW(0))
        SET(LW(1), PTR(g_EnemyNpcSetup))
        USER_FUNC(evtTot_GetEnemyNpcInfo,
            LW(0), LW(1), LW(2), LW(3), LW(4), LW(5), LW(6), EVT_NULLPTR)
        
        USER_FUNC(evt_npc_entry, PTR(kPitNpcName), LW(2))
        USER_FUNC(evt_npc_set_tribe, PTR(kPitNpcName), LW(3))
        USER_FUNC(evt_npc_setup, LW(1))

        // Set scale to 1x for normal enemies, 2x for midbosses (not Atomic Boo).
        // TODO: Change '!' height.
        USER_FUNC(evtTot_IsMidbossFloor, LW(8))
        ADD(LW(8), 1)
        USER_FUNC(evt_npc_set_scale, PTR(kPitNpcName), LW(8), LW(8), LW(8))

        USER_FUNC(evt_npc_set_position, PTR(kPitNpcName), LW(4), LW(5), LW(6))
        
        IF_STR_EQUAL(LW(3), npcTribe[NpcTribeType::PIDER].nameJp)
            USER_FUNC(evt_npc_set_home_position, PTR(kPitNpcName), LW(4), LW(5), LW(6))
        END_IF()
        IF_STR_EQUAL(LW(3), npcTribe[NpcTribeType::ARANTULA].nameJp)
            USER_FUNC(evt_npc_set_home_position, PTR(kPitNpcName), LW(4), LW(5), LW(6))
        END_IF()
        
        USER_FUNC(evtTot_SetEnemyNpcBattleInfo, PTR(kPitNpcName), /* battle id */ 0)
        
        // Asynchronously check for battle failure state.
        RUN_EVT(Tower_CheckGameOver)
        
        // Wait for the enemy to be defeated, then spawn chests.
        INLINE_EVT()
            LBL(1)
            WAIT_FRM(1)
            USER_FUNC(evt_npc_status_check, PTR(kPitNpcName), 4, LW(0))
            IF_EQUAL(LW(0), 0)
                GOTO(1)
            END_IF()
            SET(LW(15), 0)

            // Don't run grate opening animation if countdown timer is elapsed.
            IF_SMALL((int32_t)GSW_CountdownTimerState, 2)
                RUN_EVT(Tower_OpenGrate)
            END_IF()
        END_INLINE()
    
        // Wait for a chest to be opened, then unlock the exit (if necessary).
        RUN_CHILD_EVT(Tower_WaitForChestOpen)
    END_IF()
    RETURN()
EVT_END()

// Set up NPCs (only runs on rest floors).
EVT_BEGIN(Tower_NpcSetup)
    USER_FUNC(evtTot_IsRestFloor, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        // Rest floor; open grate immediately.
        SET(LW(15), 1)
        RUN_EVT(Tower_OpenGrate)
        
        USER_FUNC(evtTot_ClearEnemyInfo)
        
        // Spawn one or more NPCs as well, if floor > 0.
        USER_FUNC(evtTot_GetFloor, LW(0))
        IF_LARGE(LW(0), 0)
            USER_FUNC(evtTot_GetTowerNpcParams,
                LW(0), LW(1), LW(2),  // Charlieton name, tribe name, model
                LW(3), LW(4), LW(5),  // Secondary NPC name, tribe name, model
                LW(6))
            USER_FUNC(evt_npc_entry, LW(0), LW(2))
            USER_FUNC(evt_npc_set_tribe, LW(0), LW(1))
            IF_NOT_EQUAL(LW(3), 0)
                USER_FUNC(evt_npc_entry, LW(3), LW(5))
                USER_FUNC(evt_npc_set_tribe, LW(3), LW(4))
            END_IF()
            USER_FUNC(evt_npc_set_position, LW(0), 160, 0, -70)
            USER_FUNC(evt_npc_set_ry_lr, LW(0), 0)
            IF_NOT_EQUAL(LW(3), 0)
                USER_FUNC(evt_npc_set_position, LW(3), 160, 0, 70)
                USER_FUNC(evt_npc_set_ry_lr, LW(3), 0)
            END_IF()
            USER_FUNC(evt_npc_setup, LW(6))

            // Also spawn a Heart Block, statically.
            SET(LW(15), 1)
            RUN_EVT(Tower_SpawnHeartSaveBlock)
        END_IF()

        // Wait for a chest to be opened, then unlock the exit (if necessary).
        RUN_CHILD_EVT(Tower_WaitForChestOpen)
    END_IF()
    RETURN()
EVT_END()

// Prompts if the player wants to give up if they check the left door.
EVT_BEGIN(Tower_LeaveEarly)
    USER_FUNC(evt_mario_key_onoff, 0)
    USER_FUNC(evt_msg_print, 0, PTR("tot_quitearly"), 0, PTR("me"))
    USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
    USER_FUNC(evt_msg_continue)
    // Chose "continue" option.
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(evt_mario_key_onoff, 1)
    ELSE()
        // Despawn partner.
        USER_FUNC(evt_mario_goodbye_party, 0)
        // Reload into lobby.
        USER_FUNC(evtTot_SetPreviousPartner, 0)
        USER_FUNC(evt_fade_set_mapchange_type, 0, 2, 300, 1, 300)
        USER_FUNC(evt_bero_mapchange, PTR("gon_00"), PTR("w_bero"))
    END_IF()
    RETURN()
EVT_END()

// Setting up loading zones, etc.
EVT_BEGIN(Tower_BeroSetup)
    // Only run partial logic on final floor.
    USER_FUNC(evtTot_IsFinalFloor, LW(0))
    IF_EQUAL(LW(0), 1)
        SET(LW(0), PTR(&boss_room_entry_data))
        USER_FUNC(evt_bero_get_info)
        RETURN()
    END_IF()

    SET(LW(0), PTR(&normal_room_entry_data))
    USER_FUNC(evt_bero_get_info)
    RUN_CHILD_EVT(PTR(&evt_bero_info_run))

    // Disable left loading zone, and have prompt to quit early.
    SET(LW(0), PTR("w_bero"))
    RUN_CHILD_EVT(bero_case_switch_off)
    USER_FUNC(evt_run_case_evt, 9, 1, PTR("w_bero"), 0, PTR(&Tower_LeaveEarly), 0)
    RETURN()
EVT_END()

// Main room initialization event.
EVT_BEGIN(gon_01_InitEvt)
    SET((int32_t)GSW_Tower_ChestClaimed, 0)
    SET((int32_t)GSW_Tower_DisplayChestIcons, 0)
    SET((int32_t)GSW_Tower_RevealUsedThisFloor, 0)
    // Flags to track chest / lock opening.
    SET((int32_t)GSWF_Chest_0, 0)
    SET((int32_t)GSWF_Chest_1, 0)
    SET((int32_t)GSWF_Chest_2, 0)
    SET((int32_t)GSWF_Chest_3, 0)
    SET((int32_t)GSWF_Lock, 0)
    
    // For normal play (not continuing from a Game Over)...
    IF_EQUAL((int32_t)GSW_Tower_ContinuingFromGameOver, 0)
        // Set up NPCs, enemies, etc. ...
        USER_FUNC(evtTot_IsFinalFloor, LW(0))
        IF_EQUAL(LW(0), 1)
            RUN_CHILD_EVT(PTR(&Tower_FinalBossSetup))
        ELSE()
            // Set up enemies, then chests, then regular NPCs.
            RUN_CHILD_EVT(PTR(&Tower_EnemySetup))
            RUN_CHILD_EVT(PTR(&Tower_SpawnChests))
            RUN_CHILD_EVT(PTR(&Tower_NpcSetup))
        END_IF()
        
        // Set up loading zones.
        RUN_CHILD_EVT(PTR(&Tower_BeroSetup))

        // Put lock on right door until enemies beaten + chests claimed.
        USER_FUNC(evtTot_IsFinalFloor, LW(0))
        IF_EQUAL(LW(0), 0)
            USER_FUNC(evt_mobj_lock, PTR("mobj_lock_00"), 12, /* dummy key */
                190, 10, 0, 270,  /* position + rotation */
                PTR(&Tower_Lock_Check), PTR(&Tower_Lock_Unlock),
                (int32_t)GSWF_Lock)
            
            SET(LW(0), PTR("e_bero"))
            RUN_CHILD_EVT(bero_case_switch_off)
        END_IF()
    ELSE()
        // Continuing from a Game Over...
        // Set up loading zones.
        RUN_CHILD_EVT(PTR(&Tower_BeroSetup))
        // Spawn Heart + Save Block statically.
        USER_FUNC(evtTot_GetFloor, LW(15))
        IF_LARGE(LW(15), 0)
            SET(LW(15), 1)
            RUN_EVT(Tower_SpawnHeartSaveBlock)
        END_IF()
        // Open grate statically.
        SET(LW(15), 1)
        RUN_EVT(Tower_OpenGrate)

        SET((int32_t)GSW_Tower_ContinuingFromGameOver, 0)
        SET((int32_t)GSW_Tower_ChestClaimed, 1)
        SET((int32_t)GSW_Tower_DisplayChestIcons, 2)
    END_IF()
        
    // Dynamically update the destination of the exit loading zone.
    USER_FUNC(evtTot_UpdateDestinationMap)
    
    USER_FUNC(evtTot_IsFinalFloor, LW(0))
    IF_EQUAL(LW(0), 0)
        // Non-boss room music / env sound:
        USER_FUNC(evt_snd_bgmon, 512, PTR("BGM_STG1_GON1"))
        USER_FUNC(evt_snd_envoff, 512)
        USER_FUNC(evt_snd_set_rev_mode, 1)

        // Spawn signboard with floor number as reminder.
        USER_FUNC(
            evt_mobj_signboard, PTR("board"), -100, 0, -140, 
            PTR(&Tower_SignEvt), LSWF(0))
    ELSE()
        // Boss room music / env sound:
        USER_FUNC(evt_snd_bgmoff, 512)
        USER_FUNC(evt_snd_envon, 272, PTR("ENV_STG0_DAN1"))
        USER_FUNC(evt_snd_set_rev_mode, 2)
    END_IF()
    
    // If countdown timer is enabled, spawn event waiting for it to hit 0.
    IF_LARGE((int32_t)GSW_CountdownTimerState, 0)
        RUN_EVT(Tower_CheckTimeUp)
    END_IF()
    
    USER_FUNC(evtTot_ToggleIGT, 1)

    RETURN()
EVT_END()

BeroEntry normal_room_entry_data[3] = {
    {
        .name = "w_bero",
        .type = BeroType::DOOR,
        .sfx_id = 0xC,
        .direction = BeroDirection::AUTO,
        .center_position = { 100000, 0, 0 },
        .length = -1,
        .entry_evt_code = nullptr,
        .case_type = -1,
        .out_evt_code = nullptr,
        .target_map = nullptr,
        .target_bero = nullptr,
        .entry_anim_type = BeroAnimType::REPEATED_EVT,
        .out_anim_type = BeroAnimType::REPEATED_EVT,
        .entry_anim_args = (void*)Tower_LeftExit_Close,
        .out_anim_args = (void*)Tower_LeftExit_Open,
    }, {
        .name = "e_bero",
        .type = BeroType::DOOR,
        .sfx_id = 0xC,
        .direction = BeroDirection::AUTO,
        .center_position = { 100000, 0, 0 },
        .length = -1,
        .entry_evt_code = nullptr,
        .case_type = -1,
        .out_evt_code = (void*)Tower_IncrementFloor,
        .target_map = "gon_01",
        .target_bero = "w_bero",
        .entry_anim_type = BeroAnimType::REPEATED_EVT,
        .out_anim_type = BeroAnimType::REPEATED_EVT,
        .entry_anim_args = (void*)Tower_RightExit_Close,
        .out_anim_args = (void*)Tower_RightExit_Open,
    }, { /* null-terminator */ },
};

BeroEntry boss_room_entry_data[2] = {
    {
        .name = "w_bero",
        .type = BeroType::DOOR,
        .sfx_id = 0xD,
        .direction = BeroDirection::AUTO,
        .center_position = { 100000, 0, 0 },
        .length = -1,
        .entry_evt_code = nullptr,
        .case_type = -1,
        .out_evt_code = nullptr,
        .target_map = nullptr,
        .target_bero = nullptr,
        .entry_anim_type = BeroAnimType::REPEATED_EVT,
        .out_anim_type = BeroAnimType::REPEATED_EVT,
        .entry_anim_args = (void*)Tower_BossDoor_Close,
        .out_anim_args = (void*)Tower_BossDoor_Open,
    }, { /* null-terminator */ },
};

const int32_t* GetTowerInitEvt() {
    return gon_01_InitEvt;
}

void UpdateDestinationMap() {
    ttyd::evtmgr::evtEntry(const_cast<int32_t*>(Tower_UpdateDestinationEvt), 0, 0);
}

EVT_DEFINE_USER_FUNC(evtTot_AwardWinnings) {
    auto& state = g_Mod->state_;
    state.ChangeOption(
        STAT_PERM_CURRENT_COINS, state.GetOption(STAT_RUN_META_COINS_EARNED));
    state.ChangeOption(
        STAT_PERM_CURRENT_SP, state.GetOption(STAT_RUN_META_SP_EARNED));
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_HasBackupSave) {
    evtSetValue(evt, evt->evtArguments[0], g_Mod->state_.HasBackupSave());
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_LoadBackupSaveData) {
    auto& state = g_Mod->state_;
    state.Load(state.GetBackupSave());
    return 2;
}

// Clear the negative result of the last battle.
EVT_DEFINE_USER_FUNC(evtTot_ClearBattleResult) {
    auto* fbat = (ttyd::npcdrv::FbatData*)ttyd::npcdrv::fbatGetPointer();
    fbat->battleInfo.wResult = 0;
    return 2;
}

// Returns a table with the preferred key type to open a given lock.
EVT_DEFINE_USER_FUNC(evtTot_MakeKeyTable) {
    static int32_t keys[] = { -1, -1, -1 };
    int32_t index = 0;
    if (evtGetValue(evt, evt->evtArguments[0]))
        keys[index++] = ItemType::TOT_TOWER_KEY;
    if (!index && evtGetValue(evt, evt->evtArguments[1]))
        keys[index++] = ItemType::TOT_MASTER_KEY;
    for (; index < 3; ++index)
        keys[index] = -1;
    evtSetValue(evt, evt->evtArguments[2], PTR(&keys[0]));
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_CheckReroll) {
    int32_t result = 0;

    uint32_t buttons = ttyd::system::keyGetButtonTrg(0);

    if (g_Mod->state_.GetOption(STAT_RUN_CHEST_REROLLS) &&
        (buttons & ButtonId::L) && !(buttons & ButtonId::A)) {
            
        if (g_Mod->state_.CheckOptionValue(OPTVAL_CHEST_REROLL_FIXED) ||
            g_Mod->state_.CheckOptionValue(OPTVAL_CHEST_REROLL_REFILL)) {
            g_Mod->state_.ChangeOption(STAT_RUN_CHEST_REROLLS, -1);
            g_Mod->state_.ChangeOption(STAT_PERM_CHEST_REROLLS, 1);
            result = 1;
        } else if (!GetSWByte(GSW_Tower_RevealUsedThisFloor)) {
            g_Mod->state_.ChangeOption(STAT_RUN_CHEST_REROLLS, -1);
            g_Mod->state_.ChangeOption(STAT_PERM_CHEST_REROLLS, 1);
            SetSWByte(GSW_Tower_RevealUsedThisFloor, 1);
            result = 2;
        }
    }
    evtSetValue(evt, evt->evtArguments[0], result);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_OverrideLockKey) {
    void* lock = ttyd::mobjdrv::mobjNameToPtrNoAssert("mobj_lock_00");
    if (lock) {
        int32_t key_type = evtGetValue(evt, evt->evtArguments[0]);
        *(int32_t*)((uintptr_t)lock + 0x1a0) = key_type;
    }
    return 2;
}

// Overrides the previous party member that was out.
EVT_DEFINE_USER_FUNC(evtTot_SetPreviousPartner) {
    auto* player = ttyd::mario::marioGetPtr();
    player->prevFollowerId[0] = 0;
    return 2;
}

// Wait for the dragon to hit the ground.
EVT_DEFINE_USER_FUNC(evtTot_WaitForDragonLanding) {
    auto npc_name = (const char*)evtGetValue(evt, evt->evtArguments[0]);
    ttyd::npcdrv::NpcEntry* npc = ttyd::npcdrv::npcNameToPtr(npc_name);
    return npc->wKpaMobjDeadCheck ? 2 : 0;
}

// Wait for Mario to not have his controls disabled.
EVT_DEFINE_USER_FUNC(evtTot_WaitForPlayerActionable) {
    return ttyd::mario::marioKeyOffChk() ? 0 : 2;
}

EVT_DEFINE_USER_FUNC(evtTot_WaitNpcAnimFrame) {
    auto npc_name = (const char*)evtGetValue(evt, evt->evtArguments[0]);
    int32_t frame = evtGetValue(evt, evt->evtArguments[1]);
    auto* npc = ttyd::npcdrv::npcNameToPtr(npc_name);
    float cur_frame = 
        ttyd::animdrv::animPoseGetAnimPosePtr(npc->poseId)->anim_frame;

    if (frame < 0) {
        // Block until loop number changes.
        int32_t loop_cnt = ttyd::animdrv::animPoseGetLoopTimes(npc->poseId);
        if (isFirstCall) {
            evt->lwData[15] = loop_cnt;
        }
        return evt->lwData[15] != loop_cnt ? 2 : 0;
    }
    return cur_frame >= frame ? 2 : 0;
}

EVT_DEFINE_USER_FUNC(evtTot_GetContinueDestinationMap) {
    static const char* kMaps[] = {
        "gon_01", "gon_02", "gon_03", "gon_04"
    };
    int32_t map_index = (g_Mod->state_.floor_ - 1) / 16;
    evtSetValue(evt, evt->evtArguments[0], PTR(kMaps[map_index]));
    return 2;
}

// Dynamically change the destination of the exit pipe.
//
// Note: This is probably overkill; I wasn't running the entry event in testing.
// It's probably sufficient to update normal_room_entry_data on leaving a floor.
EVT_DEFINE_USER_FUNC(evtTot_UpdateDestinationMap) {
    const char* exit_bero = "e_bero";

    int32_t floor = g_Mod->state_.floor_;

    static const char* kMaps[] = {
        "gon_01", "gon_02", "gon_03", "gon_04", "gon_05"
    };
    int32_t map_idx = 0;
    if (g_Mod->state_.IsFinalBossFloor(floor + 1)) {
        // If next floor is boss floor, point to final boss room.
        map_idx = 4;
    } else if (floor > 0) {
        // Floor 16-31 should point to second, 32-47 to third, etc.
        map_idx = floor / 16;
    }

    const char* next_map = kMaps[map_idx];
    const char* next_bero = "w_bero";
    
    // Update the destination of the exit loading zone to match the floor.
    BeroEntry** entries = ttyd::evt_bero::BeroINFOARR;
    for (int32_t i = 0; i < 16; ++i) {
        if (entries[i] && !strcmp(entries[i]->name, exit_bero)) {
            entries[i]->target_map = next_map;
            entries[i]->target_bero = next_bero;
            break;
        }
    }
    return 2;
}

}  // namespace mod::tot::gon