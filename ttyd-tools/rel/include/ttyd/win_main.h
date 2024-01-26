#pragma once

#include <gc/types.h>

#include <cstdint>

namespace ttyd::win_main {

extern "C" {

// winLectureCheck
// winLectureOn
// winLectureKeyMask
// unk_JP_US_EU_48_8017b84c
// unk_JP_US_EU_49_8017b864
// winIconSet
// winIconGrayInit
// winIconInit
// winTexSet_x2
// winTexInit_x2
// winTexSetRot
// unk_JP_US_EU_50_8017c9bc
// winTexSet
// winTexInit
// winFontSetLabel
// unk_US_EU_11_8017d10c
void winFontSetEdge(
    gc::vec3* pos, gc::vec3* scale, uint32_t* color, const char* str);
// winFontSetR
void winFontSetPitch(
    double length, gc::vec3* pos, gc::vec3* scale, uint32_t* color,
    const char* str);
void winFontSet(
    gc::vec3* pos, gc::vec3* scale, uint32_t* color, const char* str);
// winFontInit
// winGhostDiaryChk
// cam_r
// unk_JP_US_EU_51_8017d8f8
// party_color
// famicom_check
// itemUseFunc2
// itemUseFunc
// winDispKoopa
// winDisp
// winMain
// winOpenDisable
// winOpenEnable
// winCheck
// winReInit
// winInit
void* winGetPtr();

}

}