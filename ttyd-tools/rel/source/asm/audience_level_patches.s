.global StartSetTargetAudienceCount
.global BranchBackSetTargetAudienceCount

StartSetTargetAudienceCount:
stwu %sp, -0x18 (%sp)
mflr %r0
stw %r0, 0x1c (%sp)
stw %r3, 0xc (%sp)
stw %r4, 0x10 (%sp)
stw %r5, 0x14 (%sp)
bl setTargetAudienceCount
lwz %r3, 0xc (%sp)
lwz %r4, 0x10 (%sp)
stw %r5, 0x14 (%sp)
lwz %r0, 0x1c (%sp)
mtlr %r0
addi %sp, %sp, 0x18
BranchBackSetTargetAudienceCount:
b 0
