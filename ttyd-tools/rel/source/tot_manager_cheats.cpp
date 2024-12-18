#include "tot_manager_cheats.h"

#include "common_functions.h"
#include "common_types.h"
#include "mod.h"
#include "tot_gsw.h"
#include "tot_manager_debug.h"
#include "tot_state.h"

#include <ttyd/item_data.h>
#include <ttyd/mario.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/pmario_sound.h>
#include <ttyd/sound.h>
#include <ttyd/seqdrv.h>
#include <ttyd/swdrv.h>
#include <ttyd/system.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::tot {
    
namespace {
    
namespace ItemType = ::ttyd::item_data::ItemType;

// Constants for secret codes.
uint32_t secretCode_DebugMode       = 036363636;
uint32_t secretCode_BumpAttack      = 043652131;
uint32_t secretCode_CountdownKill   = 037373737;

}

void CheatsManager::Update() {
    // Ignore all cheat codes if not using a debug-enabled file.
    if (!g_Mod->state_.CheckOptionValue(OPTVAL_DEBUG_MODE_ENABLED)) return;

    // Process cheat codes.
    static uint32_t code_history = 0;
    int32_t code = 0;
    if (ttyd::system::keyGetButtonTrg(0) & ButtonId::A) code = 1;
    if (ttyd::system::keyGetButtonTrg(0) & ButtonId::B) code = 2;
    if (ttyd::system::keyGetButtonTrg(0) & ButtonId::L) code = 3;
    if (ttyd::system::keyGetButtonTrg(0) & ButtonId::R) code = 4;
    if (ttyd::system::keyGetButtonTrg(0) & ButtonId::X) code = 5;
    if (ttyd::system::keyGetButtonTrg(0) & ButtonId::Y) code = 6;
    if (ttyd::system::keyGetButtonTrg(0) & ButtonId::Z) code = 7;
    if (code) code_history = (code_history << 3) | code;

    if ((code_history & 0xFFFFFF) == secretCode_DebugMode) {
        code_history = 0;
        DebugManager::ChangeMode();
    }
    
    if ((code_history & 0xFFFFFF) == secretCode_BumpAttack) {
        code_history = 0;
        if (!ttyd::mario_pouch::pouchCheckItem(ItemType::BUMP_ATTACK) && 
            ttyd::mario_pouch::pouchGetHaveBadgeCnt() < 200) {
            ttyd::mario_pouch::pouchGetItem(ItemType::BUMP_ATTACK);
            ttyd::sound::SoundEfxPlayEx(0x265, 0, 0x64, 0x40);
        }
    }

    if ((code_history & 0xFFFFFF) == secretCode_CountdownKill) {
        code_history = 0;
        SetSWByte(GSW_CountdownTimerState, 2);
        ttyd::sound::SoundEfxPlayEx(0x265, 0, 0x64, 0x40);
    }
}

void CheatsManager::Draw() {}

}