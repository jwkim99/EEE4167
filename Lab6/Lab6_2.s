STACK_BASE	EQU		0x20000100
			area	lab6_2,code
			entry
__main		 proc
			export	__main [weak]
start		ldr		sp, =STACK_BASE	
			ldr		r1, =0x1234
			ldr		r2, =0x4321
			stmfd	sp!, {r2,r1}
			bl		swap
			ldmfd	sp!, {r1, r2}
; doing something here using the result
			ldr		r1, =-0x1234
			ldr		r2, =-0x4321
			stmfd	sp!, {r1, r2}
			bl		swap
			ldmfd	sp!, {r1, r2}
; doing something here using the result
stop		b		stop
			 endp
swap		 proc
			stmfd	sp!, {r4-r6,lr}
			ldr		r4, [sp, #0x10]
			ldr		r5, [sp, #0x14]
			cmp		r4, r5
			bgt		r2m
			mov		r6, r5
			mov		r5, r4
			mov		r4, r6
			str		r4, [sp,#0x10]
			str		r5, [sp,#0x14]
r2m			ldmfd	sp!, {r4-r6,pc}
			 endp
			 end
			