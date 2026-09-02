STACK_BASE	EQU 0x20000100
			area	lab6_ex, code
			entry
__main		 proc
			export	__main [weak]
start		ldr		sp, =STACK_BASE
			ldr		r1, =-0x2345
			ldr		r2, =-0x1230
			;-----------------;
			push	{r1,r2}
			bl		diff
			pop		{r0}
			add		sp, #4
			;-----------------;
stop		b		stop
			endp
			;-----------------;
diff		 proc
			push	{r3,r4,lr}
			ldr		r3, [sp,#12]
			ldr		r4, [sp,#16]
			cmp		r3, r4
			ittee	GE
			strge	r3, [sp,#-4]! 
			strge	r4,	[sp,#-4]!
			strlt	r4, [sp,#-4]!
			strlt	r3, [sp,#-4]!
			bl		subtract
			pop 	{r4}
			add		sp, #4
			str		r4, [sp,#12]
			pop		{r3,r4,pc}
			endp
subtract	 proc
			push 	{r5,r6,lr}
			ldr		r5, [sp,#12]
			ldr		r6, [sp,#16]
			sub		r6, r6, r5;
			str		r6, [sp,#12]
			pop		{r5,r6,pc}
			endp	
			;-----------------;
			end