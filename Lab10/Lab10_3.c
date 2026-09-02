#include <stm32f10x.h>

int txcnt = 0, rxcnt = 0;
char rxstring[10], txstring[10] = "20181501\n";

int main (void) {
		RCC->APB2ENR = 0x00004015;
		GPIOA->CRH &= ~(0xFFu << 4);
		GPIOA->CRH |= (0x04B << 4);
		GPIOC->CRH = 0x00800000;

		USART1->BRR = 0x0EA6;
		USART1->CR1 = 0x00000000;
		USART1->CR2 = 0x00000000;
		USART1->CR3 = 0x00000000;
		USART1->CR1 |= 0x0000000C;
		USART1->CR1 |= 0x00002000;

		NVIC->ISER[1] = (1 << 5);

		EXTI->FTSR = 0x2000;
		EXTI->IMR = 0x2000;
		AFIO->EXTICR[3]=0x20;
		NVIC->ISER[1] = (1<<8);

		USART1->CR1 |= 0x00000020;

		while (1) {}

} // end main

void USART1_IRQHandler (void){
		if ((USART1->SR & 0x80)&(USART1->CR1 & 0x80)) {
				if (txstring[txcnt] == '\n')
						USART1->CR1 &= ~0x80; //disable Tx interrupt
				else
						USART1->DR = txstring[txcnt++];
		}
		if (USART1->SR & 0x20) {
				rxstring[rxcnt++] = USART1->DR;
		}
}

void EXTI15_10_IRQHandler(void){
		USART1->CR1 |= 0x00000080;
		EXTI->PR = 0x2000;
}
