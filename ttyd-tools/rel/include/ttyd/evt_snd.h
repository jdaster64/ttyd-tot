#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_snd {

extern "C" {

EVT_DECLARE_USER_FUNC(evt_snd_sfx_all_stop, 0)
EVT_DECLARE_USER_FUNC(evt_snd_set_rev_mode, 1)
EVT_DECLARE_USER_FUNC(evt_snd_env_lpf, 2)
EVT_DECLARE_USER_FUNC(evt_snd_envoff_f, 2)
EVT_DECLARE_USER_FUNC(evt_snd_envoff, 1)
EVT_DECLARE_USER_FUNC(evt_snd_envon_f, 3)
EVT_DECLARE_USER_FUNC(evt_snd_envon, 2)
EVT_DECLARE_USER_FUNC(evt_snd_sfx_dist, 2)
EVT_DECLARE_USER_FUNC(evt_snd_sfx_pos, 4)
EVT_DECLARE_USER_FUNC(evt_snd_sfx_vol, 2)
EVT_DECLARE_USER_FUNC(evt_snd_sfx_pit, 2)
EVT_DECLARE_USER_FUNC(evt_snd_sfxchk, 2)
EVT_DECLARE_USER_FUNC(evt_snd_sfxoff, 1)
EVT_DECLARE_USER_FUNC(evt_snd_sfxon_3d_ex, 9)
EVT_DECLARE_USER_FUNC(evt_snd_sfxon_3d, 5)
EVT_DECLARE_USER_FUNC(evt_snd_sfxon_, 2)
EVT_DECLARE_USER_FUNC(evt_snd_sfxon__, 2)
EVT_DECLARE_USER_FUNC(evt_snd_sfxon, 2)
EVT_DECLARE_USER_FUNC(evt_snd_bgm_scope, 2)
EVT_DECLARE_USER_FUNC(evt_snd_bgm_freq, 2)
EVT_DECLARE_USER_FUNC(evt_snd_bgmoff_f, 2)
EVT_DECLARE_USER_FUNC(evt_snd_bgmoff, 1)
EVT_DECLARE_USER_FUNC(unk_801524c8, 4)
EVT_DECLARE_USER_FUNC(evt_snd_bgmon_f, 3)
EVT_DECLARE_USER_FUNC(evt_snd_bgmon, 2)

}

}