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
# If AC param 4 = -417 and AC output param 0 is 4 (completed set of buttons)...
lwz %r3, 0x1cd8 (%r22)
cmpwi %r3, -417
bne+ check_end_attack
lwz %r3, 0x1ce8 (%r22)
cmpwi %r3, 4
bne+ check_end_attack
# Otherwise, increment AC output param 1.
lwz	%r3, 0x1cec (%r22)
addi %r3, %r3, 1
stw %r3, 0x1cec (%r22)
# If number of bars completed >= 10, then end attack anyway.
cmpwi %r3, 10
bge- check_end_attack
# Otherwise, clear button presses.
li %r3, 0
stw %r3, 0x1f88 (%r22)
stw %r3, 0x1f8c (%r22)
stw %r3, 0x1f90 (%r22)
stw %r3, 0x1f94 (%r22)
ConditionalBranchButtonDownCheckComplete:
b 0
check_end_attack:
# Restore original opcode if not.
lwz	%r3, 0x1cb8 (%r22)
BranchBackButtonDownCheckComplete:
b 0
