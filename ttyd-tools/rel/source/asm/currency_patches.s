.global StartCheckDeleteFieldItem
.global BranchBackCheckDeleteFieldItem
.global StartSkipDrawingXIfCurrencyHigh
.global BranchBackSkipDrawingXIfCurrencyHigh

StartCheckDeleteFieldItem:
cmpwi %r0, 0x7a
beq+ checkBigCoin_Success
cmpwi %r0, 0x7c
bne- checkBigCoinFlower_Fail

checkBigCoin_Success:
li %r0, 1
b 8

checkBigCoinFlower_Fail:
li %r0, 0

cmpwi %r0, 1

BranchBackCheckDeleteFieldItem:
b 0


StartSkipDrawingXIfCurrencyHigh:
# Load currently displayed number, rather than actual currency amount.
lwz %r5, 0x4 (%r31)
cmpwi %r5, 1000
bge- 0xc
# Display icon 0x1de ('x' icon).
li %r5, 0x1de
bl iconDispGx2

BranchBackSkipDrawingXIfCurrencyHigh:
b 0