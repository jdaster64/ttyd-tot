.global StartStatusIconDisplay
.global BranchBackStatusIconDisplay
.global ConditionalBranchStatusIconDisplay

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
