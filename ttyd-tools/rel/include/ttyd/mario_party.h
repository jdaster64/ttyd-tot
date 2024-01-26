#pragma once

#include <cstdint>
#include <cstddef>

namespace ttyd::mario_party {

extern "C" {

// partyGetTechLv
// partyGetHp
// partyChkJoin
// partyLeft
// partyJoin
// marioGetExtraPartyId
// marioGetPartyId
int32_t marioGetParty();
// marioPartyKill
// marioPartyGoodbye
// marioPartyHello
// marioPartyEntry

}

}