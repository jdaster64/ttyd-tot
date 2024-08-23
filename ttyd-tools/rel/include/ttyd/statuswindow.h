#pragma once

#include <cstdint>

namespace ttyd::statuswindow {

struct StatusWinIconData {
    int16_t     x_pos;
    int16_t     y_pos;
    int16_t     icon_id;
    int16_t     pad_0x06;
    float       scale;
};
static_assert(sizeof(StatusWinIconData) == 0xc);

struct StatusWinWork {
    uint32_t    unk_0x00;
    uint16_t    disp_flags;
    uint16_t    unk_0x06;
    float       open_anim_timer;
    float       open_anim_length;
    float       open_ratio;         // 0.0 for fully closed, 1.0 for fully open
    float       open_ratio_temp;
    float       open_ratio_target;
    float       open_anim_timebase; // Always 1.0
    float       current_x;
    float       current_y;
    int8_t      unk_0x28[8];
    float       target_x_open;
    float       target_y_open;
    int8_t      unk_0x38[8];
    float       target_x_closed;
    float       target_y_closed;
    int8_t      unk_0x48[8];
    int16_t     last_hp;
    int16_t     last_max_hp;
    int16_t     last_fp;
    int16_t     last_max_fp;
    int16_t     last_party_member;
    int16_t     last_party_hp;
    int16_t     last_party_max_hp;
    int16_t     last_exp;
    int16_t     last_coins;
    int16_t     last_sp;
    int16_t     last_max_sp;
    int8_t      unk_0x66[2];
    gc::vec3    player_position;
    int32_t     player_idle_time;
    int32_t     leave_open_timer;
    int32_t     player_forced_closed;
    uint32_t    stat_updated_flags;
    int32_t     stat_updating_flags; // 1 hp, 2 fp, 8 party hp, 0x10 coins
    int32_t     stat_blink_timer;
};
static_assert(sizeof(StatusWinWork) == 0x8c);

extern "C" {

// .text
// statusPartyHPBlink
// statusMarioHPBlink
// statusFPBlink
// N_statusClearBlink
// statusAPBlink
// statusGetApPos
// statusWinForceUpdateCoin
void statusWinForceUpdate();
// statusWinCheckUpdate
// statusWinCheck
// statusWinDispOff
// statusWinDispOn
void statusWinForceOff();
// statusWinForceCloseClear
// statusWinForceClose
void statusWinForceOpen();
// statusWinClose
// statusWinOpen
// valueUpdate
// valueCheck
// statusGetValue
void statusWinDisp();
void gaugeDisp(double x, double y, int32_t star_power);
// statusWinMain
// statusWinReInit
// statusWinInit

// .data
extern StatusWinIconData alwaysDt[13];
extern StatusWinIconData partysDt[3];
extern int16_t statusWin_party_icon[8];
extern uint16_t gauge_back[8];
extern uint16_t gauge_wakka[16];
extern StatusWinWork* g_StatusWindowWork;

}

}