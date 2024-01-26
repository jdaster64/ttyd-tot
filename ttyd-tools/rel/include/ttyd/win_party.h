#pragma once

#include <cstdint>

namespace ttyd::win_party {

extern "C" {
    
struct WinPartyData {
    int32_t     partner_id;     // 1 to 7
    int32_t     icon_id;
    const char* name;
    const char* msg_menu;
    const char* eff_m_str;
    const char* unk_14;         // animation-related?
    const char* unk_18;         // "d_mario" for all partners
    const char* unk_1c;         // animation-related?
    void*       weapon_table;
} __attribute__((__packed__));

static_assert(sizeof(WinPartyData) == 0x24);

// winPartyDisp
// winPartyMain2
// winPartyMain
// winPartyExit
// winPartyInit2
// winPartyInit

extern WinPartyData g_winPartyDt[7];

}

}