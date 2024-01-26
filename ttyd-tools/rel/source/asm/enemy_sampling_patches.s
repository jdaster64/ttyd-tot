.global StartSampleRandomTarget
.global BranchBackSampleRandomTarget

StartSampleRandomTarget:
mr %r3, %r5
bl sumWeaponTargetRandomWeights

BranchBackSampleRandomTarget:
b 0