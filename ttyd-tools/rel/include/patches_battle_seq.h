#pragma once

#include <cstdint>

namespace ttyd::npcdrv {
struct FbatBattleInformation;
struct NpcEntry;
}

namespace mod::tot::patch::battle_seq {

// Apply patches to core battle flow (start + end sequences, ...)
void ApplyFixedPatches();

// Calculates how many coins should be granted after battle.
int32_t CalculateCoinDrops(
    ttyd::npcdrv::FbatBattleInformation* battleInfo,
    ttyd::npcdrv::NpcEntry* npc);

// Banks SP regeneration for 3 turns / checks whether regen is active.
void StoreGradualSpRegenEffect(int32_t turn_count = 3);
void CheckGradualSpRegenEffect();

}