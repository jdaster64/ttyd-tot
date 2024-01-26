#include "patches_misc_fix.h"

#include "patch.h"

#include <ttyd/mario_pouch.h>
#include <ttyd/memory.h>

#include <cstdint>

// Assembly patch functions.
extern "C" {
    // pouch_alloc_patches.s
    void StartCheckPouchAlloc();
    void BranchBackCheckPouchAlloc();
    
    void* getOrAllocPouch(uint32_t heap, uint32_t size) {
        auto* pouch = ttyd::mario_pouch::pouchGetPtr();
        if (pouch) return pouch;
        return ttyd::memory::__memAlloc(heap, size);
    }
}

namespace mod::infinite_pit {

// Function hooks.
// Patch addresses.
extern const int32_t g_titleInit_Patch_EnableCrashHandler;
extern const int32_t g_crashHandler_Patch_LoopForever1;
extern const int32_t g_crashHandler_Patch_LoopForever2;
extern const int32_t g_crashHandler_Patch_FontScale;
extern const int32_t g_msgWindow_Entry_Patch_FixAllocSize;
extern const int32_t g_pouchInit_FixAllocLeak_BH;
extern const int32_t g_itemseq_GetItem_Patch_SkipTutorials;
extern const int32_t g_loadMain_Patch_SkipZeroingGswfs;
extern const int32_t g_continueGame_Patch_SkipZeroingGswfs;

namespace misc_fix {

void ApplyFixedPatches() {
    // Enable the crash handler.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_titleInit_Patch_EnableCrashHandler),
        0x3800FFFFU /* li r0, -1 */);

    // Make the crash handler text loop forever.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_crashHandler_Patch_LoopForever1),
        0x3b400000U /* li r26, 0 */);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_crashHandler_Patch_LoopForever2),
        0x4bfffdd4U /* b -0x22c */);
        
    // Change the size of the crash handler text.
    const float kCrashHandlerNewFontScale = 0.6f;
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_crashHandler_Patch_FontScale),
        &kCrashHandlerNewFontScale, sizeof(float));
        
    // Fix msgWindow off-by-one allocation error.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_msgWindow_Entry_Patch_FixAllocSize),
        0x38830001U /* addi r4, r3, 1 */);
        
    // Fix pouch re-allocating when starting a new file.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_pouchInit_FixAllocLeak_BH),
        reinterpret_cast<void*>(StartCheckPouchAlloc),
        reinterpret_cast<void*>(BranchBackCheckPouchAlloc));

    // Skip tutorials for boots / hammer upgrades.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_itemseq_GetItem_Patch_SkipTutorials),
        0x48000030U /* b 0x30 */);

    // Skip the calls to blank out all GSW(F)s when loading a new file.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_loadMain_Patch_SkipZeroingGswfs),
        0x48000010U /* b 0x10 */);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_continueGame_Patch_SkipZeroingGswfs),
        0x48000010U /* b 0x10 */);
}

}  // namespace misc_fix
}  // namespace mod::infinite_pit