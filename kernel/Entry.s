.section .entry
.globl Entry

Entry:
	ldr r1, memOffset
	ldr sp, InitStackAddr
	sub sp, sp, r1
	add sp, #256

	ldr r4, InitMapAddr
	mov r0, r4
	bl EntrySetupInitMap

	mcr p15, 0, r4, c2, c0, 0

	mov r0, #0xffffffff
	mcr p15, 0, r0, c3, c0, 0
	
	mrc p15, 0, r0, c1, c0, 0
	mov r1, #1
	orr r0, r1
	mcr p15, 0, r0, c1, c0, 0

	ldr sp, InitStackAddr
	add sp, #256
	
	ldr r0, EntryHighAddr
	bx r0
InitStackAddr:
	.word InitStack
InitMapAddr:
	.word InitMap
EntryHighAddr:
	.word EntryHigh
memOffset:
	.word 0xC0000000
	