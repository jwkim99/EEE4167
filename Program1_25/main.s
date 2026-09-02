			area	midcode, code
			extern	timeofday
			export	__main	[weak]
			entry
__main		proc
			ldr		sp, =0x20000080
			ldr		r0, =hour
			ldr		r1, =min
			ldr		r2, =sec
			ldr		r3, =count
			push	{r0-r3}
			bl		timeofday
			add		sp,#0x10
;
here		b		here
			endp
count		dcd		83121
			area	middata, data
hour		dcw		0
min			dcw		0
sec			dcw		0
			end