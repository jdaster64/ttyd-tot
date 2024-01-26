#include "mod_loading.h"

#include "common_functions.h"
#include "common_types.h"
#include "common_ui.h"

#include <gc/OSLink.h>
#include <ttyd/filemgr.h>
#include <ttyd/mariost.h>
#include <ttyd/memory.h>
#include <ttyd/system.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::infinite_pit {

namespace {

// Describes an imp table entry.
struct ImpEntry {
    uint32_t id;
    uint32_t offset;
};
// Describes a relocation table entry.
struct RelEntry {
    uint16_t offset;
    uint8_t type;
    uint8_t section;
    uint32_t addend;
};

// Describes how a range from a vanilla REL module maps into a custom REL.
struct RelDataMappingInfo {
    int16_t module_id;  // ModuleId::e
    int16_t sec_id;
    int32_t offset_a;   // Vanilla REL offset, relative to section start
    int32_t offset_b;   // Custom REL offset, relative to section start 
    int32_t size;
};
// Data required for non-Pit-native enemies to function.
const RelDataMappingInfo g_LookupTbl[] = {
    { ModuleId::AJI, 1, 0x8aa0, 0x0, 0x2890 },
    { ModuleId::AJI, 1, 0xbda4, 0x2890, 0x4ff8 },
    { ModuleId::AJI, 4, 0x5fb8, 0x0, 0xafc },
    { ModuleId::AJI, 4, 0x6c38, 0xb00, 0x908 },
    { ModuleId::AJI, 5, 0x2c4ec, 0x4, 0x60 },
    { ModuleId::AJI, 5, 0x2c8dc, 0x64, 0x90 },
    { ModuleId::AJI, 5, 0x2ca6c, 0xf4, 0x60 },
    { ModuleId::AJI, 5, 0x2caec, 0x154, 0x60 },
    { ModuleId::AJI, 5, 0x2cb6c, 0x1b4, 0x90 },
    { ModuleId::AJI, 5, 0x2ce5c, 0x244, 0x60 },
    { ModuleId::AJI, 5, 0x2ea18, 0x2a8, 0x85ec },
    { ModuleId::AJI, 5, 0x37350, 0x8898, 0x6cc0 },
    { ModuleId::AJI, 6, 0x1a0, 0x0, 0x14 },
    { ModuleId::AJI, 6, 0x1c0, 0x18, 0x24 },
    { ModuleId::EKI, 4, 0x1b68, 0x1408, 0x1ac },
    { ModuleId::EKI, 5, 0x5910, 0xf558, 0x60 },
    { ModuleId::EKI, 5, 0x12e30, 0xf5b8, 0x17bc },
    { ModuleId::GRA, 1, 0xea8, 0x7888, 0x548 },
    { ModuleId::GRA, 4, 0x1048, 0x15b8, 0x5c8 },
    { ModuleId::GRA, 4, 0x17d8, 0x1b80, 0x148 },
    { ModuleId::GRA, 5, 0x5748, 0x10d78, 0x90 },
    { ModuleId::GRA, 5, 0x5a08, 0x10e08, 0x90 },
    { ModuleId::GRA, 5, 0x5d28, 0x10e98, 0x90 },
    { ModuleId::GRA, 5, 0x6148, 0x10f28, 0x60 },
    { ModuleId::GRA, 5, 0x9ba8, 0x10f88, 0x3f10 },
    { ModuleId::GRA, 5, 0xee38, 0x14e98, 0xa94 },
    { ModuleId::JIN, 1, 0x57d4, 0x7dd0, 0x1544 },
    { ModuleId::JIN, 4, 0x20f0, 0x1cc8, 0x3c0 },
    { ModuleId::JIN, 4, 0x28f8, 0x2088, 0x488 },
    { ModuleId::JIN, 5, 0x104a0, 0x15930, 0x30 },
    { ModuleId::JIN, 5, 0x10e70, 0x15960, 0x30 },
    { ModuleId::JIN, 5, 0x10f40, 0x15990, 0x30 },
    { ModuleId::JIN, 5, 0x14cd0, 0x159c0, 0x1ebc },
    { ModuleId::JIN, 5, 0x19fa8, 0x17880, 0x3e4c },
    { ModuleId::JIN, 6, 0x40, 0x40, 0x8 },
    { ModuleId::LAS, 1, 0xc084, 0x9314, 0x140 },
    { ModuleId::LAS, 1, 0x1174c, 0x9454, 0x140 },
    { ModuleId::LAS, 4, 0x6db8, 0x2510, 0x55c },
    { ModuleId::LAS, 4, 0x8c08, 0x2a70, 0x520 },
    { ModuleId::LAS, 5, 0x21100, 0x1b6d0, 0x30 },
    { ModuleId::LAS, 5, 0x21150, 0x1b700, 0x30 },
    { ModuleId::LAS, 5, 0x211a0, 0x1b730, 0x30 },
    { ModuleId::LAS, 5, 0x211f0, 0x1b760, 0x30 },
    { ModuleId::LAS, 5, 0x3d0e8, 0x1b790, 0x48dc },
    { ModuleId::LAS, 5, 0x57140, 0x20070, 0x3480 },
    { ModuleId::MUJ, 1, 0xb78c, 0x9594, 0xa00 },
    { ModuleId::MUJ, 4, 0x6068, 0x2f90, 0x61c },
    { ModuleId::MUJ, 5, 0x20a78, 0x234f0, 0x90 },
    { ModuleId::MUJ, 5, 0x20c88, 0x23580, 0x30 },
    { ModuleId::MUJ, 5, 0x21278, 0x235b0, 0x60 },
    { ModuleId::MUJ, 5, 0x36660, 0x23610, 0x5034 },
    { ModuleId::MUJ, 6, 0x70, 0x48, 0x54 },
    { ModuleId::TIK, 1, 0x5818, 0x9f94, 0x118 },
    { ModuleId::TIK, 1, 0x5a14, 0xa0ac, 0x430 },
    { ModuleId::TIK, 1, 0x6058, 0xa4dc, 0x874 },
    { ModuleId::TIK, 4, 0x3c30, 0x35b0, 0x154 },
    { ModuleId::TIK, 4, 0x4990, 0x3708, 0x3f4 },
    { ModuleId::TIK, 4, 0x4eb8, 0x3b00, 0x9ac },
    { ModuleId::TIK, 5, 0x18da0, 0x28648, 0x60 },
    { ModuleId::TIK, 5, 0x19300, 0x286a8, 0x60 },
    { ModuleId::TIK, 5, 0x19600, 0x28708, 0x30 },
    { ModuleId::TIK, 5, 0x19650, 0x28738, 0x30 },
    { ModuleId::TIK, 5, 0x196a0, 0x28768, 0x30 },
    { ModuleId::TIK, 5, 0x1cd58, 0x28798, 0xb98 },
    { ModuleId::TIK, 5, 0x21af8, 0x29330, 0x2754 },
    { ModuleId::TIK, 5, 0x25cd0, 0x2ba88, 0x959c },
    { ModuleId::TOU2, 1, 0x510c, 0xad50, 0x2e4 },
    { ModuleId::TOU2, 1, 0x53f0, 0xb034, 0x188 },
    { ModuleId::TOU2, 1, 0x568c, 0xb1bc, 0x188 },
    { ModuleId::TOU2, 1, 0x5c44, 0xb344, 0x700 },
    { ModuleId::TOU2, 1, 0x6344, 0xba44, 0x1c0 },
    { ModuleId::TOU2, 1, 0x6504, 0xbc04, 0x17dc },
    { ModuleId::TOU2, 1, 0x90fc, 0xd3e0, 0x2e4 },
    { ModuleId::TOU2, 1, 0x93e0, 0xd6c4, 0x290 },
    { ModuleId::TOU2, 4, 0x3658, 0x44b0, 0x13c },
    { ModuleId::TOU2, 4, 0x3980, 0x45f0, 0x314 },
    { ModuleId::TOU2, 4, 0x3fe0, 0x4908, 0x124 },
    { ModuleId::TOU2, 4, 0x4510, 0x4a30, 0x130 },
    { ModuleId::TOU2, 4, 0x51e0, 0x4b60, 0x1a8 },
    { ModuleId::TOU2, 4, 0x56d0, 0x4d08, 0x1f4 },
    { ModuleId::TOU2, 4, 0x5b20, 0x4f00, 0x50c },
    { ModuleId::TOU2, 4, 0x6310, 0x5410, 0x9d4 },
    { ModuleId::TOU2, 4, 0x7a40, 0x5de8, 0x61c },
    { ModuleId::TOU2, 4, 0x82a0, 0x6408, 0x380 },
    { ModuleId::TOU2, 5, 0xcef0, 0x35028, 0x90 },
    { ModuleId::TOU2, 5, 0xd1b0, 0x350b8, 0x90 },
    { ModuleId::TOU2, 5, 0xd3f0, 0x35148, 0xc0 },
    { ModuleId::TOU2, 5, 0xd5d0, 0x35208, 0x60 },
    { ModuleId::TOU2, 5, 0xd650, 0x35268, 0x90 },
    { ModuleId::TOU2, 5, 0xd7b0, 0x352f8, 0x90 },
    { ModuleId::TOU2, 5, 0xd8b0, 0x35388, 0x90 },
    { ModuleId::TOU2, 5, 0xda30, 0x35418, 0xc0 },
    { ModuleId::TOU2, 5, 0x16da0, 0x354d8, 0x10e4 },
    { ModuleId::TOU2, 5, 0x19368, 0x365c0, 0x39f8 },
    { ModuleId::TOU2, 5, 0x1f638, 0x39fb8, 0x2058 },
    { ModuleId::TOU2, 5, 0x24f68, 0x3c010, 0x1a7c },
    { ModuleId::TOU2, 5, 0x2e698, 0x3da90, 0x2140 },
    { ModuleId::TOU2, 5, 0x34ef0, 0x3fbd0, 0x1238 },
    { ModuleId::TOU2, 5, 0x37ad8, 0x40e08, 0x49e4 },
    { ModuleId::TOU2, 5, 0x3e368, 0x457f0, 0xab74 },
    { ModuleId::TOU2, 5, 0x52fb8, 0x50368, 0x5344 },
    { ModuleId::TOU2, 5, 0x5a280, 0x556b0, 0x297c },
};

// Various constants (mostly precalculated start addresses for REL tables).
constexpr const uint32_t kImpTableStartAddr             = 0x6c1d8;
constexpr const uint32_t kRelTableCustomSec1StartAddr   = 0x6c1e8;
constexpr const uint32_t kRelTableCustomSec5StartAddr   = 0x6eb00;
constexpr const uint32_t kRelTableCustomEndAddr         = 0x7ae70;
constexpr const uint32_t kRelTableDolSec1StartAddr      = 0x7ae78;
constexpr const uint32_t kRelTableDolSec5StartAddr      = 0x7e078;
constexpr const uint32_t kRelTableDolEndAddr            = 0x901a0;
constexpr const uint32_t kMapAllocSize                  = 0xa7800;  // 670 KiB

// REL header and imp table, with precalculated addresses.
constexpr const uint32_t kCustomRelHeader[] = {
    0x00000028,                 // id
    0x00000000, 0x00000000,     // next, prev module
    0x0000000f,                 // number of sections
    0x0000004c,                 // section table offset
    0x00000000, 0x00000000,     // module name's offset and size
    0x00000003,                 // version
    0x0000009c,                 // .bss section size
    0x0006c1e8,                 // rel table offset
    0x0006c1d8,                 // imp table offset
    0x00000010,                 // imp table size
    0x00000000,                 // prolog, epilog, unresolved, bss sections
    0x00000000,                 // _prolog offset
    0x00000000,                 // _epilog offset
    0x00000000,                 // _unresolved offset        
    0x00000008,                 // align
    0x00000008,                 // bss align    
    0x0006c1e8,                 // fixSize
    0x00000000, 0x00000000,     // section 0 offset/size (unused)
    0x000000c5, 0x0000d954,     // section 1 offset/size (.text)
    0x0000da18, 0x00000004,     // section 2 offset/size (.ctors)
    0x0000da1c, 0x00000004,     // section 3 offset/size (.dtors)
    0x0000da20, 0x00006788,     // section 4 offset/size (.rodata)
    0x000141a8, 0x0005802c,     // section 5 offset/size (.data)
    0x00000000, 0x0000009c,     // section 6 offset/size (.bss)
};
constexpr const uint32_t kCustomRelImpTable[] = {
    0x28, kRelTableCustomSec1StartAddr, // relocations against custom module
    0x0, kRelTableDolSec1StartAddr,     // relocations against .dol
};

enum LoadingStage {
    LOADING_INIT = 0,
    LOADING_FILE_READY,
    LOADING_FILE_WAIT,
    LOADING_FILE_DONE,
    LOADING_LINK,
    LOADING_DONE,
};

// Global variables.
alignas(0x10) char  g_AdditionalRelBss[0x100];
// Used for state during loading process:
LoadingStage loading_stage_ = LOADING_DONE;
gc::OSLink::OSModuleInfo* map_alloc_ptr_;    // Pointer to the custom REL.
void* temp_alloc_ptr_;  // Pointer to the currently processed vanilla REL.
int32_t current_rel_;   // ModuleId::e of currently processed vanilla REL.
// Current table pointers + offsets for writing REL entries.
RelEntry *rel_1_tbl_, *rel_5_tbl_, *dol_1_tbl_, *dol_5_tbl_;
int32_t rel_1_offset_, rel_5_offset_, dol_1_offset_, dol_5_offset_;

// Returns the RAM address based on base pointer, section, and section offset.
uint32_t GetAddress(int32_t sec_id, int32_t offset, bool custom = true) {
    intptr_t rel_base = reinterpret_cast<intptr_t>(
        custom ? map_alloc_ptr_ : temp_alloc_ptr_);
    uint32_t section_base =
        *reinterpret_cast<uint32_t*>(rel_base + 0x4c + (sec_id * 8)) & ~3;
    return rel_base + section_base + offset;
}
// Returns the new section offset based on the old one; returns -1 if not present.
int32_t LookupNewOffset(int32_t sec_id, int32_t offset_a) {
    for (const auto& mapping : g_LookupTbl) {
        if (mapping.module_id == current_rel_ &&
            mapping.sec_id == sec_id &&
            offset_a >= mapping.offset_a && 
            offset_a < mapping.offset_a + mapping.size) {
            return mapping.offset_b + offset_a - mapping.offset_a;
        }
    }
    return -1;
}

void InitializeImpAndRelTables() {
    intptr_t rel_b_base = reinterpret_cast<intptr_t>(map_alloc_ptr_);
    // Copy imp table.
    memcpy(
        reinterpret_cast<void*>(rel_b_base + kImpTableStartAddr),
        kCustomRelImpTable, sizeof(kCustomRelImpTable));
    // Reset pointers to REL table entries per section/imp.
    rel_1_tbl_ = reinterpret_cast<RelEntry*>(
        rel_b_base + kRelTableCustomSec1StartAddr);
    rel_5_tbl_ = reinterpret_cast<RelEntry*>(
        rel_b_base + kRelTableCustomSec5StartAddr);
    dol_1_tbl_ = reinterpret_cast<RelEntry*>(
        rel_b_base + kRelTableDolSec1StartAddr);
    dol_5_tbl_ = reinterpret_cast<RelEntry*>(
        rel_b_base + kRelTableDolSec5StartAddr);
    // Write section change REL entries.
    static const RelEntry sec_1_start{0, 202, 1, 0};
    static const RelEntry sec_5_start{0, 202, 5, 0};
    memcpy(rel_1_tbl_++, &sec_1_start, sizeof(RelEntry));
    memcpy(rel_5_tbl_++, &sec_5_start, sizeof(RelEntry));
    memcpy(dol_1_tbl_++, &sec_1_start, sizeof(RelEntry));
    memcpy(dol_5_tbl_++, &sec_5_start, sizeof(RelEntry));
    // Write end-of-table REL entries.
    static const RelEntry table_end{0, 203, 0, 0};
    memcpy(
        reinterpret_cast<void*>(rel_b_base + kRelTableCustomEndAddr),
        &table_end, sizeof(RelEntry));
    memcpy(
        reinterpret_cast<void*>(rel_b_base + kRelTableDolEndAddr),
        &table_end, sizeof(RelEntry));
    // Reset offsets of last written REL entries.
    rel_1_offset_ = 0;
    rel_5_offset_ = 0;
    dol_1_offset_ = 0;
    dol_5_offset_ = 0;
}

// Copies data from a vanilla REL, and ports necessary relocation table entries.
void CopyDataFromRel() {
    intptr_t rel_a_base = reinterpret_cast<intptr_t>(temp_alloc_ptr_);    
    // Copy data ranges (excluding .bss) from the vanilla REL to the custom REL.
    for (const auto& mapping : g_LookupTbl) {
        if (mapping.module_id == current_rel_ && mapping.sec_id < 6) {
            memcpy(
                reinterpret_cast<void*>(
                    GetAddress(mapping.sec_id, mapping.offset_b, true)),
                reinterpret_cast<void*>(
                    GetAddress(mapping.sec_id, mapping.offset_a, false)),
                mapping.size);
        }
    }
    
    // Port relocation entries needed for the copied data to the custom REL.
    const auto* imp = reinterpret_cast<ImpEntry*>(
        rel_a_base + *reinterpret_cast<int32_t*>(rel_a_base + 0x28));
        
    for (int32_t i = 0; i < 2; ++i) {
        const int32_t link_module_id = imp[i].id;
        const auto* rel = reinterpret_cast<RelEntry*>(rel_a_base + imp[i].offset);
        
        int32_t current_section = 0;
        int32_t offset_a = 0;
        int32_t* offset_b = nullptr;
        RelEntry** tbl_ptr = nullptr;
        for (; rel->type != 203; ++rel) {
            // "Change section" entry.
            if (rel->type == 202) {
                current_section = rel->section;
                offset_a = 0;
                if (current_section == 1) {
                    offset_b = link_module_id ? &rel_1_offset_ : &dol_1_offset_;
                    tbl_ptr = link_module_id ? &rel_1_tbl_ : &dol_1_tbl_;
                } else if (current_section == 5) {
                    offset_b = link_module_id ? &rel_5_offset_ : &dol_5_offset_;
                    tbl_ptr = link_module_id ? &rel_5_tbl_ : &dol_5_tbl_;
                }
                continue;
            }
            offset_a += rel->offset;
            // Look up whether the current rel entry pertains to a copied range.
            int32_t new_offset = LookupNewOffset(current_section, offset_a);
            if (new_offset < 0) continue;
            // Look up the addend to use (if linking to the DOL, it's unchanged).
            uint32_t addend = link_module_id ?
                LookupNewOffset(rel->section, rel->addend) : rel->addend;
            // Update the current offset for the given section/imp.
            uint16_t offset = new_offset - *offset_b;
            *offset_b = new_offset;
            // Add the relocation to the respective table.
            RelEntry new_entry { offset, rel->type, rel->section, addend };
            memcpy((*tbl_ptr)++, &new_entry, sizeof(RelEntry));
        }
    }
}

}

void LoadingManager::Update() {
    switch (loading_stage_) {
        case LOADING_INIT: {
            map_alloc_ptr_ = ttyd::mariost::g_MarioSt->pMapAlloc;            
            // Clear the pre-made alloc, which will be used for the custom REL.
            memset(map_alloc_ptr_, 0, kMapAllocSize);
            // Copy in the REL header.
            memcpy(map_alloc_ptr_, kCustomRelHeader, sizeof(kCustomRelHeader));
            // Initialize imp / rel tables.
            InitializeImpAndRelTables();
            // Reset the counter for the REL id being processed.
            current_rel_ = 0;
            // Advance to the next phase of loading (+ fall through).
            loading_stage_ = LOADING_FILE_READY;
        }
        case LOADING_FILE_READY: {
            // Look for the next REL that needs processing.
            do {
                ++current_rel_;
                bool needed = false;
                for (const auto& mapping : g_LookupTbl) {
                    if (mapping.module_id == current_rel_) {
                        needed = true;
                        break;
                    }
                }
                if (needed) break;
            } while (current_rel_ != ModuleId::MAX_MODULE_ID);
            // If all RELs have been processed, advance to linking stage.
            if (current_rel_ == ModuleId::MAX_MODULE_ID) {
                loading_stage_ = LOADING_LINK;
                break;
            }
            // Else, next REL selected; start loading file (+ fall through).
            loading_stage_ = LOADING_FILE_WAIT;            
        }
        case LOADING_FILE_WAIT: {
            if (ttyd::filemgr::fileAsyncf(
                    nullptr, nullptr, "%s/rel/%s.rel",
                    ttyd::system::getMarioStDvdRoot(),
                    ModuleNameFromId(static_cast<ModuleId::e>(current_rel_)))) {
                // File loaded.
                auto* file = ttyd::filemgr::fileAllocf(
                    nullptr, "%s/rel/%s.rel", 
                    ttyd::system::getMarioStDvdRoot(),
                    ModuleNameFromId(static_cast<ModuleId::e>(current_rel_)));
                if (file) {
                    temp_alloc_ptr_ = ttyd::memory::_mapAlloc(
                        ttyd::memory::mapalloc_base_ptr,
                        reinterpret_cast<int32_t>(file->mpFileData[1]));
                    memcpy(
                        temp_alloc_ptr_, *file->mpFileData,
                        reinterpret_cast<int32_t>(file->mpFileData[1]));
                    ttyd::filemgr::fileFree(file);
                }
                // Ready to copy data.
                loading_stage_ = LOADING_FILE_DONE;
            }
            break;
        }
        case LOADING_FILE_DONE: {
            // Copy data and REL table entries for the current REL.
            CopyDataFromRel();
            // Free the vanilla REL and advance state to be ready for next.
            ttyd::memory::_mapFree(
                ttyd::memory::mapalloc_base_ptr, temp_alloc_ptr_);
            loading_stage_ = LOADING_FILE_READY;
            break;
        }
        case LOADING_LINK: {
            // All necessary data should be copied; link the REL.
            memset(g_AdditionalRelBss, 0, 0x100);
            gc::OSLink::OSLink(map_alloc_ptr_, g_AdditionalRelBss);
            loading_stage_ = LOADING_DONE;
            break;
        }
        case LOADING_DONE: {
            break;
        }
    }
}

void LoadingManager::Draw() {
    if (InMainGameModes() && !HasFinished()) {
        // Print a loading text string to the screen.
        char buf[32];
        if (current_rel_ > 0 && current_rel_ < ModuleId::MAX_MODULE_ID) {
            sprintf(
                buf, "Loading data from %s...", 
                ModuleNameFromId(static_cast<ModuleId::e>(current_rel_)));
            DrawText(
                buf, -260, -176, 0xFF, true, ~0U, 0.75f, /* center-left */ 3);
        }
    }
}

void LoadingManager::StartLoading() { loading_stage_ = LOADING_INIT; }

bool LoadingManager::HasFinished() { return loading_stage_ == LOADING_DONE; }

}