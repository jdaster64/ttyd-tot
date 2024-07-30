
#include "tot_gsw.h"

#include "evt_cmd.h"

#include <ttyd/swdrv.h>

namespace mod::tot {

using namespace ::ttyd::swdrv;

uint32_t GetSWF(int32_t flag_id) {
    return swGet(flag_id - EVT_HELPER_GSWF_BASE);
}

uint32_t GetSWByte(int32_t flag_id) {
    return swByteGet(flag_id - EVT_HELPER_GSW_BASE);
}

void SetSWF(int32_t flag_id, int32_t value) {
    if (value) {
        swSet(flag_id - EVT_HELPER_GSWF_BASE);
    } else {
        swClear(flag_id - EVT_HELPER_GSWF_BASE);
    }
}

uint32_t ToggleSWF(int32_t flag_id) {
    uint32_t new_value = !GetSWF(flag_id);
    SetSWF(flag_id, new_value);
    return new_value;
}

}  // namespace mod::tot