#pragma once

#include "common_types.h"

#include <ttyd/seqdrv.h>

#include <cstdint>

namespace mod {

// Returns true if the current and next game sequence matches the given one.
bool CheckSeq(ttyd::seqdrv::SeqIndex sequence);
// Returns true if in normal gameplay (not in title, game over, etc. sequence)
bool InMainGameModes();
// Returns true if the player is currently in the pause menu.
bool InPauseMenu();
// Returns the name of the current area.
const char* GetCurrentArea();
// Returns the name of the current map.
const char* GetCurrentMap();
// Returns the name of the map about to be loaded.
const char* GetNextMap();
// Returns the current number of unlocked partners.
int32_t GetNumActivePartners();
// Returns whether the partner (in expected order, not internal order) is unlocked.
bool IsPartnerActive(int32_t idx);

// For custom event support; allows calling subroutines / user_funcs in
// relocatable modules.
# define REL_PTR(module_id, offset) \
    (static_cast<int32_t>( \
        ((0x40 + static_cast<int32_t>(module_id)) << 24) + offset))

// "Links" evt code to a single module, replacing temporary addresses in ops'
// arguments with the corresponding addresses in the module, or vice versa.
// When linking:
// - Assumes all op args in range [0x4000'0000, 0x8000'0000) should be linked.
// When unlinking:
// - Assumes all op args in range [module_ptr, POINTER_BASE) should be unlinked.
// Both functions only support module ids < 0x40.
void LinkCustomEvt(ModuleId::e module_id, void* module_ptr, int32_t* evt);
void UnlinkCustomEvt(ModuleId::e module_id, void* module_ptr, int32_t* evt);

// Returns the number of bits set in a given bitfield.
int32_t CountSetBits(uint32_t x);
// Gets a 32-bit bit mask from [start_bit, end_bit].
// Assumes 0 <= start_bit <= end_bit <= 31.
uint32_t GetBitMask(uint32_t start_bit, uint32_t end_bit);
// Gets the bits [start_bit, end_bit] of x, shifted to start at the ones' place.
uint32_t GetShiftedBitMask(uint32_t x, uint32_t start_bit, uint32_t end_bit);

// Converts a positive integer under a billion to a string with 1000-separators.
// Returns the number of characters printed to the string.
int32_t IntegerToFmtString(
    int32_t val, char* out_buf, int32_t max_val = 999'999'999);
// Converts a duration expressed in centiseconds to OSTicks.
uint64_t DurationCentisecondsToTicks(int32_t val);
// Converts a duration expressed in OSTicks (40.5M / sec) to centiseconds.
uint32_t DurationTicksToCentiseconds(int64_t val);
// Converts a duration expressed in OSTicks (40.5M / sec) to HH:MM:SS.ss parts.
void DurationTicksToParts(
    int64_t val, int32_t* h, int32_t* m, int32_t* s, int32_t* cs);
// Converts a duration expressed in OSTicks (40.5M / sec) to HH:MM:SS.ss format.
// Returns the number of characters printed to the string.
int32_t DurationTicksToFmtString(int64_t val, char* out_buf);
// Converts a duration expressed in centiseconds to HH:MM:SS.ss format.
// Returns the number of characters printed to the string.
int32_t DurationCentisToFmtString(int32_t val, char* out_buf);

// Template functions for abs / min / max / clamping a value to a range.
template <class T> inline T AbsF(const T& value) {
    return value < 0 ? -value : value;
}
template <class T> inline T Min(const T& lhs, const T& rhs) {
    return lhs < rhs ? lhs : rhs;
}
template <class T> inline T Max(const T& lhs, const T& rhs) {
    return lhs > rhs ? lhs : rhs;
}
template <class T> inline T Clamp(const T& value, const T& min_value,
                                  const T& max_value) {
    return Max(Min(value, max_value), min_value);
}
inline double ClampMapRange(
    double value, double i_min, double i_max, double o_min, double o_max) {
    return o_min +
        (Clamp(value, i_min, i_max) - i_min) 
        / (i_max - i_min) 
        * (o_max - o_min);
}

}