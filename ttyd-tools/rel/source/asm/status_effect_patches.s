.global StartToggleScopedAndCheckFreezeBreak
.global BranchBackToggleScopedAndCheckFreezeBreak

StartToggleScopedAndCheckFreezeBreak:
# If this is the final hit in the attack...
lwz %r0, 0x10 (%r1)
rlwinm. %r0, %r0, 0, 0x100
beq- lbl_CheckFreezeBreak
# Toggle off Scoped status if a player attempted a harmful attack.
mr %r3, %r31
mr %r4, %r27
mr %r5, %r29
bl toggleOffScopedStatus

lbl_CheckFreezeBreak:
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
lbl_SkipFreezeBreak:
# Run original opcode.
lwz %r0, 0x10 (%r1)
BranchBackToggleScopedAndCheckFreezeBreak:
b 0

