#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_urouro {

extern "C" {

EVT_DECLARE_USER_FUNC(urouro_main_func, 1)
EVT_DECLARE_USER_FUNC(urouro_init_func, 6)

}

}