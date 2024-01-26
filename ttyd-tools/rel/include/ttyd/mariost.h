#pragma once

#include <gc/types.h>

#include <cstdint>

namespace gc::OSLink {
struct OSModuleInfo;
}

namespace ttyd::mariost {
    
struct MarioSt_Globals {
    uint32_t    flags;
    uint32_t    fps;
    uint32_t    unk_0008;
    int32_t     bDebugMode;
    int32_t     bDvdHasError;
    int32_t     bInBattle;
    uint32_t    systemLevelFlags;
    int32_t     currentRetraceCount;
    uint64_t    lastFrameRetraceLocalTime;
    uint64_t    lastFrameRetraceDeltaTime;
    uint64_t    lastFrameRetraceTime;
    uint64_t    animationTimeIncludingBattle;
    uint64_t    animationTimeNoBattle;
    uint64_t    localTimeSysLvFlag1;
    uint64_t    localTimeSysLvFlag2;
    uint64_t    localTimeSysLvFlag4;
    uint64_t    localTimeSysLvFlag8;
    uint8_t     unk_0068[0x40];
    int16_t     hllFlags;
    uint8_t     unk_00aa[6];    // padding?
    uint64_t    hllSignLastReadTime;
    int16_t     hllSignLastNumber;
    uint8_t     unk_00ba[6];    // padding?
    uint64_t    hllPickLastReceivedTime;
    int16_t     hllPickNumber;
    int16_t     hllGrandPrizeDaysRemaining;
    int16_t     hllSecondPrizeDaysRemaining;
    int16_t     hllThirdPrizeDaysRemaining;
    int16_t     hllFourthPrizeDaysRemaining;
    uint8_t     unk_00d2[0x26];
    uint32_t    nextMapChangeFadeOutType;
    uint32_t    nextMapChangeFadeOutDuration;
    uint32_t    nextMapChangeFadeInType;
    uint32_t    nextMapChangeFadeInDuration;
    uint32_t    nextAreaChangeFadeOutType;
    uint32_t    nextAreaChangeFadeOutDuration;
    uint32_t    nextAreaChangeFadeInType;
    uint32_t    nextAreaChangeFadeInDuration;
    uint32_t    bAreaChanged;
    char        currentBeroName[16];
    char        currentMapName[16];
    char        currentAreaName[16];
    char        unk_14c[16];
    gc::OSLink::OSModuleInfo* pRelFileBase;     // Used for tst, jon modules
    gc::OSLink::OSModuleInfo* pMapAlloc;        // Used for others
    uint32_t    mapInitEvtId;
    void*       fbatData;
    uint32_t    language;
    uint16_t    fbWidth;
    uint16_t    fbHeight;
    int32_t     gsw0;                           // Story sequence position
    uint8_t     gswf[0x400];
    uint8_t     gsw[0x800];
    uint8_t     lswf[0x40];
    uint8_t     lsw[0x400];
    uint32_t    unk_11b8;
    const char  saveFileName[20];
    int32_t     saveFileNumber;
    gc::vec3    saveLastPlayerPosition;
    int32_t     saveParty0Id;
    int32_t     saveParty1Id;
    uint64_t    lastSaveTime;
    int32_t     saveCounter;
    int8_t      unk_11f4[0x134];
    int8_t      misc_gamepad_related_1328[0xb0];
} __attribute__((__packed__));

static_assert(sizeof(MarioSt_Globals) == 0x13d8);
	
extern "C" {

// .text
void marioStInit();
void marioStMain();
void marioStDisp();
void marioStSystemLevel(uint32_t level);
uint32_t marioStGetSystemLevel();
void viPostCallback(uint32_t retraceCount);
void gcDvdCheckThread();
void gcRumbleCheck();

// .data
extern MarioSt_Globals* g_MarioSt;

}

}