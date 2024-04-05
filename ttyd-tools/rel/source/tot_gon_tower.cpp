#include "tot_gon_tower.h"

#include "evt_cmd.h"
#include "mod.h"
#include "tot_generate_enemy.h"
#include "tot_generate_item.h"
#include "tot_generate_reward.h"
#include "tot_gon.h"
#include "tot_gon_tower_npcs.h"
#include "tot_manager_timer.h"
#include "tot_state.h"
#include "tot_window_select.h"

#include <gc/types.h>
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
#include <ttyd/npcdrv.h>
#include <ttyd/swdrv.h>

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
using ::ttyd::npcdrv::NpcSetupInfo;

namespace BeroAnimType = ::ttyd::evt_bero::BeroAnimType;
namespace BeroDirection = ::ttyd::evt_bero::BeroDirection;
namespace BeroType = ::ttyd::evt_bero::BeroType;
namespace ItemType = ::ttyd::item_data::ItemType;

const char kPitNpcName[] = "\x93\x47";  // "enemy"
const char kPiderName[] = "\x83\x70\x83\x43\x83\x5f\x81\x5b\x83\x58";
const char kArantulaName[] = 
    "\x83\x60\x83\x85\x83\x89\x83\x93\x83\x5e\x83\x89\x81\x5b";

// Info for custom NPCs.
NpcSetupInfo g_EnemyNpcSetup[2];

}  // namespace

// USER_FUNC Declarations.
EVT_DECLARE_USER_FUNC(evtTot_ClearBattleResult, 0)
EVT_DECLARE_USER_FUNC(evtTot_HasBackupSave, 1)
EVT_DECLARE_USER_FUNC(evtTot_LoadBackupSaveData, 0)
EVT_DECLARE_USER_FUNC(evtTot_IsFinalFloor, 1)
EVT_DECLARE_USER_FUNC(evtTot_IsRestFloor, 1)
EVT_DECLARE_USER_FUNC(evtTot_SetPreviousPartner, 1)
EVT_DECLARE_USER_FUNC(evtTot_UpdateDestinationMap, 0)
EVT_DECLARE_USER_FUNC(evtTot_WaitForDragonLanding, 0)

// Other declarations.
extern BeroEntry normal_room_entry_data[3];

// Script for changing room destinations dynamically.
EVT_BEGIN(Tower_UpdateDestinationEvt)
    SET(LW(0), PTR("dokan_1"))
    RUN_CHILD_EVT(bero_case_switch_off)
    USER_FUNC(evtTot_UpdateDestinationMap)
    SET(LW(0), PTR("dokan_1"))
    RUN_CHILD_EVT(bero_case_switch_on)
    RETURN()
EVT_END()

// Script for sign that shows current floor.
EVT_BEGIN(Tower_SignEvt)
    USER_FUNC(evt_npc_stop_for_event)
    USER_FUNC(evt_mario_key_onoff, 0)
    USER_FUNC(evtTot_GetSeed, LW(0))
    USER_FUNC(evtTot_GetFloor, LW(1))
    USER_FUNC(evt_msg_print_insert, 0, PTR("tot_floor_sign"), 0, 0, LW(0), LW(1))
    USER_FUNC(evt_mario_key_onoff, 1)
    USER_FUNC(evt_npc_start_for_event)
    RETURN()
EVT_END()

// Script that runs when opening a chest.
EVT_BEGIN(Tower_ChestEvt_Core)
    // Don't allow getting a reward from more than one chest.
    IF_EQUAL(GSW(1000), 1)
        RETURN()
    END_IF()
    // Disable drawing item icons above chests.
    SET(GSW(1001), 2)
    USER_FUNC(evt_mario_key_onoff, 0)
    USER_FUNC(evtTot_GetChestData, LW(9), LW(10), LW(11), LW(12), LW(13), LW(14))
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
            // Run special event from tot_generate_reward, then null it out
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
        SET(GSW(1000), 1)
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

// Setting up loading zones, etc. for regular room.
EVT_BEGIN(Tower_BeroSetupNormal)
    SET(LW(0), PTR(&normal_room_entry_data))
    USER_FUNC(evt_bero_get_info)
    RUN_CHILD_EVT(PTR(&evt_bero_info_run))
    RETURN()
EVT_END()

// Increment floor number.
EVT_BEGIN(Tower_IncrementFloor)
    USER_FUNC(evtTot_ToggleIGT, 0)
    USER_FUNC(evtTot_IncrementFloor, 1)
    SET(LW(0), 0)
    RETURN()
EVT_END()

// Spawn heart block. LW(15) = Whether to spawn it statically on load.
EVT_BEGIN(Tower_SpawnHeartBlock)
    SET(LW(0), 40)
    SET(LW(1), 60)
    SET(LW(2), -100)
    IF_EQUAL(LW(15), 0)
        // Wait and play bomb effect if spawning dynamically.
        WAIT_MSEC(2000)
        USER_FUNC(
            evt_eff, PTR(""), PTR("bomb"), 0, LW(0), LW(1), LW(2), FLOAT(1.0),
            0, 0, 0, 0, 0, 0, 0)
        USER_FUNC(
            evt_snd_sfxon_3d, PTR("SFX_BOSS_RNPL_TRANSFORM4"), 
            -100, 60, -80, 0)
    END_IF()
    
    // Coin price = 10 + 5 for each boss floor after the first.
    USER_FUNC(evtTot_GetFloor, LW(3))
    DIV(LW(3), 8)
    ADD(LW(3), 1)
    MUL(LW(3), 5)
    // Spawn heart block.
    USER_FUNC(evt_mobj_recovery_blk, PTR("hbox"), LW(3), LW(0), LW(1), LW(2), 0, 0)

    RETURN()
EVT_END()

// Spawn chests. LW(15) = Whether to spawn them statically on load.
EVT_BEGIN(Tower_SpawnChests)
    IF_EQUAL(LW(15), 0)
        USER_FUNC(evt_mario_key_onoff, 0)
        WAIT_MSEC(2000)
    END_IF()
    USER_FUNC(evtTot_GenerateChestContents)
    
    SET(LW(10), 3)
    DO(LW(10))
        // Get contents of next chest, and break from loop if empty.
        SET(LW(11), 3)
        SUB(LW(11), LW(10))
        USER_FUNC(evtTot_GetChestData, LW(11), LW(0), LW(1), LW(2), LW(3), EVT_NULLPTR)
        IF_EQUAL(LW(3), 0)
            DO_BREAK()
        END_IF()
            
        IF_EQUAL(LW(15), 0)
            ADD(LW(1), 50)
            USER_FUNC(
                evt_eff, PTR(""), PTR("bomb"), 0, LW(0), LW(1), LW(2), FLOAT(1.0),
                0, 0, 0, 0, 0, 0, 0)
            USER_FUNC(
                evt_snd_sfxon_3d, PTR("SFX_BOSS_RNPL_TRANSFORM4"), 
                LW(0), LW(1), LW(2), 0)
            SUB(LW(1), 25)
        END_IF()
        
        SWITCH(LW(11))
            CASE_EQUAL(0)
                USER_FUNC(
                    evt_mobj_itembox, PTR("box_0"), LW(0), LW(1), LW(2), 1, 0, 
                    PTR(&Tower_ChestEvt_0), GSWF(5075))
            CASE_EQUAL(1)
                USER_FUNC(
                    evt_mobj_itembox, PTR("box_1"), LW(0), LW(1), LW(2), 1, 0, 
                    PTR(&Tower_ChestEvt_1), GSWF(5076))
            CASE_ETC()
                USER_FUNC(
                    evt_mobj_itembox, PTR("box_2"), LW(0), LW(1), LW(2), 1, 0, 
                    PTR(&Tower_ChestEvt_2), GSWF(5077))
        END_SWITCH()
    WHILE()

    IF_EQUAL(LW(15), 0)
        WAIT_MSEC(1000)
        USER_FUNC(evt_mario_key_onoff, 1)
    END_IF()
    SET(GSW(1001), 1)
    RETURN()
EVT_END()

// Spawn the exit to the floor.
EVT_BEGIN(Tower_SpawnPipe)
    USER_FUNC(evt_mario_key_onoff, 0)
    
    // Spawn pipe.
    WAIT_MSEC(500)
    SET(LW(2), PTR("a_dokan_1"))
    SET(LW(3), PTR("dokan_1_s"))
    USER_FUNC(evt_mapobj_get_position, LW(3), LW(7), LW(8), LW(9))
    USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_DOKAN_APPEAR1"), LW(7), LW(8), LW(9), 0)
    USER_FUNC(evt_hit_bind_mapobj, LW(2), LW(3))
    USER_FUNC(evt_sub_intpl_msec_init, 11, 0, 300, 500)
    LBL(0)
    USER_FUNC(evt_sub_intpl_msec_get_value)
    DIVF(LW(0), FLOAT(10.0))
    USER_FUNC(evt_mapobj_trans, 1, LW(3), 0, LW(0), 0)
    USER_FUNC(evt_hit_bind_update, LW(2))
    WAIT_FRM(1)
    IF_EQUAL(LW(1), 1)
        GOTO(0)
    END_IF()
    USER_FUNC(evt_mapobj_flag_onoff, 1, 1, PTR("dokan_1_k"), 1)

    // Spawn Heart Block on boss floors.
    USER_FUNC(evtTot_GetFloor, LW(10))
    IF_LARGE(LW(10), 0)
        MOD(LW(10), 8)
        IF_EQUAL(LW(10), 0)
            RUN_EVT(Tower_SpawnHeartBlock)
        END_IF()
    END_IF()
    
    // Delete chest objects.
    SET(LW(10), 3)
    DO(LW(10))
        SET(LW(11), 3)
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

EVT_BEGIN(Tower_RunGameOverScript)
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
    USER_FUNC(evt_msg_select, 0, PTR("tot_gameover_opt"))
    // Despawn partner.
    USER_FUNC(evt_mario_goodbye_party, 0)
    // Chose "continue" option.
    IF_EQUAL(LW(0), 0)
        USER_FUNC(evtTot_LoadBackupSaveData)
        // Resets floor-based stats without making saves, etc.
        USER_FUNC(evtTot_IncrementFloor, 0)
        // Set flag to spawn player in room with pipe already spawned.
        SET(GSW(1002), 1)
        USER_FUNC(evt_fade_set_mapchange_type, 0, 2, 300, 1, 300)
        USER_FUNC(evt_bero_mapchange, PTR("gon_01"), PTR("dokan_2"))
    ELSE()
        // Reload into lobby.
        USER_FUNC(evtTot_SetPreviousPartner, 0)
        USER_FUNC(evt_fade_set_mapchange_type, 0, 2, 300, 1, 300)
        USER_FUNC(evt_bero_mapchange, PTR("gon_00"), PTR("dokan_3"))
    END_IF()
    RETURN()
EVT_END()

// Checks for Game Over state on standard floors.
EVT_BEGIN(Tower_CheckGameOver)
    DO(0)
        USER_FUNC(evt_npc_get_battle_result, LW(0))
        IF_EQUAL(LW(0), 3)
            RUN_CHILD_EVT(&Tower_RunGameOverScript)
            RETURN()
        END_IF()
        WAIT_FRM(1)
    WHILE()
    RETURN()
EVT_END()

EVT_BEGIN(Tower_DragonFelledShake)
    USER_FUNC(evt_npc_get_position, PTR(kPitNpcName), LW(0), LW(1), LW(2))
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
    USER_FUNC(evt_map_set_fog, 2, 400, 1000, 0, 10, 40)
    USER_FUNC(evt_map_fog_onoff, 1)
    USER_FUNC(evt_mario_key_onoff, 0)
    USER_FUNC(evt_mario_normalize)
    USER_FUNC(evt_npc_set_position, PTR(kPitNpcName), 175, 0, 0)
    USER_FUNC(evt_npc_set_anim, PTR(kPitNpcName), PTR("GNB_S_1"))
    USER_FUNC(evt_mapobj_flag_onoff, 1, 1, PTR("oFF_jon_06"), 1)
    USER_FUNC(evt_seq_wait, 2)
    WAIT_MSEC(3500)
    USER_FUNC(evt_mario_get_pos, 0, LW(0), LW(1), LW(2))
    USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_VOICE_MARIO_SURPRISED2_3"), LW(0), LW(1), LW(2), 0)
    USER_FUNC(evt_eff_fukidashi, 0, PTR(""), 0, 0, 0, 0, 0, 0, 0, 0, 96)
    USER_FUNC(evt_eff_fukidashi, 3, PTR(""), 0, 0, 0, 0, 0, 0, 0, 0, 96)
    USER_FUNC(evt_mario_set_pose, PTR("M_N_5B"))
    INLINE_EVT()
        SET(LW(3), LW(0))
        ADD(LW(3), -40)
        USER_FUNC(evt_party_move_pos, 0, LW(3), LW(2), 250)
    END_INLINE()
    WAIT_MSEC(2000)
    USER_FUNC(evt_mario_set_pose, PTR("M_S_1"))
    USER_FUNC(evt_mario_set_dir_npc, PTR(kPitNpcName))
    USER_FUNC(evt_party_set_dir_npc, 0, PTR(kPitNpcName))
    USER_FUNC(evt_snd_bgmon, 512, PTR("BGM_EVT_GONBABA_FLY2"))
    INLINE_EVT()
        USER_FUNC(evt_snd_sfxon_3d_ex, PTR("SFX_STG1_GNB_ROAR2"), 0, 0, 0, 255, 255, 4300, 16, 0)
        USER_FUNC(evt_npc_set_anim, PTR(kPitNpcName), PTR("GNB_U_1"))
        USER_FUNC(evt_npc_wait_anim, PTR(kPitNpcName), FLOAT(1.0))
        USER_FUNC(evt_npc_set_anim, PTR(kPitNpcName), PTR("GNB_S_1"))
    END_INLINE()
    RUN_EVT(PTR(&Tower_DragonStandupShake))
    RUN_EVT(PTR(&Tower_DragonFogChange))
    USER_FUNC(evt_cam3d_evt_set, -755, 24, 376, -84, 114, -22, 4500, 11)
    WAIT_MSEC(4000)
    DO(30)
        USER_FUNC(evt_cam3d_evt_set, -755, 24, 376, -84, 114, -22, 15, 11)
        WAIT_MSEC(15)
        USER_FUNC(evt_cam3d_evt_set, -745, 23, 366, -84, 114, -22, 15, 11)
        WAIT_MSEC(15)
    WHILE()
    WAIT_MSEC(3000)
    USER_FUNC(evt_npc_battle_start, PTR(kPitNpcName))
    
    // Handle game over sequence.
    USER_FUNC(evt_npc_wait_battle_end)
    USER_FUNC(evt_npc_get_battle_result, LW(0))
    IF_EQUAL(LW(0), 3)
        RUN_CHILD_EVT(&Tower_RunGameOverScript)
        RETURN()
    END_IF()
    
    // Otherwise, play dragon death animation.
    USER_FUNC(evt_cam3d_evt_set, -769, 50, 527, -22, 114, 7, 1, 11)
    USER_FUNC(evt_npc_set_anim, PTR(kPitNpcName), PTR("GNB_H_1"))
    USER_FUNC(evt_map_fog_onoff, 0)
    USER_FUNC(evt_npc_set_position, PTR(kPitNpcName), 0, 60, 0)
    USER_FUNC(evt_mario_set_pos, 25, 10, 150)
    USER_FUNC(evt_party_set_pos, 0, -25, 10, 150)
    USER_FUNC(evt_mario_set_dir, 270, 0, 0)
    USER_FUNC(evt_party_set_dir, 0, 90, -1)
    RUN_EVT(PTR(&Tower_DragonFelledShake))
    INLINE_EVT()
        WAIT_MSEC(300)
        USER_FUNC(evt_npc_flag_onoff, 1, PTR(kPitNpcName), 131088)
        USER_FUNC(evtTot_WaitForDragonLanding)
        USER_FUNC(evt_npc_set_anim, PTR(kPitNpcName), PTR("GNB_H_2"))
    END_INLINE()
    USER_FUNC(evt_npc_get_position, PTR(kPitNpcName), LW(0), LW(1), LW(2))
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

    // Increment floor to lock timer.
    USER_FUNC(evtTot_ToggleIGT, 0)
    USER_FUNC(evtTot_IncrementFloor, 1)
    WAIT_MSEC(5000)
    
    // TODO: Flesh out victory animations, results, ...

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

    WAIT_MSEC(500)
    // Despawn partner.
    USER_FUNC(evt_mario_goodbye_party, 0)
    // Reload into lobby.
    USER_FUNC(evtTot_SetPreviousPartner, 0)
    USER_FUNC(evt_fade_set_mapchange_type, 0, 2, 300, 1, 300)
    USER_FUNC(evt_bero_mapchange, PTR("gon_00"), PTR("dokan_3"))

    RETURN()
EVT_END()

// Set up NPC and run async script for final boss floor.
EVT_BEGIN(Tower_FinalBossSetup)
    // Get battle / NPC info.
    USER_FUNC(evtTot_GetGonBattleDatabasePtr, LW(0))
    SET(LW(1), PTR(g_EnemyNpcSetup))
    USER_FUNC(evtTot_GetEnemyNpcInfo, 
        LW(0), LW(1), LW(2), LW(3), LW(4), LW(5), LW(6))
    
    // Set up NPC.
    USER_FUNC(evt_npc_entry, PTR(kPitNpcName), LW(2))
    USER_FUNC(evt_npc_set_tribe, PTR(kPitNpcName), LW(3))
    USER_FUNC(evt_npc_setup, LW(1))
    USER_FUNC(evt_npc_set_position, PTR(kPitNpcName), 0, 0, 0)
    USER_FUNC(evtTot_SetEnemyNpcBattleInfo, PTR(kPitNpcName), /* battle id */ 0)
    
    USER_FUNC(evt_npc_set_anim, PTR(kPitNpcName), PTR("GNB_H_3"))
    USER_FUNC(evt_npc_flag_onoff, 1, PTR(kPitNpcName), 33554496)
    USER_FUNC(evt_npc_pera_onoff, PTR(kPitNpcName), 0)
    USER_FUNC(evt_npc_set_ry, PTR(kPitNpcName), 0)
    
    RUN_EVT(&Tower_FinalBossEvent)
    RETURN()
EVT_END()

// Set up the battle / enemy NPC, or other NPCs on the floor.
EVT_BEGIN(Tower_NpcSetup)
    USER_FUNC(evtTot_IsRestFloor, LW(0))
    IF_EQUAL(LW(0), 1)
        // Rest floor; spawn chest immediately.
        SET(LW(15), 1)
        RUN_EVT(Tower_SpawnChests)
        
        USER_FUNC(evtTot_ClearEnemyInfo)
        
        // Spawn one or more NPCs as well, if floor > 0.
        USER_FUNC(evtTot_GetFloor, LW(0))
        IF_LARGE(LW(0), 0)
            USER_FUNC(evtTot_GetCharlietonNpcParams, LW(0), LW(1), LW(2))
            USER_FUNC(evt_npc_entry, LW(0), PTR("c_botta"))
            USER_FUNC(evt_npc_set_tribe, LW(0), LW(1))
            USER_FUNC(evt_npc_setup, LW(2))
            USER_FUNC(evt_npc_set_position, LW(0), 100, 0, 0)
        END_IF()
    ELSE()
        // Regular enemy floor; spawn enemies.
        USER_FUNC(evtTot_GetGonBattleDatabasePtr, LW(0))
        SET(LW(1), PTR(g_EnemyNpcSetup))
        USER_FUNC(evtTot_GetEnemyNpcInfo, 
            LW(0), LW(1), LW(2), LW(3), LW(4), LW(5), LW(6))
        
        USER_FUNC(evt_npc_entry, PTR(kPitNpcName), LW(2))
        USER_FUNC(evt_npc_set_tribe, PTR(kPitNpcName), LW(3))
        USER_FUNC(evt_npc_setup, LW(1))
        USER_FUNC(evt_npc_set_position, PTR(kPitNpcName), LW(4), LW(5), LW(6))
        
        IF_STR_EQUAL(LW(3), PTR(kPiderName))
            USER_FUNC(evt_npc_set_home_position, PTR(kPitNpcName), LW(4), LW(5), LW(6))
        END_IF()
        IF_STR_EQUAL(LW(3), PTR(kArantulaName))
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
            RUN_EVT(Tower_SpawnChests)
        END_INLINE()
    END_IF()
    
    // Wait for a chest to be opened, then spawn pipe.
    INLINE_EVT()
        LBL(2)
        WAIT_FRM(1)
        IF_EQUAL(GSW(1000), 0)
            IF_LARGE_EQUAL(GSW(1001), 1)
                USER_FUNC(evtTot_DisplayChestIcons)
            END_IF()
            GOTO(2)
        END_IF()
        RUN_EVT(Tower_SpawnPipe)
    END_INLINE()

    RETURN()
EVT_END()

// Main room initialization event.
EVT_BEGIN(gon_01_InitEvt)
    // Random unused bytes, used to track chest collection and icon display.
    SET(GSW(1000), 0)
    SET(GSW(1001), 0)
    // Flags to track chest opening.
    SET(GSWF(5075), 0)
    SET(GSWF(5076), 0)
    SET(GSWF(5077), 0)

    USER_FUNC(evt_run_case_evt, 9, 1, PTR("a_kanban"), 0, PTR(Tower_SignEvt), 0)
    
    // Is the player continuing from a Game Over?
    IF_EQUAL(GSW(1002), 0)
        // If not, set up NPCs, enemies, etc.
        USER_FUNC(evtTot_IsFinalFloor, LW(0))
        IF_EQUAL(LW(0), 1)
            RUN_CHILD_EVT(PTR(&Tower_FinalBossSetup))
        ELSE()
            RUN_CHILD_EVT(PTR(&Tower_NpcSetup))
        END_IF()
        // Set up loading zones.
        RUN_CHILD_EVT(PTR(&Tower_BeroSetupNormal))
    ELSE()
        RUN_CHILD_EVT(PTR(&Tower_BeroSetupNormal))
        // Spawn Heart Block statically.
        USER_FUNC(evtTot_GetFloor, LW(15))
        IF_LARGE(LW(15), 0)
            SET(LW(15), 1)
            RUN_EVT(Tower_SpawnHeartBlock)
        END_IF()
        // Enable exit pipe immediately.
        USER_FUNC(evt_hit_bind_mapobj, PTR("a_dokan_1"), PTR("dokan_1_s"))
        USER_FUNC(evt_mapobj_trans, 1, PTR("dokan_1_s"), 0, 30, 0)
        USER_FUNC(evt_hit_bind_update, PTR("a_dokan_1"))
        USER_FUNC(evt_mapobj_flag_onoff, 1, 1, PTR("dokan_1_k"), 1)
        
        SET(GSW(1002), 0)
    END_IF()
        
    // Dynamically update the destination of the exit loading zone.
    USER_FUNC(evtTot_UpdateDestinationMap)
    
    USER_FUNC(evtTot_IsFinalFloor, LW(0))
    IF_EQUAL(LW(0), 0)
        // Non-boss room music / env sound:
        USER_FUNC(evt_snd_bgmon, 512, PTR("BGM_STG0_100DN1"))
        USER_FUNC(evt_snd_envoff, 512)
        USER_FUNC(evt_snd_set_rev_mode, 1)
    ELSE()
        // Boss room music / env sound:
        USER_FUNC(evt_snd_bgmoff, 512)
        USER_FUNC(evt_snd_envon, 272, PTR("ENV_STG0_DAN1"))
        USER_FUNC(evt_snd_set_rev_mode, 2)
    END_IF()
    
    USER_FUNC(evtTot_ToggleIGT, 1)

    RETURN()
EVT_END()

BeroEntry normal_room_entry_data[3] = {
    {
        .name = "dokan_2",
        .type = BeroType::PIPE,
        .sfx_id = 0,
        .direction = BeroDirection::UP,
        .center_position = { 100000, 0, 0 },
        .length = -1,
        .entry_evt_code = nullptr,
        .case_type = 6,
        .out_evt_code = nullptr,
        .target_map = nullptr,
        .target_bero = nullptr,
        .entry_anim_type = BeroAnimType::ANIMATION,
        .out_anim_type = BeroAnimType::ANIMATION,
        .entry_anim_args = nullptr,
        .out_anim_args = nullptr,
    }, {
        .name = "dokan_1",
        .type = BeroType::PIPE,
        .sfx_id = 0,
        .direction = BeroDirection::DOWN,
        .center_position = { 100000, 0, 0 },
        .length = -1,
        .entry_evt_code = nullptr,
        .case_type = 6,
        .out_evt_code = (void*)Tower_IncrementFloor,
        .target_map = "gon_01",
        .target_bero = "dokan_2",
        .entry_anim_type = BeroAnimType::ANIMATION,
        .out_anim_type = BeroAnimType::ANIMATION,
        .entry_anim_args = nullptr,
        .out_anim_args = nullptr,
    }, { /* null-terminator */ },
};

const int32_t* GetTowerInitEvt() {
    return gon_01_InitEvt;
}

void UpdateDestinationMap() {
    ttyd::evtmgr::evtEntry(const_cast<int32_t*>(Tower_UpdateDestinationEvt), 0, 0);
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

// Returns whether the floor is a "rest floor" (no enemies).
EVT_DEFINE_USER_FUNC(evtTot_IsRestFloor) {
    auto& state = g_Mod->state_;
    int32_t floor = state.floor_;
    bool is_rest_floor = floor == 0 || (floor % 8 == 7);
    evtSetValue(evt, evt->evtArguments[0], is_rest_floor);
    return 2;
}

// Returns whether the floor is the final floor.
EVT_DEFINE_USER_FUNC(evtTot_IsFinalFloor) {
    bool is_final_floor = g_Mod->state_.IsFinalBossFloor();
    evtSetValue(evt, evt->evtArguments[0], is_final_floor);
    return 2;
}

// Overrides the previous party member that was out.
EVT_DEFINE_USER_FUNC(evtTot_SetPreviousPartner) {
    // TODO: Read from saved data if continuing, or will that be handled?
    auto* player = ttyd::mario::marioGetPtr();
    player->prevFollowerId[0] = 0;
    return 2;
}

// Wait for the dragon to hit the ground.
EVT_DEFINE_USER_FUNC(evtTot_WaitForDragonLanding) {
  ttyd::npcdrv::NpcEntry* npc = ttyd::npcdrv::npcNameToPtr(kPitNpcName);
  return npc->wKpaMobjDeadCheck ? 2 : 0;
}

// Dynamically change the destination of the exit pipe.
//
// Note: This is probably overkill; I wasn't running the entry event in testing.
// It's probably sufficient to update normal_room_entry_data on leaving a floor.
EVT_DEFINE_USER_FUNC(evtTot_UpdateDestinationMap) {
    const char* exit_bero = "dokan_1";
    
    int32_t floor = g_Mod->state_.floor_;
    bool is_penultimate_floor = g_Mod->state_.IsFinalBossFloor(floor + 1);
    const char* next_map = is_penultimate_floor ? "gon_05" : "gon_01";
    
    // Update the destination of the exit loading zone to match the floor.
    BeroEntry** entries = ttyd::evt_bero::BeroINFOARR;
    for (int32_t i = 0; i < 16; ++i) {
        if (entries[i] && !strcmp(entries[i]->name, exit_bero)) {
            entries[i]->target_map = next_map;
            entries[i]->target_bero = "dokan_2";
            break;
        }
    }
    return 2;
}

}  // namespace mod::tot::gon