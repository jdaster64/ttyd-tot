.global StartPreventDpadShortcutsOutsidePit
.global ConditionalBranchPreventDpadShortcutsOutsidePit
.global BranchBackPreventDpadShortcutsOutsidePit
.global StartHideTopBarInSomeWindows
.global BranchBackHideTopBarInSomeWindows

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

StartHideTopBarInSomeWindows:
mr %r3, %r31

# Check if this window is a type that wants the top bar hidden.
bl checkHideTopBarInWindow
cmpwi %r3, 1
beq- 0x8
# Otherwise, force window open as usual.
bl statusWinForceOpen

BranchBackHideTopBarInSomeWindows:
b 0