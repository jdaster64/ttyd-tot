#include "tot_manager_title.h"

#include "common_functions.h"
#include "common_ui.h"

#include <ttyd/seqdrv.h>
#include <ttyd/seq_title.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::tot {

namespace {

constexpr const char* kTitleInfo = "Project ToT alpha r20 2024-08-20";
    
// constexpr const char* kTitleInfo =
    // "PM:TTYD Infinite Pit v2.40 r64 by jdaster64\n"
    // "https://github.com/jdaster64/ttyd-infinite-pit\n"
    // "Guide / Other mods: https://goo.gl/vjJjVd";
    
}

void TitleScreenManager::Update() {}

void TitleScreenManager::Draw() {
    // If on the title screen and the curtain is not fully down.
    if (CheckSeq(ttyd::seqdrv::SeqIndex::kTitle)) {
        const uint32_t curtain_state = *reinterpret_cast<uint32_t*>(
            reinterpret_cast<uintptr_t>(ttyd::seq_title::seqTitleWorkPointer2)
            + 0x8);
        if (curtain_state >= 2 && curtain_state < 12) {
            // Draw title screen information.
            DrawCenteredTextWindow(
                kTitleInfo, 0, -50, 0xFFu, true, 0xFFFFFFFFu, 0.7f, 
                0x000000E5u, 15, 10);
        }
    }
}

}