#pragma once

#include <cstdint>

namespace mod::tot {

namespace CosmeticType {
    enum e {
        ATTACK_FX = 0,
        MARIO_COSTUME,
        YOSHI_COSTUME,
    };
}

struct CosmeticData {
    int32_t icon;
    const char* name_msg;
    const char* help_msg;
};
    
class CosmeticsManager {
public:
    // Returns the CosmeticData for a given type / id.
    static const CosmeticData* GetData(int32_t type, int32_t id);

    // Selects an Attack FX sound based on currently active selections.
    static int32_t PickActiveFX();
    // Gets the sound effect name to play based on the id.
    static const char* GetFXName(int32_t id);
};

}