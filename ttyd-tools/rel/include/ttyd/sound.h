#pragma once

#include <cstdint>

namespace ttyd::sound {

extern "C" {

// SoundAIDMACallback
// SoundSSMainInt
// SoundSSMain
// SoundSSMuteOff
// SoundSSMuteOn
// SoundSSCheck
// SoundSSSetPlayFreqCh
// SoundSSSetSrndPanCh
// SoundSSGetVolCh
// SoundSSSetVolCh
// SoundSSSetPanCh
// SoundSSContinueCh
// SoundSSFadeoutCh
// SoundSSStopCh
// SoundSSPlayChEx_main
// SoundSSPlayChEx
// SoundSSPlayCh
// _sscallback
// ssDecodeDPCM
// _ssDVDReadAsync_cache_next
// _ssDVDReadAsync_cache_aram
// cache_flush
// _ssDVDReadAsync_activeChk
// SoundDVDMain
// SoundEfxMain
// SoundEfxCheck
// SoundEfxSetLPF
// SoundEfxSetSrndPan
// SoundEfxSetPan
// SoundEfxSetAux1
// SoundEfxGetVolume
// SoundEfxSetVolume
// SoundEfxSetPitch
// SoundEfxStop
int32_t SoundEfxPlayEx(
    int32_t sound_id, uint32_t unk0 /* channel? */, uint32_t unk1 /* volume? */,
    uint32_t unk2);
// SoundSongCheck
// SoundSongGetVolCh
// SoundSongSetVolCh
// SoundSongFadeinCh
// SoundSongFadeoutCh
// SoundSongStopCh
// SoundSongContinueCh
// SoundSongPlayCh
// SoundCloseCover
// SoundOpenCover
// SoundSetOutputMode
// SoundDropData
// SoundSLibLoadDVD
// SoundLoadDVD2Free
// SoundLoadDVD2PushGroup
// SoundLoadDVD2
// loadDVD_callback
// SoundSetFadeTime
// SoundMainInt
// SoundMain
// SoundInit
// sndFree
// sndMalloc

}

}