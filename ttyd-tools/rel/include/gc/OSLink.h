#pragma once

#include <cstdint>

namespace gc::OSLink {

extern "C" {
    
struct OSModuleInfo {
    uint32_t        id;                     // unique identifier for the module
    void*           next;                   // next and previous modules
    void*           prev;
    uint32_t        num_sections;           // # of sections
    uint32_t        section_info_offset;    // offset to section info table
    uint32_t        name_offset;            // offset to module name
    uint32_t        name_size;              // size of module name
    uint32_t        version;                // version number
    uint32_t        bss_size;
    uint32_t        rel_offset;
    uint32_t        imp_offset;
    uint32_t        imp_size;
    int8_t          prolog_section;
    int8_t          epilog_section;
    int8_t          unresolved_section;
    int8_t          bss_section;
    uint32_t        prolog;
    uint32_t        epilog;
    uint32_t        unresolved;
    uint32_t        align;
    uint32_t        bss_align;
    uint32_t        fix_size;               // only present in v3 rels, unknown
};

// OSNotifyLink
// OSNotifyUnlink
// Relocate
// Link
bool OSLink(OSModuleInfo* new_module, void* bss);
// Undo
bool OSUnlink(OSModuleInfo* module);
// __OSModuleInit

}

}