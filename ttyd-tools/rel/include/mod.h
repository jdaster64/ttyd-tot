#pragma once

#include "tot_state.h"

#include <cstdint>

namespace mod {

class Mod {
public:
	Mod();
    
    // Sets up necessary hooks for the mod's code to run.
    void Init();
    // Code that runs every frame.
    void Update();
    // Code that runs drawing-related code every frame.
    void Draw();
    
    // Holds state specific to the Tower of Trials mod.
    tot::StateManager state_;
};

extern Mod* g_Mod;

}