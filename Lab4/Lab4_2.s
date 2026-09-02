		area	lab4_2, code
		entry
__main	 proc
		export	__main [weak]
start
		ldr		r0, data3
		asrs	r1, r0, #0x3
;
;
		ldr		r0, data2
		mov		r1, #0x02
		mul		r2, r0, r1
;
		ldr		r2, data1
		umull	r3, r4, r0, r2
;
		ldr		r0, =data1
		ldm		r0, {r1,r2,r3,r4}
		mov		r0, #0x04
		udiv	r5, r1, r0
		udiv	r6, r1, r2
		sdiv	r5, r4, r0
		sdiv	r6, r4, r3
;
		b .
		endp
		align
data1	 dcd	0x67000005
data2	 dcd	0x41000000
data3	 dcd	0xb7000005
data4	 dcd	0xb1234567
data5	 dcd	0xa0000000
		end