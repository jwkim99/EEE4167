		area	lab5_1, code
		entry
__main	proc
		export __main [weak]
start	mov		r1, #8
		mov		r2, #32
gcd		cmp		r1,r2
		beq		done
		blt		less
		sub		r1,r1,r2
		b		gcd
less	sub		r2,r2,r1		
		b	gcd
;		space 0x800
done	
		b 		.
;		ldr		r3, =start
;		bx		r3
		align
		endp
		end