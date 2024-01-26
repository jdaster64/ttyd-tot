#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_memcard {

extern "C" {

// .text
EVT_DECLARE_USER_FUNC(memcard_write, 0)
// memcard_file_existance
// memcard_ipl
// memcard_header_write
// memcard_copy
// memcard_delete
// memcard_create
// memcard_format
// memcard_load
// unk_JP_US_EU_64_8025c380
// unk_JP_US_EU_65_8025c3a4
// unk_JP_US_EU_66_8025c40c
// unk_JP_US_EU_67_8025c430
EVT_DECLARE_USER_FUNC(memcard_code, 1)

// .data
extern int32_t evt_memcard_nosave[1];
extern int32_t evt_memcard_check[1];
extern int32_t evt_memcard_start[1];
extern int32_t evt_memcard_delete[1];
extern int32_t evt_memcard_copy[1];
extern int32_t evt_memcard_save[1];
extern int32_t unk_0x803bac3c[1];

}

}