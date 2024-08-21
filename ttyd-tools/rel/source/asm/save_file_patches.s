.global StartSaveSlotData
.global BranchBackSaveSlotData

StartSaveSlotData:
mr %r3, %r31
bl saveSlotData

BranchBackSaveSlotData:
b 0