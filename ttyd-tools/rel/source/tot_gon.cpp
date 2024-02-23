#include "tot_gon.h"

#include "evt_cmd.h"

#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/database.h>
#include <ttyd/evt_bero.h>
#include <ttyd/evt_cam.h>
#include <ttyd/evt_mario.h>
#include <ttyd/evt_mobj.h>
#include <ttyd/evt_msg.h>
#include <ttyd/evt_npc.h>
#include <ttyd/evt_snd.h>
#include <ttyd/mapdata.h>

namespace mod::tot::gon {

namespace {

// Including entire namespaces for convenience.
using namespace ::ttyd::evt_bero;
using namespace ::ttyd::evt_cam;
using namespace ::ttyd::evt_mario;
using namespace ::ttyd::evt_mobj;
using namespace ::ttyd::evt_msg;
using namespace ::ttyd::evt_npc;
using namespace ::ttyd::evt_snd;

using ::ttyd::evt_bero::BeroEntry;

}  // namespace

// Dummy setup data / database definitions (currently empty).
ttyd::battle_database_common::BattleSetupData g_SetupDataTbl[1];
ttyd::database::DatabaseDefinition g_SetupNoTbl[1];

extern const BeroEntry gon_00_entry_data[5];

EVT_BEGIN(oshimodo_event)
    INLINE_EVT()
        USER_FUNC(evt_cam_ctrl_onoff, 4, 0)
        USER_FUNC(evt_mario_key_onoff, 0)
        USER_FUNC(evt_bero_exec_wait, 65536)
        WAIT_MSEC(750)
        // TODO: New strings for map.
        USER_FUNC(evt_msg_print, 0, PTR("tik_06_01"), 0, 0)
        USER_FUNC(evt_mario_key_onoff, 1)
        USER_FUNC(evt_cam_ctrl_onoff, 4, 1)
    END_INLINE()
    SET(LW(0), 0)
    RETURN()
EVT_END()

EVT_BEGIN(evt_kanban)
    USER_FUNC(evt_mario_key_onoff, 0)
    USER_FUNC(evt_mario_normalize)
    USER_FUNC(evt_msg_print, 0, PTR("tik_06_02"), 0, 0)
    USER_FUNC(evt_mario_key_onoff, 1)
    RETURN()
EVT_END()

EVT_BEGIN(evt_kanban2)
    USER_FUNC(evt_npc_stop_for_event)
    USER_FUNC(evt_mario_key_onoff, 0)
    USER_FUNC(evt_mario_normalize)
    
    // Calculate vanilla sign stats.
    SET(LW(0), GSW(33))
    SET(LW(1), GSW(32))
    SET(LW(4), GSW(35))
    MUL(LW(4), 256)
    ADD(LW(1), LW(4))
    SET(LW(2), GSW(34))
    SET(LW(3), 100)
    USER_FUNC(
        evt_msg_print_insert, 0, PTR("msg_jon_kanban_3"), 0, 0,
        LW(0), LW(1), LW(2), LW(3))

    USER_FUNC(evt_mario_key_onoff, 1)
    USER_FUNC(evt_npc_start_for_event)
    RETURN()
EVT_END()

EVT_BEGIN(gon_00_init)
    USER_FUNC(evt_snd_bgmoff, 512)
    USER_FUNC(evt_snd_envon, 272, PTR("ENV_STG0_TIK1"))
    USER_FUNC(evt_snd_set_rev_mode, 2)
    
    SET(LW(0), PTR(&gon_00_entry_data))
    USER_FUNC(evt_bero_get_info)
    RUN_CHILD_EVT(evt_bero_info_run)
    
    // Spawn signs.
    USER_FUNC(
        evt_mobj_signboard, PTR("board"), -50, 0, 50, 
        PTR(&evt_kanban), LSWF(0))
    USER_FUNC(
        evt_mobj_signboard, PTR("board2"), 50, 0, -250, 
        PTR(&evt_kanban2), LSWF(0))
        
    RETURN()
EVT_END()

const BeroEntry gon_00_entry_data[5] = {
    {
        .name = "dokan_1",
        .unk_04 = 0x00020000,
        .unk_08 = 9,
        .unk_0c = 100000,
        .unk_10 = 0,
        .unk_14 = 0,
        .unk_18 = -1,
        .entry_evt_code = nullptr,
        .unk_20 = 6,
        .out_evt_code = nullptr,
        .target_map = "jon_00",
        .target_bero = "dokan_2",
        .unk_30 = 1,
        .unk_32 = 1,
        .unk_close_evt_code = nullptr,
        .unk_open_evt_code = nullptr,
    },{
        .name = "dokan_2",
        .unk_04 = 0x00020000,
        .unk_08 = 8,
        .unk_0c = 100000,
        .unk_10 = 0,
        .unk_14 = 0,
        .unk_18 = -1,
        .entry_evt_code = nullptr,
        .unk_20 = 6,
        .out_evt_code = nullptr,
        .target_map = "tik_18",
        .target_bero = "dokan_1",
        .unk_30 = 1,
        .unk_32 = 1,
        .unk_close_evt_code = nullptr,
        .unk_open_evt_code = nullptr,
    },{
        .name = "dokan_3",
        .unk_04 = 0x00020000,
        .unk_08 = 9,
        .unk_0c = 100000,
        .unk_10 = 0,
        .unk_14 = 0,
        .unk_18 = -1,
        .entry_evt_code = nullptr,
        .unk_20 = 6,
        .out_evt_code = (void*)oshimodo_event,
        .target_map = nullptr,
        .target_bero = "dokan_3",
        .unk_30 = 1,
        .unk_32 = 1,
        .unk_close_evt_code = nullptr,
        .unk_open_evt_code = nullptr,
    },{
        .name = "e_bero",
        .unk_04 = 0,
        .unk_08 = 20000,
        .unk_0c = 100000,
        .unk_10 = 0,
        .unk_14 = 0,
        .unk_18 = -1,
        .entry_evt_code = nullptr,
        .unk_20 = -1,
        .out_evt_code = nullptr,
        .target_map = "gon_00",
        .target_bero = "e_bero",
        .unk_30 = 1,
        .unk_32 = 1,
        .unk_close_evt_code = nullptr,
        .unk_open_evt_code = nullptr,
    }, { /* null-terminator */ },
};

void Prolog() {
    // TODO: Add evt.
    ttyd::mapdata::relSetEvtAddr("gon_00", gon_00_init);
    ttyd::mapdata::relSetBtlAddr("gon", g_SetupDataTbl, g_SetupNoTbl);
}

}  // namespace mod::tot::gon