#pragma once

#include <cstdint>

namespace ttyd::pmario_sound {

extern "C" {

// psndENV_LPF
// psndENVOff_f_d
// psndENVOff
// psndENVOn_f_d
// psndENVOn
// psndENVMain
// psndENV_stop
// psndSFX_get_vol
// psndSFXChk
// psndSFXOff
// psndSFX_dist
// psndSFX_pos
// psndSFX_pit
// psndSFX_vol
// psndSFXOnEx_3D
// psndSFXOn_3D
// psndSFXOnVol
// psndSFXOn
// psndSFXOn_
// __psndSFXOn
// psndSFXMain
// psndBGMPlayTime
// psndBGMStartCheck
// psndBGMScope
// psndBGMChkSilent
// psndBGMChk
// psndBGMOff_f_d
// psndBGMOff
uint32_t psndBGMOn_f_d(
    uint32_t unk0, const char* name, uint32_t fadein_time, uint16_t unk1);
// unk_800db778
// psndBGMOn
// psndBGMMain
// psndBGM_rate
// L_psndBGM_stop
// psndSFXAllOff
// psndMapChange
// psndGetFlag
// psndClearFlag
// psndSetFlag
// psndSetReverb
void psndStopAllFadeOut();
// psndSetPosDirListener
// psndPushGroup
// psndExit
// psndMainInt
// psndMain
// psndInit
// searchPSSFXList
// calc3D
// angleABf

}

}