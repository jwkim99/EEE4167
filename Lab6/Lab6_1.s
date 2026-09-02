STACK_BASE 	EQU 0x20000100
			area	lab6_1, code
			entry
__main		 proc
			export	__main [weak]
start		ldr		sp,=STACK_BASE
			ldr		r1, =0x1234
			ldr		r2, =0x4321
			bl		swap
; doing something here using the result
			ldr		r1, =-0x4321
			ldr		r2, =-0x1234
			bl		swap
; doing something here using the result
stop		b		stop
			 endp
swap		 proc
			stmea	sp!, {r3, lr}
			cmp		r1, r2
			bgt		r2m
			mov		r3, r2
			mov		r2, r1
			mov		r1, r3
r2m			ldmea	sp!, {r3, pc}
			 endp
			 end