#include "tot_gon.h"

#include "evt_cmd.h"
#include "tot_gon_lobby.h"
#include "tot_gon_tower.h"

#include <ttyd/battle_database_common.h>
#include <ttyd/database.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/mapdata.h>

namespace mod::tot::gon {

namespace {

using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

}  // namespace

// Dummy setup data / database definitions (currently empty).
ttyd::battle_database_common::BattleSetupData g_SetupDataTbl[1];
ttyd::database::DatabaseDefinition g_SetupNoTbl[1];

void Prolog() {
    ttyd::mapdata::relSetEvtAddr("gon_00", GetLobbyInitEvt());
    ttyd::mapdata::relSetEvtAddr("gon_01", GetTowerInitEvt());
    ttyd::mapdata::relSetBtlAddr("gon", g_SetupDataTbl, g_SetupNoTbl);
}

EVT_DEFINE_USER_FUNC(evtTot_GetGonBattleDatabasePtr) {
    evtSetValue(evt, evt->evtArguments[0], PTR(g_SetupDataTbl));
    return 2;
}

}  // namespace mod::tot::gon