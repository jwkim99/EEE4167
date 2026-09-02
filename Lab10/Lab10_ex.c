#include <stm32f10x.h>

int i = 0, j = 0;
char txstring[10] = "20181501\n";
char rxstring[10];

int main (void) {
			// RCC setting
		RCC->APB2ENR = 0x00004004;	// USART1 GPIOA EN
		RCC->APB1ENR = 0x00020000;	// USART2 EN
	
			// USART1 Tx & Rx setting
		GPIOA->CRH &= ~(0xFFu << 4);	// PA9~10 configuratoin reset
		GPIOA->CRH |= (0x04B << 4);	// PA10: input floating, PA9: AF open-drain output(2MHz)
			
			// USART2 Tx & Rx setting
		GPIOA->CRL &= ~(0xFFu << 8); // PA2~3 configuration reset
		GPIOA->CRL |= (0x04B << 8); // PA3: input floating, PA2: AF open-drain output(2MHz)
			
			// H.W. connection between PA3 & PA9
	
			// USART1 configuration
		USART1->BRR = 0x0EA6;	// 72MHz/(16*234.375) = 19.2 KHz
		USART1->CR1 = 0x00000000;
		USART1->CR2 = 0x00000000;
		USART1->CR3 = 0x00000000;
		USART1->CR1 |= 0x00000004;	// USART1_RE = 1: Receiver

			// USART2 configuration
		USART2->BRR = 0x0753;	// 36MHz/(16*117.1875) = 19.2 KHz
		USART2->CR1 = 0x00000000;
		USART2->CR2 = 0x00000000;
		USART2->CR3 = 0x00000000;
		USART2->CR1 |= 0x00000008;	// USART2_TE = 1: Transmitter
			
			// USART Enable
		USART1->CR1 |= 0x00002000;	// USART1_UE = 1
		USART2->CR1 |= 0x00002000;	// USART2_UE = 1
		
			// Interrupt setting
		NVIC->ISER[1] |= (3 << 5);	// Interrupt #53(USART1)& #54(USART2) Enable
		USART1->CR1 |= 0x00000020;	// USART1_RXEIE = 1: RXNE interrupt enable
		USART2->CR1 |= 0x00000080;	// USART2_TXEIE = 1: TXE interrupt enable	
		
		while (1) {}

} // end main

void USART2_IRQHandler (void){
		if (USART2->SR & 0x80) {	// If TXE == 1: Ready to Send
				if (txstring[i] == '\n')	// End of String
						USART2->CR1 &= ~0x80; //disable Tx interrupt
				else
						USART2->DR = txstring[i++];	// Send (i)th character
		}
}

void USART1_IRQHandler (void){
		if (USART1->SR & 0x20)	// If RXNE == 1: Ready to receive
				rxstring[j++] = USART1->DR;	// Store (j)th character
}