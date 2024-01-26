#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_pouch {

extern "C" {

EVT_DECLARE_USER_FUNC(evt_pouch_get_yoshiname, 1)
// evt_pouch_set_yoshiname
// evt_pouch_open_mail
// evt_pouch_receive_mail
// evt_pouch_majinai_set
// evt_pouch_get_equipbadgecnt
EVT_DECLARE_USER_FUNC(evt_pouch_get_havebadgecnt, 1)
// evt_pouch_get_haveitemcnt
// evt_pouch_all_party_recovery
// evt_pouch_mario_recovery
// evt_pouch_party_left
EVT_DECLARE_USER_FUNC(evt_pouch_party_join, 1)
// evt_pouch_set_ap
// evt_pouch_get_max_ap
// L_evt_pouch_set_hp
// L_evt_pouch_get_hp
// evt_pouch_check_item
// evt_pouch_remove_keepitem
// evt_pouch_add_keepitem
EVT_DECLARE_USER_FUNC(N_evt_pouch_remove_item_index, 3)
// evt_pouch_remove_item
EVT_DECLARE_USER_FUNC(evt_pouch_add_item, 2)
// evt_pouch_get_starstone
// evt_pouch_add_starpiece
// evt_pouch_get_starpiece
// evt_pouch_set_supercoin
// evt_pouch_get_supercoin
// evt_pouch_set_coin
EVT_DECLARE_USER_FUNC(evt_pouch_add_coin, 1)
EVT_DECLARE_USER_FUNC(evt_pouch_get_coin, 1)

}

}