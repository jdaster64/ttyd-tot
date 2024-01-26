.global StartCheckPouchAlloc
.global BranchBackCheckPouchAlloc

StartCheckPouchAlloc:
bl getOrAllocPouch

BranchBackCheckPouchAlloc:
b 0