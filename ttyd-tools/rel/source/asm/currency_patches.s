.global StartCheckDeleteFieldItem
.global BranchBackCheckDeleteFieldItem

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