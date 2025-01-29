#pragma once

#include "evt_cmd.h"

#include <ttyd/evtmgr.h>

#include <cstdint>

namespace mod::tot {
    
class OptionsManager {
public:
    // Applies the currently selected preset's run options, if not CUSTOM.
    // If 'first_time' is set, enforces default timer type, and sets DEFAULT
    // settings when changing to CUSTOM as well.
    static void ApplyCurrentPresetOptions(bool first_time = false);
    // Handles advancing to the next possible option.
    static void AdvanceOption(uint32_t option, int32_t change = 1);
    // Selects a random value for the current option, either explicitly,
    // i.e. the user randomized that option alone, or implicitly, if the
    // user randomizes the Custom preset.
    // Returns false if no valid value found, or option cannot be randomized.
    static bool RandomizeOption(uint32_t option, bool explicitly = true);
    
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
    // Verifies whether all settings are at least Intensity-neutral.
    static bool NoIntensityReduction();

    // Returns a string encoding of all currently selected options.
    static const char* GetEncodedOptions();

    // For applying run options when starting, resuming or ending a run.
    // Resets options, stats, etc. from cold boot / when loading lobby or hub.
    static void ResetAfterRun();
    // Sets up options, stats, etc. on run start.
    static void OnRunStart();
    // Sets up internal data when continuing a run from an auto-save.
    static void OnRunResumeFromAutoSave();
    // Sets up internal data when continuing a run from a hard save.
    static void OnRunResumeFromFileSelect();

    // Updates HP, FP, and BP according to current fields in tot_state.
    static void UpdateLevelupStats();
};

// Wrapper to ResetAfterRun, run when loading gon_00 (lobby) or gon_10 (hub).
EVT_DECLARE_USER_FUNC(evtTot_ResetSettingsAfterRun, 0)

}