#pragma once

#include "common_types.h"

#include <cstdint>

namespace mod::infinite_pit::field {

// Apply patches to field-related events in the Pit.
void ApplyFixedPatches();

// Returns an array of back-order items for sale on the shop sign.
int16_t* GetShopBackOrderItems();

}