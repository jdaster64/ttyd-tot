#pragma once

#include <cstdint>

namespace mod::tot {

class DebugManager {
public:
    // Code that runs every frame.
    static void Update();
    // Code that runs drawing-related code every frame.
    static void Draw();
    
    // Toggles between different debug modes.
    static void ChangeMode();
    // Returns the set of enemies to use for debugging.
    static int32_t* GetEnemies();
    // Sets up certain options when starting a file in Race / 100% mode.
    static void SpecialFileSetup();
};
 
}