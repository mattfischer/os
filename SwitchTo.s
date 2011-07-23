.section .text
.globl SwitchTo
.type SwitchTo,%function
SwitchTo:
	cmp r0, #0
	beq switch
	stm r0, {r0-r15}
	adr r2, finish
	str r2, [r0, #60]

switch:
	ldm r1, {r0-r15}
	
finish:
	bx lr
.size SwitchTo, .-SwitchTo

