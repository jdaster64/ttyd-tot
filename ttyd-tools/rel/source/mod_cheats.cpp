#include "mod_cheats.h"

#include "common_functions.h"
#include "common_types.h"
#include "common_ui.h"
#include "mod.h"
#include "mod_debug.h"
#include "mod_menu.h"
#include "mod_state.h"

#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/pmario_sound.h>
#include <ttyd/sound.h>
#include <ttyd/swdrv.h>
#include <ttyd/system.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::infinite_pit {
    
namespace {
    
namespace ItemType = ::ttyd::item_data::ItemType;

// Constants for secret codes.
uint32_t secretCode_BonusOptions1   = 012651265;
uint32_t secretCode_BonusOptions2   = 043652131;
uint32_t secretCode_BonusOptions3   = 031313141;
uint32_t secretCode_RaceMode        = 013341336;
uint32_t secretCode_RtaTimer        = 034345566;
uint32_t secretCode_ShowAtkDef      = 023122312;
uint32_t secretCode_UnlockFxBadges  = 026122146;
uint32_t secretCode_ObfuscateItems  = 046362123;
uint32_t secretCode_DebugMode       = 036363636;

bool g_DrawRtaTimer = false;

}

void CheatsManager::Update() {
    // Process cheat codes.
    static uint32_t code_history = 0;
    int32_t code = 0;
    if (ttyd::system::keyGetButtonTrg(0) & ButtonId::A) code = 1;
    if (ttyd::system::keyGetButtonTrg(0) & ButtonId::B) code = 2;
    if (ttyd::system::keyGetButtonTrg(0) & ButtonId::L) code = 3;
    if (ttyd::system::keyGetButtonTrg(0) & ButtonId::R) code = 4;
    if (ttyd::system::keyGetButtonTrg(0) & ButtonId::X) code = 5;
    if (ttyd::system::keyGetButtonTrg(0) & ButtonId::Y) code = 6;
    if (code) code_history = (code_history << 3) | code;
    
    if (g_Mod->state_.GetOptionNumericValue(OPT_RACE_MODE)) {
        // Automatically turn on RTA timer if loading race mode file.
        g_DrawRtaTimer = true;
    }
    
    if ((code_history & 0xFFFFFF) == secretCode_RaceMode) {
        code_history = 0;
        if (InMainGameModes() && 
            !g_Mod->state_.GetOptionNumericValue(OPT_HAS_STARTED_RUN) &&
            !g_Mod->state_.GetOptionNumericValue(OPT_RACE_MODE)) {
            // Enable standard options for community races.
            g_DrawRtaTimer = true;
            g_Mod->state_.EnableRaceOptions();
            ttyd::sound::SoundEfxPlayEx(0x265, 0, 0x64, 0x40);
        }
    }
    if ((code_history & 0xFFFFFF) == secretCode_RtaTimer) {
        code_history = 0;
        // Display the RTA time since the current Pit run was started.
        g_DrawRtaTimer = true;
        ttyd::sound::SoundEfxPlayEx(0x265, 0, 0x64, 0x40);
    }
    if ((code_history & 0xFFFFFF) == secretCode_BonusOptions1) {
        code_history = 0;
        // Unlock the first page of bonus options.
        MenuManager::SetMenuPageVisibility(6, true);
        ttyd::sound::SoundEfxPlayEx(0x265, 0, 0x64, 0x40);
    }
    if ((code_history & 0xFFFFFF) == secretCode_BonusOptions2) {
        code_history = 0;
        // Unlock the second page of bonus options.
        MenuManager::SetMenuPageVisibility(7, true);
        ttyd::sound::SoundEfxPlayEx(0x265, 0, 0x64, 0x40);
    }
    if ((code_history & 0xFFFFFF) == secretCode_BonusOptions3) {
        code_history = 0;
        // Toggle on/off background music from playing or starting.
        g_Mod->state_.ChangeOption(OPT_BGM_DISABLED);
        if (g_Mod->state_.GetOptionNumericValue(OPT_BGM_DISABLED)) {
            ttyd::pmario_sound::psndStopAllFadeOut();
        }
        ttyd::sound::SoundEfxPlayEx(0x265, 0, 0x64, 0x40);
    }
    if ((code_history & 0xFFFFFF) == secretCode_ShowAtkDef) {
        code_history = 0;
        // Toggle on/off ability to show ATK/DEF of enemies by default.
        // (Cannot be turned off if in race mode)
        if (!g_Mod->state_.GetOptionNumericValue(OPT_RACE_MODE)) {
            g_Mod->state_.ChangeOption(OPT_SHOW_ATK_DEF);
        }
        ttyd::sound::SoundEfxPlayEx(0x265, 0, 0x64, 0x40);
    }
    if ((code_history & 0xFFFFFF) == secretCode_UnlockFxBadges) {
        code_history = 0;
        // Check Journal for whether the FX badges were already unlocked.
        bool has_fx_badges = ttyd::swdrv::swGet(
            ItemType::ATTACK_FX_R - ItemType::POWER_JUMP + 0x80);
        if (!has_fx_badges && ttyd::mario_pouch::pouchGetHaveBadgeCnt() < 196) {
            ttyd::mario_pouch::pouchGetItem(ItemType::ATTACK_FX_P);
            ttyd::mario_pouch::pouchGetItem(ItemType::ATTACK_FX_G);
            ttyd::mario_pouch::pouchGetItem(ItemType::ATTACK_FX_B);
            ttyd::mario_pouch::pouchGetItem(ItemType::ATTACK_FX_Y);
            ttyd::mario_pouch::pouchGetItem(ItemType::ATTACK_FX_R);
            ttyd::sound::SoundEfxPlayEx(0x265, 0, 0x64, 0x40);
        }
    }
    if ((code_history & 0xFFFFFF) == secretCode_ObfuscateItems) {
        code_history = 0;
        if (InMainGameModes() && 
            !g_Mod->state_.GetOptionNumericValue(OPT_HAS_STARTED_RUN)) {
            g_Mod->state_.ChangeOption(OPT_OBFUSCATE_ITEMS);
            ttyd::sound::SoundEfxPlayEx(0x3c, 0, 0x64, 0x40);
        }
    }
    // TODO: Turn off before releases?
    if ((code_history & 0xFFFFFF) == secretCode_DebugMode) {
        code_history = 0;
        DebugManager::ChangeMode();
        g_Mod->state_.SetOption(OPT_DEBUG_MODE_USED, 1);
    }
}

void CheatsManager::Draw() {
    if (InMainGameModes() && g_DrawRtaTimer &&
        g_Mod->state_.GetOptionNumericValue(OPT_HAS_STARTED_RUN)) {
        // Print the current RTA timer and floor number to the screen.
        char buf[32];
        sprintf(buf, "%s", g_Mod->state_.GetCurrentTimeString());
        DrawText(buf, -260, -195, 0xFF, true, ~0U, 0.75f, /* center-left */ 3);
        if (!strcmp(GetCurrentArea(), "jon")) {
            sprintf(buf, "Floor %" PRId32, g_Mod->state_.floor_ + 1);
            DrawText(
                buf, 260, -195, 0xFF, true, ~0U, 0.75f, /* center-right */ 5);
        }
    }
}

}