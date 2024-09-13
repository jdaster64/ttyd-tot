#pragma once

#include <cstdint>
#include <cstddef>

namespace ttyd::mario_party {

extern "C" {

// partyGetTechLv
int32_t partyGetHp(int32_t party_idx);
// partyChkJoin
// partyLeft
// partyJoin
int32_t marioGetExtraPartyId();
int32_t marioGetPartyId();
int32_t marioGetParty();
// marioPartyKill
// marioPartyGoodbye
// marioPartyHello
// marioPartyEntry

}

}