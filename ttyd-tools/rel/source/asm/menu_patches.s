# win_root
.global StartHakoGxInitializeFields
.global BranchBackHakoGxInitializeFields
.global StartHakoGxCheckDrawNoItemBox
.global ReturnHakoGxCheckDrawNoItemBoxNoItemCase
.global ReturnHakoGxCheckDrawNoItemBoxItemCase
.global StartHakoGxCheckDrawItemIcon
.global ReturnHakoGxCheckDrawItemIconNoItemCase
.global ReturnHakoGxCheckDrawItemIconItemCase
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


StartHakoGxInitializeFields:
mr. %r0, %r15
bne+ lbl_HakoGxInit_ItemSetup

lbl_HakoGxInit_BadgeSetup:
lfs %f31, 0xe54 (%r23)  // badge_log_scroll_y
lwz %r24, 0xe44 (%r23)  // badge_log_total_count
lwz %r0, 0xe50 (%r23)   // badge_log_page_num
stw %r0, 0x588 (%r1)
b lbl_HakoGxInit_Exit

lbl_HakoGxInit_ItemSetup:
lfs %f31, 0xf34 (%r23)  // recipe_log_scroll_y
lwz %r24, 0xf24 (%r23)  // recipe_log_total_count
lwz %r0, 0xf30 (%r23)   // recipe_log_page_num
stw %r0, 0x588 (%r1)

lbl_HakoGxInit_Exit:
BranchBackHakoGxInitializeFields:
b 0


StartHakoGxCheckDrawNoItemBox:
mr %r3, %r23
mr %r4, %r27
mr %r5, %r30
rlwinm %r5, %r5, 31, 0x7fffffff
bl getIconForBadgeOrItemLogEntry
cmpwi %r3, -1
beq+ 8

ReturnHakoGxCheckDrawNoItemBoxItemCase:
b 0

ReturnHakoGxCheckDrawNoItemBoxNoItemCase:
b 0


StartHakoGxCheckDrawItemIcon:
mr %r3, %r23
mr %r4, %r27
mr %r5, %r16
rlwinm %r5, %r5, 31, 0x7fffffff
bl getIconForBadgeOrItemLogEntry
cmpwi %r3, -1
beq+ 8

ReturnHakoGxCheckDrawItemIconItemCase:
b 0

ReturnHakoGxCheckDrawItemIconNoItemCase:
b 0


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
