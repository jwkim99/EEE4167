#include <stm32f10x.h>

u16 ADCConverted;

int main (void) {
		RCC->APB2ENR |= 0x00000005;
		GPIOA->CRL = 0x44404444;

		RCC->APB2ENR |= 0x00000200;
		ADC1->CR1 = 0x00000000;
		ADC1->CR2 = 0x001E0002;
		ADC1->SMPR2 = 0x00007000;
		ADC1->SQR1 = 0x00000000;
		ADC1->SQR2 = 0x00000000;
		ADC1->SQR3 = 0x00000004;

		ADC1->CR2 |= 0x00000001;
		ADC1->CR2 |= (1 << 22);

		while (1) {
				while (!(ADC1->SR & 0x02)) ;
				ADCConverted = ADC1->DR;
		} 
} 
