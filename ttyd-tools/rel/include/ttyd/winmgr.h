#pragma once

#include <cstdint>

namespace ttyd::evtmgr {
struct EvtEntry;
}

namespace ttyd::winmgr {

namespace WinMgrDesc_FadeMode {
    enum e {
        SCALE_AND_ROTATE = 0,
        SCALE_AND_ALPHA = 1,
        INSTANT = 2,
    };
}

namespace WinMgrDesc_HeadingType {
    enum e {
        NONE = 0,
        SINGLE_CENTERED = 1,
        DOUBLE_LEFT = 3,
        DOUBLE_LONG_SHORT = 4,
        DOUBLE_LONG_SHORT_DUPLICATE = 5,
    };
}

namespace WinMgrEntry_FadeState {
    enum e {
        IDLE = 0,
        FADING_IN = 1,
        FADING_OUT = 2,
    };
}

namespace WinMgrEntry_Flags {
    enum e {
        IN_USE = 1,
        VISIBLE = 2,
        IN_FADE = 4,
        AUTO_DELETE_AFTER_FADE_OUT = 8,
    };
}

namespace WinMgrSelectEntry_Flags {
    enum e {
        CANCELLABLE = 0x100,
        FINISHED_SELECTION = 0x1000,
        CANCELLED = 0x2000,
    };
}

namespace WinMgrSelectEntry_State {
    enum e {
        START_FADE_IN = 0,
        IN_SELECTION = 1,
        START_FADE_OUT = 2,
        END_SELECTION = 3,
        DONE = 4,
    };
}

namespace WinMgrSelectEntry_Type {
    enum e {
        UNK_0 = 0,
        UNK_1,
        UNK_2,
        UNK_3,
        UNK_4,
        UNK_5,
        MERLON,
        CHET_RIPPO_PARTY_UP,
        CHET_RIPPO_PARTY_DOWN,
        CHET_RIPPO_MARIO_UP,
        CHET_RIPPO_MARIO_DOWN,
        UNK_11,
        LOVELY_HOWZ_OF_BADGES,
        DAZZLE,
        CHARLIETON,
        CHARLIETON_PIT,
        PIANTA_PARLOR_SHOP,
        GRIFTY_INFO,
        LUIGI_TALES,
    };
}

namespace WinMgrSelectEntryRow_Flags {
    enum e {
        GREYED_OUT = 1,
        UNSELECTABLE = 2,
        CHET_RIPPO_POINTS_CAPPED = 4,
        HAS_BADGE_EQUIPPED = 8,
    };
}

struct WinMgrDesc {
    int32_t     fade_mode;      // WinMgrDesc_FadeMode
    int32_t     heading_type;   // WinMgrDesc_HeadingType
    int32_t     camera_id;
    int32_t     x;
    int32_t     y;
    int32_t     width;
    int32_t     height;
    uint32_t    color;
    void*       main_func;
    void*       disp_func;
} ;

static_assert(sizeof(WinMgrDesc) == 0x28);

struct WinMgrSelectDescList {
    int16_t     num_descs;
    int16_t     pad_0x02;
    WinMgrDesc* descs;
} ;

static_assert(sizeof(WinMgrSelectDescList) == 8);

struct WinMgrEntry {
    uint32_t    flags;          // WinMgrEntry_Flags
    uint32_t    fade_state;     // WinMgrEntry_FadeState
    int32_t     fade_frame_counter;
    uint32_t    window_alpha;
    float       scale;
    float       rot_deg_z;
    int32_t     x;
    int32_t     y;
    int32_t     width;
    int32_t     height;
    WinMgrDesc* desc;
    void*       param;
    uint32_t    priority;
    const char* help_msg;
    int32_t     help_line_cursor_index;
    int32_t     help_line_count;
    int32_t     unk_0x40;
} ;

static_assert(sizeof(WinMgrEntry) == 0x44);

struct WinMgrSelectEntryRow {
    uint16_t    flags;  // WinMgrSelectEntryRow_Flags
    uint16_t    value;
} ;

static_assert(sizeof(WinMgrSelectEntryRow) == 4);

struct WinMgrSelectEntry {
    uint16_t    flags;      // WinMgrSelectEntry_Flags
    uint16_t    pad_0x02;
    int32_t     type;       // WinMgrSelectEntry_Type
    int32_t     state;      // WinMgrSelectEntry_State
    int32_t     cursor_index;
    int32_t     list_row_offset;
    float       cursor_x;
    float       cursor_y;
    float       list_y_offset;  // interpolated between row offsets
    int32_t     num_entries;
    int32_t     entry_indices[3];
    WinMgrSelectEntryRow* row_data;
    int32_t     num_rows;
    int32_t     new_item;  // when tossing items away, this is the one picked up
} ;

static_assert(sizeof(WinMgrSelectEntry) == 0x3c);

struct WinMgrWork {
    int32_t         num_entries;
    WinMgrEntry*    entries;
} ;

static_assert(sizeof(WinMgrWork) == 8);

extern "C" {

// .text

// select_disp3_mario
// select_disp3_party
// select_disp3
void select_main3(WinMgrEntry* winMgrEntry);
// select_disp2
// select_disp_luigi
// select_disp_mario
// select_disp_party
// select_disp
void select_main(WinMgrEntry* winMgrEntry);
// unk_8023cf04
// winMgrSelectDelete
int32_t winMgrSelectOther(
    WinMgrSelectEntry* sel_entry, ttyd::evtmgr::EvtEntry* evt);
// winMgrSelect
// unk_8023d524
// unk_8023d59c
// unk_8023d5e4
WinMgrSelectEntry* winMgrSelectEntry(
    int32_t type, int32_t new_item, int32_t cancellable);
void winMgrHelpDraw(WinMgrEntry* winMgrEntry);
// winMgrHelpMain
// winMgrHelpInit
// winMgrGetPtr
// winMgrSetPriority
// winMgrSetSize
// winMgrAction
// winMgrDelete
// winMgrCloseAutoDelete
// winMgrClose
// winMgrOpen
// winMgrSetParam
// winMgrEntry
// winMgrSeq
// winMgrMain
// winMgrDisp
// winMgrReInit
// winMgrInit
// unk_8023f79c
// unk_8023f8d0

// .data

extern WinMgrSelectDescList select_desc_tbl[19];
extern WinMgrWork* winmgr_work;

}

}