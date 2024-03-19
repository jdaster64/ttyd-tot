.global StartHappyHeartProc
.global BranchBackHappyHeartProc
.global StartHappyFlowerProc
.global BranchBackHappyFlowerProc

StartHappyHeartProc:
# Check if turn count is odd...
lha %r3, 0 (%r31)
rlwinm %r3, %r3, 0, 0x1
cmpwi %r3, 1
# Skip adding 1 to heal, if so.
beq+ 8
addi %r27, %r27, 1

BranchBackHappyHeartProc:
b 0

StartHappyFlowerProc:
# Check if turn count is odd...
lha %r3, 0 (%r31)
rlwinm %r3, %r3, 0, 0x1
cmpwi %r3, 1
# Skip adding 1 to heal, if so.
beq+ 8
addi %r27, %r27, 1

BranchBackHappyFlowerProc:
b 0
