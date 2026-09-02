			area 	projcode, code
			entry
__main		proc
			export	__main [weak]
;---------------insert your code between the lines-----------------
			extern	itoa
			extern	atoi
	; itoa subroutine			
start		ldrh	r1, num1		; load num1
			ldrh	r2, num2		; load num2
			ldr		r0, = sum		; address of sum
			add		r3, r1, r2		; adding num1 & num2
			str		r3, [r0]		; storing sum
			ldr		r0,	= sumascii	; store address
			mov		r4, #16			; base select
			push	{r0,r3,r4}		; passing parameter
			bl		itoa			; call subroutine
			add		sp, #12			; SP initialization	
	; atoi subroutine
			mov		r4, #10			; base select
			push 	{r0,r4}
			bl		atoi
			pop		{r5}
			add		sp, #4
;------------------------------------------------------------------
here		b		here
			 endp
			align
num1		dcw		23
num2		dcw		100
			align	
			area	projdata, data
sum			dcd		0
sumascii	space	4
			end
				