.global StartDispTattleStats
.global BranchBackDispTattleStats

StartDispTattleStats:
# Move pointer to current BattleWorkUnit into fifth parameter slot.
mr %r7, %r29
# Replace call to iconNumberDispGx (which would draw HP) with custom function
# that additionally draws ATK and DEF numbers in some cases.
# Note: parameters 1-4 are the originally intended iconNumberDispGx parameters.
bl dispTattleStats

BranchBackDispTattleStats:
b 0