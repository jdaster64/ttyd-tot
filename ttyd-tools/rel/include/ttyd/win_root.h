#pragma once

#include <cstdint>

// Forward declarations.
namespace ttyd::effdrv { struct EffEntry; }
namespace ttyd::evtmgr { struct EvtEntry; }
namespace ttyd::filemgr { struct File; }

namespace ttyd::win_root {

extern "C" {

struct SortMethodInfo {
    const char* type;
    void*       func;
};
static_assert(sizeof(SortMethodInfo) == 8);

struct WinTabHeaderInfo {
    int32_t     id;
    float       x;
    float       y;
    int32_t     state;
    int32_t     timer;
};
static_assert(sizeof(WinTabHeaderInfo) == 0x14);

struct WinTabBodyInfo {
    int32_t     id;
    float       x;
    float       y;
    int32_t     state;
    int32_t     timer;
};
static_assert(sizeof(WinTabBodyInfo) == 0x14);

struct WinEmailInfo {
    uint8_t     order;
    uint8_t     id;
};
static_assert(sizeof(WinEmailInfo) == 2);

struct WinEquippedBadgeInfo {
    int32_t     is_equipped;
    int32_t     badge_menu_idx;
    int32_t     badge_id;
};
static_assert(sizeof(WinEquippedBadgeInfo) == 0xc);

struct WinLogSubmenuInfo {
    int32_t     id;
    float       x;
    float       y;
    float       target_x;
    float       target_y;
    int32_t     state;
    int32_t     timer;
    const char* help_msg;
};
static_assert(sizeof(WinLogSubmenuInfo) == 0x20);

struct WinLogTattleInfo {
    uint8_t     order;
    uint8_t     id;
};
static_assert(sizeof(WinLogTattleInfo) == 2);

struct WinLogTattleMenuWork {
    int32_t     state;
    int32_t     enemy_id;
    filemgr::File* win_tpl;
    filemgr::File* mono_dat;
    float       x;
    float       y;
    float       target_x;
    float       target_y;
    int32_t     anim_pose;
    int32_t     anim_pose_2;
    void*       texture_obj;
    int32_t     alpha;
};
static_assert(sizeof(WinLogTattleMenuWork) == 0x30);

struct WinPauseMenu {
    uint16_t    flags;                                      // 0x0000
    int8_t      unk_0x0002[2];                              // 0x0002
    uint32_t    buttons_pressed;                            // 0x0004
    uint32_t    buttons_repeated;                           // 0x0008
    uint32_t    dirs_pressed;                               // 0x000c
    uint32_t    dirs_repeated;                              // 0x0010
    int32_t     lecture_state;                              // 0x0014
    evtmgr::EvtEntry* lecture_evt;                          // 0x0018
    int32_t     initial_root_menu_id;                       // 0x001c
    int32_t     menu_state;                                 // 0x0020
    int32_t     root_menu_state;                            // 0x0024
    filemgr::File* win_tpl;                                 // 0x0028
    filemgr::File* mail_tpl;                                // 0x002c
    filemgr::File* map_tpl;                                 // 0x0030
    void*       framebuffer_work;                           // 0x0034
    int32_t     framebuffer_finished;                       // 0x0038
    evtmgr::EvtEntry* use_item_evt;                         // 0x003c
    int32_t     root_cursor_idx;                            // 0x0040
    int32_t     root_cursor_max;                            // 0x0044
    WinTabHeaderInfo tab_header_info[6];                    // 0x0048
    WinTabBodyInfo   tab_body_info[5];                      // 0x00c0
    // Used by sort menu, possibly description window as well?
    int32_t     parent_menu_state;                          // 0x0124
    float       msg_win_x;                                  // 0x0128
    float       msg_win_y;                                  // 0x012c
    int32_t     msg_win_state;                              // 0x0130
    int32_t     msg_win_timer;                              // 0x0134
    int32_t     msg_win_item_id;                            // 0x0138
    char*       msg_win_body;                               // 0x013c
    char*       msg_win_header;                             // 0x0140
    char*       msg_win_body_copy;                          // 0x0144
    int32_t     msg_win_line_current;                       // 0x0148
    int32_t     msg_win_line_count;                         // 0x014c
    float       main_cursor_x;                              // 0x0150
    float       main_cursor_y;                              // 0x0154
    float       main_cursor_target_x;                       // 0x0158
    float       main_cursor_target_y;                       // 0x015c

    // "Mario" menu + sort menu.
    int32_t     mario_menu_state;                           // 0x0160
    float       sort_menu_x;                                // 0x0164
    float       sort_menu_y;                                // 0x0168
    float       sort_menu_target_x;                         // 0x016c
    float       sort_menu_target_y;                         // 0x0170
    int32_t     sort_cursor_idx;                            // 0x0174
    int32_t     sort_cursor_max;                            // 0x0178
    int32_t     sort_menu_state;                            // 0x017c
    int32_t     has_sorted;                                 // 0x0180
    int32_t     sort_menu_type;                             // 0x0184
    int32_t     mario_anim_pose_id;                         // 0x0188
    int32_t     mario_anim_timer;                           // 0x018c
    int32_t     special_move_menu_opened;                   // 0x0190
    int32_t     special_move_cursor_idx;                    // 0x0194
    int32_t     special_move_count;                         // 0x0198

    // "Party" menu.
    int32_t     is_swapping_party;                          // 0x019c
    int32_t     party_anim_pose_ids[7];                     // 0x01a0
    int32_t     party_member_ids[7];                        // 0x01bc
    int32_t     active_party_winPartyDt_idx;                // 0x01d8
    int32_t     party_cursor_idx;                           // 0x01dc
    int32_t     party_count;                                // 0x01e0
    float       party_circle_rotation;                      // 0x01e4
    float       party_circle_target_rotation;               // 0x01e8
    int8_t      unk_0x01ec[16];                             // 0x01ec
    int32_t     current_field_party_idx;                    // 0x01fc
    int32_t     party_moves_menu_opened;                    // 0x0200
    int32_t     party_moves_count;                          // 0x0204
    int32_t     party_moves_cursor_idx;                     // 0x0208

    // "Item" menu.
    int32_t     item_menu_state;                            // 0x020c
    int32_t     item_submenu_id;                            // 0x0210
    // Indexed by item_submenu_id.
    int32_t     items_cursor_idx[2];                        // 0x0214
    int32_t     items_page_num[2];                          // 0x021c
    float       items_offset_y[2];                          // 0x0224
    float       items_target_y[2];                          // 0x022c
    float       super_luigi_win_offset_x;                   // 0x0234
    float       super_luigi_win_target_x;                   // 0x0238
    float       super_luigi_win_offset_y;                   // 0x023c
    float       super_luigi_win_target_y;                   // 0x0240
    float       mail_win_offset_x;                          // 0x0244
    float       mail_win_target_x;                          // 0x0248
    float       mail_win_offset_y;                          // 0x024c
    float       mail_win_target_y;                          // 0x0250
    int32_t     emails_count;                               // 0x0254
    int32_t     emails_cursor_idx;                          // 0x0258
    int32_t     emails_page_num;                            // 0x025c
    float       email_scroll_y;                             // 0x0260
    float       email_target_y;                             // 0x0264
    int32_t     email_line_current;                         // 0x0268
    int16_t     email_line_count;                           // 0x026c
    WinEmailInfo email_ids[45];                             // 0x026e
    int32_t     super_luigi_book_id;                        // 0x02c8
    int32_t     super_luigi_page_count;                     // 0x02cc
    int32_t     super_luigi_page;                           // 0x02d0
    int32_t     use_item_type;                              // 0x02d4
    int32_t     use_item_idx;                               // 0x02d8
    int32_t     use_item_menu_cursor_idx;                   // 0x02dc
    int32_t     use_item_timer;                             // 0x02e0
    int8_t      unk_0x02e4[4];                              // 0x02e4
    int16_t     key_items[121];                             // 0x02e8
    int16_t     key_items_count;                            // 0x03da

    // "Badges" menu.
    int32_t     badge_menu_state;                           // 0x03dc
    int32_t     badge_submenu_id;                           // 0x03e0
    // Indexed by badge_submenu_id.
    int32_t     badges_cursor_idx[2];                       // 0x03e4
    int32_t     badges_page_num[2];                         // 0x03ec
    float       badges_offset_y[2];                         // 0x03f4
    float       badges_target_y[2];                         // 0x03fc
    WinEquippedBadgeInfo equipped_badge_ids[200];           // 0x0404
    int32_t     equipped_badges_count;                      // 0x0d64

    // "Log" menu + submenus.
    // Note that fields from 0xe14 ~ 0x1030 were changed from vanilla!
    int32_t     log_menu_state;                             // 0x0d68
    int32_t     log_submenu_cursor_idx;                     // 0x0d6c
    int32_t     log_submenu_count;                          // 0x0d70
    WinLogSubmenuInfo log_submenu_info[6];                  // 0x0d74
    // "Badge Log"
    float       badge_log_win_offset_x;                     // 0x0e34
    float       badge_log_win_target_x;                     // 0x0e38
    float       badge_log_win_offset_y;                     // 0x0e3c
    float       badge_log_win_target_y;                     // 0x0e40
    int32_t     badge_log_total_count;                      // 0x0e44
    int32_t     badge_log_obtained_count;                   // 0x0e48
    int32_t     badge_log_cursor_idx;                       // 0x0e4c
    int32_t     badge_log_page_num;                         // 0x0e50
    float       badge_log_scroll_y;                         // 0x0e54
    float       badge_log_scroll_target_y;                  // 0x0e58
    float       badge_log_showcased_x;                      // 0x0e5c
    float       badge_log_showcased_target_x;               // 0x0e60
    float       badge_log_showcased_y;                      // 0x0e64
    float       badge_log_showcased_target_y;               // 0x0e68
    int16_t     badge_log_ids[84];                          // 0x0e6c
    // "Recipe Log"
    float       recipe_log_win_offset_x;                    // 0x0f14
    float       recipe_log_win_target_x;                    // 0x0f18
    float       recipe_log_win_offset_y;                    // 0x0f1c
    float       recipe_log_win_target_y;                    // 0x0f20
    int32_t     recipe_log_total_count;                     // 0x0f24
    int32_t     recipe_log_obtained_count;                  // 0x0f28
    int32_t     recipe_log_cursor_idx;                      // 0x0f2c
    int32_t     recipe_log_page_num;                        // 0x0f30
    float       recipe_log_scroll_y;                        // 0x0f34
    float       recipe_log_scroll_target_y;                 // 0x0f38
    float       recipe_log_showcased_x;                     // 0x0f3c
    float       recipe_log_showcased_target_x;              // 0x0f40
    float       recipe_log_showcased_y;                     // 0x0f44
    float       recipe_log_showcased_target_y;              // 0x0f48
    int16_t     recipe_log_ids[84];                         // 0x0f4c
    // Reserved space for other menus.
    int8_t      unused_0x0ff4[60];                          // 0x0ff4
    // "Tattle Log"
    int8_t      unk_0x1030[16];                             // 0x1030
    int32_t     tattle_log_total_count;                     // 0x1040
    int32_t     tattle_log_obtained_count;                  // 0x1044
    int32_t     tattle_log_cursor_idx;                      // 0x1048
    int32_t     tattle_log_page_num;                        // 0x104c
    int8_t      unk_0x1050[8];                              // 0x1050
    WinLogTattleInfo tattle_logs[0xd8];                     // 0x1058
    WinLogTattleMenuWork* tattle_log_menu_work;             // 0x1208
    int8_t      unk_0x120c[4];                              // 0x120c

    // WinMgrEntry indices for additional window dialogs
    // (used when choosing an item to use in the "Items" menu).
    int32_t     winmgr_entry_1;                             // 0x1210
    int32_t     winmgr_entry_2;                             // 0x1214
};
static_assert(sizeof(WinPauseMenu) == 0x1218);

// .text
// winMailDisp
// winZClear
void winKageGX(
    float x, float y, float z, float scale, WinPauseMenu* menu, uint32_t* color);
uint32_t winSortWait(WinPauseMenu* menu);
void winSortEntry(double x, double y, WinPauseMenu* menu, int sortType);
// winSortGX
// winSortMain
// sort_8_2_func
// compare_func5_r
// compare_func5
// sort_8_1_func
// compare_func4_r
// compare_func4
void sort_7_3_func(WinPauseMenu* menu);
// compare_func3_r
// compare_func3
// sort_7_2_func
// compare_func2_r
// compare_func2
// sort_7_1_func
// compare_func1_r
// compare_func1
// sort_6_2_func
// sort_6_1_func
int32_t compare_func6_2_r(int16_t* param_1,int16_t* param_2);
int32_t compare_func6_2(int16_t* param_1,int16_t* param_2);
int32_t compare_func6_1_r(int16_t* param_1,int16_t* param_2);
int32_t compare_func6_1(int16_t* param_1,int16_t* param_2);
// sort_5_3_func
// sort_5_2_func
// sort_5_1_func
int32_t compare_func5_2_r(int16_t* param_1,int16_t* param_2);
int32_t compare_func5_2(int16_t* param_1,int16_t* param_2);
int32_t compare_func5_1_r(int16_t* param_1,int16_t* param_2);
int32_t compare_func5_1(int16_t* param_1,int16_t* param_2);
// sort_4_3_func
// sort_4_2_func
// sort_4_1_func
// N_compare_func4_2_r
// N_compare_func4_2
// N_compare_func4_1_r
// N_compare_func4_1
// sort_3_3_func
// sort_3_2_func
// sort_3_1_func
// sort_2_2_func
// sort_2_1_func
// sort_1_2_func
// sort_1_1_func
void winWazaGX(
    float x, float y, float w, float h, WinPauseMenu* menu, int32_t type);
void winHakoGX(
    float x, float y, WinPauseMenu* menu, int32_t type);
// winMailGX
// winHalfBookGX
// winBookGX
void winNameGX(
    float x, float y, float w, float h, WinPauseMenu* menu, int32_t type);
void winKirinukiGX(
    float x, float y, float w, float h, WinPauseMenu* menu, int32_t type);
void winMsgEntry(
    WinPauseMenu* menu, int32_t item_id, const char* msg, const char* name);
// winMsgDisp
// winMsgMain
void winBgGX(double x, double y, WinPauseMenu* menu, int32_t type);
// winBgMain
// winRootDisp
// winRootMain

// .data
extern uint8_t enemy_monoshiri_sort_table[0xe7];
extern SortMethodInfo sort_1[3];
extern SortMethodInfo sort_2[3];
extern SortMethodInfo sort_3[4];
extern SortMethodInfo sort_4[4];
extern SortMethodInfo sort_5[4];
extern SortMethodInfo sort_6[3];
extern SortMethodInfo sort_7[4];
extern SortMethodInfo sort_8[3];

}

}