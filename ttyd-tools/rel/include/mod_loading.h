#pragma once

#include <cstdint>

namespace mod::infinite_pit {

// Handles loading additional module code when changing maps.
class LoadingManager {
public:
    // Code that runs every frame.
    static void Update();
    // Code that runs drawing-related code every frame.
    static void Draw();
    
    // Starts the loading process.
    static void StartLoading();
    // Returns whether the loading process is completed.
    static bool HasFinished();
};
 
}