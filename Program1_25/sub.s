			area	midcode_sub, code
			export	timeofday
			entry
timeofday	proc
;-------------- insert your code between the lines --------------
		; saving context
			push 	{r0-r5,lr}
		; load parameter
			ldr		r1, [sp, #0x28] ; address of count
		; calculating hh, mm, ss
			mov		r4, #60			; saving divisor
			ldr		r1, [r1]		; load count number
			udiv	r2, r1, r4		; r2: min (min + hour*60)	; min = count // 60
		; get integer ss
			mls		r1, r2, r4, r1	; r1: sec					; sec = count % 60
		; calling dec2ascii
			mov		r0, r1
			bl		dec2ascii
			ldr		r5, [sp,#0x24]	; address of sec
			strh	r0, [r5]		; save ascii ss to sec
		; get integer hh
			udiv	r3, r2, r4		; r3: hour					; hour = min // 60
		; calling dec2ascii
			mov		r0, r3
			bl		dec2ascii
			ldr		r5, [sp,#0x1C]	; address of hour
			strh	r0, [r5]		; save ascii hh to hour
		; get integer mm
			mls		r2, r3, r4, r2	; r4: min (min % 60)		; min = min % 60
		; calling dec2ascii
			mov 	r0, r2
			bl		dec2ascii
			ldr		r5, [sp,#0x20]	; address of min
			strh	r0, [r5]		; save ascii mm to min	
		; return to main.s
			pop		{r0-r5,pc}
			endp
				
dec2ascii	proc
			push	{r4-r6}			; parameter passing by r0
			mov		r4, #10			; saving divisor
			udiv	r5, r0, r4		; r5: integer tens
			mls		r6, r5, r4, r0	; r6: integer ones
			add		r5, #0x30		; r5: ascii tens
			add		r6, #0x30		; r6: ascii ones
			orr		r0, r6, r5, LSL #8	; r0 <= (r5 << 8) | r6
			pop		{r4-r6}
			bx		lr
			endp
;----------------------------------------------------------------
			endp
			end