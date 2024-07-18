#pragma once

#include <cstdint>

namespace ttyd::npcdrv {
struct FbatBattleInformation;
struct NpcEntry;
}

namespace mod::infinite_pit::battle_seq {

// Apply patches to core battle flow (start + end sequences, ...)
void ApplyFixedPatches();

// Calculates how many coins should be granted after battle.
int32_t CalculateCoinDrops(
    ttyd::npcdrv::FbatBattleInformation* battleInfo,
    ttyd::npcdrv::NpcEntry* npc);

// Banks SP regeneration for 3 turns / checks whether regen is active.
void StoreGradualSpRegenEffect();
void CheckGradualSpRegenEffect();

}