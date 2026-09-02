// L_SENSOR (PA0)
// R_SENSOR (PA1)
// EMER			(PA2)

#include <stm32f10x.h>

/*----------------Global Variables----------------*/
static u8	l_dir = 0;	// 1 - Opening 0 - Closing
static u8	r_dir = 0;	// 1 - Opening 0 - Closing
static u8 emergency = 0;	// 0 - Normal 1 - Emergency situation
static u8 l_state = 0;	// 0 ~ 4 : 0 - fully closed, 4 - fully opened
static u8 r_state = 0;	// 0 ~ 4 : 0 - fully closed, 4 - fully opened

/*---------------Function Declaration-------------*/
int main(void){
// RCC setting
		RCC->APB2ENR = 0x0015; // IOP-C&A, AFIO Enable
		RCC->APB1ENR = 0x0007; // TIM2, TIM3, TIM4 Enable
	
// GPIO setting
		GPIOA->CRL = 0x00000888;	// PA[0:2]- Input (PA[0]->L_SENSOR, PA[1]->R_SENSOR, PA[2]->EMERG)
		GPIOA->ODR = 0x00000000;	// PA[0:2]- Pull down switch (On->High, Off->Low)
		GPIOC->CRL = 0x33333333;	// PC[0:7]- Output (push-pull)

// EXTI setting
		EXTI->IMR = 0x07;	// EXTI_0~2 enable
		EXTI->RTSR = 0x07; // EXTI_0~2 rising-edge detect
		EXTI->FTSR = 0x07; // EXTI_0~2 falling-edge detect

// TIM2 setting (10Hz - pulse for Door opening)
		TIM2->CR1 = 0x04;	// URS set, DIR = 0 (Upcounting)
		TIM2->CR2 = 0x00;	
		TIM2->PSC = 719;	// PSC + 1 = 720 (72MHz -> 100KHz)
		TIM2->ARR = 9999;	// ARR + 1 = 10000 (100KHz -> 10Hz)
		TIM2->CCMR1 = 0x1010; // CCR_1&2 : Output compare mode, Active on match
		TIM2->CCER = 0x0011;	// CCR_1&2 enabled
		TIM2->DIER = 0x06;	// CC1IE, CC2IE set -> interrupt enable
		TIM2->CCR1 = 0xFFFF;	// default - CH1 Compare OFF	
		TIM2->CCR2 = 0xFFFF;	// default - CH2 Compare OFF

// TIM3 setting (5Hz - pulse for Door closing)
		TIM3->CR1 = 0x04;	// URS set, DIR = 0 (Upconting)
		TIM3->CR2 = 0x00;	
		TIM3->PSC = 1439;	// PSC + 1 = 1440 (72MHz -> 50KHz)
		TIM3->ARR = 9999;	// ARR + 1 = 10000 (50KHz -> 5Hz)
		TIM3->CCMR1 = 0x1010; // CCR_1&2 : Output compare mode, Active on match
		TIM3->CCER = 0x0011;	// CCR_1&2 enabled
		TIM3->DIER = 0x06;	// CC1IE, CC2IE set -> interrupt enabled
		TIM3->CCR1 = 0xFFFF;	// default - CH1 Compare OFF		
		TIM3->CCR2 = 0xFFFF; 	// default - CH2 Compare OFF
		
// TIM4 setting (0.5Hz -> Door open for 2 seconds)
		TIM4->CR1 = 0x04;	// URS set, DIR = 0 (Upcounting)
		TIM4->CR2 = 0x00;	
		TIM4->PSC = 7199;	// PSC + 1 = 14400 (72MHz -> 5KHz)
		TIM4->ARR = 9999;	// ARR + 1 = 10000 (5KHz -> 0.5Hz)
		TIM4->CCMR1 = 0x1010; // CCR_1&2 : Output compare mode, Active level on match
		TIM4->CCER = 0x0011;	// CCR_1&2 enabled
		TIM4->DIER = 0x06;	// CC1IE, CC2IE set -> interrupt enabled
		TIM4->CCR1 = 0xFFFF;	// default - CH1 Compare OFF
		TIM4->CCR2 = 0xFFFF;	// default - CH2 Compare OFF

// Interrupt setting
		NVIC->ISER[0] = 0x700001C0;	// TIM4, TIM3, TIM2, EXTI2, EXTI1, EXTI0 Interrupt Enable
		NVIC->IP[6] = 0x80;					// EXTI_0 Priority Low
		NVIC->IP[7] = 0x80;					// EXTI_1 Priority Low
		NVIC->IP[8] = 0x80;					// EXTI_2 priority Low
		NVIC->IP[28] = 0x00;				// TIM2 Priority High
		NVIC->IP[29] = 0x00;				// TIM3 Priority High
		NVIC->IP[30] = 0x80;				// TIM4 Priority Low
		
// Module enable
		TIM2->CR1 |= 0x01;
		TIM3->CR1 |= 0x01;
		TIM4->CR1 |= 0x01;			// TIM Enable
		
		
		
/*----------------main function----------------*/
		while(1)
				__WFI();
}

/*---------------ISR declaration---------------*/
void EXTI0_IRQHandler(void){		// L_SENSOR
		static u16 tim;
		if (GPIOA->IDR & 0x01){			// Rising_edgel
				if (l_dir == 0) {			// if door was closing
						TIM3->CCR1 = 0xFFFF;	// Stop generating Close pulse
						TIM3->SR = 0;					// Clear TIM3 interrupt pending
						l_dir = 1;				// door open
						tim = TIM2->CNT;
						TIM2->CCR1 = (tim)? tim - 1 : 9999;	// Start generating Open pulse
				}
				else									// if door was opening
						TIM4->CCR1 = 0xFFFF; 	// Reset counting 2 seconds
		}
		else {											// Falling_edge
				if (l_state == 4){			// if door was fully opened
						tim = TIM4->CNT;
						TIM4->CCR1 = (tim)? tim - 1: 9999;	// Start counting 2 seconds
				}
		}
		EXTI->PR |= 0x01;						// Pending clear
}

void EXTI1_IRQHandler(void){		// R_SENSOR
		static u16 tim;
		if (GPIOA->IDR & 0x02){			// Rising_edgel
				if (r_dir == 0) {			// if door was closing
						TIM3->CCR2 = 0xFFFF;	// Stop generating Close pulse
						TIM3->SR = 0;					// Clear TIM3 interrupt pending
						r_dir = 1;				// door open
						tim = TIM2->CNT;
						TIM2->CCR2 = (tim)? tim - 1 : 9999;	// Start generating Open pulse
				}
				else									// if door was opening
						TIM4->CCR2 = 0xFFFF; 	// Reset counting 2 seconds
		}
		else {											// Falling_edge
				if (r_state == 4){			// if door was fully opened
						tim = TIM4->CNT;
						TIM4->CCR2 = (tim)? tim - 1: 9999;	// Start counting 2 seconds
				}
		}
		EXTI->PR |= 0x02;
}

void EXTI2_IRQHandler(void){		// EMER
		static u16 tim;
		if (GPIOA->IDR & 0x04){			// Rising-edge
				emergency = 1;					// turn on emergency state
				EXTI->IMR &= ~(0x03u);		// disable L_SENSOR & R_SENSOR
				EXTI->PR &= ~(0x03u);			// clear L_SENSOR & R_SENSOR interrupt pending
				if (l_dir == 0){				//  if left door was closing
						TIM3->CCR1 = 0xFFFF;	// Stop generating Close pulse
						TIM3->SR = 0;					// Clear TIM3 interrupt pending
						l_dir = 1;				// door open
						tim = TIM2->CNT;
						TIM2->CCR1 = (tim)? tim - 1 : 9999;	// Start generating Open pulse
				}
				else										// if left door was opening
						TIM4->CCR1 = 0xFFFF;	// stop counting 2 seconds
				
				if (r_dir == 0){				//  if right door was closing
						TIM3->CCR2 = 0xFFFF;	// Stop generating Close pulse
						TIM3->SR = 0;					// Clear TIM3 interrupt pending
						r_dir = 1;				// door open
						tim = TIM2->CNT;
						TIM2->CCR2 = (tim)? tim - 1 : 9999;	// Start generating Open pulse
				}
				else										// if right door was opening
						TIM4->CCR2 = 0xFFFF; // Stop counting 2 seconds

		}
		else{													// Falling-edge
				while((l_state != 4) || (r_state != 4))	{__WFI();} 	// wait until door is fully opened
				l_dir = 0;
				r_dir = 0;
				tim = TIM3->CNT; 
				TIM3->CCR1 = (tim)? tim - 1 : 9999;	
				TIM3->CCR2 = (tim)? tim - 1 : 9999;	// start generating Close pulse
				while((l_state != 0) || (r_state != 0)) {__WFI();}  // wait until door was fully closed
				emergency = 0;
				EXTI->IMR |= 0x03;		// enable L_SENSOR & R_SENSOR
				if (GPIOA->IDR & 0x01)	// if L_SENSOR on
						EXTI->SWIER |= 0x01;	// L_Door open
				if (GPIOA->IDR & 0x02)	// if R_SENSOR on
						EXTI->SWIER |= 0x02;	// R_Door open
		}
		EXTI->PR |= 0x04;
}

void TIM2_IRQHandler(void){	// Open pulse
		static u16 tim;
		if (TIM2->SR & 0x02) {	// CH1 - Left door
				l_state++;
				if (l_state >= 4){	// if door is fully opened
						TIM2->CCR1 = 0xFFFF; // Stop generating Open pulse
						GPIOC->BSRR = (1<<0);
						if (emergency + (GPIOA->IDR & 0x01) == 0){	// if Normal state & L_SENSOR off
								tim = TIM4->CNT;
								TIM4->CCR1 = (tim)? tim - 1 : 9999;	// Start counting 2 seconds
						}
				}
				else{
						GPIOC->BSRR = 1 << (4 - l_state);
				}
		}
		if (TIM2->SR & 0x04) { // CH2 - Right door
				r_state++;
				if (r_state >= 4){	// if door is fully opened
						TIM2->CCR2 = 0xFFFF; // Stop generating Open pulse
						GPIOC->BSRR = (1<<7);
						if (emergency + (GPIOA->IDR & 0x02) == 0){	// if Normal state & R_SENSOR off
								tim = TIM4->CNT;
								TIM4->CCR2 = (tim)? tim - 1 : 9999;	// Start counting 2 seconds
						}
				}
				else{
						GPIOC->BSRR = 1 << (r_state + 3);
				}
		}
		TIM2->SR = 0;
}

void TIM3_IRQHandler(void){	// Close pulse
		if (TIM3->SR & 0x02) {	// CH1 - Left door
				if (l_state){
						GPIOC->BRR = 1 << (4 - l_state);
						l_state--;
						if (l_state == 0)
								TIM3->CCR1 = 0xFFFF;	
				}
				else{											
						GPIOC->BRR = 0x0F;	
						TIM3->CCR1 = 0xFFFF;	// Stop generating Close pulse
				}
		}
		if (TIM3->SR & 0x04) {	// CH2 - Right door
				if (r_state){
						GPIOC->BRR = 1 << (r_state + 3);
						r_state--;
						if (r_state == 0)
								TIM3->CCR2 = 0xFFFF;	
				}
				else{											
						GPIOC->BRR = 0x0F;	
						TIM3->CCR2 = 0xFFFF;	// Stop generating Close pulse
				}
		}
		TIM3->SR = 0;
}

void TIM4_IRQHandler(void){	// 2 seconds
		static u16 tim;
		if (TIM4->SR & 0x02) {	// CH1
				TIM4->CCR1 = 0xFFFF;	// Stop counting 2 seconds
				l_dir = 0;
				tim = TIM3->CNT;
				TIM3->CCR1 = (tim)? tim - 1: 9999;	// start generating Close pulse
		}
		if (TIM4->SR & 0x04) {	// CH2
				TIM4->CCR2 = 0xFFFF;	// Stop counting 2 seconds
				r_dir = 0;
				tim = TIM3->CNT;
				TIM3->CCR2 = (tim)? tim - 1: 9999;	// start generating Close pulse
		}
		TIM4->SR = 0;
}