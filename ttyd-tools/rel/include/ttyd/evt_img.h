#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_img {

extern "C" {

EVT_DECLARE_USER_FUNC(N_evt_img_set_z, 2)
EVT_DECLARE_USER_FUNC(evt_img_set_color, 5)
EVT_DECLARE_USER_FUNC(evt_img_set_paper_timerate, 2)
EVT_DECLARE_USER_FUNC(evt_img_set_shadow, 2)
EVT_DECLARE_USER_FUNC(evt_img_wait_animend, 1)
EVT_DECLARE_USER_FUNC(evt_img_release, 1)
EVT_DECLARE_USER_FUNC(evt_img_clear_virtual_point, 1)
EVT_DECLARE_USER_FUNC(evt_img_set_virtual_point, 4)
EVT_DECLARE_USER_FUNC(evt_img_onoff, 2)
EVT_DECLARE_USER_FUNC(evt_img_free_capture, 2)
EVT_DECLARE_USER_FUNC(evt_img_alloc_capture, 8)
EVT_DECLARE_USER_FUNC(evt_img_set_paper_anim, 2)
EVT_DECLARE_USER_FUNC(evt_img_set_paper, 2)
EVT_DECLARE_USER_FUNC(evt_img_set_position, 3)
EVT_DECLARE_USER_FUNC(evt_img_entry, 1)

}

}