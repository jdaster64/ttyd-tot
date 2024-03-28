.global StartGivePlayerInvuln
.global BranchBackGivePlayerInvuln
.global StartBtlSeqEndJudgeRule
.global BranchBackBtlSeqEndJudgeRule
.global StartCalculateCoinDrops
.global BranchBackCalculateCoinDrops

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