.global StartButtonDownChooseButtons
.global BranchBackButtonDownChooseButtons
.global StartButtonDownCheckComplete
.global BranchBackButtonDownCheckComplete
.global ConditionalBranchButtonDownCheckComplete

# Override button choices if using custom mode...
StartButtonDownChooseButtons:
# If AC param 4 = -417...
lwz %r0, 0x1cd8 (%r22)
cmpwi %r0, -417
bne+ 0x24
# Set buttons to A, B, Y, X in order.
li %r0, 0x100
stw %r0, 0x1f54 (%r22)
li %r0, 0x200
stw %r0, 0x1f58 (%r22)
li %r0, 0x800
stw %r0, 0x1f5c (%r22)
li %r0, 0x400
stw %r0, 0x1f60 (%r22)
# Original opcode:
li %r0, 99
BranchBackButtonDownChooseButtons:
b 0

# Reset buttons rather than ending command if using custom mode...
StartButtonDownCheckComplete:
# If AC param 4 = -417...
lwz %r3, 0x1cd8 (%r22)
cmpwi %r3, -417
beq- 0xc
# Restore original opcode if not.
lwz	%r3, 0x1cb8 (%r22)
BranchBackButtonDownCheckComplete:
b 0
# Otherwise, increment AC output param 1, and clear button press history.
lwz	%r3, 0x1cec (%r22)
addi %r3, %r3, 1
stw %r3, 0x1cec (%r22)
li %r3, 0
stw %r3, 0x1f88 (%r22)
stw %r3, 0x1f8c (%r22)
stw %r3, 0x1f90 (%r22)
stw %r3, 0x1f94 (%r22)
ConditionalBranchButtonDownCheckComplete:
b 0
