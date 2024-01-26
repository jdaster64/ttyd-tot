#pragma once

#include <cstdint>

namespace ttyd::evt_badgeshop {

extern "C" {

// .text
// U_badgeShop_SpecialCheck
// badgeShop_ThrowCheck
// evt_badgeShop_bteresa_get_kind_cnt
// U_evt_badgeShop_get_special_zaiko
// evt_badgeShop_bottakuru_get_kind_cnt
// evt_badgeShop_starmaniac_get_kind_cnt
// evt_badgeShop_throw_get_kind_cnt
// evt_badgeShop_bottakuru_dec
// evt_badgeShop_starmaniac_dec
// evt_badgeShop_special_dec
// evt_badgeShop_throw_dec
// evt_badgeShop_throw_inc
// badgeShop_getBargainTable
// badgeShop_add
// badgeShop_get
// badgeShop_set
// badgeShop_bargainGeneration
// badgeShop_bteresaGeneration
// badgeShop_bottakuruGeneration
// bottakuruComp
void badgeShop_init();
// getBadgeBteresaTableMaxCount
// getBadgeStarmaniacTableMaxCount
// getBadgeBottakuru100TableMaxCount

// .data
extern int32_t badge_special_table[10];
extern int32_t badge_special_table_stage_1_clear[4];
extern int32_t badge_special_table_stage_2_clear[4];
extern int32_t badge_special_table_stage_3_clear[4];
extern int32_t badge_special_table_stage_4_clear[4];
extern int32_t badge_special_table_stage_5_clear[4];
extern int32_t badge_special_table_stage_6_clear[4];
extern int32_t badge_starmaniac_table[16];
extern int32_t badge_bottakuru_table[17];
extern int32_t badge_bottakuru100_table[7];
extern int32_t badge_bteresa_table[5];
extern int32_t badge_bteresa_table_card_special[5];
extern int32_t badge_bteresa_table_card_silver[5];
extern int32_t badge_bteresa_table_card_gold[5];
extern int32_t badge_bteresa_table_card_platinum[5];

}

}