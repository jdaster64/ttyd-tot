.global StartGetItemMax
.global BranchBackGetItemMax
.global StartRemoveItemMax
.global BranchBackRemoveItemMax
.global StartRemoveItemIndexMax
.global BranchBackRemoveItemIndexMax
.global StartGetEmptyItemSlotsMax
.global BranchBackGetEmptyItemSlotsMax

StartGetItemMax:
bl getTotItemInventorySize
mr %r4, %r3
# Restore original opcode.
li %r0, 1

BranchBackGetItemMax:
b 0

StartRemoveItemMax:
bl getTotItemInventorySize
mr %r4, %r3

BranchBackRemoveItemMax:
b 0

StartRemoveItemIndexMax:
bl getTotItemInventorySize
mr %r4, %r3

BranchBackRemoveItemIndexMax:
b 0

StartGetEmptyItemSlotsMax:
bl getTotItemInventorySize
mr %r0, %r3
# Restore original opcode.
lwz %r4, 0x1be0 (%r13)

BranchBackGetEmptyItemSlotsMax:
b 0