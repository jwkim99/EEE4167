// 1. Door Direction Convert
// 2. lasting S1 
// 3. Debouncing


#include <stm32f10x.h>

/* Prototyping */
typedef enum{
	FULL_CLOSED = 0, 
	OPENING, 
	FULL_OPENED, 
	CLOSING
}Type_State;

typedef enum{
	NORMAL = 0,
	INSPECT = 1,
	LOCKED = 2
}Type_Mode;

/* Global Variable */
volatile Type_State state = FULL_CLOSED;
volatile Type_Mode	mode = NORMAL;
volatile char door;
volatile char wait, row, blink;

/* Function Declaration */
int main(void){
/* Peripheral Configuration */
	// Peripheral Enable
	RCC->APB1ENR = 0x00000001; // TIM2EN
	RCC->APB2ENR = 0X0000081D; // TIM1EN, IOPC, IOPB, IOPA, AFIO EN
	GPIOA->CRL = 0x88000000;	// PA7,PA6: Input
	GPIOA->CRH = 0X00000008;	// PA8: Input
	GPIOA->ODR = 0x000001C0;	// PA8,PA7,PA6: Pull-up Input (Default: High, Push: Low)
	GPIOC->CRL = 0x33333333;	// PC0-7: Output (Dot matrix row)
	GPIOB->CRH = 0x33333333;	// PB8-15: Output (Dot matrix col)
	GPIOB->ODR = 0x00000000;	// Initial Door Condition: Full-closed
	EXTI->FTSR = 0x00C0;	// EXTI7, EXTI6: Falling Edge Trigger
	EXTI->IMR = 0x00C0;		// EXTI7, EXTI6 Enable
	TIM1->CR2 = 0x00;
	TIM1->PSC = 7199;	// 72MHz -> 10KHz
	TIM1->ARR = 999;	// 10KHz -> 10Hz
	TIM1->DIER= 0x0041;	// UIE, TIE : High (Update Interrupt, Trigger Interrupt Enable)
	TIM1->SMCR= 0X8054;	// ETP, MSM: High, Edge Detector, Reset Mode
	TIM2->CR2 = 0x00;
	TIM2->PSC = 5999; // 72MHz -> 12KHz
	TIM2->ARR = 99;	// 12KHz -> 120Hz 
	TIM2->DIER= 0x0001; // TIE: High (Update Interrupt)
/* Interrupt Configuration */
	NVIC->ISER[0] = 0x16800000;	// TIM2,TIM1_UP,TIM1_TRG,EXTI9_5 Interrupt

/* Main Behavior */
	TIM1->CR1 |= 0x01;			// TIM1 CEN: High
	TIM2->CR1 |= 0x01;		// TIM2 CEN: High
	while(1)
		__WFI();
}


void EXTI9_5_IRQHandler(void) {
	if(EXTI->PR == 0x0040){	// PA6(S3)
		mode = (mode==INSPECT)? NORMAL:INSPECT;
		if(state == FULL_CLOSED)
			state = OPENING;
		else if(state == CLOSING)
			state = OPENING;
		else
			wait = 0;
		EXTI->PR = 0x0040;
	}
	else if(EXTI->PR == 0x0080){ // PA7(S2)
		if(mode != INSPECT){
			mode = (mode)? NORMAL:LOCKED;
			if(state == OPENING)
				state = CLOSING;
			else if(state == FULL_OPENED){
				wait = 0;
				state = CLOSING;
			}
		}
		EXTI->PR = 0x0080;
	}
}

void TIM1_UP_IRQHandler(void) {	// 0.1Hz Timer
	if(TIM1->SR & 0x0001){	// UIF
		if(mode != NORMAL)
			blink = (blink)? 0:1;
		else
			blink = 0;
		if(state == OPENING){
			if(door < 0xFF)
				door = door*2 + 1;
			else if(door == 0xFF)
				state = FULL_OPENED;
		}
		else if(state == FULL_OPENED){
			if(mode != INSPECT){
				wait++;
				if(wait >= 9){
					state = CLOSING;
					wait = 0;
				}
			}
		}
		else if(state == CLOSING){
			if(wait){
				door /= 2;
				if(door == 0){
					wait = 0;
					state = FULL_CLOSED;
				}
				wait = 0;
			}
			else
				wait = 1;
		}
		TIM1->SR &= ~0x0001;
	}
}

void TIM1_TRG_COM_IRQHandler(void){ // 0.1Hz Timer
	if(TIM1->SR & 0x0040){	// TIF (S1)
		if(mode == NORMAL){
			if(state == FULL_CLOSED){
				TIM1->CR1 |= 0x0001;	// CEN: High
				state = OPENING;
			}
			else if(state == CLOSING) {
				state = OPENING;
			}
			TIM1->SR &= ~0x0040;
		}
	}
}

void TIM2_IRQHandler(void) {	// Dot Matrix Scanning
	if(TIM2->SR & 0x0001){	// UIF
		if(row){
			GPIOC->ODR = ~(1<<row);
			GPIOB->ODR = (3-mode*blink) << 14;
			row = 0;
		}
		else{
			GPIOC->ODR = ~(1<<row);
			GPIOB->ODR = door << 8;
			row = 7;
		}
		TIM2->SR &= ~0x0001;
	}
}
