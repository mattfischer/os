.section .entry
.globl entry

.extern initStack

entry:
	ldr sp, initStackAddr
	add sp, #256
	b start_stub
initStackAddr:
	.word initStack
	