#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_hit {

extern "C" {

EVT_DECLARE_USER_FUNC(evt_hitobj_name_callback, 2)
// name_callback_sub
EVT_DECLARE_USER_FUNC(evt_hitobj_attr_onoff, 4)
EVT_DECLARE_USER_FUNC(L_evt_hitobj_flag_onoff, 4)
EVT_DECLARE_USER_FUNC(evt_hit_get_position, 4)
EVT_DECLARE_USER_FUNC(evt_hit_bind_update, 1)
EVT_DECLARE_USER_FUNC(evt_hit_bind_mapobj, 2)
EVT_DECLARE_USER_FUNC(evt_hit_damage_return_set, 1)
EVT_DECLARE_USER_FUNC(evt_hitobj_onoff, 3)

}

}