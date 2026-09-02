#include <stm32f10x.h>

#define USART1_DR_BASE 0x40013804
u8 txdata[]="DMA1/USART2 interface\r\n";

int main (void) {
		RCC->APB2ENR |= 0x00004004;
		GPIOA->CRH &= ~(0xFFu << 4);
		GPIOA->CRH |= (0x04B << 4);

		USART1->BRR = 0x0EA6;
		USART1->CR1 = 0x00000000;
		USART1->CR2 = 0x00000000;
		USART1->CR3 = 0x00000000;
		USART1->CR1 |= 0x00000008;
		USART1->CR3 |= 0x00000080;

		RCC->AHBENR |= 0x00000001;
		DMA1_Channel4->CCR = 0x00000090;
		DMA1_Channel4->CNDTR = sizeof(txdata)/sizeof(*(txdata))-1;
		DMA1_Channel4->CPAR = USART1_DR_BASE;
		DMA1_Channel4->CMAR = (u32)txdata;

		DMA1_Channel4->CCR |= 0x00000001;

		USART1->CR1 |= 0x00002000;

		while(!(DMA1->ISR & 0x00002000)){;}

		while (1) {;}
} // end main