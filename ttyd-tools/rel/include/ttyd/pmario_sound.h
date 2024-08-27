#pragma once

#include <gc/types.h>

#include <cstdint>

namespace ttyd::pmario_sound {

struct PSndSfxListEntry {
    const char* name;
    uint16_t flags;
    uint16_t id;
    const char* unk_0x08;
    int8_t vol;
    int8_t unk_0x0d;
    int8_t unk_0x0e[6];
};
static_assert(sizeof(PSndSfxListEntry) == 0x14);

extern "C" {

// .text
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
void psndSFX_pit(uint32_t sfxIndex, int16_t pitch);
// psndSFX_vol
// psndSFXOnEx_3D
uint32_t psndSFXOn_3D(const char* id, gc::vec3* position);
// psndSFXOnVol
void psndSFXOn(const char* id);
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

// .data
extern PSndSfxListEntry pssfxlist[4242];
    
}

}