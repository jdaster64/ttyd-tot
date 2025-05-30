#include "tot_gon_lobby.h"

#include "evt_cmd.h"
#include "mod.h"
#include "tot_gon_tower_npcs.h"
#include "tot_gsw.h"
#include "tot_manager_options.h"
#include "tot_state.h"
#include "tot_window_select.h"

#include <ttyd/battle_event_cmd.h>
#include <ttyd/database.h>
#include <ttyd/evt_bero.h>
#include <ttyd/evt_cam.h>
#include <ttyd/evt_mario.h>
#include <ttyd/evt_mobj.h>
#include <ttyd/evt_msg.h>
#include <ttyd/evt_npc.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_window.h>
#include <ttyd/evtmgr_cmd.h>
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
using namespace ::ttyd::evt_window;

using ::ttyd::evt_bero::BeroEntry;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

namespace BeroAnimType = ::ttyd::evt_bero::BeroAnimType;
namespace BeroDirection = ::ttyd::evt_bero::BeroDirection;
namespace BeroType = ::ttyd::evt_bero::BeroType;

}  // namespace

extern const BeroEntry gon_00_entry_data[4];

EVT_DECLARE_USER_FUNC(evtTot_TowerInitFromOptions, 0)
EVT_DECLARE_USER_FUNC(evtTot_InConfirmTriggerVolume, 4)

EVT_BEGIN(Lobby_EnterTowerEvt)
    // Initialize stats, etc. based on selected options.
    USER_FUNC(evtTot_TowerInitFromOptions)
    // Select secondary NPCs that will be encountered during the run.
    USER_FUNC(evtTot_SelectSecondaryNpcs)
    RETURN()
EVT_END()

EVT_BEGIN(Lobby_ExitTowerEvt)
    // Reset settings from last run, if not done already.
    USER_FUNC(evtTot_ResetSettingsAfterRun)
    RETURN()
EVT_END()

EVT_BEGIN(Lobby_FrontSignEvt)
    USER_FUNC(evt_mario_key_onoff, 0)
    USER_FUNC(evt_mario_normalize)

    IF_EQUAL((int32_t)GSWF_RunOptionsTutorial, 0)
        USER_FUNC(evt_msg_print, 0, PTR("tot_lobby_opttutorial"), 0, 0)
        // Explanation complete.
        SET((int32_t)GSWF_RunOptionsTutorial, 1)
        GOTO(99)
    END_IF()

    // Pull up 'run options' dialog.
    USER_FUNC(evt_win_other_select, window_select::MenuType::RUN_OPTIONS)
    USER_FUNC(evt_mario_key_onoff, 1)
    RETURN()
EVT_END()

EVT_BEGIN(Lobby_BackSignEvt)
    USER_FUNC(evt_npc_stop_for_event)
    USER_FUNC(evt_mario_key_onoff, 0)
    USER_FUNC(evt_mario_normalize)
    
    // Sign text will be dynamically filled in.
    USER_FUNC(evt_msg_print, 0, PTR("tot_lobby_backsign"), 0, 0)

    USER_FUNC(evt_mario_key_onoff, 1)
    USER_FUNC(evt_npc_start_for_event)
    RETURN()
EVT_END()

EVT_BEGIN(Lobby_ConfirmStart)
    USER_FUNC(evt_mario_key_onoff, 0)
    USER_FUNC(evt_mario_set_mov_spd, FLOAT(0.00))

    // TODO: Change text for first Gloomtail?
    SWITCH((int32_t)GSW_Tower_TutorialClears)
        CASE_EQUAL(0)
            USER_FUNC(evt_msg_print, 0, PTR("tot_lobby_confirmstart_1st"), 0, 0)
        CASE_EQUAL(1)
            USER_FUNC(evt_msg_print, 0, PTR("tot_lobby_confirmstart_2nd"), 0, 0)
        CASE_ETC()
            USER_FUNC(evt_msg_print, 0, PTR("tot_lobby_confirmstart"), 0, 0)
    END_SWITCH()

    USER_FUNC(evt_msg_select, 0, PTR("tot_lobby_optyesno"))
    USER_FUNC(evt_msg_continue)
    SET(LW(3), LW(0))
    USER_FUNC(evt_mario_get_pos, 0, LW(0), LW(1), LW(2))
    IF_NOT_EQUAL(LW(3), 0)
        // Chose "cancel" option.
        USER_FUNC(evt_mario_mov_pos2, 140, 0, 60)
        SET((int32_t)GSWF_Lobby_InConfirm, 0)
        USER_FUNC(evt_mario_key_onoff, 1)
    ELSE()
        SET((int32_t)GSWF_Lobby_Confirmed, 1)
        USER_FUNC(evt_mario_mov_pos2, 180, 0, 120)
        INLINE_EVT()
            USER_FUNC(evt_mario_mov_pos2, 250, 0, 120)
        END_INLINE()
        RUN_CHILD_EVT(Lobby_EnterTowerEvt)
        USER_FUNC(evt_bero_mapchange, PTR("gon_01"), PTR("w_bero"))
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(Lobby_ConfirmationTrigger)
    DO(0)
        IF_EQUAL((int32_t)GSWF_Lobby_InConfirm, 0)
            USER_FUNC(evt_mario_get_pos, 0, LW(0), LW(1), LW(2))
            USER_FUNC(evtTot_InConfirmTriggerVolume, LW(0), LW(1), LW(2), LW(3))
            IF_EQUAL(LW(3), 1)
                RUN_EVT(Lobby_ConfirmStart)
                SET((int32_t)GSWF_Lobby_InConfirm, 1)
            END_IF()
        END_IF()
        WAIT_FRM(1)
    WHILE()
    RETURN()
EVT_END()

EVT_BEGIN(gon_00_InitEvt)
    SET((int32_t)GSWF_Lobby_InConfirm, 0)
    SET((int32_t)GSWF_Lobby_Confirmed, 0)

    USER_FUNC(evt_snd_bgmoff, 512)
    USER_FUNC(evt_snd_envon, 272, PTR("ENV_STG1_GON1"))
    USER_FUNC(evt_snd_set_rev_mode, 2)
    
    SET(LW(0), PTR(&gon_00_entry_data))
    USER_FUNC(evt_bero_get_info)
    RUN_CHILD_EVT(evt_bero_info_run)
    
    // Spawn options + current setting signs if done with tutorial runs.
    IF_LARGE_EQUAL((int32_t)GSW_Tower_TutorialClears, 2)
        USER_FUNC(
            evt_mobj_signboard, PTR("board"), -95, 0, -40,
            PTR(&Lobby_FrontSignEvt), LSWF(0))
        USER_FUNC(
            evt_mobj_signboard, PTR("board2"), 95, 0, -40, 
            PTR(&Lobby_BackSignEvt), LSWF(0))
    END_IF()

    RUN_EVT(Lobby_ConfirmationTrigger)
        
    RETURN()
EVT_END()

const BeroEntry gon_00_entry_data[4] = {
    {
        .name = "w_bero",
        .type = BeroType::ROAD,
        .sfx_id = 0,
        .direction = BeroDirection::AUTO,
        .center_position = { 100000, 0, 0 },
        .length = -1,
        .entry_evt_code = (void*)Lobby_ExitTowerEvt,
        .case_type = -1,
        .out_evt_code = nullptr,
        .target_map = "gon_10",
        .target_bero = "w_bero",
        .entry_anim_type = BeroAnimType::ANIMATION,
        .out_anim_type = BeroAnimType::ANIMATION,
        .entry_anim_args = nullptr,
        .out_anim_args = nullptr,
    },{
        .name = "e_bero_1",
        .type = BeroType::ROAD,
        .sfx_id = 0,
        .direction = BeroDirection::AUTO,
        .center_position = { 100000, 0, 0 },
        .length = -1,
        .entry_evt_code = nullptr,
        .case_type = -1,
        .out_evt_code = nullptr,
        .target_map = "gon_00",
        .target_bero = "w_bero",
        .entry_anim_type = BeroAnimType::ANIMATION,
        .out_anim_type = BeroAnimType::ANIMATION,
        .entry_anim_args = nullptr,
        .out_anim_args = nullptr,
    },{
        .name = "e_bero_3",
        .type = BeroType::ROAD,
        .sfx_id = 0,
        .direction = BeroDirection::AUTO,
        .center_position = { 100000, 0, 0 },
        .length = -1,
        .entry_evt_code = nullptr,
        .case_type = -1,
        .out_evt_code = nullptr,
        .target_map = "gon_00",
        .target_bero = "w_bero",
        .entry_anim_type = BeroAnimType::ANIMATION,
        .out_anim_type = BeroAnimType::ANIMATION,
        .entry_anim_args = nullptr,
        .out_anim_args = nullptr,
    },
    { /* null-terminator */ },
};

const int32_t* GetLobbyInitEvt() {
    return gon_00_InitEvt;
}

EVT_DEFINE_USER_FUNC(evtTot_TowerInitFromOptions) {
    OptionsManager::OnRunStart();
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_InConfirmTriggerVolume) {
    int32_t x = evtGetValue(evt, evt->evtArguments[0]);
    int32_t z = evtGetValue(evt, evt->evtArguments[2]);
    bool in_volume = false;
    if (150 <= x && x <= 200) {
        if (-50 <= z && z <= 50) {
            in_volume = true;
        }
    }
    evtSetValue(evt, evt->evtArguments[3], in_volume);
    return 2;
}

}  // namespace mod::tot::gon