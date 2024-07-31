#pragma once

#include <cstdint>

namespace mod::tot {
    
class CosmeticsManager {
public:
    // Selects an Attack FX sound based on currently active selections.
    static int32_t PickActiveFX(bool in_battle);
    // Gets the sound effect name for a given FX id, or nullptr if id == 0.
    static const char* GetFXName(int32_t id);
};

}