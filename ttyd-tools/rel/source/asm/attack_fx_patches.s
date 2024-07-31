.global StartCheckPlayAttackFX
.global BranchBackCheckPlayAttackFX
.global ConditionalBranchCheckPlayAttackFX
.global StartPlayFieldHammerFX
.global BranchBackPlayFieldHammerFX

StartCheckPlayAttackFX:
# Pointer to sound effect 3d location.
addi %r4, %r1, 0x54
bl checkPlayAttackFx
cmpwi %r3, 0
beq+ 8

ConditionalBranchCheckPlayAttackFX:
b 0

BranchBackCheckPlayAttackFX:
b 0


StartPlayFieldHammerFX:
addi %r3, %r31, 0x8c
bl playFieldHammerFX

BranchBackPlayFieldHammerFX:
b 0