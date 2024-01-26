.global StartGivePlayerInvuln
.global BranchBackGivePlayerInvuln
.global StartBtlSeqEndJudgeRule
.global BranchBackBtlSeqEndJudgeRule

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