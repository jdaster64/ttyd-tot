.global CharlietonPitPriceListPatchStart
.global CharlietonPitPriceListPatchEnd
.global CharlietonPitPriceItemPatchStart
.global CharlietonPitPriceItemPatchEnd

CharlietonPitPriceListPatchStart:
# Add 11 to 0-index floor number. (reward floors become 20, 30, ... 100)
addi %r5, %r3, 11
li %r0, 1
mullw %r4, %r4, %r5
# Multiply by 1/100 of 2**32 and keep high 32 bits. (prices = 20%, 30% ... 100%)
lis %r3, 0x28f
addi %r3, %r3, 0x5c29
mulhw %r23, %r3, %r4
cmpwi %r23, 999
ble+ 0x8
li %r23, 999

CharlietonPitPriceListPatchEnd:
nop

CharlietonPitPriceItemPatchStart:
lis %r5, 0x8031
ori %r0, %r5, 0x08a8
addi %r5, %r3, 11
add %r3, %r0, %r27
lhz %r0, 0x0014 (%r3)
mullw %r0, %r0, %r5
lis %r4, 0x28f
addi %r4, %r4, 0x5c29
mulhw %r0, %r4, %r0
cmpwi %r0, 999
ble+ 0x8
li %r0, 999
stw %r0, 0x00a8 (%r31)
lbz %r0, 0x001c (%r3)
extsb %r0, %r0
stw %r0, 0x00ac (%r31)

CharlietonPitPriceItemPatchEnd:
nop