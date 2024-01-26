.global StartSweetTreatSetupTargets
.global BranchBackSweetTreatSetupTargets
.global StartSweetTreatBlinkNumbers
.global BranchBackSweetTreatBlinkNumbers
.global StartEarthTremorNumberOfBars
.global BranchBackEarthTremorNumberOfBars
.global StartArtAttackCalculateDamage
.global BranchBackArtAttackCalculateDamage

StartSweetTreatSetupTargets:
# Replace existing logic to set up how many of each target type appears.
bl sweetTreatSetupTargets

BranchBackSweetTreatSetupTargets:
b 0

StartSweetTreatBlinkNumbers:
# Replace existing logic to support Sweet Feast's values being divided by 5.
bl sweetTreatBlinkNumbers

BranchBackSweetTreatBlinkNumbers:
b 0

StartEarthTremorNumberOfBars:
# Override the normal length of the minigame (r0 = number of bars completed).
bl getEarthTremorNumberOfBars
cmpw %r0, %r3

BranchBackEarthTremorNumberOfBars:
b 0

StartArtAttackCalculateDamage:
# r27 has the percentage of the enemy circled (integer out of 100).
mr %r3, %r27
# Replace existing logic to calculate the damage dealt for a given percentage.
bl getArtAttackPower
stw %r3, 0x0 (%r31)

BranchBackArtAttackCalculateDamage:
b 0