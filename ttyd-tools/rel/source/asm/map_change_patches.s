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
# Call C function to perform extra logic immediately before unloading a map.
bl onMapUnload
# Replace original opcode.
bl fadeIsFinish

BranchBackOnMapUnload:
b 0