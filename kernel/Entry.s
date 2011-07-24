.section .entry
.globl Entry

Entry:
	ldr r1, memOffset
	ldr sp, InitStackAddr
	sub sp, sp, r1
	add sp, #256

	ldr r4, KernelMapAddr
	sub r4, r4, r1
	mov r0, r4
	bl EntryInitKernelMap

	mcr p15, 0, r4, c2, c0, 0

	mov r0, #0xffffffff
	mcr p15, 0, r0, c3, c0, 0
	
	mrc p15, 0, r0, c1, c0, 0
	mov r1, #1
	lsl r1, r1, #13
	orr r0, r1
	orr r0, #1
	mcr p15, 0, r0, c1, c0, 0

	ldr sp, InitStackAddr
	add sp, #256
	
	ldr r0, EntryHighAddr
	bx r0
InitStackAddr:
	.word InitStack
KernelMapAddr:
	.word KernelMap
EntryHighAddr:
	.word EntryHigh
memOffset:
	.word 0xC0000000

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
	stmfd sp!,{lr}
	stmfd sp,{r0-r14}^
	sub sp, #60

	ldr ip, SysEntryAddr
	blx ip

	ldmfd sp,{r0-r14}^
	add sp, #60
	ldmfd sp!,{pc}^

vecPrefetchAbort:
vecDataAbort:
vecIRQ:
vecFIQ:
SysEntryAddr:
	.word SysEntry
.globl vectorEnd
vectorEnd:
