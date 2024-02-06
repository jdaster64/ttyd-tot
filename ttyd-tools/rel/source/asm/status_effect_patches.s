.global StartFreezeBreakOnAllAttacks
.global BranchBackFreezeBreakOnAllAttacks

StartFreezeBreakOnAllAttacks:
# Check for ice element, and skip new code if present.
lbz %r0, 0x6c (%r29)
cmpwi %r0, 2
beq- lbl_SkipFreezeBreak
# Check for Freeze status on the target.
lbz %r0, 0x122 (%r27)
cmpwi %r0, 0
ble+ lbl_SkipFreezeBreak
# Check that the hit will do damage.
cmpwi %r22, 0
ble- lbl_SkipFreezeBreak
# Apply "Freeze break" effect to next hit.
lwz %r0, 0x2c (%r1)
ori %r0, %r0, 0x8000
stw %r0, 0x2c (%r1)
# Original opcode.
lbl_SkipFreezeBreak:
lwz %r0, 0x10 (%r1)
BranchBackFreezeBreakOnAllAttacks:
b 0