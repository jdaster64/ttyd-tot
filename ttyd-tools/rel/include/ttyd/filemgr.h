#pragma once

namespace ttyd::filemgr {
    
struct filemgr__File {
    uint8_t         mState;
    uint8_t         mFileKind;
    uint8_t         unk_02[0x1e];
    const char      mFilename[64];
    uint8_t         unk_60[0x40];
    void**          mpFileData;
    filemgr__File*  mpNextFile;
    void**          mpDoneCallback;
    int32_t         unk_ac;
} __attribute__((__packed__));

static_assert(sizeof(filemgr__File) == 0xb0);

extern "C" {

// fileSetCurrentArchiveType
// fileAsync
int32_t fileAsyncf(void* unk1, void* unk2, const char* fn, ...);
// dvdReadDoneCallBack
void fileFree(filemgr__File* file);
// _fileAlloc
// fileAlloc
filemgr__File* fileAllocf(void* unk1, const char* fn, ...);
// _fileGarbage
// fileGarbageMoveMem
// fileGarbageDataAdrSet
// fileGarbageDataAdrClear
// fileInit

}

}
