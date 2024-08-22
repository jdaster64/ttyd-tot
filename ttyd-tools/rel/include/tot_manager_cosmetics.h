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

struct CosmeticGroupData {
    int32_t icon;
    const char* name_msg;
};

struct AttackFxData {
    const char* name_msg;
    const char* help_msg;
    const char* sounds[4];
    int16_t num_sounds;
    int16_t randomize_pitch;
    int16_t icon;
    int16_t group_id;
};

struct MarioCostumeData {
    const char* name_msg;
    const char* help_msg;
    const char* models[4];
    int16_t icon;
    int16_t group_id;
};

struct YoshiCostumeData {
    const char* name_msg;
    const char* help_msg;
    const char* models[2];
    int16_t icon;
    int16_t icon_hud;
    int32_t group_id;
};
    
class CosmeticsManager {
public:
    // For achievements; returns the CosmeticGroupData for a given type / index.
    static const CosmeticGroupData* GetGroupData(int32_t type, int32_t group_id);

    // Returns whether the current cosmetic type is on / off.
    static bool IsEquipped(int32_t type, int32_t id);
    // Toggles the current cosmetic type on / off; returns whether 'on'.
    static bool ToggleEquipped(int32_t type, int32_t id);
    // Marks the current cosmetic as purchased.
    static void MarkAsPurchased(int32_t type, int32_t id);
    // Returns whether the given cosmetic is unlocked + purchased.
    static bool IsAvailable(int32_t type, int32_t id);

    // Returns the data for the given Attack FX option or Mario / Yoshi costume.
    static const AttackFxData* GetAttackFxData(int32_t id);
    static const MarioCostumeData* GetMarioCostumeData(int32_t id);
    static const YoshiCostumeData* GetYoshiCostumeData(int32_t id);

    // Selects a Yoshi color at random based on currenly active selections.
    static void PickYoshiColor();

    // Selects an Attack FX sound based on currently active selections.
    static int32_t PickActiveFX();
    // Gets the sound effect name to play based on the id.
    static const char* GetFXName(int32_t id);
};

}