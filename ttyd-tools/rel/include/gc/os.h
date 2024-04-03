#pragma once

#include <cstdint>

namespace gc::os {

struct ChunkInfo
{
	ChunkInfo *prev;
	ChunkInfo *next;
	uint32_t size;
} ;

struct HeapInfo
{
	uint32_t capacity;
	ChunkInfo *firstFree;
	ChunkInfo *firstUsed;
} ;

extern "C" {

extern HeapInfo *OSAlloc_HeapArray;
extern int OSAlloc_NumHeaps;

}

}