.global StartCheckBadgeEvasion
.global ConditionalBranchCheckBadgeEvasion
.global BranchBackCheckBadgeEvasion

StartCheckBadgeEvasion:
stwu %sp, -0x20 (%sp)
stmw %r26, 0x8 (%sp)

# Replaces the usual badge checks for evasion.
mr %r3, %r27
bl checkBadgeEvasion

lmw %r26, 0x8 (%sp)
addi %sp, %sp, 0x20

# If checkEvasion returned 1, jump to code that returns with the "Lucky" state;
# otherwise, continue with the remaining evasion checks (Dodgy, fog, etc.)
cmpwi %r3, 0
bne- 0x8
BranchBackCheckBadgeEvasion:
b 0
ConditionalBranchCheckBadgeEvasion:
b 0