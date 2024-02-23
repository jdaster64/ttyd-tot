.global StartMapLoad
.global BranchBackMapLoad
.global StartOnMapUnload
.global BranchBackOnMapUnload

StartMapLoad:
# Call C function that replaces the existing logic for loading a map.
bl mapLoad
# r3 should be implicitly populated by mapLoad's return value.

BranchBackMapLoad:
b 0

StartOnMapUnload:
# Call C function that replaces the existing logic for unloading map files.
bl onMapUnload

BranchBackOnMapUnload:
b 0