.global StartDispEnemyHeldItem
.global BranchBackDispEnemyHeldItem

StartDispEnemyHeldItem:
# Call C function which calls item dispEntry iff no clone enemies are present.
bl dispEnemyHeldItem

BranchBackDispEnemyHeldItem:
b 0