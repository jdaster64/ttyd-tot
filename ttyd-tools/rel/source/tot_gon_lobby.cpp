#include "tot_gon_lobby.h"

#include "evt_cmd.h"
#include "mod.h"
#include "tot_gon_tower_npcs.h"
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

namespace BeroAnimType = ::ttyd::evt_bero::BeroAnimType;
namespace BeroDirection = ::ttyd::evt_bero::BeroDirection;
namespace BeroType = ::ttyd::evt_bero::BeroType;

}  // namespace

extern const BeroEntry gon_00_entry_data[5];

EVT_DECLARE_USER_FUNC(evtTot_TowerInitLobby, 0)
EVT_DECLARE_USER_FUNC(evtTot_TowerInitFromOptions, 0)

EVT_BEGIN(Lobby_EnterTowerEvt)
    // Initialize stats, etc. based on selected options.
    USER_FUNC(evtTot_TowerInitFromOptions)
    // Select secondary NPCs that will be encountered during the run.
    USER_FUNC(evtTot_SelectSecondaryNpcs)
    RETURN()
EVT_END()

EVT_BEGIN(Lobby_ExitTowerEvt)
    // Re-init everything from scratch.
    USER_FUNC(evtTot_TowerInitLobby)
    RETURN()
EVT_END()

EVT_BEGIN(Lobby_ExitReentryRejectEvt)
    INLINE_EVT()
        USER_FUNC(evt_cam_ctrl_onoff, 4, 0)
        USER_FUNC(evt_mario_key_onoff, 0)
        USER_FUNC(evt_bero_exec_wait, 65536)
        WAIT_MSEC(750)
        USER_FUNC(evt_msg_print, 0, PTR("tot_lobby_reentry"), 0, 0)
        USER_FUNC(evt_mario_key_onoff, 1)
        USER_FUNC(evt_cam_ctrl_onoff, 4, 1)
    END_INLINE()
    SET(LW(0), 0)
    RETURN()
EVT_END()

EVT_BEGIN(Lobby_FrontSignEvt)
    USER_FUNC(evt_mario_key_onoff, 0)
    USER_FUNC(evt_mario_normalize)
    // Pull up 'run options' dialog.
    USER_FUNC(evt_win_other_select, window_select::MenuType::RUN_OPTIONS)
    USER_FUNC(evt_mario_key_onoff, 1)
    RETURN()
EVT_END()

EVT_BEGIN(Lobby_BackSignEvt)
    USER_FUNC(evt_npc_stop_for_event)
    USER_FUNC(evt_mario_key_onoff, 0)
    USER_FUNC(evt_mario_normalize)
    
    USER_FUNC(evtTot_GetSeed, LW(0))
    USER_FUNC(evtTot_GetEncodedOptions, LW(1))
    USER_FUNC(evt_msg_print_insert, 0, PTR("tot_lobby_backsign"), 0, 0, LW(0), LW(1))

    USER_FUNC(evt_mario_key_onoff, 1)
    USER_FUNC(evt_npc_start_for_event)
    RETURN()
EVT_END()

EVT_BEGIN(gon_00_InitEvt)
    USER_FUNC(evt_snd_bgmoff, 512)
    USER_FUNC(evt_snd_envon, 272, PTR("ENV_STG0_TIK1"))
    USER_FUNC(evt_snd_set_rev_mode, 2)
    
    SET(LW(0), PTR(&gon_00_entry_data))
    USER_FUNC(evt_bero_get_info)
    RUN_CHILD_EVT(evt_bero_info_run)
    
    // Spawn signs.
    USER_FUNC(
        evt_mobj_signboard, PTR("board"), -50, 0, 50, 
        PTR(&Lobby_FrontSignEvt), LSWF(0))
    USER_FUNC(
        evt_mobj_signboard, PTR("board2"), 50, 0, -250, 
        PTR(&Lobby_BackSignEvt), LSWF(0))
        
    RETURN()
EVT_END()

const BeroEntry gon_00_entry_data[5] = {
    {
        .name = "dokan_1",
        .type = BeroType::PIPE,
        .sfx_id = 0,
        .direction = BeroDirection::DOWN,
        .center_position = { 100000, 0, 0 },
        .length = -1,
        .entry_evt_code = nullptr,
        .case_type = 6,
        .out_evt_code = (void*)Lobby_EnterTowerEvt,
        .target_map = "gon_01",
        .target_bero = "dokan_2",
        .entry_anim_type = BeroAnimType::ANIMATION,
        .out_anim_type = BeroAnimType::ANIMATION,
        .entry_anim_args = nullptr,
        .out_anim_args = nullptr,
    },{
        .name = "dokan_2",
        .type = BeroType::PIPE,
        .sfx_id = 0,
        .direction = BeroDirection::UP,
        .center_position = { 100000, 0, 0 },
        .length = -1,
        .entry_evt_code = nullptr,
        .case_type = 6,
        .out_evt_code = nullptr,
        .target_map = "tik_18",
        .target_bero = "dokan_1",
        .entry_anim_type = BeroAnimType::ANIMATION,
        .out_anim_type = BeroAnimType::ANIMATION,
        .entry_anim_args = nullptr,
        .out_anim_args = nullptr,
    },{
        .name = "dokan_3",
        .type = BeroType::PIPE,
        .sfx_id = 0,
        .direction = BeroDirection::DOWN,
        .center_position = { 100000, 0, 0 },
        .length = -1,
        .entry_evt_code = (void*)Lobby_ExitTowerEvt,
        .case_type = 6,
        .out_evt_code = (void*)Lobby_ExitReentryRejectEvt,
        .target_map = nullptr,
        .target_bero = "dokan_3",
        .entry_anim_type = BeroAnimType::ANIMATION,
        .out_anim_type = BeroAnimType::ANIMATION,
        .entry_anim_args = nullptr,
        .out_anim_args = nullptr,
    },{
        .name = "e_bero",
        .type = BeroType::ROAD,
        .sfx_id = 0,
        .direction = BeroDirection::AUTO,
        .center_position = { 100000, 0, 0 },
        .length = -1,
        .entry_evt_code = nullptr,
        .case_type = -1,
        .out_evt_code = nullptr,
        .target_map = "gon_00",
        .target_bero = "e_bero",
        .entry_anim_type = BeroAnimType::ANIMATION,
        .out_anim_type = BeroAnimType::ANIMATION,
        .entry_anim_args = nullptr,
        .out_anim_args = nullptr,
    }, { /* null-terminator */ },
};

const int32_t* GetLobbyInitEvt() {
    return gon_00_InitEvt;
}

EVT_DEFINE_USER_FUNC(evtTot_TowerInitLobby) {
    OptionsManager::InitLobby();
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_TowerInitFromOptions) {
    OptionsManager::InitFromSelectedOptions();
    return 2;
}

}  // namespace mod::tot::gon