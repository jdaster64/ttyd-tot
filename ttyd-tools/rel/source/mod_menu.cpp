#include "mod_menu.h"

#include "common_functions.h"
#include "common_types.h"
#include "common_ui.h"
#include "mod.h"
#include "mod_state.h"

#include <ttyd/mariost.h>
#include <ttyd/system.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::infinite_pit {

namespace {

// Pre-Pit room name.
const char kStartRoomName[] = "tik_06";

// Menu constants.
const int32_t kMenuX                = -260;
const int32_t kMenuY                = -85;
const int32_t kMenuWidth            = 395;
const int32_t kMenuPadding          = 15;
const int32_t kNumOptionPages       = 8;
const int32_t kOptionsPerPage       = 5;
const int32_t kFadeoutStartTime     = 600;
const int32_t kFadeoutEndTime       = 600 + 15;
const int32_t kFadeinTextStartTime  = 600 + 105;
const int32_t kFadeinTextEndTime    = 600 + 120;
const uint16_t kMenuUpCommand       = ButtonId::Z | ButtonId::DPAD_UP;
const uint16_t kMenuDownCommand     = ButtonId::Z | ButtonId::DPAD_DOWN;
const uint16_t kMenuLeftCommand     = ButtonId::Z | ButtonId::DPAD_LEFT;
const uint16_t kMenuRightCommand    = ButtonId::Z | ButtonId::DPAD_RIGHT;
const uint16_t kMenuSelectCommand   = ButtonId::Z | ButtonId::L;

// For automatically ticking options +1/-1 at a time if a direction is held.
const int32_t kMenuCommandSlowTickStart     = 24;
const int32_t kMenuCommandSlowTickRate      = 12;
const int32_t kMenuCommandFastTickStart     = 96;
const int32_t kMenuCommandFastTickRate      = 2;

// Global menu state variables.
int32_t pages_unlocked_ = 0b111110;
uint16_t last_command_ = 0;
int32_t time_button_held_ = kFadeoutEndTime;
int32_t menu_selection_ = 1;
int32_t menu_page_ = 1;
int32_t menu_state_ = 0;

bool ShouldDisplayMenu() {
    return InMainGameModes() && !strcmp(GetCurrentMap(), kStartRoomName) &&
           !((ttyd::mariost::marioStGetSystemLevel() & 0xf) == 0xf);  // !paused
}

bool ShouldControlMenu() {
    return ShouldDisplayMenu() && !strcmp(GetNextMap(), kStartRoomName) &&
           !g_Mod->state_.GetOptionNumericValue(OPT_HAS_STARTED_RUN);
}

bool ShouldTickOrAutotick(int32_t time_held) {
    if (time_held == 0) return true;
    if (time_held >= kMenuCommandFastTickStart &&
        time_held % kMenuCommandFastTickRate == 0) return true;
    if (time_held >= kMenuCommandSlowTickStart &&
        time_held % kMenuCommandSlowTickRate == 0) return true;
    return false;
}

int32_t GetMenuState(int32_t page, int32_t selection) {
    switch (100 * page + selection) {
        case 101: return OPT_CHEST_REWARDS;
        case 102: return OPT_NO_EXP_MODE;
        case 103: return OPT_BATTLE_REWARD_MODE;
        case 104: return OPT_DISABLE_CHEST_HEAL;
        
        case 201: return OPT_PARTNERS_OBTAINED;
        case 202: return OPT_FIRST_PARTNER;
        case 203: return OPT_PARTNER_RANK;
        case 204: return OPT_BADGE_MOVE_LEVEL;
        
        case 301: return OPT_MOVERS_ENABLED;
        case 302: return OPT_CHET_RIPPO_APPEARANCE;
        case 303: return OPT_MERLEE_CURSE;
        case 304: return OPT_STARTER_ITEMS;
        
        case 401: return OPTNUM_ENEMY_HP;
        case 402: return OPTNUM_ENEMY_ATK;
        case 403: return OPT_BOSS_SCALING;
        case 404: return OPT_FLOOR_100_SCALING;
        
        case 501: return OPTNUM_SP_REGEN_MODIFIER;
        case 502: return OPTNUM_SUPERGUARD_SP_COST;
        case 503: return OPTNUM_SWITCH_PARTY_FP_COST;
        case 504: return MENU_SET_DEFAULT;
        
        case 601: return OPT_PERCENT_BASED_DANGER;
        case 602: return OPT_WEAKER_RUSH_BADGES;
        case 603: return OPT_EVASION_BADGES_CAP;
        case 604: return OPT_64_STYLE_HP_FP_DRAIN;
        
        case 701: return OPT_STAGE_HAZARDS;
        case 702: return OPT_STAGE_RANK;
        case 703: return OPT_RANDOM_DAMAGE;
        case 704: return OPT_AUDIENCE_RANDOM_THROWS;
        
        default:  return MENU_CHANGE_PAGE;
    }
}

uint32_t GetActiveColor(int32_t selection, uint8_t alpha) {
    // Return a dull grey / bright red if selection is "reset to default".
    if (GetMenuState(menu_page_, selection) == MENU_SET_DEFAULT) {
        return (menu_selection_ == selection ? 0xffU << 24 : 0xc0c0c000U) | alpha;
    }
    // Otherwise, return yellow if the option is selected, and white otherwise.
    return (menu_selection_ == selection ? ~0x12ffffU : ~0xffU) | alpha;
}

void DrawMenuString(
    const char* str, float x, float y, uint32_t color, int32_t alignment) {
    DrawText(str, x, y, 0xffu, true, color, 0.75f, alignment);
}

void ChangePage(int32_t direction) {
    for (int32_t i = 0; i < kNumOptionPages; ++i) {
        menu_page_ += direction;
        if (menu_page_ < 1) menu_page_ += kNumOptionPages;
        if (menu_page_ > kNumOptionPages) menu_page_ -= kNumOptionPages;
        if (pages_unlocked_ & (1 << menu_page_)) break;
    }
}

void ChangeSelection(int32_t direction) {
    for (int32_t i = 1; i <= kOptionsPerPage; ++i) {
        menu_selection_ += direction;
        if (menu_selection_ < 1) menu_selection_ += kOptionsPerPage;
        if (menu_selection_ > kOptionsPerPage) menu_selection_ -= kOptionsPerPage;
        if (GetMenuState(menu_page_, menu_selection_) != MENU_EMPTY_OPTION) break;
    }
}

}

void MenuManager::Update() {
    // Not in / leaving Pre-Pit room; prevent input and fade menu / text out.
    if (!ShouldControlMenu()) {
        last_command_ = 0;
        if (time_button_held_ < kFadeoutStartTime) {
            time_button_held_ = kFadeoutStartTime;
        } else if (time_button_held_ < kFadeoutEndTime) {
            ++time_button_held_;
        } else if (time_button_held_ > kFadeoutEndTime) {
            --time_button_held_;
        }
        return;
    }
    
    // If race mode is enabled, prevent input and force-close menu.
    if (g_Mod->state_.GetOptionNumericValue(OPT_RACE_MODE)) {
        if (time_button_held_ < kFadeoutStartTime) {
            time_button_held_ = kFadeoutStartTime;
        }
        last_command_ = 0;
        ++time_button_held_;
        return;
    }
    
    uint16_t buttons = ttyd::system::keyGetButton(0);
    
    if (last_command_ && (buttons & last_command_) == last_command_) {
        // If last command is non-empty and still held, increment timer.
        ++time_button_held_;
    } else {
        const uint16_t prev_command = last_command_;
        uint16_t new_command = 0;
        if ((buttons & kMenuSelectCommand) == kMenuSelectCommand) {
            new_command = kMenuSelectCommand;
        } else if ((buttons & kMenuUpCommand) == kMenuUpCommand) {
            new_command = kMenuUpCommand;
        } else if ((buttons & kMenuDownCommand) == kMenuDownCommand) {
            new_command = kMenuDownCommand;
        } else if ((buttons & kMenuLeftCommand) == kMenuLeftCommand) {
            new_command = kMenuLeftCommand;
        } else if ((buttons & kMenuRightCommand) == kMenuRightCommand) {
            new_command = kMenuRightCommand;
        }
        last_command_ = new_command;
        
        if (new_command && !prev_command && time_button_held_ >= kFadeoutEndTime) {
            // Command input when menu was previously not displayed;
            // no action should be taken until the next command is input.
            time_button_held_ = -999999999;
        } else if (new_command != prev_command) {
            // New command input; reset timer.
            time_button_held_ = 0;
        } else {
            // Still no command input; increment timer anyway.
            if (++time_button_held_ > kFadeinTextEndTime)
                time_button_held_ = kFadeinTextEndTime;
        }
    }
        
    menu_state_ = GetMenuState(menu_page_, menu_selection_);
    StateManager_v2& state = g_Mod->state_;
    
    if (time_button_held_ < 0) return;
    switch (last_command_) {
        case kMenuUpCommand: {
            if (time_button_held_ == 0) ChangeSelection(-1);
            break;
        }
        case kMenuDownCommand: {
            if (time_button_held_ == 0) ChangeSelection(1);
            break;
        }
        case kMenuLeftCommand:
        case kMenuRightCommand:
        case kMenuSelectCommand: {
            int32_t direction = 0;
            if (last_command_ == kMenuRightCommand) direction = 1;
            if (last_command_ == kMenuLeftCommand) direction = -1;
            
            switch (menu_state_) {
                case MENU_CHANGE_PAGE: {
                    if (time_button_held_ == 0) {
                        ChangePage(direction ? direction : 1);
                    }
                    break;
                }
                case MENU_SET_DEFAULT: {
                    if (time_button_held_ == 0 && direction == 0) {
                        state.SetDefaultOptions();
                    }
                    break;
                }
                default: {
                    bool press_registered =
                        (menu_state_ >> 28 >= 3)  // If option is numeric
                            ? ShouldTickOrAutotick(time_button_held_)
                            : time_button_held_ == 0;
                    if (press_registered) {
                        state.ChangeOption(menu_state_, direction);
                    }
                    break;
                }
            }
            break;
        }
    }
}

void MenuManager::Draw() {
    if (!ShouldDisplayMenu()) return;
    
    uint32_t alpha = 0xff;
    
    // Show text displaying how to open the options menu.
    if (!last_command_ && time_button_held_ >= kFadeinTextStartTime) {
        alpha = 0xff * (time_button_held_ - kFadeinTextStartTime)
                     / (kFadeinTextEndTime - kFadeinTextStartTime);
        if (alpha > 0xff) alpha = 0xff;
        
        if (g_Mod->state_.GetOptionNumericValue(OPT_RACE_MODE)) {
            DrawText(
                "Glitz Pit community race settings activated.\n"
                "Options can no longer be changed.",
                0, -150, alpha, true, 0xffed00ffU, 0.75f, 
                /* alignment = center */ 4);
        } else {
            DrawText(
                "Press L+Z to open the options menu, then\n"
                "hold Z and press L or the D-Pad to make selections.",
                0, -150, alpha, true, ~0U, 0.75f, /* alignment = center */ 4);
        }
        return;
    }
    
    // If menu hasn't been touched, but not time for text to pop up, just return.
    if (!last_command_ && time_button_held_ >= kFadeoutEndTime) {
        return;
    }
    
    // Otherwise, draw the menu.
    if (!last_command_ && time_button_held_ >= kFadeoutStartTime) {
        alpha = 0xff * (kFadeoutEndTime - time_button_held_)
                     / (kFadeoutEndTime - kFadeoutStartTime);
    }

    const uint32_t window_color = static_cast<uint8_t>(alpha * 4 / 5);
    
    const uint32_t menu_height = 19 * kOptionsPerPage - 4 + kMenuPadding * 2;
    const uint32_t menu_width = kMenuWidth;
    
    DrawWindow(window_color, kMenuX, kMenuY, menu_width, menu_height, 10);
    
    const int32_t kTextX = kMenuX + kMenuPadding;
    const int32_t kValueX = kMenuX + menu_width - kMenuPadding;
    int32_t kRowY = kMenuY - kMenuPadding - 8;
    
    char name_buf[128];
    char value_buf[64];
    uint32_t color;
    bool is_default, affects_seeding;
    
    const StateManager_v2& state = g_Mod->state_;
    
    for (int32_t selection = 1; selection < kOptionsPerPage; ++selection) {
        // Get text strings / info and color for current option.
        int32_t menu_state = GetMenuState(menu_page_, selection);
        state.GetOptionStrings(
            menu_state, name_buf, value_buf, &is_default, &affects_seeding);
        color = GetActiveColor(selection, alpha);
        // Draw the option's description and current value.
        DrawMenuString(name_buf, kTextX, kRowY, color, /* left-center */ 3);
        // If changed from default, color the value slightly darker and bluer.
        color -= (is_default ? 0 : 0x60400000U);
        DrawMenuString(value_buf, kValueX, kRowY, color, /* right-center */ 5);
        // Advance to the next row's Y position.
        kRowY -= 19;
    }
    
    // Print the current page information in the bottom row.
    color = GetActiveColor(kOptionsPerPage, alpha);
    if (menu_page_ <= 5) {
        sprintf(
            name_buf, "Change Page (%" PRId32 "/%" PRId32 ")", menu_page_, 5);
    } else {
        sprintf(
            name_buf, "Change Page (Bonus %" PRId32 ")", menu_page_ - 5);
    }
    DrawMenuString(name_buf, kTextX, kRowY, color, /* left-center */ 3);
    
    // Print a warning over selections that change seeding.
    state.GetOptionStrings(
        GetMenuState(menu_page_, menu_selection_),
        name_buf, value_buf, &is_default, &affects_seeding);
    if (affects_seeding) {
        sprintf(name_buf, "*Affects seeding");
        DrawText(
            name_buf, kValueX, kRowY + 1, 0xffu, true, 
            /* color = red */ 0xff000000U | alpha, 0.575f, /* right-top */ 2);
    }
}

void MenuManager::SetMenuPageVisibility(int32_t page, bool enabled) {
    if (page < 1 || page > kNumOptionPages) return;
    if (enabled) {
        pages_unlocked_ |= (1 << page);
    } else {
        pages_unlocked_ &= ~(1 << page);
    }
}

}