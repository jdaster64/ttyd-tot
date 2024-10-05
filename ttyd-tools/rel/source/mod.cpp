#include "mod.h"

#include "common_ui.h"
#include "mod_gfxtest.h"
#include "patch.h"
#include "patches_apply.h"
#include "tot_gon.h"
#include "tot_manager_cheats.h"
#include "tot_manager_debug.h"
#include "tot_manager_timer.h"
#include "tot_manager_title.h"

#include <ttyd/dispdrv.h>
#include <ttyd/fontmgr.h>
#include <ttyd/mariost.h>

#include <cstdint>
#include <cstring>

namespace mod {

void main() {
	Mod* mod = new Mod();
	mod->Init();
    tot::gon::Prolog();
}

}

namespace mod {

// Global instance of Mod class.
Mod* g_Mod = nullptr;
    
namespace {
    
using ::ttyd::dispdrv::CameraId;
    
// Main trampoline to call once-a-frame update logic from.
void (*marioStMain_trampoline_)() = nullptr;

}

Mod::Mod() {}

void Mod::Init() {
    // Initialize global mod instance variable.
	g_Mod = this;
	
    // Hook the game's main function, so Update runs exactly once per frame.
	marioStMain_trampoline_ = mod::hookFunction(
        ttyd::mariost::marioStMain, [](){
            // Call the mod's update and draw functions, then run game code.
            g_Mod->Update();
            g_Mod->Draw();
            marioStMain_trampoline_();
        });

	// Initialize typesetting early (to display mod information on title screen)
	ttyd::fontmgr::fontmgrTexSetup();
	mod::hookFunction(ttyd::fontmgr::fontmgrTexSetup, [](){});
    
    // Hook / patch other functions with custom logic.
    tot::patch::ApplyAllFixedPatches();
}

void Mod::Update() {
    tot::DebugManager::Update();
    tot::CheatsManager::Update();
    tot::TimerManager::Update();
    tot::TitleScreenManager::Update();
}

void Mod::Draw() {
    RegisterDrawCallback(tot::DebugManager::Draw, CameraId::kDebug3d);
    RegisterDrawCallback(tot::CheatsManager::Draw, CameraId::kDebug3d);
    RegisterDrawCallback(tot::TimerManager::Draw, CameraId::k2d);
    RegisterDrawCallback(tot::TitleScreenManager::Draw, CameraId::k2d);
}

}