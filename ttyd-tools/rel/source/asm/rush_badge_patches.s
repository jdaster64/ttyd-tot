.global StartGetDangerStrength
.global BranchBackGetDangerStrength
.global StartGetPerilStrength
.global BranchBackGetPerilStrength

StartGetDangerStrength:
stwu %sp, -0x18 (%sp)
mflr %r0
stw %r0, 0x1c (%sp)
stw %r4, 0xc (%sp)
stw %r5, 0x10 (%sp)
bl getDangerStrength
lwz %r4, 0xc (%sp)
lwz %r5, 0x10 (%sp)
lwz %r0, 0x1c (%sp)
mtlr %r0
addi %sp, %sp, 0x18
BranchBackGetDangerStrength:
b 0

StartGetPerilStrength:
stwu %sp, -0x18 (%sp)
mflr %r0
stw %r0, 0x1c (%sp)
stw %r4, 0xc (%sp)
stw %r5, 0x10 (%sp)
bl getPerilStrength
lwz %r4, 0xc (%sp)
lwz %r5, 0x10 (%sp)
lwz %r0, 0x1c (%sp)
mtlr %r0
addi %sp, %sp, 0x18
BranchBackGetPerilStrength:
b 0