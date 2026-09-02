			area	lab6_4, code
			export	fill_array
			export	fill_pointer
			entry
;
;		fill_array(int array[], int size, int pattern)
;		{		int i;
;				for (i = 0; i < size; i += 1)
;						array[i] = pattern;
;		}
fill_array	 proc
			push	{r0-r3, lr}
			ldr		r0, [sp, #0x14]	;ARRAY_BASE
			ldr		r1, [sp, #0x18]	;ARRAY_SIZE
			ldr		r2, [sp, #0x1c]	;FILL_PATTERN
			mov		r3, #0
lp1			str		r2, [r0, r3, LSL #2]
			add		r3, #1
			cmp		r3, r1
			blt		lp1
			pop		{r0-r3,pc}
			 endp
;
;		fill_pointer(int *array, int size, int pattern)
;		{		int *p;
;				for (p = &array[0]; p < &array[size]; p = p + 1)
;						*p = pattern;
;		}
fill_pointer proc
			push	{r0-r3,lr}
			ldr		r0, [sp,#0x14]	;ARRAY_BASE
			ldr		r1, [sp,#0x18]	;ARRAY_SIZE
			ldr		r2, [sp,#0x1c]	;ARRAY_SIZE
			add		r3, r0, r1, LSL #2
lp2			str		r2, [r0], #4
			cmp		r0, r3
			blt		lp2
			pop		{r0-r3,pc}
			 endp
			 end