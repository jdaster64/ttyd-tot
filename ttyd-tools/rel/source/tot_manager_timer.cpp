#include "tot_manager_timer.h"

#include "common_functions.h"
#include "evt_cmd.h"
#include "mod.h"
#include "tot_state.h"

#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/icondrv.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/mariost.h>

namespace mod::tot {
    
namespace {

using ::ttyd::evtmgr_cmd::evtGetValue;

namespace IconType = ::ttyd::icondrv::IconType;

}  // namespace

void TimerManager::Update() {
    g_Mod->state_.TimerTick();
}

void TimerManager::Draw() {
    auto& state = g_Mod->state_;
    if (!state.GetOption(OPT_RUN_STARTED)) return;
    
    uint64_t time_ticks;
    switch (state.GetOptionValue(OPT_TIMER_DISPLAY)) {
        case OPTVAL_TIMER_IGT: {
            time_ticks = state.current_total_igt_;
            break;
        }
        case OPTVAL_TIMER_RTA: {
            time_ticks = 
                ttyd::mariost::g_MarioSt->lastFrameRetraceTime -
                state.run_start_time_rta_;
            break;
        }
        default: return;
    }
    
    int32_t parts[4];
    DurationTicksToParts(time_ticks, &parts[0], &parts[1], &parts[2], &parts[3]);
    
    gc::mtx34 mtx = { { 0 } };
    mtx.m[0][0] = 1.0f;
    mtx.m[1][1] = 1.0f;
    mtx.m[2][2] = 1.0f;
    mtx.m[0][3] = -260.0f;
    mtx.m[1][3] = -195.0f;
    uint32_t color = ~0U;
    
    // TODO: Draw colons, use different sizes, etc.
    for (int32_t i = 0; i < 4; ++i) {
        ttyd::icondrv::iconDispGxCol(
            &mtx, 0x10, IconType::NUMBER_0 + parts[i] / 10, &color);
        mtx.m[0][3] += 20.f;
        ttyd::icondrv::iconDispGxCol(
            &mtx, 0x10, IconType::NUMBER_0 + parts[i] % 10, &color);
        mtx.m[0][3] += 20.f;
    }
}

EVT_DEFINE_USER_FUNC(evtTot_ToggleIGT) {
    bool toggle = evtGetValue(evt, evt->evtArguments[0]);
    g_Mod->state_.ToggleIGT(toggle);
    return 2;
}

}  // namespace mod::tot