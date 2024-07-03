.global StartGivePlayerInvuln
.global BranchBackGivePlayerInvuln
.global StartBtlSeqEndJudgeRule
.global BranchBackBtlSeqEndJudgeRule
.global StartCalculateCoinDrops
.global BranchBackCalculateCoinDrops
.global StartSkipBanditEscapedCheck
.global BranchBackSkipBanditEscapedCheck
.global ConditionalBranchBackSkipBanditEscapedCheck

StartGivePlayerInvuln:
li %r3, 3000
bl marioSetMutekiTime

BranchBackGivePlayerInvuln:
b 0

StartBtlSeqEndJudgeRule:
bl BtlActRec_JudgeRuleKeep
mr %r3, %r27

BranchBackBtlSeqEndJudgeRule:
b 0

StartCalculateCoinDrops:
mr %r3, %r31  # FbatBattleInformation*
mr %r4, %r29  # NpcEntry*
bl calculateCoinDrops
stb %r3, 0x315 (%r29)
# Restore skipped opcode.
cmpwi %r26, 0

BranchBackCalculateCoinDrops:
b 0

StartSkipBanditEscapedCheck:
# See if option to skip Bandit refights is enabled.
bl checkLetBanditEscape
cmpwi %r3, 0
beq- lbl_DoStolenItemsChecks

# If so, skip stolen item checks and count as victory.
ConditionalBranchBackSkipBanditEscapedCheck:
b 0

# Otherwise, do them as usual.
lbl_DoStolenItemsChecks:
lwz %r0, 0x28c (%r29)
cmpwi %r0, 0x0

BranchBackSkipBanditEscapedCheck:
b 0