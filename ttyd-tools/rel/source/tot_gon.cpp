#include "tot_gon.h"

#include "tot_gon_lobby.h"
#include "tot_gon_tower.h"

#include <ttyd/battle_database_common.h>
#include <ttyd/database.h>
#include <ttyd/mapdata.h>

namespace mod::tot::gon {

namespace {

}  // namespace

// Dummy setup data / database definitions (currently empty).
ttyd::battle_database_common::BattleSetupData g_SetupDataTbl[1];
ttyd::database::DatabaseDefinition g_SetupNoTbl[1];

void Prolog() {
    ttyd::mapdata::relSetEvtAddr("gon_00", GetLobbyInitEvt());
    ttyd::mapdata::relSetEvtAddr("gon_01", GetTowerInitEvt());
    ttyd::mapdata::relSetBtlAddr("gon", g_SetupDataTbl, g_SetupNoTbl);
}

}  // namespace mod::tot::gon