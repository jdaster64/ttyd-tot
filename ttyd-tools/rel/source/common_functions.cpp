#include "common_functions.h"

#include "evt_cmd.h"

#include <ttyd/mario_pouch.h>
#include <ttyd/mariost.h>
#include <ttyd/seqdrv.h>
#include <ttyd/seq_mapchange.h>
#include <ttyd/seq_title.h>
#include <ttyd/system.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod {

bool CheckSeq(ttyd::seqdrv::SeqIndex sequence) {
    const ttyd::seqdrv::SeqIndex next_seq = ttyd::seqdrv::seqGetNextSeq();
    const ttyd::seqdrv::SeqIndex cur_seq = ttyd::seqdrv::seqGetSeq();
    return cur_seq == next_seq && next_seq == sequence;
}

bool InMainGameModes() {
    int32_t sequence = static_cast<int32_t>(ttyd::seqdrv::seqGetNextSeq());
    int32_t game = static_cast<int32_t>(ttyd::seqdrv::SeqIndex::kGame);
    int32_t game_over = static_cast<int32_t>(ttyd::seqdrv::SeqIndex::kGameOver);

    bool next_map_demo = !strcmp(ttyd::seq_mapchange::NextMap, "dmo_00");
    bool next_map_title = !strcmp(ttyd::seq_mapchange::NextMap, "title");
    
    return (sequence >= game) && (sequence <= game_over) && 
           !next_map_demo && !next_map_title;
}

bool InPauseMenu() {
    return ttyd::mariost::marioStGetSystemLevel() == 15U;
}

const char* GetCurrentArea() {
    return ttyd::mariost::g_MarioSt->currentAreaName;
}

const char* GetCurrentMap() {
    return ttyd::mariost::g_MarioSt->currentMapName;
}

const char* GetNextMap() {
    return ttyd::seq_mapchange::NextMap;
}

int32_t GetNumActivePartners() {
    auto* pouch = ttyd::mario_pouch::pouchGetPtr();
    int32_t num_partners = 0;
    for (int32_t i = 1; i <= 7; ++i) {
        if (pouch->party_data[i].flags & 1) {
            ++num_partners;
        }
    }
    return num_partners;
}

bool IsPartnerActive(int32_t idx) {
    auto* pouch = ttyd::mario_pouch::pouchGetPtr();
    switch (idx) {
        case 1: return pouch->party_data[1].flags & 1;
        case 2: return pouch->party_data[2].flags & 1;
        case 3: return pouch->party_data[5].flags & 1;
        case 4: return pouch->party_data[4].flags & 1;
        case 5: return pouch->party_data[6].flags & 1;
        case 6: return pouch->party_data[3].flags & 1;
        case 7: return pouch->party_data[7].flags & 1;
    }
    return false;
}
    
void LinkCustomEvt(ModuleId::e module_id, void* module_ptr, int32_t* evt) {
    int32_t op;
    do {
        op = *evt++;
        // Check each of the operation's arguments.
        for (int32_t* next_op = evt + (op >> 16); evt < next_op; ++evt) {
            if (static_cast<uint32_t>(*evt) >= 0x4000'0000U &&
                static_cast<uint32_t>(*evt) < 0x8000'0000U) {
                *evt = static_cast<int32_t>(
                    static_cast<uint32_t>(*evt) - ((0x40U + module_id) << 24) +
                    reinterpret_cast<uint32_t>(module_ptr));
            }
        }
    } while (op != 1);
}

void UnlinkCustomEvt(ModuleId::e module_id, void* module_ptr, int32_t* evt) {
    int32_t op;
    do {
        op = *evt++;
        // Check each of the operation's arguments.
        for (int32_t* next_op = evt + (op >> 16); evt < next_op; ++evt) {
            if (static_cast<uint32_t>(*evt) >=
                reinterpret_cast<uint32_t>(module_ptr) &&
                static_cast<uint32_t>(*evt) < 
                static_cast<uint32_t>(EVT_HELPER_POINTER_BASE)) {
                *evt = static_cast<int32_t>(
                    static_cast<uint32_t>(*evt) + ((0x40U + module_id) << 24) -
                    reinterpret_cast<uint32_t>(module_ptr));
            }
        }
    } while (op != 1);
}

int32_t CountSetBits(uint32_t x) {
    // PowerPC has no built-in pop_cnt instruction, apparently
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    return (((x + (x >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

uint32_t GetBitMask(uint32_t start_bit, uint32_t end_bit) {
    return (~0U >> (31-end_bit)) - (1U << start_bit) + 1;
}

uint32_t GetShiftedBitMask(uint32_t x, uint32_t start_bit, uint32_t end_bit) {
    return (x & GetBitMask(start_bit, end_bit)) >> start_bit;
}

int32_t IntegerToFmtString(int32_t val, char* out_buf, int32_t max_val) {
    bool is_negative = false;
    if (val < 0) {
        val = -val;
        is_negative = true;
    }
    if (val > max_val) val = max_val;
    if (val >= 1'000'000) {
        return sprintf(
            out_buf, "%s%" PRId32 ",%03" PRId32 ",%03" PRId32,
            is_negative ? "-" : "",
            val / 1'000'000, val / 1000 % 1000, val % 1000);
    } else if (val >= 1'000) {
        return sprintf(
            out_buf, "%s%" PRId32 ",%03" PRId32,
            is_negative ? "-" : "", val / 1000, val % 1000);
    }
    return sprintf(out_buf, "%" PRId32, val);
}

uint64_t DurationCentisecondsToTicks(int32_t val) {
    const uint64_t kTicksPerCentisecond =
        *reinterpret_cast<const int32_t*>(0x800000f8) / 400;
    if (val < 0) val = 0;
    return val * kTicksPerCentisecond;
}

uint32_t DurationTicksToCentiseconds(int64_t val) {
    const int32_t kTicksPerCentisecond =
        *reinterpret_cast<const int32_t*>(0x800000f8) / 400;
    val /= kTicksPerCentisecond;
    // Maximum duration = 100 hours' worth of centiseconds.
    if (val >= 100 * 60 * 60 * 100 || val < 0) {
        val = 100 * 60 * 60 * 100 - 1;
    }
    return val;
}

void DurationTicksToParts(
    int64_t val, int32_t* h, int32_t* m, int32_t* s, int32_t* cs) {
    uint32_t cs_val = DurationTicksToCentiseconds(val);
    *h = cs_val / (60 * 60 * 100);
    *m = cs_val / (60 * 100) % 60;
    *s = cs_val / 100 % 60;
    *cs = cs_val % 100;
}

int32_t DurationTicksToFmtString(int64_t val, char* out_buf) {
    // Divide by the number of ticks in a centisecond (bus speed / 400).
    int32_t h, m, s, cs;
    DurationTicksToParts(val, &h, &m, &s, &cs);
    return sprintf(
        out_buf, "%02" PRId32 ":%02" PRId32 ":%02" PRId32 ".%02" PRId32,
        h, m, s, cs);
}

int32_t DurationCentisToFmtString(int32_t cs_val, char* out_buf) {
    int32_t h = cs_val / (60 * 60 * 100);
    int32_t m = cs_val / (60 * 100) % 60;
    int32_t s = cs_val / 100 % 60;
    int32_t cs = cs_val % 100;
    return sprintf(
        out_buf, "%02" PRId32 ":%02" PRId32 ":%02" PRId32 ".%02" PRId32,
        h, m, s, cs);
}

}