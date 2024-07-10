#include "patches_field.h"

#include "mod.h"

#include <ttyd/battle_monosiri.h>
#include <ttyd/npc_data.h>
#include <ttyd/npcdrv.h>

#include <cstring>

namespace mod::infinite_pit {

// Declarations of patches.
extern const int32_t g_chorobon_move_event_Patch_SetScale;
extern const int32_t g_chorobon_find_event_Patch_SetScale;
extern const int32_t g_chorobon_lost_event_Patch_SetScale;
extern const int32_t g_chorobon_return_event_Patch_SetScale;

namespace field {

namespace {
    
using ::ttyd::npcdrv::NpcTribeDescription;

namespace BattleUnitType = ::ttyd::battle_database_common::BattleUnitType;

}

void ApplyFixedPatches() {        
    // Correcting heights in NPC tribe description data.
    NpcTribeDescription* tribe_descs = ttyd::npc_data::npcTribe;
    // Shady Paratroopa
    tribe_descs[291].height = 30;
    // Fire Bro
    tribe_descs[293].height = 40;
    // Boomerang Bro
    tribe_descs[294].height = 40;
    // Craw
    tribe_descs[298].height = 40;
    // Atomic Boo
    tribe_descs[148].height = 100;
    
    // Copying tribe description data for Bob-omb, Atomic Boo over slots for
    // Bald + Hyper Bald Clefts, so they can be used for variants.
    memcpy(&tribe_descs[238], &tribe_descs[283], sizeof(NpcTribeDescription));
    memcpy(&tribe_descs[288], &tribe_descs[148], sizeof(NpcTribeDescription));
    // Set unique names + model filenames.
    tribe_descs[238].nameJp = "hyper_bomb";
    tribe_descs[238].modelName = "c_bomhey_h";
    tribe_descs[288].nameJp = "cosmic_boo";
    tribe_descs[288].modelName = "c_atmic_trs_p";

    // Null out set scale events for Fuzzies on overworld.
    memset((void*)g_chorobon_move_event_Patch_SetScale, 0, 0x18);
    memset((void*)g_chorobon_find_event_Patch_SetScale, 0, 0x18);
    memset((void*)g_chorobon_lost_event_Patch_SetScale, 0, 0x18);
    memset((void*)g_chorobon_return_event_Patch_SetScale, 0, 0x18);
    
    // TODO: Move to patches_ui.h?
    // Fix captures / location information in Tattle menu.
    auto* tattle_inf = ttyd::battle_monosiri::battleGetUnitMonosiriPtr(0);
    tattle_inf[BattleUnitType::TOT_COSMIC_BOO].model_name = "c_atmic_trs_p";
    tattle_inf[BattleUnitType::TOT_COSMIC_BOO].pose_name = "Z_1";
    tattle_inf[BattleUnitType::TOT_COSMIC_BOO].location_name = "menu_monosiri_shiga";
    tattle_inf[BattleUnitType::TOT_HYPER_BOB_OMB].model_name = "c_bomhey_h";
    tattle_inf[BattleUnitType::TOT_HYPER_BOB_OMB].pose_name = "BOM_Z_1";
    tattle_inf[BattleUnitType::TOT_HYPER_BOB_OMB].location_name = "menu_monosiri_shiga";
    
}

}  // namespace field
}  // namespace mod::infinite_pit