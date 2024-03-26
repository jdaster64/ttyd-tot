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
# win_item
.global StartFixItemWinPartyDispOrder
.global BranchBackFixItemWinPartyDispOrder
.global StartFixItemWinPartySelectOrder
.global BranchBackFixItemWinPartySelectOrder
.global StartCheckForUnusableItemInMenu
.global ConditionalBranchCheckForUnusableItemInMenu
.global BranchBackCheckForUnusableItemInMenu
.global StartUseSpecialItems
.global BranchBackUseSpecialItems
# win_log
.global StartInitTattleLog
.global BranchBackInitTattleLog

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

StartFixItemWinPartyDispOrder:
mr %r3, %r5
bl getPartyMemberMenuOrder

BranchBackFixItemWinPartyDispOrder:
b 0

StartFixItemWinPartySelectOrder:
mr %r3, %r5
bl getPartyMemberMenuOrder

BranchBackFixItemWinPartySelectOrder:
b 0

StartCheckForUnusableItemInMenu:
# Check to see if the player is trying to use an item on an invalid target.
bl checkForUnusableItemInMenu
cmpwi %r3, 0
# If so, branch past code responsible for processing the item use.
bne- 0xc
lwz %r3, 0x4 (%r28)
BranchBackCheckForUnusableItemInMenu:
b 0
ConditionalBranchCheckForUnusableItemInMenu:
b 0

StartUseSpecialItems:
# Call C function to check whether the item being used is a Shine Sprite or
# Strawberry Cake.
addi %r3, %r1, 0x8
bl useSpecialItems
# Restore existing opcode.
lwz %r0, 0x2dc(%r28)
BranchBackUseSpecialItems:
b 0

StartInitTattleLog:
# Move win_log struct pointer into first parameter slot.
mr %r3, %r28
bl initTattleLog
# Restore existing opcode.
lmw	%r27, 0x4c (%sp)
BranchBackInitTattleLog:
b 0
