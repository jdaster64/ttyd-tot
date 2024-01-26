#pragma once

#include <cstdint>

namespace gc::OSTime {

extern "C" {

uint64_t OSGetTime();
// OSGetTick
// __OSGetSystemTime();
// GetDates
// OSTicksToCalendarTime

}

}