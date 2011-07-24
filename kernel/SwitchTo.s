.section .text

.globl switchToFirstAsm
.type switchToFirstAsm,%function
switchToFirstAsm:
	ldm r0, {r0-r15}
.size switchToAsm, . - switchToAsm

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

	stmfd sp!,{r0}
	mov r0, #0
	stmfd sp!,{r0}
	stmfd sp!,{r1}
	mov r0,sp
	ldmfd r0,{sp,lr}^
	add sp, #8

	mov r0,#0
	mov r1,#0
	mov r2,#0
	mov r3,#0
	mov r4,#0
	mov r5,#0
	mov r6,#0
	mov r7,#0
	mov r8,#0
	mov r9,#0
	mov r10,#0
	mov r11,#0
	mov r12,#0
	ldmfd sp!,{pc}^

.size EnterUser, . - EnterUser
