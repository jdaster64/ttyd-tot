#include "tot_gon_opening.h"

#include "evt_cmd.h"
#include "mod.h"
#include "tot_custom_rel.h"
#include "tot_gon_tower_npcs.h"
#include "tot_gsw.h"
#include "tot_manager_options.h"
#include "tot_state.h"
#include "tot_window_select.h"

#include <ttyd/battle_event_cmd.h>
#include <ttyd/database.h>
#include <ttyd/evt_bero.h>
#include <ttyd/evt_cam.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_fade.h>
#include <ttyd/evt_hit.h>
#include <ttyd/evt_map.h>
#include <ttyd/evt_mario.h>
#include <ttyd/evt_mobj.h>
#include <ttyd/evt_msg.h>
#include <ttyd/evt_npc.h>
#include <ttyd/evt_party.h>
#include <ttyd/evt_seq.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/mapdata.h>
#include <ttyd/mario.h>
#include <ttyd/npcdrv.h>

namespace mod::tot::gon {

namespace {

// Including entire namespaces for convenience.
using namespace ::ttyd::evt_bero;
using namespace ::ttyd::evt_cam;
using namespace ::ttyd::evt_eff;
using namespace ::ttyd::evt_fade;
using namespace ::ttyd::evt_hit;
using namespace ::ttyd::evt_map;
using namespace ::ttyd::evt_mario;
using namespace ::ttyd::evt_mobj;
using namespace ::ttyd::evt_msg;
using namespace ::ttyd::evt_npc;
using namespace ::ttyd::evt_party;
using namespace ::ttyd::evt_seq;
using namespace ::ttyd::evt_snd;
using namespace ::ttyd::evt_sub;

using ::ttyd::evt_bero::BeroEntry;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::npcdrv::NpcSetupInfo;

namespace BeroAnimType = ::ttyd::evt_bero::BeroAnimType;
namespace BeroDirection = ::ttyd::evt_bero::BeroDirection;
namespace BeroType = ::ttyd::evt_bero::BeroType;

}  // namespace

extern const BeroEntry gon_12_entry_data[2];
extern const NpcSetupInfo gon_12_npc_data[2];
const char kHooktailNpcName[] = "\x83\x6f\x83\x6f";
const char kHooktailNpcLongName[] = "\x83\x53\x83\x93\x83\x6f\x83\x6f\x89\x65";
const char kMiniHooktailNpcName[] = "\x81\x69\x81\x45\x82\x98\x81\x45\x81\x6a";

EVT_DECLARE_USER_FUNC(evtTot_SetPreviousPartnerToNone, 0)

EVT_BEGIN(Opening_CutsceneFirstEvt)
    USER_FUNC(evt_snd_bgmon, 512, PTR("BGM_STG1_HEI1"))
    USER_FUNC(evt_snd_envon, 272, PTR("ENV_STG1_HEI1"))
    USER_FUNC(evt_snd_bgm_scope, 0, 1)
    USER_FUNC(evt_mario_key_onoff, 0)
    USER_FUNC(evt_mario_set_party, 0, 1)
    USER_FUNC(evt_party_stop, 0)
    USER_FUNC(evt_fade_entry, 0x1c, 0, 0, 0, 0)     // Manually open curtain
    USER_FUNC(evt_fade_set_mapchange_type, 0, -1, -1, 9, 300)
    USER_FUNC(evt_fade_set_mapchange_type, 1, -1, -1, 9, 300)
    USER_FUNC(evt_cam_letter_box_onoff, 1, 0)
    USER_FUNC(evt_mario_dispflag_onoff, 1, 1)
    USER_FUNC(evt_party_dispflg_onoff, 0, 1, 1)
    USER_FUNC(evt_mario_cont_onoff, 0)
    USER_FUNC(evt_cam3d_evt_set, 250, 350, 1041, 250, 130, -36, 0, 11)
    USER_FUNC(evt_seq_wait, 2)
    USER_FUNC(evt_cam3d_evt_set, -250, 350, 1041, -250, 130, -36, 6000, 11)
    WAIT_MSEC(6000)
    USER_FUNC(evt_mario_cont_onoff, 1)
    SET(GF(0), 0)
    WAIT_FRM(1)
    USER_FUNC(evt_mario_dispflag_onoff, 0, 1)
    USER_FUNC(evt_party_dispflg_onoff, 0, 1, 0)
    USER_FUNC(evt_bero_exec_wait, 65536)
    USER_FUNC(evt_party_stop, 0)
    WAIT_MSEC(500)
    INLINE_EVT()
        USER_FUNC(evt_mario_mov_pos2, -325, 0, FLOAT(60.0))
    END_INLINE()
    USER_FUNC(evt_cam3d_evt_set, -340, 175, 263, -340, 125, -33, 1000, 11)
    WAIT_MSEC(1000)
    USER_FUNC(evt_party_stop, 0)
    WAIT_FRM(1)
    USER_FUNC(evt_mario_dispflag_onoff, 1, 2)
    USER_FUNC(evt_mario_get_pos, 0, LW(0), LW(1), LW(2))
    ADD(LW(0), -1)
    ADD(LW(2), 1)
    USER_FUNC(evt_mario_mov_pos, LW(0), LW(2), 30)
    USER_FUNC(evt_mario_dispflag_onoff, 0, 2)

    // DO(2)
    //     USER_FUNC(evt_party_set_dir, 0, 270, 300)
    //     WAIT_MSEC(600)
    //     USER_FUNC(evt_party_set_dir, 0, 90, 300)
    //     WAIT_MSEC(600)
    // WHILE()

    WAIT_MSEC(500)
    USER_FUNC(evt_msg_print, 0, PTR("stg1_hei_00"), 0, PTR("party"))
    USER_FUNC(evt_mario_set_dir, 90, 300, 0)
    USER_FUNC(evt_cam3d_evt_off, 500, 11)
    SET(GSW(0), 23)
    USER_FUNC(evt_mario_key_onoff, 1)
    USER_FUNC(evt_party_run, 0)
    RETURN()
EVT_END()

EVT_BEGIN(Opening_TriggerHooktailEvt)
    LBL(1)
    USER_FUNC(evt_mario_get_pos, 0, LW(0), LW(1), LW(2))
    IF_SMALL(LW(0), -100)
        WAIT_FRM(1)
        GOTO(1)
    END_IF()
    USER_FUNC(evt_mario_get_party, LW(0))
    IF_NOT_EQUAL(LW(0), 1)
        USER_FUNC(evt_mario_goodbye_party, 0)
        USER_FUNC(evt_mario_hello_party, 0, 1)
    END_IF()
    USER_FUNC(evt_mario_key_onoff, 0)
    WAIT_FRM(1)
    USER_FUNC(evt_snd_bgmoff, 1024)
    USER_FUNC(evt_snd_bgmon, 1, PTR("BGM_EVT_GONBABA_FLY1"))
    USER_FUNC(evt_snd_envoff, 1024)
    USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_STG1_GNB_ROAR1"), -117, 300, 1000, 0)
    WAIT_MSEC(700)
    USER_FUNC(evt_eff_fukidashi, 0, PTR("eff0"), 0, 0, 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(evt_eff_fukidashi, 3, PTR("eff1"), 0, 0, 0, 0, 0, 0, 0, 0, 0)
    WAIT_MSEC(500)
    USER_FUNC(evt_party_set_dir, 0, 270, 200)
    USER_FUNC(evt_party_set_pose, 0, PTR("PKR_S_1"))
    WAIT_MSEC(500)
    USER_FUNC(evt_mario_set_dir, 270, 300, 0)
    INLINE_EVT()
        WAIT_MSEC(500)
        USER_FUNC(evt_mario_set_pose, PTR("M_N_5B"))
        WAIT_MSEC(800)
        USER_FUNC(evt_mario_set_pose, PTR("M_I_N"))
    END_INLINE()
    WAIT_MSEC(1000)

    RUN_CHILD_EVT(tot::custom::evt_hei_00_gonbaba_shadow_evt)

    USER_FUNC(evt_mario_get_pos, 0, LW(0), LW(2), LW(2))
    USER_FUNC(evt_party_get_pos, 0, LW(3), LW(4), LW(5))
    IF_SMALL(LW(0), LW(3))
        USER_FUNC(evt_party_set_dir, 0, 270, 200)
    ELSE()
        USER_FUNC(evt_party_set_dir, 0, 90, 200)
    END_IF()
    USER_FUNC(evt_party_stop, 0)
    WAIT_MSEC(300)
    USER_FUNC(evt_mario_set_pose, PTR("M_S_1"))
    USER_FUNC(evt_msg_print, 0, PTR("stg1_hei_01"), 0, PTR("party"))
    USER_FUNC(evt_npc_set_position, PTR(kMiniHooktailNpcName), 1000, -1000, -1000)
    USER_FUNC(evt_mario_set_pose, PTR("M_I_N"))
    USER_FUNC(evt_mario_set_dir_npc, PTR(kMiniHooktailNpcName))
    WAIT_MSEC(1000)
    USER_FUNC(evt_mario_cont_onoff, 0)
    USER_FUNC(evt_npc_set_position, PTR(kMiniHooktailNpcName), 0, 210, -500)
    USER_FUNC(evt_npc_set_anim, PTR(kMiniHooktailNpcName), PTR("GNB_F_1"))
    USER_FUNC(evt_npc_pera_onoff, PTR(kMiniHooktailNpcName), 0)
    USER_FUNC(evt_npc_set_ry, PTR(kMiniHooktailNpcName), 135)
    INLINE_EVT()
        WAIT_MSEC(1000)
        USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_STG1_GNB_ROAR1"), -117, 300, -433, 0)
        WAIT_MSEC(3000)
        USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_STG1_GNB_ROAR1"), 0, 290, -500, 0)
        WAIT_MSEC(3000)
        USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_STG1_GNB_ROAR1"), 80, 280, -600, 0)
    END_INLINE()
    INLINE_EVT()
        SET_FRAME_FROM_MSEC(LW(0), 10000)
        USER_FUNC(evt_sub_intpl_init, 0, 75, 18, LW(0))
        DO(601)
            USER_FUNC(evt_sub_intpl_get_value)
            DIVF(LW(0), FLOAT(100.0))
            USER_FUNC(evt_npc_set_scale, PTR(kMiniHooktailNpcName), LW(0), LW(0), LW(0))
            WAIT_FRM(1)
        WHILE()
    END_INLINE()
    INLINE_EVT()
        SET_FRAME_FROM_MSEC(LW(0), 4166)
        USER_FUNC(evt_sub_intpl_init, 11, 210, 114, LW(0))
        DO(251)
            USER_FUNC(evt_sub_intpl_get_value)
            USER_FUNC(evt_npc_get_position, PTR(kMiniHooktailNpcName), LW(2), LW(3), LW(4))
            USER_FUNC(evt_npc_set_position, PTR(kMiniHooktailNpcName), LW(2), LW(0), LW(4))
            WAIT_FRM(1)
        WHILE()
    END_INLINE()
    INLINE_EVT()
        SET_FRAME_FROM_MSEC(LW(0), 10000)
        USER_FUNC(evt_sub_intpl_init, 11, 0, 290, 600)
        DO(601)
            USER_FUNC(evt_sub_intpl_get_value)
            USER_FUNC(evt_npc_get_position, PTR(kMiniHooktailNpcName), LW(2), LW(3), LW(4))
            USER_FUNC(evt_npc_set_position, PTR(kMiniHooktailNpcName), LW(0), LW(3), LW(4))
            WAIT_FRM(1)
        WHILE()
    END_INLINE()
    INLINE_EVT()
        SET_FRAME_FROM_MSEC(LW(0), 10000)
        USER_FUNC(evt_sub_intpl_init, 0, -287, -480, 600)
        DO(601)
            USER_FUNC(evt_sub_intpl_get_value)
            USER_FUNC(evt_npc_get_position, PTR(kMiniHooktailNpcName), LW(2), LW(3), LW(4))
            USER_FUNC(evt_npc_set_position, PTR(kMiniHooktailNpcName), LW(2), LW(3), LW(0))
            WAIT_FRM(1)
        WHILE()
    END_INLINE()
    WAIT_MSEC(1000)
    USER_FUNC(evt_mario_get_pos, 0, LW(0), LW(1), LW(2))
    USER_FUNC(evt_cam3d_evt_set_xyz, 270, LW(1), LW(2), 0, 7000, 11)
    WAIT_MSEC(9500)
    USER_FUNC(evt_npc_set_position, PTR(kMiniHooktailNpcName), 0, -1000, 0)
    USER_FUNC(evt_cam3d_evt_xyz_off, 0, 11)

    USER_FUNC(tot::custom::evt_hei_00_cam_test)

    USER_FUNC(evt_cam3d_evt_set, LW(0), LW(1), LW(2), LW(3), LW(4), LW(5), 0, 11)
    WAIT_FRM(1)
    USER_FUNC(evt_mario_get_pos, 0, LW(0), LW(1), LW(2))
    USER_FUNC(evt_party_get_pos, 0, LW(3), LW(4), LW(5))
    USER_FUNC(evt_party_set_pos, 0, LW(3), LW(4), LW(2))
    USER_FUNC(evt_snd_envoff, 513)
    USER_FUNC(evt_snd_envon, 288, 0)
    USER_FUNC(evt_mario_set_pose, PTR("M_S_1"))
    USER_FUNC(evt_mario_set_dir, 270, 300, 0)
    USER_FUNC(evt_msg_print, 0, PTR("stg1_hei_02"), 0, PTR("party"))
    USER_FUNC(evt_mario_set_dir, 270, 300, 0)

    // Add Mario nodding animation.
    WAIT_MSEC(500)
    USER_FUNC(evt_mario_get_pos, 0, LW(0), LW(1), LW(2))
    USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_VOICE_MARIO_NOD1_4"), LW(0), LW(1), LW(2), 0)
    USER_FUNC(evt_mario_set_pose, PTR("M_N_2"))
    WAIT_MSEC(1000)

    INLINE_EVT()
        USER_FUNC(evt_mario_cont_onoff, 1)
        USER_FUNC(evt_mario_get_pos, 0, LW(3), LW(4), LW(5))
        ADD(LW(3), 120)
        USER_FUNC(evt_mario_mov_pos2, LW(3), LW(5), FLOAT(120.0))
    END_INLINE()
    INLINE_EVT()
        USER_FUNC(evt_party_get_pos, 0, LW(0), LW(1), LW(2))
        ADD(LW(0), 96)
        USER_FUNC(evt_party_move_pos2, 0, LW(0), LW(2), FLOAT(120.0))
        // Remove partner from overworld when loading the next screen.
        USER_FUNC(evt_mario_goodbye_party, 0)
        USER_FUNC(evtTot_SetPreviousPartnerToNone)
    END_INLINE()

    WAIT_FRM(15)
    // Set up special conversation event for first visit to Petalburg.
    SET((int32_t)GSW_NpcA_SpecialConversation, 1)
    // Load player into Petalburg west map.
    USER_FUNC(evt_snd_bgmoff, 513)
    USER_FUNC(evt_fade_set_mapchange_type, 0, 2, 300, 1, 300)
    USER_FUNC(evt_bero_mapchange, PTR("gon_10"), PTR("w_bero"))
    
    RETURN()
EVT_END()

EVT_BEGIN(Opening_PipeFirstEvt)
    SET(GF(0), 1)
    HALT(GF(0))
    SET(LW(0), 0)
    RETURN()
EVT_END()

EVT_BEGIN(gon_12_InitEvt)
    SET(LW(0), PTR(&gon_12_entry_data))
    USER_FUNC(evt_bero_get_info)
    RUN_CHILD_EVT(evt_bero_info_run)
    USER_FUNC(evt_npc_setup, PTR(&gon_12_npc_data))

    RUN_EVT(Opening_CutsceneFirstEvt)
    RUN_EVT(Opening_TriggerHooktailEvt)

    USER_FUNC(evt_map_playanim, PTR("anu_mugi_1"), 1, 0)
    USER_FUNC(evt_npc_pera_onoff, PTR(kHooktailNpcLongName), 0)

    // Enable interactable trees (set the flags for their items as collected).
    SET(LW(0), tot::custom::hei_00_ki_data)
    RUN_CHILD_EVT(evt_sub_tree_access_entry)
    SET(GSWF(1778), 1)
    SET(GSWF(5598), 1)

    // Enable pipe to Hooktail Castle for visual continuity with TTYD's story.
    USER_FUNC(evt_mapobj_flag_onoff, 0, 1, PTR("isi_0"), 1)
    USER_FUNC(evt_hitobj_onoff, PTR("a_isi_0"), 0, 0)
    USER_FUNC(evt_mapobj_trans, 0, PTR("h_dokan"), 0, 0, 0)
    USER_FUNC(evt_mapobj_flag_onoff, 0, 0, PTR("h_dokan"), 1)

    USER_FUNC(evt_mapobj_flag_onoff, 1, 1, PTR("s_moji"), 1)
    USER_FUNC(evt_hitobj_onoff, PTR("a_moji"), 1, 0)

    // Turn on Sun and Moon stones for similar purpose.
    USER_FUNC(evt_mapobj_flag_onoff, 1, 1, PTR("taiyou_ana"), 1)
    USER_FUNC(evt_mapobj_flag_onoff, 1, 0, PTR("taiyounoisi"), 1)
    USER_FUNC(evt_mapobj_trans, 1, PTR("isi_4"), -80, 0, 0)
    USER_FUNC(evt_mapobj_trans, 1, PTR("taiyounoisi"), -80, 0, 0)
    USER_FUNC(evt_hit_bind_mapobj, PTR("a_isi_4"), PTR("isi_4"))
    USER_FUNC(evt_hit_bind_update, PTR("a_isi_4"))

    USER_FUNC(evt_mapobj_flag_onoff, 1, 1, PTR("tuki_ana"), 1)
    USER_FUNC(evt_mapobj_flag_onoff, 1, 0, PTR("tukinoisi"), 1)
    USER_FUNC(evt_mapobj_trans, 1, PTR("isi_3"), 80, 0, 0)
    USER_FUNC(evt_mapobj_trans, 1, PTR("tukinoisi"), 80, 0, 0)
    USER_FUNC(evt_hit_bind_mapobj, PTR("a_isi_3"), PTR("isi_3"))
    USER_FUNC(evt_hit_bind_update, PTR("a_isi_3"))

    USER_FUNC(evt_snd_bgmon, 512, PTR("BGM_STG1_HEI1"))
    USER_FUNC(evt_snd_envon, 512, PTR("ENV_STG1_HEI1"))

    RETURN()
EVT_END()

const BeroEntry gon_12_entry_data[2] = {
    {
        .name = "dokan_2",
        .type = BeroType::PIPE,
        .sfx_id = 0,
        .direction = BeroDirection::DOWN,
        .center_position = { 100000, 0, 0 },
        .length = -1,
        .entry_evt_code = (void*)Opening_PipeFirstEvt,
        .case_type = 6,
        .out_evt_code = nullptr,
        .target_map = "gon_12",
        .target_bero = "dokan_2",
        .entry_anim_type = BeroAnimType::ANIMATION,
        .out_anim_type = BeroAnimType::ANIMATION,
        .entry_anim_args = nullptr,
        .out_anim_args = nullptr,
    },
    { /* null-terminator */ },
};

const NpcSetupInfo gon_12_npc_data[2] = {
    {
        .name = kHooktailNpcName,
        .initEvtCode = (void*)ttyd::evt_npc::npc_init_evt,
    },
    { /* null-terminator */ },
};

const int32_t* GetOpeningInitEvt() {
    return gon_12_InitEvt;
}

EVT_DEFINE_USER_FUNC(evtTot_SetPreviousPartnerToNone) {
    auto* player = ttyd::mario::marioGetPtr();
    player->prevFollowerId[0] = 0;
    return 2;
}

}  // namespace mod::tot::gon