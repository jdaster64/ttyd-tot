#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_case {

extern "C" {

EVT_DECLARE_USER_FUNC(evt_set_case_wrk, 3)
EVT_DECLARE_USER_FUNC(evt_del_case_evt, 2)
EVT_DECLARE_USER_FUNC(evt_exit_case_evt, 0)
// evtRunCaseEntry
EVT_DECLARE_USER_FUNC(evt_run_case_evt_bero, 6)
EVT_DECLARE_USER_FUNC(evt_run_case_evt, 6)

}

}