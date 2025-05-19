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
    const char* sounds[5];
    int8_t num_sounds;
    int8_t randomize_pitch;
    int16_t price = 5;
    int16_t icon;
    int8_t secret = false;
    int8_t group_id;
};

struct MarioCostumeData {
    const char* name_msg;
    const char* help_msg;
    const char* models[4];
    int16_t price = 5;
    int16_t icon;
    int16_t secret = false;
    int8_t group_id;
    int8_t sort_order;
};

struct YoshiCostumeData {
    const char* name_msg;
    const char* help_msg;
    const char* models[3];
    int16_t price = 5;
    int16_t icon;
    int16_t icon_hud;
    int8_t secret = false;
    int8_t group_id;
    int32_t sort_order;
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
    // Returns whether the given cosmetic is unlockable but not purchased.
    static bool IsPurchaseable(int32_t type, int32_t id);
    // Returns whether the given cosmetic is currently unlocked and purchased.
    static bool IsAvailable(int32_t type, int32_t id);

    // Returns the data for the given Attack FX option or Mario / Yoshi costume.
    static const AttackFxData* GetAttackFxData(int32_t id);
    static const MarioCostumeData* GetMarioCostumeData(int32_t id);
    static const YoshiCostumeData* GetYoshiCostumeData(int32_t id);

    // Returns the number of active cosmetics of the type, and copies a sorted
    // array of all the unlocked cosmetics to arr.
    static int32_t GetActiveCosmetics(int32_t type, int8_t* arr);

    // Returns one past the last valid id for each cosmetic type.
    static int32_t GetCosmeticCount(int32_t type);

    // Selects a Yoshi color at random based on currenly active selections.
    static void PickYoshiColor();

    // Selects an Attack FX sound based on currently active selections.
    static int32_t PickActiveFX();
    // Gets the sound effect name to play based on the id.
    static const char* GetSoundFromFXGroup(int32_t id);
};

}