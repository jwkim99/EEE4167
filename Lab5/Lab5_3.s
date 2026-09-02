		area	lab5_3, code
		entry
__main	proc
		export	__main [weak]
		mov		r3, #1	; comp type - 0:add, 1:sub, 2:mul, 3:div
		mov		r1, #8
		mov		r2, #3
		adr		r4, jtable
		tbb 	[r4, r3]
doadd	add		r0,r1,r2
		b		svr0
dosub	sub		r0,r1,r2	
		b		svr0
domul	mul		r0,r1,r2			
		usat	r0,#8,r0
		b		svr0
dodiv	udiv	r0,r1,r2
svr0	ldr		r5, =result
		str		r0, [r5]
		b		.
		align
jtable	
		dcb		0
		dcb		((dosub-doadd)/2)
		dcb		((domul-doadd)/2)
		dcb		((dodiv-doadd)/2)
		endp
		area	lab5data,data
result	dcd		0
		end
		