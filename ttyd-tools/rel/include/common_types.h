#pragma once

#include <cstdint>

namespace mod {

// Gamepad inputs.
namespace ButtonId {
    enum e {
        DPAD_LEFT   = 0x0001,
        DPAD_RIGHT  = 0x0002,
        DPAD_DOWN   = 0x0004,
        DPAD_UP     = 0x0008,
        Z           = 0x0010,
        R           = 0x0020,
        L           = 0x0040,
        //          = 0x0080,
        A           = 0x0100,
        B           = 0x0200,
        X           = 0x0400,
        Y           = 0x0800,
        START       = 0x1000,
    };
}

// Gamepad directional inputs (used in keyGetDirTrg, etc.)
namespace DirectionInputId {
    enum e {
        CSTICK_UP   = 0x100,
        CSTICK_DOWN = 0x200,
        CSTICK_LEFT = 0x400,
        CSTICK_RIGHT = 0x800,
        ANALOG_UP   = 0x1000,
        ANALOG_DOWN = 0x2000,
        ANALOG_LEFT = 0x4000,
        ANALOG_RIGHT = 0x8000,
    };
};

// Relocatable module ids.
namespace ModuleId {
    enum e {
        INVALID_MODULE = 0,
        AAA,
        AJI,  // x-naut fortress
        BOM,  // fahr outpost
        DMO,
        DOU,  // pirate's grotto
        EKI,  // riverside station
        END,  // credits?
        GON,  // hooktail's castle
        GOR,  // rogueport
        GRA,  // twilight trail
        HEI,  // petal meadows
        HOM,  // riverside station facade / train cutscenes
        JIN,  // creepy steeple
        JON,  // pit of 100 trials
        KPA,  // bowser's castle intermissions
        LAS,  // palace of shadow
        MOO,  // moon
        MRI,  // great tree
        MUJ,  // keelhaul key
        NOK,  // petalburg
        PIK,  // poshley heights / sanctum
        RSH,  // excess express
        SYS,
        TIK,  // rogueport underground
        TOU,  // outer glitz pit
        TOU2, // glitz pit arena
        USU,  // twilight town
        WIN,  // boggly woods
        YUU,
        MAX_MODULE_ID,
        
        // Not used in vanilla game.
        CUSTOM = 40,
    };
}

}