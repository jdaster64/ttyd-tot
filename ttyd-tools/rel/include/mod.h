#pragma once

#include "mod_state.h"

#include <cstdint>

namespace mod::infinite_pit {

class Mod {
public:
	Mod();
    
    // Sets up necessary hooks for the mod's code to run.
    void Init();
    // Code that runs every frame.
    void Update();
    // Code that runs drawing-related code every frame.
    void Draw();
    
    // Holds state specific to the Infinite Pit mod.
    StateManager_v2 state_;
};

extern Mod* g_Mod;

}