.global StartCommandHandleSideSelection
.global BranchBackCommandHandleSideSelection
.global StartCommandDisplaySideSelection
.global BranchBackCommandDisplaySideSelection
.global ConditionalBranchCommandDisplaySideSelection

StartCommandHandleSideSelection:
bl commandHandleSideSelection
# Restore original op.
li %r3, 0x200

BranchBackCommandHandleSideSelection:
b 0

StartCommandDisplaySideSelection:
mr %r3, %r27
bl checkOnSelectedSide
cmpwi %r3, 1
beq+ 0x8

# If not on selected side, skip to next target.
ConditionalBranchCommandDisplaySideSelection:
b 0

# Otherwise, restore original op and continue.
addi %r0, %r27, 0xa6d

BranchBackCommandDisplaySideSelection:
b 0