#include "patches_core.h"

#include "common_functions.h"
#include "common_types.h"
#include "mod.h"
#include "patch.h"
#include "patches_apply.h"
#include "patches_mario_move.h"
#include "patches_options.h"
#include "tot_manager_options.h"
#include "tot_manager_progress.h"
#include "tot_manager_strings.h"
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
    // save_file_patches.s
    void StartSaveSlotData();
    void BranchBackSaveSlotData();
    
    int32_t mapLoad() { return mod::tot::patch::core::LoadMap(); }
    void onMapUnload() { mod::tot::patch::core::OnMapUnloaded(); }

    void saveSlotData(mod::tot::TotSaveSlot* slot_data) {
        mod::g_Mod->state_.Save(slot_data);
    }
}

namespace mod::tot::patch {
    
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
extern const int32_t g_create_main_Patch_SaveFileGameName;
extern const int32_t g_read_all_main_Patch_SaveFileGameName;
extern const int32_t g_cardWriteHeader_Patch_SaveFileGameName;
extern const int32_t g_write_main_Patch_SaveFileGameName;
extern const int32_t g_cardInit_Patch_SaveFileGameName;
extern const int32_t g_cardBufReset_Patch_SaveFileGameName;
extern const int32_t g_cardWrite_SaveSlotData_BH;
extern const int32_t g_cardWrite_SaveSlotData_EH;
extern const int32_t g_cardmgr_Patch_SaveFileName;
extern const int32_t g_cardmgr_Patch_SaveFileGameName;

// All for just changing the hardcoded size of the save data; sigh.
extern const int32_t g_read_all_main_Patch_savesize_1;
extern const int32_t g_read_all_main_Patch_savesize_by_10_1;
extern const int32_t g_read_all_main_Patch_savesize_2;
extern const int32_t g_read_all_main_Patch_savesize_by_10_2;
extern const int32_t g_read_main_Patch_savesize_1;
extern const int32_t g_read_main_Patch_savesize_by_10_1;
extern const int32_t g_read_main_Patch_savesize_2;
extern const int32_t g_read_main_Patch_savesize_by_10_2;
extern const int32_t g_write_main_Patch_savesize_1;
extern const int32_t g_write_main_Patch_savesize_by_10_1;
extern const int32_t g_write_main_Patch_savesize_2;
extern const int32_t g_write_main_Patch_savesize_by_10_2;
extern const int32_t g_create_main_Patch_savesize_1;
extern const int32_t g_create_main_Patch_savesize_2;
extern const int32_t g_create_main_Patch_savesize_by_10_1;
extern const int32_t g_cardInit_Patch_savesize_1;
extern const int32_t g_cardInit_Patch_savesize_2;
extern const int32_t g_cardInit_Patch_savesize_by_10_1;
extern const int32_t g_cardErase_Patch_savesize_1;
extern const int32_t g_cardErase_Patch_savesize_2;
extern const int32_t g_cardErase_Patch_savesize_by_10_1;

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
    if (g_Mod->state_.GetOption(OPT_MERLEE_CURSE)) {
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
bool FreshFileInit() {
    ttyd::mario_pouch::pouchInit();
    // Initialize other systems / data.
    ttyd::evt_badgeshop::badgeShop_init();
    ttyd::evt_yuugijou::yuugijou_init();
    ttyd::evt_johoya::johoya_init();
    
    ttyd::mario::marioSetCharMode(0);
    ttyd::statuswindow::statusWinForceUpdate();
    ttyd::mariost::g_MarioSt->lastFrameRetraceLocalTime = 0ULL;
    // Spawns Mario from the intended loading zone; also has other effects
    // on first file boot like suppressing pulling back the curtain.
    ttyd::mariost::g_MarioSt->flags &= ~1U;
    
    // Set story progress / some tutorial flags.
    ttyd::swdrv::swInit();
    ttyd::swdrv::swByteSet(0, 405);     // post-game vanilla story progress
    ttyd::swdrv::swSet(0);              // Shop tutorial
    ttyd::swdrv::swSet(0xe9);           // Save Block tutorial
    ttyd::swdrv::swSet(0xea);           // Heart Block tutorial
    ttyd::swdrv::swSet(0xeb);           // Item tutorial
    ttyd::swdrv::swSet(0xec);           // Save Block tutorial-related

    // Give core key items.
    ttyd::mario_pouch::pouchGetItem(ItemType::BOOTS);
    ttyd::mario_pouch::pouchGetItem(ItemType::HAMMER);
    
    // Initialize Tower of Trials-specific state & pouch stuff.
    g_Mod->state_.Init();
    OptionsManager::ResetAfterRun();

    // Turn background music on, by default.
    SetSWF(GSWF_BgmEnabled);

    return true;
}

}  // namespace
    
void ApplyFixedPatches() {
    // For debugging utility, save pointer to mod's state right after pouch.
    auto* state = &g_Mod->state_;
    mod::writePatch(
        reinterpret_cast<void*>(0x8041eb04U),
        reinterpret_cast<void*>(&state), sizeof(void*));

    // Load file from new save; replaces old logic completely.
    g_stg0_00_init_trampoline = mod::hookFunction(
        ttyd::event::stg0_00_init, []() { FreshFileInit(); });
        
    // Load file from existing save; replaces old logic completely.
    g_cardCopy2Main_trampoline = mod::hookFunction(
        ttyd::cardmgr::cardCopy2Main, [](int32_t save_file_number) {
            auto* save_data = reinterpret_cast<TotSaveSlot*>(
                *(uintptr_t*)((uintptr_t)ttyd::cardmgr::g_CardmgrWork + 0xa8)
                + save_file_number * sizeof(TotSaveSlot) + 0x2000);

            if (!g_Mod->state_.Load(save_data)) {
                g_CueGameOver = true;
            } else {
                // Apply options specific to loading run in-progress.
                OptionsManager::OnRunResumeFromFileSelect();
            }
        });
    
    // Replace logic to save data to save slot.
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_cardWrite_SaveSlotData_BH),
        reinterpret_cast<void*>(g_cardWrite_SaveSlotData_EH),
        reinterpret_cast<void*>(StartSaveSlotData),
        reinterpret_cast<void*>(BranchBackSaveSlotData));

    // Change name of save file to be distinct from vanilla TTYD's.
    const char kSaveFileName[] = "tot_save_file";
    mod::writePatch(
        reinterpret_cast<void*>(g_cardmgr_Patch_SaveFileName),
        kSaveFileName, sizeof(kSaveFileName));

    // Change displayed name of game in memory card entry.
    const char kGameName[] = "Tower of Trials";
    mod::writePatch(
        reinterpret_cast<void*>(g_cardmgr_Patch_SaveFileGameName),
        kGameName, sizeof(kGameName));

    // Move address referenced for writing game name back 4 bytes.
    mod::writePatch(
        reinterpret_cast<void*>(g_create_main_Patch_SaveFileGameName),
        0x389f1e10  /* addi r4, r31, 0x1e10 */);
    mod::writePatch(
        reinterpret_cast<void*>(g_read_all_main_Patch_SaveFileGameName),
        0x389f1e10  /* addi r4, r31, 0x1e10 */);
    mod::writePatch(
        reinterpret_cast<void*>(g_cardWriteHeader_Patch_SaveFileGameName),
        0x389f1e10  /* addi r4, r31, 0x1e10 */);
    mod::writePatch(
        reinterpret_cast<void*>(g_write_main_Patch_SaveFileGameName),
        0x389f1e10  /* addi r4, r31, 0x1e10 */);
    mod::writePatch(
        reinterpret_cast<void*>(g_cardInit_Patch_SaveFileGameName),
        0x389d1e10  /* addi r4, r29, 0x1e10 */);
    mod::writePatch(
        reinterpret_cast<void*>(g_cardBufReset_Patch_SaveFileGameName),
        0x389d1e10  /* addi r4, r29, 0x1e10 */);
    
    g_OSLink_trampoline = mod::hookFunction(
        gc::OSLink::OSLink, [](OSModuleInfo* new_module, void* bss) {
            bool result = g_OSLink_trampoline(new_module, bss);
            if (new_module != nullptr && result) {
                OnModuleLoaded(new_module);
            }
            return result;
        });
    
    g_seqSetSeq_trampoline = mod::hookFunction(
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
                // If loading a new file, load the player into the opening map.
                mapName = "gon_12";
                beroName = "dokan_2";
            }
            g_seqSetSeq_trampoline(seq, mapName, beroName);
        });
        
    g_msgSearch_trampoline = mod::hookFunction(
        ttyd::msgdrv::msgSearch, [](const char* msg_key) {
            const char* replacement = StringsManager::LookupReplacement(msg_key);
            if (replacement) return replacement;
            return g_msgSearch_trampoline(msg_key);
        });
    
    // Don't play BGM tunes if the BGM option is currently toggled off.
    g_psndBGMOn_f_d_trampoline = mod::hookFunction(
        ttyd::pmario_sound::psndBGMOn_f_d, [](
            uint32_t unk0, const char* name, uint32_t fadein_time,
            uint16_t unk1) {
            // Only disable background music after loading a save file.
            if (!strcmp(GetCurrentArea(), "gon") &&
                !GetSWF(GSWF_BgmEnabled)) {
                return 0U;
            }
            return g_psndBGMOn_f_d_trampoline(unk0, name, fadein_time, unk1);
        });
    
    // Apply patches to seq_mapChangeMain code to run additional logic when
    // loading or unloading a map.
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_seq_mapChangeMain_MapLoad_BH),
        reinterpret_cast<void*>(g_seq_mapChangeMain_MapLoad_EH),
        reinterpret_cast<void*>(StartMapLoad),
        reinterpret_cast<void*>(BranchBackMapLoad));
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_seq_mapChangeMain_OnMapUnload_BH),
        reinterpret_cast<void*>(g_seq_mapChangeMain_OnMapUnload_EH),
        reinterpret_cast<void*>(StartOnMapUnload),
        reinterpret_cast<void*>(BranchBackOnMapUnload));
        
    // Patch to skip demo without pressing A/Start.
    mod::writePatch(
        reinterpret_cast<void*>(g_seq_logoMain_Patch_AlwaysSkipIntro),
        0x60000000U /* nop */);
    // Patch to never autoplay demo, rather than waiting ~19 seconds.
    mod::writePatch(
        reinterpret_cast<void*>(g_titleMain_Patch_NeverPlayDemo),
        0x480001f0 /* unconditional branch 0x1f0 */);

    // Change the hardcoded size of the save data from 0x2260 to 0x3800
    // in a ridiculous number of places. Thanks, inlining!

    const int32_t save_data_size_addresses[12] = {
        g_read_all_main_Patch_savesize_1,
        g_read_all_main_Patch_savesize_2,
        g_read_main_Patch_savesize_1,
        g_read_main_Patch_savesize_2,
        g_write_main_Patch_savesize_1,
        g_write_main_Patch_savesize_2,
        g_create_main_Patch_savesize_1,
        g_create_main_Patch_savesize_2,
        g_cardInit_Patch_savesize_1,
        g_cardInit_Patch_savesize_2,
        g_cardErase_Patch_savesize_1,
        g_cardErase_Patch_savesize_2
    };
    for (int32_t i = 0; i < 12; ++i) {
        const int16_t value = 0x3800;
        mod::writePatch(
            reinterpret_cast<void*>(save_data_size_addresses[i] + 2),
            &value, sizeof(int16_t));
    }

    const int32_t save_data_size_by_10_addresses[9] = {
        g_read_all_main_Patch_savesize_by_10_1,
        g_read_all_main_Patch_savesize_by_10_2,
        g_read_main_Patch_savesize_by_10_1,
        g_read_main_Patch_savesize_by_10_2,
        g_write_main_Patch_savesize_by_10_1,
        g_write_main_Patch_savesize_by_10_2,
        g_create_main_Patch_savesize_by_10_1,
        g_cardInit_Patch_savesize_by_10_1,
        g_cardErase_Patch_savesize_by_10_1,
    };
    for (int32_t i = 0; i < 9; ++i) {
        const int16_t value = 0x3800 >> 4;
        mod::writePatch(
            reinterpret_cast<void*>(save_data_size_by_10_addresses[i] + 2),
            &value, sizeof(int16_t));
    }
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

        // Update the completion percentage every time a map is loaded.
        ProgressManager::RefreshCache();
        ProgressManager::GetOverallProgression();

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
}  // namespace mod::tot::patch