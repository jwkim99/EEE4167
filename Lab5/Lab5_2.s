		area lab5_2, code
		entry
__main	proc
		export	__main [weak]
		mov		r1, #2
		mov		r2, #8
		bl		sub1
		mov		r1, #5
		mov		r2, #3
		bl		sub1
		b		.
		endp
sub1	proc
		add		r3,r1,r2, LSL #2 ; r3=r1+r2*4
		bx		lr
		endp
		end