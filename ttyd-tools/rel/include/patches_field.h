#pragma once

#include "common_types.h"

#include <cstdint>

namespace mod::tot::patch::field {

// Apply patches to field-related events in the Pit.
void ApplyFixedPatches();

// Returns an array of back-order items for sale on the shop sign.
int16_t* GetShopBackOrderItems();

}