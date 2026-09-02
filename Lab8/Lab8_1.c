#include <stm32f10x.h>

static char sw;

int main (void) {

RCC->APB2ENR = 0x00000015;
GPIOC->CRH = 0x00800000;
GPIOA->CRL = 0x00300000;

EXTI->FTSR = 0x2000;
EXTI->IMR = 0x2000;
AFIO->EXTICR[3]=0x20;
NVIC->ISER[1] = (1<<8);

GPIOA->ODR = 0x0000;
sw=0;

while (1) { __WFI(); }

} // end main

void EXTI15_10_IRQHandler(void){
	if (EXTI->PR == 0x2000){
		if (sw) {
			GPIOA->ODR = 0x0;
			sw = 0;
		}
		else {
			GPIOA->ODR=0x20;
			sw = 1;
		}
		EXTI->PR = 0x2000;
	}
}
