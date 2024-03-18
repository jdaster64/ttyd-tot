.global StartSpendFpOnSwitchPartner
.global BranchBackSpendFpOnSwitchPartner
.global StartFixMarioSingleMoveCheck
.global BranchBackFixMarioSingleMoveCheck

StartSpendFpOnSwitchPartner:
mr %r3, %r28
bl signalPlayerInitiatedPartySwitch
lwz %r0, 0x8 (%r28)  # Original opcode replaced.

BranchBackSpendFpOnSwitchPartner:
b 0

StartFixMarioSingleMoveCheck:
bl checkMarioSingleJumpHammer

BranchBackFixMarioSingleMoveCheck:
b 0