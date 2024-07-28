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
void winIconSet(int32_t icon, gc::vec3* pos, gc::vec3* scale, uint32_t* color);
void winIconGrayInit();
void winIconInit();
void winTexSet_x2(
    uint32_t unk1, uint32_t unk2, gc::vec3* pos, gc::vec3* scale, uint32_t* color);
void winTexInit_x2(void* tpl_file_data);
// winTexSetRot
// unk_JP_US_EU_50_8017c9bc
void winTexSet(uint32_t unk, gc::vec3* pos, gc::vec3* scale, uint32_t* color);
void winTexInit(void* tpl_file_data);
// winFontSetLabel
void winFontSetEdgeWidth(
    gc::vec3* pos, gc::vec3* scale, uint32_t* color, double width,
    const char* str);
void winFontSetEdge(
    gc::vec3* pos, gc::vec3* scale, uint32_t* color, const char* str);
void winFontSetR(
    gc::vec3* pos, gc::vec3* scale, uint32_t* color, const char* str, ...);
void winFontSetWidth(
    gc::vec3* pos, gc::vec3* scale, uint32_t* color, double length,
    const char* str);
void winFontSet(
    gc::vec3* pos, gc::vec3* scale, uint32_t* color, const char* str, ...);
void winFontInit();
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