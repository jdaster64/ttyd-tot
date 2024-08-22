#pragma once

#include <cstdint>

namespace ttyd::filemgr {
    
struct File {
    uint8_t         mState;
    uint8_t         mFileKind;
    uint8_t         unk_02[0x1e];
    const char      mFilename[64];
    uint8_t         unk_60[0x40];
    void**          mpFileData;
    File*           mpNextFile;
    void**          mpDoneCallback;
    int32_t         unk_ac;
} ;

static_assert(sizeof(File) == 0xb0);

extern "C" {

// fileSetCurrentArchiveType
// fileAsync
int32_t fileAsyncf(void* unk1, void* unk2, const char* fn, ...);
// dvdReadDoneCallBack
void fileFree(File* file);
File* _fileAlloc(const char* fn, uint32_t unk2);
// fileAlloc
File* fileAllocf(void* unk1, const char* fn, ...);
// _fileGarbage
// fileGarbageMoveMem
// fileGarbageDataAdrSet
// fileGarbageDataAdrClear
// fileInit

}

}
