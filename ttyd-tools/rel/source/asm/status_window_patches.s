.global StartPreventDpadShortcutsOutsidePit
.global ConditionalBranchPreventDpadShortcutsOutsidePit
.global BranchBackPreventDpadShortcutsOutsidePit

StartPreventDpadShortcutsOutsidePit:
# Returns whether the player is currently outside the Pit.
bl checkOutsidePit
cmpwi %r3, 0
# If so, always prevent the D-Pad pause menu shortcut icon from displaying.
bne- 0xc
lwz %r4, 0x1ccc (%r13)
BranchBackPreventDpadShortcutsOutsidePit:
b 0
ConditionalBranchPreventDpadShortcutsOutsidePit:
b 0