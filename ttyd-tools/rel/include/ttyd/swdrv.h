#pragma once

#include <cstdint>

namespace ttyd::swdrv {

extern "C" {

// _swByteGet
// _swByteSet
// _swClear
// _swGet
// _swSet
uint32_t swByteGet(int32_t swIndex);
void swByteSet(int32_t swIndex, int32_t value);
void swClear(uint32_t swfIndex);
uint32_t swGet(uint32_t swfIndex);
void swSet(uint32_t swfIndex);
// swReInit
void swInit();

}

}