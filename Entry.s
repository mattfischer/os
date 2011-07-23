.section .entry
.globl entry

.extern initStack

entry:
	ldr r1, memOffset
	ldr sp, initStackAddr
	sub sp, sp, r1
	add sp, #256

	ldr r4, initMapAddr
	mov r0, r4
	bl entry_setupInitMap

	mcr p15, 0, r4, c2, c0, 0

	mov r0, #0xffffffff
	mcr p15, 0, r0, c3, c0, 0
	
	mrc p15, 0, r0, c1, c0, 0
	mov r1, #1
	orr r0, r1
	mcr p15, 0, r0, c1, c0, 0

	ldr sp, initStackAddr
	add sp, #256
	
	ldr r0, entry_mmuAddr
	bx r0
initStackAddr:
	.word initStack
initMapAddr:
	.word initMap
entry_mmuAddr:
	.word entry_mmu
memOffset:
	.word 0xC0000000
	