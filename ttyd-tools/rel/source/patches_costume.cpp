#include "patches_costume.h"

#include "common_functions.h"
#include "patch.h"
#include "tot_gsw.h"
#include "tot_manager_cosmetics.h"

#include <ttyd/filemgr.h>
#include <ttyd/icondrv.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/memory.h>
#include <ttyd/pmario_sound.h>

#include <cstdint>
#include <cstdio>
#include <cstring>

// Assembly patch functions.
extern "C" {
    // costume_patches.s
    void StartBattleGetPartyIconYoshi();
    void BranchBackBattleGetPartyIconYoshi();
    void StartUnitWinDispYoshiIcon();
    void BranchBackUnitWinDispYoshiIcon();
    void StartWinPartyInitYoshiIcon();
    void BranchBackWinPartyInitYoshiIcon();
    void StartSelectDispPartyYoshiIcon();
    void BranchBackSelectDispPartyYoshiIcon();
    void StartSweetTreatYoshiIcon();
    void BranchBackSweetTreatYoshiIcon();

    int32_t getYoshiIcon(int32_t color) {
        return mod::tot::CosmeticsManager::GetYoshiCostumeData(color)->icon;
    }

    int32_t getYoshiHpIcon(int32_t color) {
        return mod::tot::CosmeticsManager::GetYoshiCostumeData(color)->icon_hud;
    }
}

namespace mod::infinite_pit {

// Function hooks.
extern void (*g_pouchSetPartyColor_trampoline)(int32_t, int32_t);
extern int32_t (*g_pouchGetPartyColor_trampoline)(int32_t);
extern ttyd::filemgr::File* (*g__fileAlloc_trampoline)(const char*, uint32_t);
// Patch addresses.
extern const int32_t g_marioMain_Patch_CheckEmblem1;
extern const int32_t g_marioMain_Patch_CheckEmblem2;
extern const int32_t g_marioSetCharMode_Patch_CheckEmblem1;
extern const int32_t g_marioSetCharMode_Patch_CheckEmblem2;
extern const int32_t g__mario_super_emblem_anim_set_Patch_CheckEmblem1;
extern const int32_t g__mario_super_emblem_anim_set_Patch_CheckEmblem2;
extern const int32_t g_mario_change_Patch_CheckEmblem1;
extern const int32_t g_mario_change_Patch_CheckEmblem2;
extern const int32_t g_partySetFamicomMode_Patch_YoshiColor1;
extern const int32_t g_partySetFamicomMode_Patch_YoshiColor2;
extern const int32_t g_partyReInit_Patch_YoshiColor;
extern const int32_t g_partyEntry2Pos_Patch_YoshiColor;
extern const int32_t g_partyEntry2Hello_Patch_YoshiColor;
extern const int32_t g_partyEntry2_Patch_YoshiColor;
extern const int32_t g_partyEntryMain_Patch_YoshiColor;
extern const int32_t g_winPartyInit_Patch_YoshiColor;
extern const int32_t g_yoshi_original_color_anim_set_Patch_YoshiColor;
extern const int32_t g__battleGetPartyIcon_YoshiIcon_BH;
extern const int32_t g__battleGetPartyIcon_YoshiIcon_EH;
extern const int32_t g_winPartyInit_YoshiIcon_BH;
extern const int32_t g_winPartyInit_YoshiIcon_EH;
extern const int32_t g_evt_unitwin_disp_func_YoshiIcon_BH;
extern const int32_t g_evt_unitwin_disp_func_YoshiIcon_EH;
extern const int32_t g_select_disp_party_YoshiIcon_BH;
extern const int32_t g_select_disp_party_YoshiIcon_EH;
extern const int32_t g_sac_genki_disp_3D_YoshiHeartIcon_BH;
extern const int32_t g_sac_genki_disp_3D_YoshiHeartIcon_EH;

namespace costume {

void ApplyFixedPatches() {
    // Replace filename to load when loading Mario or Yoshi's default model.
    g__fileAlloc_trampoline = patch::hookFunction(
        ttyd::filemgr::_fileAlloc, [](const char* filename, uint32_t unk0) {
            // Replace Mario's models.
            auto* mario_data = tot::CosmeticsManager::GetMarioCostumeData(0);
            for (int32_t i = 0; i < 4; ++i) {
                if (!strcmp(filename, mario_data->models[i])) {
                    int32_t color = tot::GetSWByte(tot::GSW_MarioCostume);
                    filename =
                        tot::CosmeticsManager::GetMarioCostumeData(color)->models[i];
                }
            }
            // Replace Yoshi's models.
            auto* yoshi_data = tot::CosmeticsManager::GetYoshiCostumeData(0);
            for (int32_t i = 0; i < 3; ++i) {
                if (!strcmp(filename, yoshi_data->models[i])) {
                    int32_t color = ttyd::mario_pouch::pouchGetPartyColor(4);
                    filename =
                        tot::CosmeticsManager::GetYoshiCostumeData(color)->models[i];
                }
            }

            // Run original logic.
            return g__fileAlloc_trampoline(filename, unk0);
        });

    // Use 5 bits for Yoshi color instead of 3 to allow for more costumes.
    g_pouchSetPartyColor_trampoline = patch::hookFunction(
        ttyd::mario_pouch::pouchSetPartyColor, [](int32_t party, int32_t color) {
            auto& data = ttyd::mario_pouch::pouchGetPtr()->party_data[party];
            data.flags = (data.flags & 0x7ff) | (color << 11);
        });
    g_pouchGetPartyColor_trampoline = patch::hookFunction(
        ttyd::mario_pouch::pouchGetPartyColor, [](int32_t party) {
            auto& data = ttyd::mario_pouch::pouchGetPtr()->party_data[party];
            int32_t max_color = tot::CosmeticsManager::GetCosmeticCount(
                tot::CosmeticType::YOSHI_COSTUME) - 1;
            int32_t color = Clamp(data.flags >> 11, 0, max_color);
            return color;
        });

    // Replace L/W emblem checks and Yoshi color checks that lead to
    // animPoseEntries with li r3, 0, making them all load the default costume.
    const uint32_t li_r3_0 = 0x38600000;    // li r3, 0
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_marioMain_Patch_CheckEmblem1), li_r3_0);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_marioMain_Patch_CheckEmblem2), li_r3_0);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_marioSetCharMode_Patch_CheckEmblem1), li_r3_0);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_marioSetCharMode_Patch_CheckEmblem2), li_r3_0);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g__mario_super_emblem_anim_set_Patch_CheckEmblem1), li_r3_0);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g__mario_super_emblem_anim_set_Patch_CheckEmblem2), li_r3_0);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_mario_change_Patch_CheckEmblem1), li_r3_0);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_mario_change_Patch_CheckEmblem2), li_r3_0);    
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_partySetFamicomMode_Patch_YoshiColor1), li_r3_0);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_partySetFamicomMode_Patch_YoshiColor2), li_r3_0);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_partyReInit_Patch_YoshiColor), li_r3_0);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_partyEntry2Pos_Patch_YoshiColor), li_r3_0);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_partyEntry2Hello_Patch_YoshiColor), li_r3_0);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_partyEntry2_Patch_YoshiColor), li_r3_0);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_partyEntryMain_Patch_YoshiColor), li_r3_0);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_winPartyInit_Patch_YoshiColor), li_r3_0);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_yoshi_original_color_anim_set_Patch_YoshiColor), li_r3_0);

    // Replace logic that changes the color of Yoshi's icons.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g__battleGetPartyIcon_YoshiIcon_BH),
        reinterpret_cast<void*>(g__battleGetPartyIcon_YoshiIcon_EH),
        reinterpret_cast<void*>(StartBattleGetPartyIconYoshi),
        reinterpret_cast<void*>(BranchBackBattleGetPartyIconYoshi));
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_winPartyInit_YoshiIcon_BH),
        reinterpret_cast<void*>(g_winPartyInit_YoshiIcon_EH),
        reinterpret_cast<void*>(StartWinPartyInitYoshiIcon),
        reinterpret_cast<void*>(BranchBackWinPartyInitYoshiIcon));
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_evt_unitwin_disp_func_YoshiIcon_BH),
        reinterpret_cast<void*>(g_evt_unitwin_disp_func_YoshiIcon_EH),
        reinterpret_cast<void*>(StartUnitWinDispYoshiIcon),
        reinterpret_cast<void*>(BranchBackUnitWinDispYoshiIcon));
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_select_disp_party_YoshiIcon_BH),
        reinterpret_cast<void*>(g_select_disp_party_YoshiIcon_EH),
        reinterpret_cast<void*>(StartSelectDispPartyYoshiIcon),
        reinterpret_cast<void*>(BranchBackSelectDispPartyYoshiIcon));
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_sac_genki_disp_3D_YoshiHeartIcon_BH),
        reinterpret_cast<void*>(g_sac_genki_disp_3D_YoshiHeartIcon_EH),
        reinterpret_cast<void*>(StartSweetTreatYoshiIcon),
        reinterpret_cast<void*>(BranchBackSweetTreatYoshiIcon));

    // Change base volume of some Attack FX sounds to be more consistent.
    // Peach voice clips...
    ttyd::pmario_sound::pssfxlist[245].vol = 100;
    ttyd::pmario_sound::pssfxlist[252].vol = 100;
    ttyd::pmario_sound::pssfxlist[255].vol = 100;
    // Bowser voice clips...
    ttyd::pmario_sound::pssfxlist[270].vol = 100;
    ttyd::pmario_sound::pssfxlist[273].vol = 100;
    ttyd::pmario_sound::pssfxlist[278].vol = 100;
    ttyd::pmario_sound::pssfxlist[290].vol = 100;
}

}  // namespace costume
}  // namespace mod::infinite_pit