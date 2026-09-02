		area	lab4_ex, code
		entry
__main	 proc
		export	__main [weak]
start	ldr		r0, data1
		ldr		r1, data3
		mov		r2, #0x02
		smull	r3, r4, r1, r2
		smull	r3, r4, r0, r1
		ldr		r0, data4
		smull	r3, r4, r0, r1
; ~Ex_1
		bfi		r1, r0, #8, #12
; ~Ex_2
		rev16	r0, r0
; ~Ex_3
		ldr		r0, =0x7FFFFFF0
		ldr		r1, =0x8000000F
		cmp		r0, r1
		ldrlt	r0, data1
		ldrle	r1, data2
		add		r2, r0, r1
; ~Ex_5		
		b .
		endp
		align
data1	 dcd	0x67000005
data2	 dcd	0x41000000
data3	 dcd	0xb7000000
data4	 dcd	0xb1234567
data5	 dcd	0xa0000000
		end