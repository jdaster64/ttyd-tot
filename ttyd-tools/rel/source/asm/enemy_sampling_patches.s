.global StartFixScreenNukeTarget
.global BranchBackFixScreenNukeTarget
.global StartSampleRandomTarget
.global BranchBackSampleRandomTarget

StartFixScreenNukeTarget:

// Check for multi-target move
lwz %r0, 0x64 (%r3)
rlwinm. %r0, %r0, 0, 0x2000000
beq+ lbl_ChoiceEnemy_SkipTarget

// Check for damaging move
lwz %r0, 0x1c (%r3)
cmpwi %r0, 0
beq- lbl_ChoiceEnemy_SkipTarget

// Check for enemy being a Bones type
lwz %r0, 0x4 (%r21)
cmpwi %r0, 0x16
beq- lbl_ChoiceEnemy_FixNukePatchEnd
cmpwi %r0, 0x82
beq- lbl_ChoiceEnemy_FixNukePatchEnd
cmpwi %r0, 0x83
beq- lbl_ChoiceEnemy_FixNukePatchEnd

// If any of the above are not true, don't consider dead enemy a target
lbl_ChoiceEnemy_SkipTarget:
li %r0, 1
stwx %r0, %r30, %r31

lbl_ChoiceEnemy_FixNukePatchEnd:
BranchBackFixScreenNukeTarget:
b 0


StartSampleRandomTarget:
mr %r3, %r5
bl sumWeaponTargetRandomWeights

BranchBackSampleRandomTarget:
b 0