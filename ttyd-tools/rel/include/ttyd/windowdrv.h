#pragma once

#include <cstdint>

namespace ttyd::windowdrv {

extern "C" {

// getWakuTexObj
// windowGetPointer
// windowCheckID
// windowDispGX2_Waku_col
void windowDispGX_Waku_col(uint16_t GXTexMapID, uint8_t color[4], float x, float y, float width, float height, float curve);
// windowDispGX_Message
// _windowDispGX_Message
// windowDispGX_System
// windowDispGX_Kanban
// windowMain
// windowDeleteID
// windowDelete
// windowEntry
// windowReInit
// windowTexSetup
// unk_JP_US_PAL_012
// windowInit

}

}