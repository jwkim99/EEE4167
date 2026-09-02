		area	lab5_ex, code
		entry
__main	proc
		export	__main [weak]
;			; Data address & Loop count & initial value
		adr		r0, data1
		mov		r1, #0x10	; 16 times of iteration
		mov		r2, #0xFF	; initial minumum value
		mov		r3, #0x00	; initial maximum value
;			; Define Loop
loop	ldrb	r5, [r0], #1
		cmp		r5, r2
		movlt	r2, r5
		cmp		r5, r3
		movgt	r3, r5
		subs	r1, #1
		bne		loop
;			; Store data
		ldr		r4, =data_s
		strb	r2, [r4]
		strb	r3, [r4, #1]
		b		.
data1	dcd		0x56781234
data2	dcd		0xf7c23d8a
data3	dcd		0x6b9a105f
data4	dcd		0xa8e6d47b
		endp
		align
		area	lab5t_data, data
data_s	space	2
		end
			