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

constexpr const char* kVersion = "v3.02";
constexpr const char* kTitleInfo = "v3.02 r52" "\n" "2025-07-31";

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
                kTitleInfo, 230, -180, 0xFFu, true, 0xFFFFFFFFu, 0.52f, 
                0x000000E5u, 8, 8);
        }
    }
}

const char* TitleScreenManager::GetVersionString() {
    return kVersion;
}

}