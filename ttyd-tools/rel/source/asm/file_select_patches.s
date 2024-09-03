.global StartDrawFileLevelString
.global BranchBackDrawFileLevelString
.global StartDrawFileCrystalStars
.global BranchBackDrawFileCrystalStars

StartDrawFileLevelString:
# Replace with drawing completion percentage.
mr %r3, %r30
mr %r4, %r29
lwz %r5, -0x6550 (%r13)
lwz %r5, 0x20 (%r5)
bl dispFileSelectProgress

BranchBackDrawFileLevelString:
b 0


StartDrawFileCrystalStars:
# Skip original logic entirely.

BranchBackDrawFileCrystalStars:
b 0