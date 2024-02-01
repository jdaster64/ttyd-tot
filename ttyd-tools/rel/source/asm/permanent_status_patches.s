.global StartCheckRecoveryStatus
.global BranchBackCheckRecoveryStatus
.global ConditionalBranchCheckRecoveryStatus
.global StartStatusIconDisplay
.global BranchBackStatusIconDisplay
.global ConditionalBranchStatusIconDisplay

# Skip turn count decrement if current turn count is > 100 ('permanent').
# Original opcode doesn't need to be replaced as result is still in r4.
StartCheckRecoveryStatus:
lbz %r4, 9 (%r1)
extsb %r0, %r4
cmpwi %r0, 100
bge- 0x8
BranchBackCheckRecoveryStatus:
b 0
ConditionalBranchCheckRecoveryStatus:
b 0

# Skip drawing icons for permanent statuses.
StartStatusIconDisplay:
lbz %r28, 9 (%r1)
extsb %r28, %r28
cmpwi %r28, 100
bge- 0xc
# No permanent status; run original opcode.
stw %r28, 0 (%r30)
BranchBackStatusIconDisplay:
b 0
# Return false (no new icon added).
li %r3, 0
ConditionalBranchStatusIconDisplay:
b 0
