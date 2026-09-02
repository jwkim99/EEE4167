#include <stm32f10x.h>

int main (void) {

		RCC->APB2ENR = 0x00000005;
		GPIOA->CRL = 0x00300008;
		GPIOA->ODR = 0x01;

		RCC->APB1ENR |= 0x00000001;
		TIM2->CR1 = 0x04;
		TIM2->CR2 = 0x00;
		TIM2->PSC = 0x0;
		TIM2->ARR = 10;
		TIM2->DIER = 0x0001;

		TIM2->SMCR = 0x4077;

		NVIC->ISER[0] = (1 << 28);
		TIM2->CR1 |= 0x0001;

		while (1) {}

} // end main

void TIM2_IRQHandler (void){
		if ((TIM2->SR & 0x0001) != 0){
				if (GPIOA->IDR & 0x20) GPIOA->BSRR = 1 << 21;
				else GPIOA->BSRR = 1 << 5;
				TIM2->SR &= ~(1 << 0); // clear UIF
		}
}