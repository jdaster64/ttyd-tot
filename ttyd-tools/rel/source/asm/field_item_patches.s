.global StartCheckItemFreeze
.global BranchBackCheckItemFreeze

StartCheckItemFreeze:
rlwinm. %r0, %r0, 0, 0x1c, 0x1f
bne- lbl_SkipCheckToTItemFreeze
bl checkItemFreeze
mr %r0, %r3

lbl_SkipCheckToTItemFreeze:
# Repeat the check to take original branch.
rlwinm. %r0, %r0, 0, 0x1c, 0x1f

BranchBackCheckItemFreeze:
b 0