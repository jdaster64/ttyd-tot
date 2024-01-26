#pragma once

#include <cstdint>

namespace ttyd::memory {

extern "C" {

// memInit
// memClear
void *__memAlloc(uint32_t heap, uint32_t size);
void __memFree(uint32_t heap, void *ptr);
void *_mapAlloc(void* heap, uint32_t size);
// _mapAllocTail
void _mapFree(void* heap, void* ptr);
// smartInit
// smartReInit
// smartAutoFree
// smartFree
// smartAlloc
// smartGarbage
// smartTexObj

// .data
extern void* mapalloc_base_ptr;

}

}