#pragma once

namespace mod::infinite_pit {

class StringsManager {
public:
    // Looks up whether there is a replacement string for a given msgSearch key.
    // Returns nullptr if the original string should be used.
    static const char* LookupReplacement(const char* msg_key);
};
 
}