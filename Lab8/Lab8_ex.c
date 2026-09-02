#include <stm32f10x.h>

u32 i, key_scan, key_col, mat_scan, mat_col, mat_row, col_scan;

int main(void) {

	RCC->APB2ENR = 0x0000001D;// GPIOA-C & AFIO ENA

	GPIOC->CRH = 0x00003333; 	// KEY MATRIX ROW : OUTPUT
	GPIOA->CRH = 0x00008888; 	// KEY MATRIX COL : INPUT
	GPIOA->ODR = 0x0F00;		 	// KEY MATRIX COL : PULL-UP mode (Active Low)

	GPIOC->CRL = 0x33333333;	// DOT MATRIX ROW : OUTPUT
	GPIOB->CRH = 0x33333333;	// DOT MATRIX COL : OUTPUT

	key_scan = 0x01;						// Setting initial value : KEY MATRIX ROW[0]
	mat_scan = 0x01;						// Setting initial value : DOT MATRIX ROW[0]
	
	mat_col = 0x0;
	mat_row = 0x01;

	while(1) {
		GPIOC->BSRR = (~(key_scan << 8) & 0x0F00) | (key_scan << 24);
		GPIOC->BSRR = ((~mat_scan) & 0x0FF) | (mat_scan << 16);
		key_col = GPIOA->IDR & 0xF00;
		col_scan = 0x0100;
		for ( i = 0; i < 4; i++) {
			if (( key_col & col_scan ) == 0){
				mat_col = 0x08 / key_scan;
				mat_row = col_scan >> 8;
			}
			col_scan = col_scan << 1;
		}
		if ( mat_scan == mat_row )
			GPIOB->ODR = mat_col << 8;
		else
			GPIOB->ODR = 0;
		for ( i = 0; i < 10000; i++ ){ ; }
		key_scan = key_scan << 1;
		mat_scan = mat_scan << 1;
		if ( key_scan == 0x10 )
			key_scan = 0x01;
		if ( mat_scan == 0x10 )
			mat_scan = 0x01;
	}
}
