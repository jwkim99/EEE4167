sram_base	equ		0x20000000
bb_alias	equ		0x22000000
		area lab3_2, code
		entry
__main	 proc
		export	__main	[weak]
start
		ldr		r0, =bb_alias
		ldr		r4, =sram_base
		ldr		r3, cmask
		ldr		r1, cdata
		and		r1, r3
		str		r1, [r4]
;
		ldr 	r1, cdata
		str		r1, [r4]
		mov		r5, #0
		str		r5, [r0, #0x2c]
;		
		ldr		r6, =0xbabeface
; 		space	0x1000
		b .
cdata	 dcd		0x34567890
cmask	 dcd		0xfffff7ff
		endp
		align
		area	lab3data, data
sdata	 space	0x100
		end