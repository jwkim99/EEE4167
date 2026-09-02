#include <stm32f10x.h>

u32 i, row, col;

int main(void) {

	RCC->APB2ENR = 0x0000001D;
	GPIOC->CRL = 0x33333333;
	GPIOB->CRH = 0x33333333;

	row = 0;
	col = 0x0000;
	
	 u32 pattern[8] = {0b11111111,
										 0b01111111,
										 0b00111111,
										 0b00011111,
										 0b00001111,
										 0b00000111,
										 0b00000011,
										 0b00000001};

	while(1) {
		col = pattern[row]<<8;
		GPIOC->ODR = ~(1<<row);
		GPIOB->ODR = col;	

		for(i=0; i<5000; i++){;}

		row += 1;
		if (row == 8) row = 0;
	}
}
