#include "patches_core.h"

#include "common_functions.h"
#include "common_types.h"
#include "custom_strings.h"
#include "mod.h"
#include "mod_state.h"
#include "patch.h"
#include "patches_apply.h"
#include "patches_mario_move.h"
#include "patches_options.h"
#include "tot_manager_options.h"
#include "tot_gsw.h"
#include "tot_state.h"

#include <gc/OSLink.h>
#include <ttyd/cardmgr.h>
#include <ttyd/event.h>
#include <ttyd/evt_badgeshop.h>
#include <ttyd/evt_johoya.h>
#include <ttyd/evt_yuugijou.h>
#include <ttyd/filemgr.h>
#include <ttyd/item_data.h>
#include <ttyd/mario.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/mariost.h>
#include <ttyd/memory.h>
#include <ttyd/msgdrv.h>
#include <ttyd/pmario_sound.h>
#include <ttyd/seq_mapchange.h>
#include <ttyd/seqdrv.h>
#include <ttyd/statuswindow.h>
#include <ttyd/swdrv.h>
#include <ttyd/system.h>

#include <cstdint>
#include <cstring>

// Assembly patch functions.
extern "C" {
    // map_change_patches.s
    void StartMapLoad();
    void BranchBackMapLoad();
    void StartOnMapUnload();
    void BranchBackOnMapUnload();
    
    int32_t mapLoad() { return mod::infinite_pit::core::LoadMap(); }
    void onMapUnload() { mod::infinite_pit::core::OnMapUnloaded(); }
}

namespace mod::infinite_pit {
    
namespace {

using ::gc::OSLink::OSModuleInfo;
using ::ttyd::mario_pouch::PouchData;
using ::ttyd::seqdrv::SeqIndex;
using ::ttyd::system::getMarioStDvdRoot;

namespace ItemType = ::ttyd::item_data::ItemType;

}

// Function hooks.
extern void (*g_stg0_00_init_trampoline)(void);
extern void (*g_cardCopy2Main_trampoline)(int32_t);
extern bool (*g_OSLink_trampoline)(OSModuleInfo*, void*);
extern const char* (*g_msgSearch_trampoline)(const char*);
extern void (*g_seqSetSeq_trampoline)(SeqIndex, const char*, const char*);
extern uint32_t (*g_psndBGMOn_f_d_trampoline)(
    uint32_t, const char*, uint32_t, uint16_t);
// Patch addresses.
extern const int32_t g_seq_mapChangeMain_MapLoad_BH;
extern const int32_t g_seq_mapChangeMain_MapLoad_EH;
extern const int32_t g_seq_mapChangeMain_OnMapUnload_BH;
extern const int32_t g_seq_mapChangeMain_OnMapUnload_EH;
extern const int32_t g_titleMain_Patch_NeverPlayDemo;
extern const int32_t g_seq_logoMain_Patch_AlwaysSkipIntro;

namespace core {

namespace {

// Global variables.
uintptr_t   g_PitModulePtr = 0;
bool        g_CueGameOver = false;

// Code that runs after linking a new module.
void OnModuleLoaded(OSModuleInfo* module) {
    if (module == nullptr) return;
    int32_t module_id = module->id;
    uintptr_t module_ptr = reinterpret_cast<uintptr_t>(module);
    
    if (module_id == ModuleId::JON) g_PitModulePtr = module_ptr;
    
    // Regardless of module loaded, reset Merlee curses if enabled.
    if (g_Mod->state_.GetOption(tot::OPT_MERLEE_CURSE)) {
        PouchData& pouch = *ttyd::mario_pouch::pouchGetPtr();
        // If the player somehow managed to run out of curses, reset completely.
        if (pouch.merlee_curse_uses_remaining < 1) {
            pouch.turns_until_merlee_activation = -1;
            pouch.next_merlee_curse_type = 0;
        }
        pouch.merlee_curse_uses_remaining = 99;
    }
}

// Initializes various game data when loading a new file; analogous to /
// replaces the behavior of stg0_00_init.
void OnFileLoad(bool new_file = true) {
    if (new_file) {
        ttyd::mario_pouch::pouchInit();
        // Initialize other systems / data.
        ttyd::evt_badgeshop::badgeShop_init();
        ttyd::evt_yuugijou::yuugijou_init();
        ttyd::evt_johoya::johoya_init();
        
        ttyd::mario::marioSetCharMode(0);
        ttyd::statuswindow::statusWinForceUpdate();
        ttyd::mariost::g_MarioSt->lastFrameRetraceLocalTime = 0ULL;
        // Makes Mario spawn walking into the room normally if loading a new file,
        // rather than in place in the center of the room.
        ttyd::mariost::g_MarioSt->flags &= ~1U;
        
        // Set story progress / some tutorial flags.
        ttyd::swdrv::swInit();
        ttyd::swdrv::swByteSet(0, 405);     // post-game vanilla story progress
        ttyd::swdrv::swSet(0xe9);           // Save Block tutorial
        ttyd::swdrv::swSet(0xea);           // Heart Block tutorial
        ttyd::swdrv::swSet(0xeb);           // Item tutorial
        ttyd::swdrv::swSet(0xec);           // Save Block tutorial-related
        
        // Initializes the mod's InfPit state and copies it to the pouch.
        g_Mod->inf_state_.Load(/* new_save = */ true);
        g_Mod->inf_state_.Save();
        
        // Initialize Tower of Trials-specific state & pouch stuff.
        tot::OptionsManager::InitLobby();
    }
}

}
    
void ApplyFixedPatches() {
    g_stg0_00_init_trampoline = patch::hookFunction(
        ttyd::event::stg0_00_init, []() {
            // Replaces existing logic, includes loading the mod's state.
            OnFileLoad(/* new_file = */ true);
        });
        
    g_cardCopy2Main_trampoline = patch::hookFunction(
        ttyd::cardmgr::cardCopy2Main, [](int32_t save_file_number) {
            g_cardCopy2Main_trampoline(save_file_number);
            OnFileLoad(/* new_file = */ false);

            // TODO: Remove this once ToT is changed to use its own save file.
            // If invalid InfPit file loaded, give the player a Game Over.
            if (!g_Mod->inf_state_.Load(/* new_save = */ false)) {
                g_CueGameOver = true;
            }
            // TODO: Apply options if loading file in-progress.
            // tot::OptionsManager::ApplyOptionsOnLoad();
        });
    
    g_OSLink_trampoline = patch::hookFunction(
        gc::OSLink::OSLink, [](OSModuleInfo* new_module, void* bss) {
            bool result = g_OSLink_trampoline(new_module, bss);
            if (new_module != nullptr && result) {
                OnModuleLoaded(new_module);
            }
            return result;
        });
    
    g_seqSetSeq_trampoline = patch::hookFunction(
        ttyd::seqdrv::seqSetSeq, 
        [](SeqIndex seq, const char* mapName, const char* beroName) {
            // Check for failed file load.
            if (g_CueGameOver) {
                seq = SeqIndex::kGameOver;
                mapName = reinterpret_cast<const char*>(1);
                beroName = 0;
                g_CueGameOver = false;
            } else if (
                seq == SeqIndex::kMapChange && !strcmp(mapName, "aaa_00") && 
                !strcmp(beroName, "prologue")) {
                // If loading a new file, load the player into the tower lobby.
                mapName = "gon_00";
                beroName = "w_bero";
            }
            g_seqSetSeq_trampoline(seq, mapName, beroName);
        });
        
    g_msgSearch_trampoline = patch::hookFunction(
        ttyd::msgdrv::msgSearch, [](const char* msg_key) {
            const char* replacement = StringsManager::LookupReplacement(msg_key);
            if (replacement) return replacement;
            return g_msgSearch_trampoline(msg_key);
        });
    
    // Don't play BGM tunes if the BGM option is currently toggled off.
    g_psndBGMOn_f_d_trampoline = patch::hookFunction(
        ttyd::pmario_sound::psndBGMOn_f_d, [](
            uint32_t unk0, const char* name, uint32_t fadein_time,
            uint16_t unk1) {
            if (g_Mod->state_.GetOption(tot::OPT_BGM_DISABLED)) {
                return 0U;
            }
            return g_psndBGMOn_f_d_trampoline(unk0, name, fadein_time, unk1);
        });
    
    // Apply patches to seq_mapChangeMain code to run additional logic when
    // loading or unloading a map.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_seq_mapChangeMain_MapLoad_BH),
        reinterpret_cast<void*>(g_seq_mapChangeMain_MapLoad_EH),
        reinterpret_cast<void*>(StartMapLoad),
        reinterpret_cast<void*>(BranchBackMapLoad));
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_seq_mapChangeMain_OnMapUnload_BH),
        reinterpret_cast<void*>(g_seq_mapChangeMain_OnMapUnload_EH),
        reinterpret_cast<void*>(StartOnMapUnload),
        reinterpret_cast<void*>(BranchBackOnMapUnload));
        
    // Patch to skip demo without pressing A/Start.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_seq_logoMain_Patch_AlwaysSkipIntro),
        0x60000000U /* nop */);
    // Patch to never autoplay demo, rather than waiting ~19 seconds.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_titleMain_Patch_NeverPlayDemo),
        0x480001f0 /* unconditional branch 0x1f0 */);
}

int32_t LoadMap() {
    auto* mario_st = ttyd::mariost::g_MarioSt;
    
    const char* area = ttyd::seq_mapchange::NextArea;
    if (!strcmp(ttyd::seq_mapchange::NextMap, "title")) {
        strcpy(mario_st->unk_14c, mario_st->currentMapName);
        strcpy(mario_st->currentAreaName, "");
        strcpy(mario_st->currentMapName, "");
        ttyd::seqdrv::seqSetSeq(
            ttyd::seqdrv::SeqIndex::kTitle, nullptr, nullptr);
        return 1;
    }
    
    // The main mod rel is also the gon map rel, and is already loaded / linked.
    if (!strcmp(area, "gon")) {
        mario_st->pRelFileBase = mario_st->pMapAlloc;
        ttyd::seq_mapchange::_load(
            mario_st->currentMapName, ttyd::seq_mapchange::NextMap,
            ttyd::seq_mapchange::NextBero);
        return 2;
    }
    
    if (!strcmp(area, "tou")) {
        if (ttyd::seqdrv::seqGetSeq() == ttyd::seqdrv::SeqIndex::kTitle) {
            area = "tou2";
        } else if (!strcmp(ttyd::seq_mapchange::NextMap, "tou_03")) {
            area = "tou2";
        }
    }
    
    // Start loading the relocatable module associated with the current area.
    if (ttyd::filemgr::fileAsyncf(
        nullptr, nullptr, "%s/rel/%s.rel", getMarioStDvdRoot(), area)) {
        auto* file = ttyd::filemgr::fileAllocf(
            nullptr, "%s/rel/%s.rel", getMarioStDvdRoot(), area);
        if (file) {
            // Always load onto heap, since alloc is taken by mod rel.
            auto* module_info = reinterpret_cast<OSModuleInfo*>(
                ttyd::memory::_mapAlloc(
                    ttyd::memory::mapalloc_base_ptr,
                    reinterpret_cast<int32_t>(file->mpFileData[1])));
            mario_st->pRelFileBase = module_info;
            memcpy(
                mario_st->pRelFileBase, *file->mpFileData,
                reinterpret_cast<int32_t>(file->mpFileData[1]));
            ttyd::filemgr::fileFree(file);
        }
        if (mario_st->pRelFileBase != nullptr) {
            memset(&ttyd::seq_mapchange::rel_bss, 0, 0x3c4);
            gc::OSLink::OSLink(
                mario_st->pRelFileBase, &ttyd::seq_mapchange::rel_bss);
        }
        ttyd::seq_mapchange::_load(
            mario_st->currentMapName, ttyd::seq_mapchange::NextMap,
            ttyd::seq_mapchange::NextBero);
        
        reinterpret_cast<void(*)(void)>(mario_st->pRelFileBase->prolog)();
        return 2;
    }
    return 1;
}

void OnMapUnloaded() {
    // If it's not the main REL, run the epilog, and unlink/free it.
    auto* mario_st = ttyd::mariost::g_MarioSt;
    if (mario_st->pRelFileBase != nullptr) {
        if (mario_st->pRelFileBase != mario_st->pMapAlloc) {
            reinterpret_cast<void(*)(void)>(mario_st->pRelFileBase->epilog)();
            gc::OSLink::OSUnlink(mario_st->pRelFileBase);
            ttyd::memory::_mapFree(
                ttyd::memory::mapalloc_base_ptr, mario_st->pRelFileBase);
            mario_st->pRelFileBase = nullptr;
        }
    }
}

}  // namespace core
}  // namespace mod::infinite_pit