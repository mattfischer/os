.section .low
.globl EntryAsm

EntryAsm:
	ldr r1, memOffset
	ldr sp, =InitStack
	sub sp, r1
	add sp, #256

	bl EntryLow

	ldr r1, memOffset
	ldr r0, =KernelTablePAddr
	sub r0, r1
	ldr r0, [r0]
	mcr p15, 0, r0, c2, c0, 0

	ldr r0, domainValue
	mcr p15, 0, r0, c3, c0, 0
	
	mrc p15, 0, r0, c1, c0, 0
	mov r1, #1
	lsl r1, #13
	orr r0, r1
	orr r0, #1
	mcr p15, 0, r0, c1, c0, 0

	ldr sp, =InitStack
	add sp, #256
	
	ldr r0, =Entry
	bx r0
memOffset:
	.word 0xC0000000
domainValue:
	.word 0x55555555

.section .text
.globl vectorStart
vectorStart:
	b vecReset
	b vecUndef
	b vecSWI
	b vecPrefetchAbort
	b vecDataAbort
	.word 0
	b vecIRQ
	b vecFIQ

vecReset:
vecUndef:
vecSWI:
	stmfd sp, {r1-r14}^
	sub sp, #56
	stmfd sp!, {lr}

	ldr r4, [sp, #52]
	ldm r4, {r4}
	stmfd sp!, {r4}

	ldr ip, SysEntryAddr
	blx ip

	add sp, #4

	ldmfd sp!, {lr}
	ldmfd sp, {r1-r14}^
	add sp, #56
	movs pc, lr

vecPrefetchAbort:
vecDataAbort:
vecIRQ:
vecFIQ:
SysEntryAddr:
	.word SysEntry
.globl vectorEnd
vectorEnd:
