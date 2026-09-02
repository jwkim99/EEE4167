			area	subcode, code
			entry
atoi		 proc
			export	atoi
;-----------------------------------------
			push 	{r0-r3,lr}
			ldr		r1, [sp, #20] ; ascii address
			ldr		r2, [sp, #24] ; base
			mov		r3, #0		  ; initial integer
lp			ldrb	r0, [r1], #1  ; load 1 byte
			cmp 	r0, #0x00	  ; if ascii code is NULL
			beq		ret
			sub		r0, #0x30	  ; ascii_code --> decimal
			mla		r3, r3, r2, r0
			b		lp
ret			str		r3, [sp, #20] ; store integer value	
			pop 	{r0-r3,pc}
;-----------------------------------------
			 endp
			end