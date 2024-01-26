#pragma once

#include <cstdint>

namespace mod::infinite_pit {

class MenuManager {
public:
    // Code that runs every frame.
    static void Update();
    // Code that runs drawing-related code every frame.
    static void Draw();
    
    // Determines whether individual pages of the options menu can be accessed.
    // Used to unlock the two pages of bonus options.
    static void SetMenuPageVisibility(int32_t page, bool enabled);
};
 
}