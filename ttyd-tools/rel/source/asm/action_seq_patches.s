.global StartSetConfuseProcRate
.global BranchBackSetConfuseProcRate

StartSetConfuseProcRate:
# Load confusion proc rate from unused byte in unit params.
lbz %r0, 0x136 (%r31)
cmpw %r3, %r0

BranchBackSetConfuseProcRate:
b 0