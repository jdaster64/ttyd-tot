.global StartAudienceCheckPlayerTarget
.global BranchBackAudienceCheckPlayerTarget
# Code by Zephiles
.global StartAudienceItem
.global BranchBackAudienceItem
.global StartAudienceItemSpaceFix
.global BranchBackAudienceItemSpaceFix

StartAudienceCheckPlayerTarget:
# Check if target is >= type 0xde (Mario, Shell Shield or partner).
lwz %r0, 0x8 (%r27)
cmpwi %r0, 0xde
# If so, skip to vanilla alliance check.
bge+ 0xc

# Otherwise, set alliance to 1 (enemy) and skip loading alliance.
li %r0, 1
b 0x8

# Check alliance.
lbz %r0, 0xc (%r27)
# Sign-extend and compare (should compare eq only if player-controlled target).
extsb. %r0, %r0

BranchBackAudienceCheckPlayerTarget:
b 0

StartAudienceItem:
stwu %sp,-0x18(%sp)
stmw %r29,0x8(%sp)
mr %r31,%r3
mr %r30,%r4
mr %r29,%r6

mr %r3,%r0
bl getAudienceItem
mr %r0,%r3

mr %r3,%r31
mr %r4,%r30
mr %r6,%r29
lmw %r29,0x8(%sp)
addi %sp,%sp,0x18

# Restore overwritten assembly
stw %r0,0x10(%r6)

BranchBackAudienceItem:
b 0

StartAudienceItemSpaceFix:
# r3 already contains pouchGetEmptyHaveItemCnt
lwz %r4,0x10(%r24) # Crowd Item
bl audienceFixItemSpaceCheck

# Restore overwritten assembly
cmpwi %r3,0

BranchBackAudienceItemSpaceFix:
b 0