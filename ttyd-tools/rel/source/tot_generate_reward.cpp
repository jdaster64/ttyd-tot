#include "tot_generate_reward.h"

#include "evt_cmd.h"

#include <ttyd/dispdrv.h>
#include <ttyd/evt_mario.h>
#include <ttyd/evt_msg.h>
#include <ttyd/evt_window.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/icondrv.h>
#include <ttyd/item_data.h>
#include <ttyd/swdrv.h>

#include <cstring>

namespace mod::tot {

namespace {

// For convenience.
using namespace ::ttyd::evt_mario;
using namespace ::ttyd::evt_msg;
using namespace ::ttyd::evt_window;

using ::ttyd::dispdrv::CameraId;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::item_data::itemDataTable;

namespace IconType = ::ttyd::icondrv::IconType;
namespace ItemType = ::ttyd::item_data::ItemType;

struct ChestData {
    gc::vec3    home_pos;
    int32_t     item;
    void*       pickup_script;
};
ChestData g_Chests[5];
int32_t g_ChestDrawAlpha = 0;

int32_t GetIcon(ChestData& chest) {
    return itemDataTable[chest.item].icon_id;
}

void DisplayIcons(CameraId camera, void* user_data) {
    // Ideally this would fade, but alpha doesn't seem supported for this cam.
    int32_t value = ttyd::swdrv::swByteGet(1001);
    if (g_ChestDrawAlpha < 0xff && value == 1) g_ChestDrawAlpha = 0xff;
    if (g_ChestDrawAlpha > 0 && value != 1) g_ChestDrawAlpha = 0;
    
    auto* chest = (ChestData*)user_data;
    for (; chest->item; ++chest) {
        gc::vec3 pos = chest->home_pos;
        pos.y += 75.f;
        ttyd::icondrv::iconDispGxAlpha(
            1.0f, &pos, 0, GetIcon(*chest), g_ChestDrawAlpha);
    }
}

// Dummy script to run for picking up an Ultra Shroom specifically.
EVT_BEGIN(DummyChestEvtSpecial)
    USER_FUNC(evt_win_other_select, 19)
    USER_FUNC(
        evt_msg_print_insert, 0, PTR("zz_test_win_select"), 0, 0,
        LW(1), LW(2), LW(3), LW(4))
    USER_FUNC(evt_mario_key_onoff, 1)
    SET(GSW(1000), 1)
    RETURN()
EVT_END()

}  // namespace

// Selects the contents of the chests.
// TODO: Spawn chests based on Mario's current position.
EVT_DEFINE_USER_FUNC(evtTot_GenerateChestContents) {
    memset(g_Chests, 0, sizeof(g_Chests));
    
    g_Chests[0].home_pos = { 0.0, 0.0, -100.0 };
    g_Chests[0].item = ItemType::COIN;
    g_Chests[0].pickup_script = nullptr;
    g_Chests[1].home_pos = { -80.0, 0.0, -100.0 };
    g_Chests[1].item = ItemType::MUSHROOM;
    g_Chests[1].pickup_script = nullptr;
    g_Chests[2].home_pos = { 80.0, 0.0, -100.0 };
    g_Chests[2].item = ItemType::ULTRA_SHROOM;
    g_Chests[2].pickup_script = (void*)DummyChestEvtSpecial;
    
    return 2;
}

// Gets chest's XYZ position and contents.
EVT_DEFINE_USER_FUNC(evtTot_GetChestData) {
    int32_t idx = evtGetValue(evt, evt->evtArguments[0]);
    evtSetValue(evt, evt->evtArguments[1], g_Chests[idx].home_pos.x);
    evtSetValue(evt, evt->evtArguments[2], g_Chests[idx].home_pos.y);
    evtSetValue(evt, evt->evtArguments[3], g_Chests[idx].home_pos.z);
    evtSetValue(evt, evt->evtArguments[4], g_Chests[idx].item);
    evtSetValue(evt, evt->evtArguments[5], PTR(g_Chests[idx].pickup_script));
    return 2;
}

// Displays item icons above the chests.
EVT_DEFINE_USER_FUNC(evtTot_DisplayChestIcons) {
    ttyd::dispdrv::dispEntry(
        CameraId::k3d, 1, /* order = */ 900.f, DisplayIcons, g_Chests);
    return 2;
}

}  // namespace mod::tot