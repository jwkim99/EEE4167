		area 	lab5_4, code
		entry
__main	proc
		export	__main [weak]
		mov		r3, #0x08
		ldr		r1, =s_data
		ldr		r2, =d_data
lp		ldrb	r0, [r1], #1
		strb	r0, [r2], #1
		ldrb	r0, [r1], #1
		strb	r0, [r2], #1
		subs	r3, #1
		bne		lp
here	b		here
s_data	dcb		"Sogang Univerisy"
		endp
		align
		area lab5data, data
d_data	space 16
		end