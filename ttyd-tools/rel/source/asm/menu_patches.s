# win_mario
.global StartCheckOpenMarioMoveMenu
.global BranchBackCheckOpenMarioMoveMenu
.global StartMarioMoveMenuDisp
.global BranchBackMarioMoveMenuDisp
.global StartMarioMoveMenuMsgEntry
.global BranchBackMarioMoveMenuMsgEntry
# win_party
.global StartPartySetPartnerDescAndMoveCount
.global BranchBackPartySetPartnerDescAndMoveCount
.global StartPartyOverrideMoveTextAndCursorPos
.global BranchBackPartyOverrideMoveTextAndCursorPos
.global StartPartyDispHook1
.global BranchBackPartyDispHook1
.global StartPartyDispHook2
.global BranchBackPartyDispHook2

StartCheckOpenMarioMoveMenu:
# Check for opening the menu for jumps and hammers as well.
mr %r3, %r29
bl checkOpenMarioMoveMenu
cmpwi %r3, 1

BranchBackCheckOpenMarioMoveMenu:
b 0

StartMarioMoveMenuDisp:
# Move win_root pointer to r3.
mr %r3, %r30
bl marioMoveMenuDisp

BranchBackMarioMoveMenuDisp:
b 0

StartMarioMoveMenuMsgEntry:
# Get the correct Star Power description, given the cursor position in the menu
# (win_root pointer already in r3).
bl marioMoveMenuMsgEntry

BranchBackMarioMoveMenuMsgEntry:
b 0

StartPartySetPartnerDescAndMoveCount:
mr %r3, %r31
bl partyMenuSetupPartnerDescAndMoveCount

BranchBackPartySetPartnerDescAndMoveCount:
b 0

StartPartyOverrideMoveTextAndCursorPos:
mr %r3, %r31
bl partyMenuSetMoveDescAndCursorPos

BranchBackPartyOverrideMoveTextAndCursorPos:
b 0

StartPartyDispHook1:
nop
BranchBackPartyDispHook1:
b 0

StartPartyDispHook2:
mr %r3, %r30
bl partyMenuDispStats

BranchBackPartyDispHook2:
b 0
