.section .text
.globl switchToAsm
.type switchToAsm,%function
switchToAsm:
	cmp r0, #0
	beq switch
	stm r0, {r0-r15}
	adr r2, finish
	str r2, [r0, #60]

switch:
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
