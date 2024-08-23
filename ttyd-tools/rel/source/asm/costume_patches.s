.global StartBattleGetPartyIconYoshi
.global BranchBackBattleGetPartyIconYoshi
.global StartUnitWinDispYoshiIcon
.global BranchBackUnitWinDispYoshiIcon
.global StartWinPartyInitYoshiIcon
.global BranchBackWinPartyInitYoshiIcon
.global StartSelectDispPartyYoshiIcon
.global BranchBackSelectDispPartyYoshiIcon
.global StartSweetTreatYoshiIcon
.global BranchBackSweetTreatYoshiIcon

StartBattleGetPartyIconYoshi:
bl getYoshiIcon

BranchBackBattleGetPartyIconYoshi:
b 0


StartUnitWinDispYoshiIcon:
bl getYoshiIcon
mr %r4, %r3

BranchBackUnitWinDispYoshiIcon:
b 0


StartWinPartyInitYoshiIcon:
bl getYoshiIcon
mr %r0, %r3
# Store in winPartyDt[3].icon_id.
lis %r3, 0x8037
ori %r3, %r3, 0x73d8
stw %r0, 0x70 (%r3)

BranchBackWinPartyInitYoshiIcon:
b 0


StartSelectDispPartyYoshiIcon:
bl getYoshiIcon

BranchBackSelectDispPartyYoshiIcon:
b 0


StartSweetTreatYoshiIcon:
bl getYoshiHpIcon
mr %r24, %r3

BranchBackSweetTreatYoshiIcon:
b 0