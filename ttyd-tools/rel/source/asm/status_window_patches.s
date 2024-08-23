.global StartHideTopBarInSomeWindows
.global BranchBackHideTopBarInSomeWindows


StartHideTopBarInSomeWindows:
mr %r3, %r31

# Check if this window is a type that wants the top bar hidden.
bl checkHideTopBarInWindow
cmpwi %r3, 1
beq- 0x8
# Otherwise, force window open as usual.
bl statusWinForceOpen

BranchBackHideTopBarInSomeWindows:
b 0