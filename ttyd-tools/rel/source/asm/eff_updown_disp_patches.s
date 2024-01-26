.global StartDispUpdownNumberIcons
.global BranchBackDispUpdownNumberIcons

StartDispUpdownNumberIcons:
# Call C function to replace the logic for drawing number icons.
mr %r3, %r28            # Number
addi %r4, %r1, 0x8      # GXTexObj
# Use a different icon transformation matrix based on unknown value.
lwz %r0, 0x20 (%r29)
cmplwi %r0, 0xf
bge- 0xc
    addi %r5, %r1, 0x58 # Transformation matrix w/ scale factor
    b 0x8
addi %r5, %r1, 0x28     # Transformation matrix w/o scale factor
addi %r6, %r30, 0x11c   # View matrix
lbz %r7, 0x24 (%r29)    # unknown value
bl dispUpdownNumberIcons
# No logic left to run in original function; branch back to end of function.

BranchBackDispUpdownNumberIcons:
b 0