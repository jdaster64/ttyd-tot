#include "patches_core.h"

#include "common_functions.h"
#include "common_types.h"
#include "custom_enemy.h"
#include "custom_item.h"
#include "custom_strings.h"
#include "mod.h"
#include "mod_loading.h"
#include "mod_state.h"
#include "patch.h"
#include "patches_apply.h"
#include "patches_mario_move.h"
#include "patches_options.h"

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

namespace core {

namespace {

// Global variables.
uintptr_t   g_PitModulePtr = 0;
char        g_LastModuleLoaded[16] = {0};
bool        g_WaitingForCustomLoad = false;
bool        g_PromptSave = false;
bool        g_CueGameOver = false;

// Code that runs after linking a new module.
void OnModuleLoaded(OSModuleInfo* module) {
    if (module == nullptr) return;
    int32_t module_id = module->id;
    uintptr_t module_ptr = reinterpret_cast<uintptr_t>(module);
    
    LinkAllCustomEvts(module, static_cast<ModuleId::e>(module_id), /* link */ true);
    ApplyAllModuleLevelPatches(module, static_cast<ModuleId::e>(module_id));
    
    if (module_id == ModuleId::JON) g_PitModulePtr = module_ptr;
    
    // Regardless of module loaded, reset Merlee curses if enabled.
    if (g_Mod->state_.GetOptionNumericValue(OPT_MERLEE_CURSE)) {
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
        PouchData& pouch = *ttyd::mario_pouch::pouchGetPtr();
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
        ttyd::swdrv::swByteSet(0, 405);     // post-game story progress
        ttyd::swdrv::swSet(0xe9);           // Save Block tutorial
        ttyd::swdrv::swSet(0xea);           // Heart Block tutorial
        ttyd::swdrv::swSet(0xeb);           // Item tutorial
        ttyd::swdrv::swSet(0xec);           // Save Block tutorial-related
        ttyd::swdrv::swSet(0x15d9);         // Star piece in Pit room collected
        
        // Initializes the mod's state and copies it to the pouch.
        g_Mod->state_.Load(/* new_save = */ true);
        g_Mod->state_.Save();
        
        // Update any stats / equipment / flags as necessary.
        ttyd::mario_pouch::pouchGetItem(ItemType::BOOTS);
        ttyd::mario_pouch::pouchGetItem(ItemType::HAMMER);
        ttyd::mario_pouch::pouchSetCoin(0);
        ttyd::mario_pouch::pouchGetItem(ItemType::W_EMBLEM);
        ttyd::mario_pouch::pouchGetItem(ItemType::L_EMBLEM);
        // Start with FX badges equipped if option is set.
        if (g_Mod->state_.GetOptionNumericValue(OPT_START_WITH_FX)) {
            ttyd::mario_pouch::pouchGetItem(ItemType::ATTACK_FX_P);
            ttyd::mario_pouch::pouchGetItem(ItemType::ATTACK_FX_G);
            ttyd::mario_pouch::pouchGetItem(ItemType::ATTACK_FX_B);
            ttyd::mario_pouch::pouchGetItem(ItemType::ATTACK_FX_Y);
            ttyd::mario_pouch::pouchGetItem(ItemType::ATTACK_FX_R);
        }
        ttyd::mario_pouch::pouchGetItem(ItemType::PEEKABOO);
        ttyd::mario_pouch::pouchEquipBadgeID(ItemType::PEEKABOO);
        ttyd::mario_pouch::pouchGetItem(ItemType::FP_PLUS);
        ttyd::mario_pouch::pouchEquipBadgeID(ItemType::FP_PLUS);
        ttyd::mario_pouch::pouchGetItem(ItemType::HP_PLUS);
        ttyd::mario_pouch::pouchEquipBadgeID(ItemType::HP_PLUS);
        
        pouch.current_hp = 15;
        pouch.current_fp = 10;
        // Start with 9 base BP instead of 3 BP to give the player an edge.
        pouch.total_bp = 9;
        pouch.unallocated_bp = 3;
        ttyd::mario_pouch::pouchReviseMarioParam();
        // Assign Yoshi a totally random color.
        ttyd::mario_pouch::pouchSetPartyColor(4, g_Mod->state_.Rand(7));
    }
    g_PromptSave = false;
    
    // If previous file loaded had obfuscated items, turn off and wipe flag.
    if (g_Mod->state_.GetOptionNumericValue(OPT_OBFUSCATE_ITEMS)) {
        g_Mod->state_.SetOption(OPT_OBFUSCATE_ITEMS, 0);
        ObfuscateItems(false);
    }
}

}
    
void ApplyFixedPatches() {
    g_stg0_00_init_trampoline = patch::hookFunction(
        ttyd::event::stg0_00_init, []() {
            // Replaces existing logic, includes loading the mod's state.
            OnFileLoad(/* new_file = */ true);
            options::ApplySettingBasedPatches();
        });
        
    g_cardCopy2Main_trampoline = patch::hookFunction(
        ttyd::cardmgr::cardCopy2Main, [](int32_t save_file_number) {
            g_cardCopy2Main_trampoline(save_file_number);
            OnFileLoad(/* new_file = */ false);
            // If invalid Infinite Pit file loaded, give the player a Game Over.
            if (!g_Mod->state_.Load(/* new_save = */ false)) {
                g_CueGameOver = true;
            }
            options::ApplySettingBasedPatches();
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
            // Initialize Mario's move power when entering/exiting a battle.
            mario_move::OnEnterExitBattle(
                /* is_start = */ seq == SeqIndex::kBattle);
            // Check for failed file load.
            if (g_CueGameOver) {
                seq = SeqIndex::kGameOver;
                mapName = reinterpret_cast<const char*>(1);
                beroName = 0;
                g_CueGameOver = false;
                // Reset RTA timer variables so they don't carry across files.
                g_Mod->state_.pit_start_time_ = 0;
                g_Mod->state_.last_save_time_ = 0;
            } else if (
                seq == SeqIndex::kMapChange && !strcmp(mapName, "aaa_00") && 
                !strcmp(beroName, "prologue")) {
                // If loading a new file, load the player into the pre-Pit room.
                mapName = "tik_06";
                beroName = "e_bero";
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
            if (g_Mod->state_.GetOptionNumericValue(OPT_BGM_DISABLED)) {
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
        reinterpret_cast<void*>(StartOnMapUnload),
        reinterpret_cast<void*>(BranchBackOnMapUnload));
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
    if (!strcmp(area, "tou")) {
        if (ttyd::seqdrv::seqGetSeq() == ttyd::seqdrv::SeqIndex::kTitle) {
            area = "tou2";
        } else if (!strcmp(ttyd::seq_mapchange::NextMap, "tou_03")) {
            area = "tou2";
        }
    }
    
    if (g_WaitingForCustomLoad) {
        if (!LoadingManager::HasFinished()) return 1;
    } else {
        if (!strcmp(area, "jon") && strcmp(g_LastModuleLoaded, "jon")) {
            // If loading the Pit after a different area, load custom REL first.
            LoadingManager::StartLoading();
            g_WaitingForCustomLoad = true;
            return 1;
        } else if (!strcmp(g_LastModuleLoaded, "jon") && strcmp(area, "jon")) {
            // If unloading the Pit, unlink the custom REL.
            gc::OSLink::OSUnlink(mario_st->pMapAlloc);
            strcpy(g_LastModuleLoaded, "");
        }
    }
    
    // Start loading the relocatable module associated with the current area.
    if (ttyd::filemgr::fileAsyncf(
        nullptr, nullptr, "%s/rel/%s.rel", getMarioStDvdRoot(), area)) {
        auto* file = ttyd::filemgr::fileAllocf(
            nullptr, "%s/rel/%s.rel", getMarioStDvdRoot(), area);
        if (file) {
            if (!strncmp(area, "tst", 3) || !strncmp(area, "jon", 3)) {
                auto* module_info = reinterpret_cast<OSModuleInfo*>(
                    ttyd::memory::_mapAlloc(
                        ttyd::memory::mapalloc_base_ptr,
                        reinterpret_cast<int32_t>(file->mpFileData[1])));
                mario_st->pRelFileBase = module_info;
            } else {
                mario_st->pRelFileBase = mario_st->pMapAlloc;
            }
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

        if (g_PitModulePtr) {
            SelectEnemies(g_Mod->state_.floor_);
        }
        g_WaitingForCustomLoad = false;
        strcpy(g_LastModuleLoaded, area);
        reinterpret_cast<void(*)(void)>(mario_st->pRelFileBase->prolog)();
        return 2;
    }
    return 1;
}

void OnMapUnloaded() {
    if (g_PitModulePtr) {
        LinkAllCustomEvts(
            reinterpret_cast<void*>(g_PitModulePtr), ModuleId::JON,
            /* link */ false);
        g_PitModulePtr = 0;
    }
    // Normal unloading logic follows...
}

uintptr_t GetPitModulePtr() { return g_PitModulePtr; }

bool GetShouldPromptSave() { return g_PromptSave; }

void SetShouldPromptSave(bool should_save) { g_PromptSave = should_save; }

}  // namespace core
}  // namespace mod::infinite_pit