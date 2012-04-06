# Entry points, both system startup and syscall.

.section .low
.globl EntryAsm

EntryAsm:
	# Beginning of the world.  MMU is off, so we are still
	# in low addresses.  First order of business is to get the
	# MMU on, and pivot up to high addresses.

	# Set up stack.  The InitStack symbol is a high address, so
	# we have to subtract the difference to get a low address.
	ldr r1, memOffset
	ldr sp, =InitStack
	sub sp, r1
	add sp, #4096

	# Jump to C++ code to build the initial page table
	bl BuildInitPageTable
	mcr p15, 0, r0, c2, c0, 0

	# Enable all domains
	ldr r0, domainValue
	mcr p15, 0, r0, c3, c0, 0

	# This is it.  The page tables are all set, so set high
	# exception vectors, and enable the MMU.
	mrc p15, 0, r0, c1, c0, 0
	mov r1, #1
	lsl r1, #13
	orr r0, r1
	orr r0, #1
	mcr p15, 0, r0, c1, c0, 0

	# High addresses are now valid, so no more memOffset
    # subtraction from here on out.  Reset the stack pointer
	# to the high address.
	ldr sp, =InitStack
	add sp, #4096

	# Set up interrupt-mode stack
	mov r0, #0xd2
	msr cpsr, r0
	ldr sp, =InitStack
	add sp, #4096
	mov r0, #0xd3
	msr cpsr, r0

	# Assembly init is done.  Jump to C++.
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
	# Syscall entry.  Save all userspace registers to the
	# SVC-mode stack.
	stmfd sp, {r1-r14}^
	sub sp, #56

	# Save userspace return address, located in lr.
	stmfd sp!, {lr}

	# r0-r3 contain the first three parameters.  Grab the fourth
	# off of the userspace stack, and store it to the bottom of
	# the SVC-mode stack.
	ldr r4, [sp, #52]
	ldm r4, {r4}
	stmfd sp!, {r4}

	# Jump to the C++ syscall handler
	ldr ip, SysEntryAddr
	blx ip

	# Increment past the stack-spilled parameters from above
	add sp, #4

	# Load the userspace return address
	ldmfd sp!, {lr}

	# Restore the rest of the registers
	ldmfd sp, {r1-r14}^
	add sp, #56

	# Jump back to userspace
	movs pc, lr

vecPrefetchAbort:
vecDataAbort:
	b vecDataAbort

vecIRQ:
	# IRQ entry. Save the caller-saved registers
	stmfd sp!, {r0-r3,ip,lr}

	# Jump to the C++ IRQ handler
	ldr ip, IRQEntryAddr
	blx ip

	# Restore the caller-saved registers
	ldmfd sp!, {r0-r3,ip,lr}

	# Jump back to userspace
	subs pc, lr, #4

vecFIQ:
SysEntryAddr:
	.word SysEntry
IRQEntryAddr:
	.word IRQEntry
.globl vectorEnd
vectorEnd:
