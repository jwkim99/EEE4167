#include <stm32f10x.h>

int i = 0;
char string[10];

int main (void) {
		RCC->APB2ENR = 0x00004004;
		GPIOA->CRH &= ~(0xFFu << 4);
		GPIOA->CRH |= (0x04B << 4);

		USART1->BRR = 0x0EA6;
		USART1->CR1 = 0x00000000;
		USART1->CR2 = 0x00000000;
		USART1->CR3 = 0x00000000;
		USART1->CR1 |= 0x00000004;
		USART1->CR1 |= 0x00002000;

		NVIC->ISER[1] |= (1 << 5);
		USART1->CR1 |= 0x00000020;

		while (1) {}

} // end main

void USART1_IRQHandler (void){
		if (USART1->SR & 0x20)
				string[i++] = USART1->DR;
}