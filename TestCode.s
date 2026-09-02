		area	lab2_1, code
				entry
__main	proc
		export	__main[weak]
start	mov		r1, #8
		mov		r2, #36
		
gcd		cmp		r1, r2
		sublt	r2, r1
		subgt	r1, r2
		bne		gcd
;		
;gcd		cmp		r1, r2;
;		beq		done
;		blt		less
;		sub		r1, r1, r2
;		b		gcd
;less	sub		r2, r2, r1
;		b		gcd

done	b		.
		endp
		end