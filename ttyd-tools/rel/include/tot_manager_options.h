#pragma once

#include "evt_cmd.h"

#include <ttyd/evtmgr.h>

#include <cstdint>

namespace mod::tot {
    
class OptionsManager {
public:
    // Applies the currently selected preset's run options, if not CUSTOM.
    static void ApplyCurrentPresetOptions();
    
    // Returns the default value for a given option, or -1 if there is none.
    static int32_t GetDefaultValue(uint32_t option);
    // Returns whether the given OPT_ / OPTNUM_ option is set to its default.
    // Preset, tower type, and timer type will always return 'true'.
    static bool IsDefault(uint32_t option);
    // Verifies whether all settings are set to default.
    static bool AllDefault();
    // Verifies whether all settings are set to default, aside from Mario
    // and party's stat levels, which can optionally be set to 0.
    static bool AllDefaultExceptZeroStatLevels();

    // Returns the intensity value for the given option's current value.
    static int32_t GetIntensity(uint32_t option);
    // Returns the total intensity value given all options' settings.
    static int32_t GetTotalIntensity();

    // Returns a string encoding of all currently selected options.
    static const char* GetEncodedOptions();

    // For applying run options when starting, resuming or ending a run.
    // Resets options, stats, etc. from cold boot / when loading lobby or hub.
    static void ResetAfterRun();
    // Sets up options, stats, etc. on run start.
    static void OnRunStart();
    // Sets up options when loading a file with a run already in progress.
    static void OnRunResumeFromFileSelect();

    // Updates HP, FP, and BP according to current fields in tot_state.
    static void UpdateLevelupStats();
};

// Wrapper to ResetAfterRun, run when loading gon_00 (lobby) or gon_10 (hub).
EVT_DECLARE_USER_FUNC(evtTot_ResetSettingsAfterRun, 0)

}