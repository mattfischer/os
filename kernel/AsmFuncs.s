# Various assembly utility functions.

.section .text

# Switch to new task.  r0 = outgoing registers, r1 = new registers
.globl SwitchToAsm
.type SwitchToAsm,%function
SwitchToAsm:
	# Save outgoing registers
	stm r0, {r0-r15}

	# Patch up the saved PC to point to finish.  Execution will
	# resume there when we switch back to this task.
	adr r2, finish
	str r2, [r0, #60]

	# Load incoming registers, including PC
	ldm r1, {r0-r15}
	
finish:
	# Tasks resume here
	bx lr
.size SwitchToAsm, . - SwitchToAsm

# Set page table address.  r0 = physical address
.globl SetMMUBase
.type SetMMUBase,%function
SetMMUBase:
	# Set translation table base address
	mcr p15, 0, r0, c2, c0, 0

	# Flush TLB
	mov r0, #0
	mcr p15, 0, r0, c8, c5, 0

	bx lr
.size SetMMUBase, . - SetMMUBase

# Flush the TLB
.globl FlushTLB
.type FlushTLB,%function
FlushTLB:
	mov r0, #0
	mcr p15, 0, r0, c8, c5, 0
	bx lr
.size FlushTLB, . - FlushTLB

# Enter userspace for the first time.  r0 = starting pc, r1 = starting sp
.globl EnterUser
.type EnterUser,%function
EnterUser:
	# Prepare the usermode CPSR by clearing the mode bits
	mrs r5, cpsr
	bic r5, #0x8f
	msr spsr, r5

	# Carve out a space on the stack to use for initializing
	# the registers
	sub r5, sp, #60
	mov r3, #0

	# Now zero them all out
	mov r4, #16
clearLoop:
	sub r4, r4, #1
	str r3, [r5, r4, lsl #2]
	cmp r4, #0
	bne clearLoop

	# Save the starting stack value into the slot for sp
	str r1, [r5, #52]

	# Save the command line into the slot for r0
	str r2, [r5]

	# Move starting pc into lr, since it's not banked
	mov lr, r0

	# Now initialize all usermode registers to the values
	# prepared above
	ldm r5, {r0-r14}^

	# ARM manual says to nop between accessing different
	# register banks
	nop

	# Everything's ready--transfer to usermode.
	movs pc, lr

	# Poof!
.size EnterUser, . - EnterUser

# Wait for an interrupt to be received
.globl WaitForInterrupt
.type WaitForInterrupt,%function
WaitForInterrupt:
	mov r0, #0
	wfi
	bx lr
.size WaitForInterrupt, . - WaitForInterrupt
