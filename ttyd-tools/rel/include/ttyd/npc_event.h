#pragma once

#include "evtmgr.h"

#include <cstdint>

namespace ttyd::npc_event {

extern "C" {

// .text
// gesso_ground_check
// set_bottle_flag_init
// unk_JP_US_EU_28_800f1ac8
// zakoEntryFall
// dokanCheck
// zakoEntryDokan
// limitSpd
// getSpdRun
// getSpdWalk
// upperCheck
// cliffCheck
// camGetPos
// chain_main
// kamec_fire_magic_delete
// kamec_fire_magic
// piders_draw_yawn
// piders_draw_yarn_sub
// piders_get_height
// killer_make_name
// _2d_get_dead_jump_offset
// _2d_dead_jump
// pressCheck
// _wait_blow_end
// npc_check_wall_stop
// mahoon_get_groupname

// .data
// Events for enemy overworld behavior.
extern int32_t enemy_common_dead_event[1];
extern int32_t enemy_event_dead_event_sub[1];
extern int32_t enemy_event_dead_event[1];
extern int32_t enemy_slave_common_dead_event[1];
extern int32_t enemy_common_blow_event[1];
extern int32_t enemy_2d_common_dead_event[1];
extern int32_t _2d_dead_rotate_event[1];
extern int32_t unk_0x8033f9b0[1];
extern int32_t enemy_common_flag_init_event[1];
extern int32_t testnpc_move_event[1];
extern int32_t testenemynpc_init_event[1];
extern int32_t testenemynpc_move_event[1];
extern int32_t npc_urouro_init_event[1];
extern int32_t npc_urouro_move_event[1];
extern int32_t npc_urouro_return_event[1];
extern int32_t kuriboo_init_event[1];
extern int32_t kuriboo_move_event[1];
extern int32_t kuriboo_find_event[1];
extern int32_t kuriboo_lost_event[1];
extern int32_t kuriboo_return_event[1];
extern int32_t patakuri_init_event[1];
extern int32_t patakuri_move_event[1];
extern int32_t patakuri_find_event[1];
extern int32_t patakuri_lost_event[1];
extern int32_t patakuri_return_event[1];
extern int32_t nokonoko_shell_return_event[1];
extern int32_t nokonoko_init_event[1];
extern int32_t nokonoko_move_event[1];
extern int32_t nokonoko_find_event[1];
extern int32_t nokonoko_lost_event[1];
extern int32_t nokonoko_return_event[1];
extern int32_t togenoko_shell_return_event[1];
extern int32_t togenoko_init_event[1];
extern int32_t togenoko_move_event[1];
extern int32_t togenoko_find_event[1];
extern int32_t togenoko_lost_event[1];
extern int32_t togenoko_return_event[1];
extern int32_t patapata_init_event[1];
extern int32_t patapata_move_event[1];
extern int32_t patapata_find_event[1];
extern int32_t patapata_lost_event[1];
extern int32_t patapata_return_event[1];
extern int32_t met_init_event_common[1];
extern int32_t met_set_anim_wait_event[1];
extern int32_t met_set_anim_walk_event[1];
extern int32_t met_init_event[1];
extern int32_t met_init_event_ceil[1];
extern int32_t togemet_init_event[1];
extern int32_t togemet_init_event_ceil[1];
extern int32_t met_move_event[1];
extern int32_t met_find_event[1];
extern int32_t met_lost_event[1];
extern int32_t met_return_event[1];
extern int32_t patamet_init_event[1];
extern int32_t patamet_move_event[1];
extern int32_t patamet_find_event[1];
extern int32_t patamet_lost_event[1];
extern int32_t patamet_return_event[1];
extern int32_t chorobon_init_event[1];
extern int32_t chorobon_move_event[1];
extern int32_t chorobon_find_event[1];
extern int32_t chorobon_lost_event[1];
extern int32_t chorobon_return_event[1];
extern int32_t pansy_init_event[1];
extern int32_t pansy_move_event[1];
extern int32_t pansy_find_event[1];
extern int32_t pansy_lost_event[1];
extern int32_t pansy_return_event[1];
extern int32_t twinkling_pansy_init_event[1];
extern int32_t twinkling_pansy_move_event[1];
extern int32_t twinkling_pansy_find_event[1];
extern int32_t twinkling_pansy_lost_event[1];
extern int32_t twinkling_pansy_return_event[1];
extern int32_t karon_init_event[1];
extern int32_t karon_move_event[1];
extern int32_t karon_dead_event[1];
extern int32_t karon_find_event[1];
extern int32_t karon_lost_event[1];
extern int32_t karon_return_event[1];
extern int32_t karon_blow_event[1];
extern int32_t honenoko2_init_event[1];
extern int32_t honenoko2_move_event[1];
extern int32_t honenoko2_find_event[1];
extern int32_t honenoko2_lost_event[1];
extern int32_t honenoko2_return_event[1];
extern int32_t killer_init_event[1];
extern int32_t killer_move_event[1];
extern int32_t killer_dead_event[1];
extern int32_t killer_cannon_init_event[1];
extern int32_t killer_cannon_move_event[1];
extern int32_t sambo_init_event[1];
extern int32_t sambo_move_event[1];
extern int32_t sambo_find_event[1];
extern int32_t sambo_lost_event[1];
extern int32_t sambo_return_event[1];
extern int32_t sinnosuke_init_event[1];
extern int32_t sinnosuke_move_event[1];
extern int32_t sinnosuke_find_event[1];
extern int32_t sinnosuke_lost_event[1];
extern int32_t sinnosuke_return_event[1];
extern int32_t sinemon_init_event[1];
extern int32_t sinemon_move_event[1];
extern int32_t sinemon_find_event[1];
extern int32_t sinemon_lost_event[1];
extern int32_t sinemon_return_event[1];
extern int32_t togedaruma_init_event[1];
extern int32_t togedaruma_move_event[1];
extern int32_t togedaruma_find_event[1];
extern int32_t togedaruma_lost_event[1];
extern int32_t togedaruma_return_event[1];
extern int32_t barriern_init_event[1];
extern int32_t barriern_move_event[1];
extern int32_t barriern2_move_event[1];
extern int32_t barriern_dead_event[1];
extern int32_t barriern_find_event[1];
extern int32_t barriern_lost_event[1];
extern int32_t barriern_return_event[1];
extern int32_t barriern_blow_event[1];
extern int32_t piders_init_event[1];
extern int32_t piders_move_event[1];
extern int32_t piders_find_event[1];
extern int32_t piders_blow_event[1];
extern int32_t pakkun_attack_event[1];
extern int32_t pakkun_init_event[1];
extern int32_t pakkun_move_event[1];
extern int32_t pakkun_find_event[1];
extern int32_t pakkun_return_event[1];
extern int32_t dokugassun_init_event[1];
extern int32_t dokugassun_move_event[1];
extern int32_t dokugassun_find_event[1];
extern int32_t dokugassun_lost_event[1];
extern int32_t dokugassun_return_event[1];
extern int32_t basabasa_init_event[1];
extern int32_t basabasa_move_event[1];
extern int32_t basabasa_find_event[1];
extern int32_t basabasa_lost_event[1];
extern int32_t basabasa_return_event[1];
extern int32_t basabasa_dead_event[1];
extern int32_t basabasa2_init_event[1];
extern int32_t basabasa2_move_event[1];
extern int32_t basabasa2_find_event[1];
extern int32_t basabasa2_lost_event[1];
extern int32_t basabasa2_return_event[1];
extern int32_t basabasa2_dead_event[1];
extern int32_t teresa_init_event[1];
extern int32_t teresa_move_event[1];
extern int32_t teresa_find_event[1];
extern int32_t teresa_lost_event[1];
extern int32_t teresa_return_event[1];
extern int32_t teresa_dead_event[1];
extern int32_t bubble_init_event[1];
extern int32_t bubble_move_event[1];
extern int32_t bubble_find_event[1];
extern int32_t bubble_lost_event[1];
extern int32_t bubble_return_event[1];
extern int32_t hbom_init_event[1];
extern int32_t hbom_move_event[1];
extern int32_t hbom_find_event[1];
extern int32_t hbom_lost_event[1];
extern int32_t hbom_return_event[1];
extern int32_t zakowiz_init_event[1];
extern int32_t zakowiz_move_event[1];
extern int32_t zakowiz_dead_event[1];
extern int32_t zakowiz_find_event[1];
extern int32_t zakowiz_lost_event[1];
extern int32_t zakowiz_return_event[1];
extern int32_t zakowiz_blow_event[1];
extern int32_t hannya_init_event[1];
extern int32_t hannya_move_event[1];
extern int32_t hannya_find_event[1];
extern int32_t hannya_lost_event[1];
extern int32_t hannya_return_event[1];
extern int32_t mahoon_init_event[1];
extern int32_t mahoon_move_event[1];
extern int32_t mahoon_dead_event[1];
extern int32_t mahoon_find_event[1];
extern int32_t mahoon_lost_event[1];
extern int32_t mahoon_return_event[1];
extern int32_t kamec_init_event[1];
extern int32_t kamec_move_event[1];
extern int32_t kamec_find_event[1];
extern int32_t kamec_lost_event[1];
extern int32_t kamec_return_event[1];
extern int32_t kamec_dead_event[1];
extern int32_t kamec_blow_event[1];
extern int32_t kamec2_init_event[1];
extern int32_t kamec2_move_event[1];
extern int32_t kamec2_find_event[1];
extern int32_t kamec2_lost_event[1];
extern int32_t kamec2_return_event[1];
extern int32_t kamec2_dead_event[1];
extern int32_t kamec2_blow_event[1];
extern int32_t hbross_init_event[1];
extern int32_t hbross_move_event[1];
extern int32_t hbross_dead_event[1];
extern int32_t hbross_find_event[1];
extern int32_t hbross_lost_event[1];
extern int32_t hbross_return_event[1];
extern int32_t hbross_blow_event[1];
extern int32_t wanwan_init_event[1];
extern int32_t wanwan_init_event_sub[1];
extern int32_t wanwan_move_event[1];
extern int32_t wanwan_find_event[1];
extern int32_t kuriboo2D_init_event[1];
extern int32_t kuriboo2D_move_event[1];
extern int32_t kuriboo2D_find_event[1];
extern int32_t zako2D_init_event[1];
extern int32_t zako2D_move_event[1];
extern int32_t zako2D_find_event[1];
extern int32_t dokan2D_init_event[1];
extern int32_t dokan2D_regl_event[1];
extern int32_t fall2D_init_event[1];
extern int32_t fall2D_regl_event[1];
extern int32_t zakoM2D_init_event[1];
extern int32_t zakoM2D_move_event[1];
extern int32_t zakoM2D_find_event[1];
extern int32_t unk_0x8034bdb4[1];
extern int32_t zakoM2D_dead_event[1];
extern int32_t gesso2D_init_event[1];
extern int32_t gesso2D_move_event[1];

}

}