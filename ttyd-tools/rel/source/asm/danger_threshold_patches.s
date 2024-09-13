.global StartSetDangerThreshold
.global BranchBackSetDangerThreshold
.global StartSetPerilThreshold
.global BranchBackSetPerilThreshold
.global StartCheckMarioPinchDisp
.global BranchBackCheckMarioPinchDisp
.global StartCheckPartnerPinchDisp
.global BranchBackCheckPartnerPinchDisp

.global StartFieldDangerIdleCheck1
.global BranchBackFieldDangerIdleCheck1
.global StartFieldDangerIdleCheck2
.global BranchBackFieldDangerIdleCheck2
.global StartFieldDangerIdleCheck3
.global BranchBackFieldDangerIdleCheck3
.global StartFieldDangerIdleCheck4
.global BranchBackFieldDangerIdleCheck4

StartSetDangerThreshold:
# Save registers.
stwu %sp, -0x18 (%sp)
mflr %r0
stw %r0, 0x1c (%sp)
stw %r4, 0xc (%sp)
stw %r5, 0x10 (%sp)
stw %r6, 0x14 (%sp)
# r3 = BattleUnitKind of actor to check status.
# r4, r5 = max HP, base max HP (former for player actors, latter for enemies).
# r6 = Danger (0) or Peril (1).
lha %r4, 0x108 (%r30)
lha %r5, 0x10a (%r30)
li %r6, 0
bl setPinchThreshold
# Load registers.
lwz %r4, 0xc (%sp)
lwz %r5, 0x10 (%sp)
lwz %r6, 0x14 (%sp)
lwz %r0, 0x1c (%sp)
mtlr %r0
addi %sp, %sp, 0x18
# Loads the threshold from the returned BattleUnitKind.
lbz %r0, 0xc (%r3)
BranchBackSetDangerThreshold:
b 0

StartSetPerilThreshold:
# Save registers.
stwu %sp, -0x18 (%sp)
mflr %r0
stw %r0, 0x1c (%sp)
stw %r4, 0xc (%sp)
stw %r5, 0x10 (%sp)
stw %r6, 0x14 (%sp)
# r3 = BattleUnitKind of actor to check status.
# r4, r5 = max HP, base max HP (former for player actors, latter for enemies).
# r6 = Danger (0) or Peril (1).
lha %r4, 0x108 (%r30)
lha %r5, 0x10a (%r30)
li %r6, 1
bl setPinchThreshold
# Load registers.
lwz %r4, 0xc (%sp)
lwz %r5, 0x10 (%sp)
lwz %r6, 0x14 (%sp)
lwz %r0, 0x1c (%sp)
mtlr %r0
addi %sp, %sp, 0x18
# Loads the threshold from the returned BattleUnitKind.
lbz %r0, 0xd (%r3)
BranchBackSetPerilThreshold:
b 0

StartCheckMarioPinchDisp:
lwz %r6, 0x10 (%r31)
lbz %r5, 0xc (%r6)
lbz %r4, 0xd (%r6)
cmpw %r3, %r5
BranchBackCheckMarioPinchDisp:
b 0

StartCheckPartnerPinchDisp:
lwz %r6, 0x10 (%r31)
lbz %r5, 0xc (%r6)
lbz %r4, 0xd (%r6)
cmpw %r3, %r5
BranchBackCheckPartnerPinchDisp:
b 0

StartFieldDangerIdleCheck1:
bl getMarioHpForFieldAnim
BranchBackFieldDangerIdleCheck1:
b 0

StartFieldDangerIdleCheck2:
bl getMarioHpForFieldAnim
BranchBackFieldDangerIdleCheck2:
b 0

StartFieldDangerIdleCheck3:
bl getMarioHpForFieldAnim
BranchBackFieldDangerIdleCheck3:
b 0

StartFieldDangerIdleCheck4:
bl getMarioHpForFieldAnim
BranchBackFieldDangerIdleCheck4:
b 0
