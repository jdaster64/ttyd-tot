#pragma once

#include "evtmgr.h"

#include <cstdint>

namespace ttyd::sac_bakugame {

extern "C" {

// bakuGameDispStar
// bakuGameDisp3D
// bakuGameDisp2D
// bakuGameAudienceSurprise
// bakuGameEnemySurprise
// bakuGameMarioSurprise
// bakuGameAudienceSurpriseReset
// bakuGameEnemySurpriseReset
// bakuGameMarioSurpriseReset
// bakuGameAudienceCanThrowPos
// bakuGamePartyExist
// bakuGameDecideButton
// bakuGameHeihoReturn
// bakuGameMain
// bakuGameBombEntry
EVT_DECLARE_USER_FUNC(bakuGameDecideWeapon, 3)
// end_bakugame
// init_bakugame
// main_star
// star_stone_appear
// GetBakuGamePtr

}

}