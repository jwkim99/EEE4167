			area	subcode, code
			entry
itoa		 proc
			export	itoa
;-----------------------------------------
			push 	{r0-r5,lr}
			ldr		r0, [sp, #28]	; store address
			ldr		r1, [sp, #32]	; integer
			ldr		r2, [sp, #36]	; base
			mov		r5, #0			; count
	; calculating conversion
lp1			udiv	r3, r1, r2		; quotient
			mls		r4, r2, r3, r1	; remainder
			push	{r4}			; storing remainder into stack
			cmp		r3, #0			; check whether quotient == 0
			add		r5, #1			; increase count
			mov		r1, r3			; update dividend
			bne		lp1				; iterate if quotient != 0
	; initializing location
			str		r3, [r0]
	; storing ascii code
			ldr		r1, =ascii
lp2			pop		{r4}			; load remainder (decimal digit)	
			subs	r5, #1			; decrease count
			ldrb	r4, [r1, r4]	; convert to ascii code
			strb	r4, [r0], #1	; store ascii code	
			bne		lp2
	; return
return		pop		{r0-r5,pc}
;-----------------------------------------
			 endp
			align
ascii		dcb		0x30	;'0'
			dcb		0x31	;'1'
			dcb		0x32	;'2'
			dcb		0x33	;'3'
			dcb		0x34	;'4'
			dcb		0x35	;'5'
			dcb		0x36	;'6'
			dcb		0x37	;'7'
			dcb		0x38	;'8'
			dcb		0x39	;'9'
			dcb		0x41	;'A'
			dcb		0x42	;'B'
			dcb		0x43	;'C'
			dcb		0x44	;'D'
			dcb		0x45	;'E'
			dcb		0x46	;'F'
			end