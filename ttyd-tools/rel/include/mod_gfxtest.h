#pragma once

#include <cstdint>

namespace mod::infinite_pit {

// For testing graphics code.
class GfxTestManager {
public:
    // Code that runs every frame.
    static void Update();
    // Code that runs drawing-related code every frame.
    static void Draw();
};
 
}