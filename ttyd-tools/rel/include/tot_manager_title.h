#pragma once

#include <cstdint>

namespace mod::tot {

class TitleScreenManager {
public:
    // Code that runs every frame.
    static void Update();
    // Code that runs drawing-related code every frame.
    static void Draw();
};
 
}