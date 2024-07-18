.global StartCheckGradualSpRecovery
.global BranchBackCheckGradualSpRecovery

StartCheckGradualSpRecovery:
bl checkGradualSpRecovery
# Restore original opcode.
mr %r3, %r31

BranchBackCheckGradualSpRecovery:
b 0