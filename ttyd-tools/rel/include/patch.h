#pragma once

#include <cstdint>

namespace mod::patch {

void clear_DC_IC_Cache(void *ptr, uint32_t size);
void writeStandardBranches(void *address, void functionStart(), void functionBranchBack());
void writeBranch(void *ptr, void *destination);
void writeBranchBL(void *ptr, void *destination);
void writeBranchMain(void *ptr, void *destination, uint32_t branch);

// Replaces the data at `destination` with an arbitrary patch.
void writePatch(void* destination, const void* patch_start, const void* patch_end);
void writePatch(void* destination, const void* patch_start, uint32_t patch_len);
void writePatch(void* destination, uint32_t patch_data);

// Wrapper for writing pair of branches, one from the start of existing code
// into new code, and one from the end of new code into existing code.
void writeBranchPair(
    void *original_start, void *original_end,
    void *new_start, void *new_end);
// Same, but assumes the original start and end points are adjacent.
void writeBranchPair(
    void *original_start, void *new_start, void *new_end);

template<typename Func, typename Dest>
Func hookFunction(Func function, Dest destination)
{
	uint32_t *instructions = reinterpret_cast<uint32_t *>(function);
	
	uint32_t *trampoline = new uint32_t[2];
	
	// Original instruction
	trampoline[0] = instructions[0];
	clear_DC_IC_Cache(&trampoline[0], sizeof(uint32_t));
	
	// Branch to original function past hook
	writeBranch(&trampoline[1], &instructions[1]);
	
	// Write actual hook
	writeBranch(&instructions[0], reinterpret_cast<void *>(static_cast<Func>(destination)));
	
	return reinterpret_cast<Func>(trampoline);
}

}