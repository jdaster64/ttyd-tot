.global StartButtonDownChooseButtons
.global BranchBackButtonDownChooseButtons
.global StartButtonDownWrongButton
.global BranchBackButtonDownWrongButton
.global ConditionalBranchButtonDownWrongButton
.global StartButtonDownCheckComplete
.global BranchBackButtonDownCheckComplete
.global ConditionalBranchButtonDownCheckComplete
.global StartGetGuardDifficulty
.global BranchBackGetGuardDifficulty

# Override button choices if using custom mode...
StartButtonDownChooseButtons:
# If AC param 4 = -417...
lwz %r0, 0x1cd8 (%r22)
cmpwi %r0, -417
bne+ 0x24
# Set buttons to A, B, Y, X in order.
li %r0, 0x100
stw %r0, 0x1f54 (%r22)
li %r0, 0x200
stw %r0, 0x1f58 (%r22)
li %r0, 0x800
stw %r0, 0x1f5c (%r22)
li %r0, 0x400
stw %r0, 0x1f60 (%r22)
# Original opcode:
li %r0, 99
BranchBackButtonDownChooseButtons:
b 0

# Don't end Action command early on wrong button if using custom mode.
StartButtonDownWrongButton:
lwz %r0, 0x1cd8 (%r22)
cmpwi %r0, -417
bne+ SnowWhirled_end_attack
ConditionalBranchButtonDownWrongButton:
b 0
SnowWhirled_end_attack:
BranchBackButtonDownWrongButton:
# Restore original opcode.
li %r0, 0
b 0

# Reset buttons rather than ending command if using custom mode...
StartButtonDownCheckComplete:
# If AC param 4 = -417, run Snow Whirled logic, otherwise end attack.
lwz %r3, 0x1cd8 (%r22)
cmpwi %r3, -417
bne+ SnowWhirled_check_end_attack
# If number of buttons is < 4, don't end attack, but don't run logic either.
lwz %r3, 0x1ce8 (%r22)
cmpwi %r3, 4
bne+ SnowWhirled_dont_increment
# If number of buttons == 4, Increment AC output param 1 (# complete cycles).
lwz %r3, 0x1cec (%r22)
addi %r3, %r3, 1
stw %r3, 0x1cec (%r22)
# If number of bars completed >= 10, then end attack anyway.
cmpwi %r3, 10
bge- SnowWhirled_check_end_attack
# Otherwise, clear button presses, readying for next bar.
li %r3, 0
stw %r3, 0x1f88 (%r22)
stw %r3, 0x1f8c (%r22)
stw %r3, 0x1f90 (%r22)
stw %r3, 0x1f94 (%r22)
SnowWhirled_dont_increment:
ConditionalBranchButtonDownCheckComplete:
b 0
SnowWhirled_check_end_attack:
# Restore original opcode if not.
lwz	%r3, 0x1cb8 (%r22)
BranchBackButtonDownCheckComplete:
b 0

# Override the ac difficulty check in BattleActionCommandCheckDefence.
StartGetGuardDifficulty:
# For whatever reason, pretty much all volatile registers are used here.
stwu %sp, -0x90 (%sp)
stmw %r3, 0x8 (%sp)
bl getActionCommandDifficulty
mr %r0, %r3
lmw %r3, 0x8 (%sp)
addi %sp, %sp, 0x90
mr %r11, %r0
BranchBackGetGuardDifficulty:
b 0
