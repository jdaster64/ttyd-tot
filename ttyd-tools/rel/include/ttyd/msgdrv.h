#pragma once

#include <cstdint>

namespace ttyd::msgdrv {

extern "C" {

// selectWindow_Disp
// selectWindow_Main
// msgWindow_Disp
// msgWindow_Clear_Main
// msgWindow_Main
// msgWindow_ForceClose
// msgWindow_Repeat
// msgWindow_Continue
// msgWindow_Add
// msgWindow_Delete
// msgWindow_Entry
// msgIconStr2ID
// msgGetCommand
// _ismbblead
const char* msgSearch(const char* msgKey);
// msg_compare
// ?msgGetWork
// msgAnalize
// msgDispKeyWait_render
// msgDisp
// msgMain
// msgLoad
// msgInit

}

}