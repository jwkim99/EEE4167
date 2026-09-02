		area	lab3_1, code
		entry
__main	 proc
		export __main [weak]
start	 ldr		r0, =0x32
		ldr 	r1, data1
		adr		r2, data1
		ldrb	r3, [r2, #0x04]
		ldr		r4, =data6
		str		r3, [r4]
		ldr		r5, data1
		ldr		r5, data2
		ldr		r5, data3
		ldr		r5, data4
		ldr		r5, data5
		str		r5, [r4, #0x4]
;		b .
		endp
		align
data1	 dcd		0x12345678, 0xbabeface
data2	 dcb		0x12, 0x34
		align
data3	 dcd		0x56, 0x78
data4	 dcw		0xab, 0xcd
data5	 dcb		"Be Smart!"
		align
		area lab3data, data
data6	 space 16
		end