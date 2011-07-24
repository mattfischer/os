.section .text

.globl runFirstAsm
.type runFirstAsm,%function
runFirstAsm:
	ldm r0, {r0-r15}
.size runFirstAsm, . - runFirstAsm

.globl switchToAsm
.type switchToAsm,%function
switchToAsm:
	stm r0, {r0-r15}
	adr r2, finish
	str r2, [r0, #60]
	ldm r1, {r0-r15}
	
finish:
	bx lr
.size switchToAsm, . - switchToAsm

.globl setMMUBase
.type setMMUBase,%function
setMMUBase:
	mcr p15, 0, r0, c2, c0, 0
	bx lr
.size setMMUBase, . - setMMUBase

.globl swi
.type swi,%function
swi:
	swi 0
	bx lr
.size swi, . - swi

.globl EnterUser
.type EnterUser,%function
EnterUser:
	mrs r2, cpsr
	bic r2, #0xf
	msr spsr, r2

	sub r2, sp, #60
	mov r3, #0
	str r3, [r2, #0]
	str r3, [r2, #4]
	str r3, [r2, #8]
	str r3, [r2, #12]
	str r3, [r2, #16]
	str r3, [r2, #20]
	str r3, [r2, #24]
	str r3, [r2, #28]
	str r3, [r2, #32]
	str r3, [r2, #36]
	str r3, [r2, #40]
	str r3, [r2, #44]
	str r3, [r2, #48]
	str r1, [r2, #52]
	str r3, [r2, #56]

	mov lr, r0
	ldmfd r2, {r0-r14}^
	nop
	movs pc, lr

.size EnterUser, . - EnterUser
