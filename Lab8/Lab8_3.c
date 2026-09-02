#include <stm32f10x.h>

u32 i, key_index, key_row, key_col, col_scan;

int main(void) {

	RCC->APB2ENR = 0x0000001D;

	GPIOC->CRH = 0x00003333;
	GPIOA->CRH = 0x00008888;
	GPIOA->ODR = 0x0000;			// changed

	GPIOC->CRL = 0x33333333;
	GPIOB->CRH = 0x33333333;

	GPIOC->BSRR = 0x0FE;

	key_index = 0;
	key_row = 0x01;

	while(1) {
		GPIOC->BSRR = (key_row << 8) | (~(key_row << 24) & 0x0F000000);	// changed
		for ( i = 0; i < 1000; i++ ){ ; }
		key_col = GPIOA->IDR;
		key_col = (key_col >> 8) & 0x0F;
		col_scan = 0x01;
		for ( i = 0; i < 4; i++) {
			if (( key_col & col_scan ) >= 1)	// changed
				GPIOB->ODR = key_index << 8;
			col_scan = col_scan << 1;
			key_index = key_index + 1;
		}
		key_row = key_row << 1;
		if ( key_row == 0x10 ) {
			key_row = 0x01;
			key_index = 0;
		}
	}
	
}
