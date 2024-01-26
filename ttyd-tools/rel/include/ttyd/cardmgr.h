#pragma once

#include <cstdint>

namespace ttyd::cardmgr {

extern "C" {

// memcard_open
// statusCallback
// readCallback
// writeCallback
// createCallback
// formatCallback
// checkCallback
// mountAttachCallback
// mountDetachCallback
// cardFormat
// create_main
// cardCreate
// read_all_main
// cardReadAll
// read_main
// write_header_main
// cardWriteHeader
// write_main
// cardWrite
// cardCopy
// cardErase
void cardCopy2Main(int32_t save_file_number);
// cardMain
uint32_t cardGetCode();
uint32_t cardIsExec();
// unk_JP_US_EU_07_800b2bdc
// unk_JP_US_EU_08_800b2c08
// unk_JP_US_EU_09_800b2c2c
// cardInit
// cardBufReset
// cardGetFilePtr

}

}