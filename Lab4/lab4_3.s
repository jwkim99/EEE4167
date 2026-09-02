		area lab4_3, code
		entry
__main	 proc
		export __main [weak]
start
		ldr		r0, data1
		bfc		r0, #4, #7
;
		ldr		r0, data1
		ldr		r1, data2
		sbfx	r0, r1, #8, #8
;
		ldr		r0, data2
		rev		r1, r0
;
		ldr		r0, data2
		ldr		r1, data1
		sxtb	r2, r0
		sxth	r2, r0
		uxth	r2, r0
		sxtb	r2, r1
;
		mov		r1, #0x2
		subs	r1, r1, #0x1
		subs	r1, r1, #0x1
		subs	r1, r1, #0x1
		subs	r1, r1, #0x1
		b .
		endp
		align
data1	 dcd	0xb1234567
data2	 dcd	0xbabeface
		end