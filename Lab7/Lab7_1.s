RCC_APB2ENR 	equ 	0x40021018
GPIOD_CRL 		equ 	0x40011400
GPIOD_ODR 		equ 	0x4001140C
EXTI_PR 		equ 	0x40010414
EXTI_IMR 		equ 	0x40010400
EXTI_RTSR 		equ 	0x40010408
EXTI_FTSR 		equ 	0x4001040C
AFIO_EXTICR4 	equ 	0x40010014
NVIC_ISER0 		equ 	0xE000E100
NVIC_ISER1 		equ 	0xE000E104
				export 	EXTI15_10_IRQHandler
				export	EXTI0_IRQHandler
				area 	lab7_1, code
				entry
__main 		 	 proc
				export 	__main [weak]
; GPIO initialization
				ldr 	r0,=RCC_APB2ENR
				ldr 	r1,=0x00000021 ;IOPDEN, AFIOEN
				str 	r1,[r0]
				ldr		r0,=0x42420300
				ldr		r1,=0x0
				str		r1, [r0]
				ldr 	r0,=GPIOD_CRL
				ldr		r1,=0x33333333
				str		r1,[r0]
; AFIO
				ldr 	r0,=AFIO_EXTICR4
				ldr 	r1,=0x00000020
				str 	r1,[r0]
				ldr		r0,=0x42200290
				ldr		r1,=0x1
				str		r1, [r0]
; EXTI initialization
				ldr 	r1,=0x00002000
				ldr 	r0,=EXTI_FTSR
				str 	r1,[r0]
				ldr		r1,=0x00000001
				ldr		r0,=EXTI_RTSR
				str		r1,[r0]
				ldr		r1,=0x00002001
				ldr 	r0,=EXTI_IMR
				str 	r1,[r0]
; NVIC_ISER
				ldr 	r0,=NVIC_ISER1
				ldr 	r1,=0x00000100
				str 	r1,[r0]
				ldr		r0,=NVIC_ISER0
				ldr		r1, =0x00000040
				str		r1, [r0]
				ldr 	r0,=GPIOD_ODR
				mov 	r5,#2_01001010
				ldr 	r3,=sdata
				str 	r5,[r3]
loop 			ldr 	r4,[r3]
				str 	r4,[r0]
				b 		loop
				 endp
;
EXTI15_10_IRQHandler proc
				push 	{r4,r5}
				ldr 	r4,=sdata
				ldr 	r5,[r4]
				add 	r5,#0x1
				str 	r5,[r4]
				ldr 	r4,=EXTI_PR
				ldr 	r5,=0x00002000
				str 	r5,[r4]
				pop 	{r4,r5}
				bx 		lr
				 endp
EXTI0_IRQHandler proc
				push 	{r4,r5}
				ldr 	r4,=sdata
				ldr 	r5,[r4]
				sub 	r5,#0x1
				str 	r5,[r4]
				ldr 	r4,=EXTI_PR
				ldr 	r5,=0x00000001
				str 	r5,[r4]
				pop 	{r4,r5}
				bx 		lr
				 endp
				align
				area 	lab7, data
sdata 			dcd 	0x00
				 end