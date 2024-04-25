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

# Consider uncapping Star Power gain in BattleAudienceCheer?
