.global StartStarPowerLevelMenuDisp
.global BranchBackStarPowerLevelMenuDisp
.global StartStarPowerGetMenuDescriptionMsg
.global BranchBackStarPowerGetMenuDescriptionMsg
.global StartFixItemWinPartyDispOrder
.global BranchBackFixItemWinPartyDispOrder
.global StartFixItemWinPartySelectOrder
.global BranchBackFixItemWinPartySelectOrder
.global StartCheckForUnusableItemInMenu
.global ConditionalBranchCheckForUnusableItemInMenu
.global BranchBackCheckForUnusableItemInMenu
.global StartUseSpecialItems
.global BranchBackUseSpecialItems
.global StartInitTattleLog
.global BranchBackInitTattleLog

StartStarPowerLevelMenuDisp:
bl starPowerMenuDisp

BranchBackStarPowerLevelMenuDisp:
b 0

StartStarPowerGetMenuDescriptionMsg:
# Save registers.
stwu %sp, -0x18 (%sp)
stw %r3, 0xc (%sp)
stw %r4, 0x10 (%sp)
stw %r6, 0x14 (%sp)
# Get the correct Star Power description, given the cursor position in the menu.
mr %r3, %r0
mflr %r0
stw %r0, 0x1c (%sp)
bl getStarPowerMenuDescriptionMsg
mr %r5, %r3
# Load registers.
lwz %r3, 0xc (%sp)
lwz %r4, 0x10 (%sp)
lwz %r6, 0x14 (%sp)
lwz %r0, 0x1c (%sp)
mtlr %r0
addi %sp, %sp, 0x18

BranchBackStarPowerGetMenuDescriptionMsg:
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
