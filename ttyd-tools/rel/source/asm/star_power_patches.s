.global StartEnableAppealCheck
.global BranchBackEnableAppealCheck
.global StartAddAudienceCheck
.global BranchBackAddAudienceCheck
.global StartDisplayAudienceCheck
.global BranchBackDisplayAudienceCheck
.global StartSaveAudienceCountCheck
.global BranchBackSaveAudienceCountCheck
.global StartSetInitialAudienceCheck
.global BranchBackSetInitialAudienceCheck
.global StartObjectFallOnAudienceCheck
.global BranchBackObjectFallOnAudienceCheck
.global StartAddPuniToAudienceCheck
.global BranchBackAddPuniToAudienceCheck
.global StartEnableIncrementingBingoCheck
.global BranchBackEnableIncrementingBingoCheck
.global StartApplySpRegenMultiplierNoBingo
.global BranchBackApplySpRegenMultiplierNoBingo
.global StartApplySpRegenMultiplierBingo
.global BranchBackApplySpRegenMultiplierBingo

# Check if Appeal should be enabled.
StartEnableAppealCheck:
# Save battleWork pointer.
mr %r29, %r3
bl checkStarPowersEnabled
mr %r0, %r3
mr %r3, %r29
BranchBackEnableAppealCheck:
b 0

# Check if audience should be added.
StartAddAudienceCheck:
bl checkStarPowersEnabled
mr %r0, %r3
BranchBackAddAudienceCheck:
b 0

# 801a6cb0 -> 801a6cb4
# Check if audience should be displayed.
StartDisplayAudienceCheck:
bl checkStarPowersEnabled
mr %r0, %r3
BranchBackDisplayAudienceCheck:
b 0

# Check if audience number should be saved at end-of-battle.
StartSaveAudienceCountCheck:
bl checkStarPowersEnabled
mr %r0, %r3
BranchBackSaveAudienceCountCheck:
b 0

# Check if audience should be initialized.
StartSetInitialAudienceCheck:
bl checkStarPowersEnabled
mr %r0, %r3
BranchBackSetInitialAudienceCheck:
b 0

# Enable objects falling on audience.
StartObjectFallOnAudienceCheck:
bl checkStarPowersEnabled
mr %r0, %r3
BranchBackObjectFallOnAudienceCheck:
b 0

# Enable adding Punis to audience.
StartAddPuniToAudienceCheck:
bl checkStarPowersEnabled
mr %r0, %r3
BranchBackAddPuniToAudienceCheck:
b 0

# Enable incrementing BINGO slots.
StartEnableIncrementingBingoCheck:
bl checkStarPowersEnabled
mr %r0, %r3
BranchBackEnableIncrementingBingoCheck:
b 0

# Apply a custom multiplier to Star Power regen from attacks.
# (There are two entry points with the same logic, based on whether there was
# a BINGO activated recently.)

StartApplySpRegenMultiplierNoBingo:
# Save registers r0, r3, r4.
stwu %sp, -0x18 (%sp)
stw %r3, 0xc (%sp)
stw %r4, 0x10 (%sp)
stw %r0, 0x14 (%sp)
mflr %r0
stw %r0, 0x1c (%sp)
# Move f0 to 1st fp parameter slot and call wrapper, then convert to int in f0.
fmr %f1, %f0
bl applySpRegenMultiplier
fctiwz %f0, %f1
# Load registers.
lwz %r0, 0x1c (%sp)
mtlr %r0
lwz %r3, 0xc (%sp)
lwz %r4, 0x10 (%sp)
lwz %r0, 0x14 (%sp)
addi %sp, %sp, 0x18
BranchBackApplySpRegenMultiplierNoBingo:
b 0

StartApplySpRegenMultiplierBingo:
# Save registers r0, r3, r4.
stwu %sp, -0x18 (%sp)
stw %r3, 0xc (%sp)
stw %r4, 0x10 (%sp)
stw %r0, 0x14 (%sp)
mflr %r0
stw %r0, 0x1c (%sp)
# Move f0 to 1st fp parameter slot and call wrapper, then convert to int in f0.
fmr %f1, %f0
bl applySpRegenMultiplier
fctiwz %f0, %f1
# Load registers.
lwz %r0, 0x1c (%sp)
mtlr %r0
lwz %r3, 0xc (%sp)
lwz %r4, 0x10 (%sp)
lwz %r0, 0x14 (%sp)
addi %sp, %sp, 0x18
BranchBackApplySpRegenMultiplierBingo:
b 0

# Consider uncapping Star Power gain in BattleAudienceCheer?
