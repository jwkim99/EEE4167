#include <stm32f10x.h>

/*-----------Prototyping-----------*/

typedef enum{					// Type_digit
	zero = 0,
	one,
	two,
	three, 
	four, 
	five, 
	six, 
	seven, 
	eight, 
	nine, 
	l, 
	x, 
	F, 
	C, 
	degree, 
	dot, 
	colon, 
	SP
}Type_digit;

typedef enum{					// Type_mode

	Time_Disp = 0, Temp_Disp, Illu_Disp, Time_Set
}Type_mode;

typedef struct{				// Struct_time;

	u8	MaxHour;	// 0: 12H, 1: 24H
	u8	HH;				// 0 ~ 23 
	u8	MM;				// 0 ~ 59
	u8	SS;				// 0 ~ 59
}Struct_time;

void Disp_Set(void);
/*-----------Global variables----------*/

u8 font5x4 [18][5] = {	// font data
	{7,5,5,5,7},	//0
	{6,2,2,2,7},	//1
	{7,1,7,4,7},	//2
	{7,1,7,1,7},	//3
	{5,5,7,1,1},	//4
	{7,4,7,1,7},	//5
	{7,4,7,5,7},	//6
	{7,1,1,1,1},	//7
	{7,5,7,5,7},	//8
	{7,5,7,1,7},	//9
	{1,1,1,1,1},	//l
	{0,0,5,2,5},	//x
	{7,4,7,4,4},	//F
	{7,4,4,4,7},	//C
	{3,3,0,0,0},	// degree
	{0,0,0,0,2},	// dot
	{0,2,0,2,0},	// colon
	{0,0,0,0,0} 	// SP
};
Type_mode mode = Time_Disp;
	// Display control signals
u32 display[5];					// Display pattern
Type_digit digits[10]; 	// Display contents
u32 brightness = 8000;	// Display brightness
u8	n_digit = 10;						// Total number of display contents
u8 	disp_idx;						// Currently displaying digit
u8	n_shift;						// Counts of shifting (0 ~ 3)
	
	// Status signal
u16	states;							// Dot matrix Row7: state representation 

	// Data storage
Struct_time	Base_Time = {1, 12, 0, 0};	// Current Time
u8 H[2], M[2], S[2], N[4];	// Variables to display TIME
Struct_time Mod_Time;		// Time to Modify
u32 Disp_TMP;						// Temperature to display
u32 Disp_ILL;						// Illumination to display
u16 ADC_TMP;						// ADC value of TMP36
u16	ADC_ILL = 8000; 		// ADC value of GL5537
	
	// Key matrix signal
u8	Key_state;
u8	Mod_cur;						// Cursor to modify
u8	Key_cnt;						// Time_Set -> Time_Disp if Key_cnt == N;

int main(void){
/*-----------Peripheral Setting-----------*/
	// RCC setting
		RCC->APB2ENR = 0x00000A1D;	// TIM1, ADC1, IOPA-C, AFIO clock enable
		RCC->APB1ENR = 0x00000003; 	// TIM3, TIM2 enable
	
	// AFIO setting
		AFIO->MAPR=0x02000000;   //REMAP PB3, PB4 as GPIO
	
	// GPIO setting
		GPIOA->CRL = 0x88880000;  // PA4-7:input (Key matrix Col)
		GPIOA->ODR = 0x00000;			// PA4-7:Pull-down (Key matrix Col)

		GPIOB->CRL = 0x33333333;	// PB0-7:output push-pull (Dot matrix Col)
		GPIOB->CRH = 0x33333333;	// PB8-15:output push-pull (Dot matrix Col)
	
		GPIOC->CRL = 0x33333333;	// PC0-7:output(Dot matrix row)
		GPIOC->CRH = 0x00003333;  // PC8-11: output push-pull (Key matrix Row)
		GPIOC->ODR = 0x01FF;			// Key_row : Active, Dot_row : inactive
		
	// ADC setting (CH0 - GL5537, CH1 - TMP36)
		ADC1->CR1 = 0x000005A0;   // JAUTO, SCAN, JEOCIE, EOCIE set
		ADC1->CR2 = 0x001EF004;		// EXTTRIG, JEXTTRIG, CAL set, CON reset, JEXTSEL : JSWSTART, EXTSEL : SWSTART
		ADC1->SMPR2 = 0x0000;   	// Sample time for channel 0 & 1: 239.5 cylces
		ADC1->SQR1 = 0x00000000;  // L = '0000' (1 conversion)
		ADC1->SQR2 = 0x00000000;	
		ADC1->SQR3 = 0x00000001; 	// Regular 1st conversion : channel 1 (temperature)
		ADC1->JSQR = 0x00000000; 	// injected 1st conversion : channel 0 (illumination), JL = '00' (1 conversion)

	// Timer_1 setting (UP - Scrolling, CH1 - Display scan, CH2 - PWM(brightness))
		TIM1->CR1 = 0x04;		// URS set
		TIM1->CR2 = 0x00;		
		TIM1->PSC = 124; 	// PSC + 1 = 125 division (72MHz -> 576KHz)
		TIM1->ARR = 599; 	// ARR + 1 = 600 division (576KHz -> 960Hz)	: Refresh rate = 120Hz
		TIM1->RCR = 119;		// RCR + 1 = 120 division (960Hz -> 8Hz) : Scrolling rate = 8Hz
		TIM1->DIER = 0x07;	// CC2IE, CC1IE, UIE set
		TIM1->CCMR1 = 0x1010;	// OC2M = '001' : Active on match, OC1M = '001' : Active on match, CC1S = '00' : Output Compare mode
		TIM1->CCER = 0x11;	// CC1E, CC2E set : Channel 1&2 enable
		TIM1->CCR1 = 0; 		// TIM1_CNT == 0 -> CC1 interrupt (Next Row ON)
		TIM1->CCR2 = 0x12C; // TIM1_CNT == 0x12C -> CC2 interrupt (Row off)
		
	// Timer_2 setting (UP - clock, CH1 - Return)
		TIM2->CR1 = 0x04;	// URS set
		TIM2->CR2 = 0x00;
		TIM2->PSC = 7199;	// PSC + 1 = 7200 division (72MHz -> 10KHz)
		TIM2->ARR = 9999;	// ARR + 1 = 10000 division (10KHz -> 1Hz)
		TIM2->DIER = 0x01;	// CC1IE reset, UIE set
		TIM2->CCMR1 = 0x0010; // OC1M = '001': Active on match, CC1S = '00' : Output Compare mode
		TIM2->CCER = 0x01; 	// CC1E set : Channel 1 enable
		TIM2->CCR1 = 0xFFFF;	// Changed in Time setting mode
		
	// Timer_3 setting (UP - Key scan)
		TIM3->CR1 = 0x04;
		TIM3->CR2 = 0x00;
		TIM3->PSC = 124;
		TIM3->ARR = 599;
		TIM3->DIER = 0x01;
				
	// USART setting
		
/*-----------Interrupt Setting-----------*/
	// NVIC register setting
		NVIC->ISER[0] = 0x3A040040;	// EXTI0, ADC1_2, TIM1_UP, TIM1_CC, TIM2, TIM3 enable
	// EXTI register setting
		EXTI->IMR = 0x01;
		EXTI->FTSR = 0x00;
		EXTI->RTSR = 0x00;
		
/*-----------Module Enabling-----------*/
	// ADC - ADON
		ADC1->CR2 |= 0x00000001;  // ADON set

	// TIM - CEN
		TIM1->CR1 |= 0x01;		// TIM1_CEN set
		TIM2->CR1 |= 0x01;		// TIM2_CEN set
		TIM3->CR1 |= 0x01;		// TIM3_CEN set
	
/*-----------Main function-----------*/
		states |= 0x0206;				// Clock undefined & P.M. & 24Hour
		
		while(1) {		
			while (mode == Time_Disp){
					H[1] = (states & 0x02) ?  Base_Time.HH : Base_Time.HH % 12;
					H[0] = H[1] % 10;
					H[1] = H[1] / 10;
					M[1] = Base_Time.MM / 10;
					M[0] = Base_Time.MM % 10;
					S[1] = Base_Time.SS / 10;
					S[0] = Base_Time.SS % 10;
				
					digits[0] = SP;
					digits[1] = SP;
					digits[2] = H[1];
					digits[3] = H[0];
					digits[4] = colon;
					digits[5] = M[1];
					digits[6] = M[0];
					digits[7] = colon;
					digits[8] = S[1];
					digits[9] = S[0];
								
					n_digit = 10;
				
					Disp_Set();
					__WFI();
			}
			while (mode == Temp_Disp){
					N[3] = Disp_TMP / 1000;
					N[2] = Disp_TMP / 100 - N[3]*10;
					N[1] = Disp_TMP /10 - N[3]*100 - N[2]*10;
					N[0] = Disp_TMP - N[3]*1000 - N[2]*100 - N[1]*10;
					
					digits[0] = SP;
					digits[1] = SP;
					digits[2] = (N[3])? N[3]: SP;
					digits[3] = N[2];
					digits[4] = N[1];
					digits[5] = dot;
					digits[6] = N[0];
					digits[7] = degree;
					digits[8] = (states & 0x400) ? F : C;
					n_digit = 9;
									
					Disp_Set();
					__WFI();
			}
			while (mode == Illu_Disp){
					N[3] = Disp_ILL / 1000;
					N[2] = Disp_ILL / 100 - N[3]*10;
					N[1] = Disp_ILL /10 - N[3]*100 - N[2]*10;
					N[0] = Disp_ILL - N[3]*1000 - N[2]*100 - N[1]*10;

					digits[0] = SP;
					digits[1] = SP;
					digits[2] = N[3];
					digits[3] = N[2];
					digits[4] = N[1];
					digits[5] = dot;
					digits[6] = N[0];
					digits[7] = l;
					digits[8] = x;				
					n_digit = 9;
					
					Disp_Set();
					__WFI();
			}
			while (mode == Time_Set) {
					n_digit = 4;
				
					H[1] = (states & 0x02) ?  Mod_Time.HH : Mod_Time.HH % 12;
					H[0] = H[1] % 10;
					H[1] = H[1] / 10;
					M[1] = Mod_Time.MM / 10;
					M[0] = Mod_Time.MM % 10;
					S[1] = Mod_Time.SS / 10;
					S[0] = Mod_Time.SS % 10;

					if (Mod_Time.HH / 12)
							states |= 0x04;
					else
							states &= ~(0x04);
												
					switch (Mod_cur) {
						case 0:		// Hour setting
							digits[0] = SP;
							digits[1] = (states & 0x01)? H[1]: SP;
							digits[2] = (states & 0x01)? H[0]: SP;
							digits[3] = colon;
							break;
						case 1:		// Minute setting
							digits[0] = colon;
							digits[1] = (states & 0x01)? M[1]: SP;
							digits[2] = (states & 0x01)? M[0]: SP;
							digits[3] = colon;
							break;
						case 2:		// Second setting
							digits[0] = colon;
							digits[1] = (states & 0x01)? S[1]: SP;
							digits[2] = (states & 0x01)? S[0]: SP;
							digits[3] = SP;							
							break;
					}
					
					Disp_Set();
					__WFI();
			}
			
		}
}

void Disp_Set(void) {
		for (int i = 0; i < 5; i++){
				int j = (disp_idx + i) % n_digit;
				display[0] = (display[0] << 4) + font5x4[digits[j]][0];
				display[1] = (display[1] << 4) + font5x4[digits[j]][1];
				display[2] = (display[2] << 4) + font5x4[digits[j]][2];
				display[3] = (display[3] << 4) + font5x4[digits[j]][3];
				display[4] = (display[4] << 4) + font5x4[digits[j]][4];						
		}
		display[0] = display[0] << n_shift;
		display[1] = display[1] << n_shift;
		display[2] = display[2] << n_shift;
		display[3] = display[3] << n_shift;
		display[4] = display[4] << n_shift;
}



/*-----------ISR Handler-----------*/
void ADC1_2_IRQHandler(void) {
		if (ADC1->SR & 0x02){
				ADC_TMP = ADC1->DR;
				ADC1->SR &= ~(0x02);
				ADC_TMP = (states & 0x400)? ((ADC_TMP*72)/50 + 536) : ((ADC_TMP*10)/80 + 140);	// Fahrenheit : Celcius
		}
		if (ADC1->SR & 0x04){
				ADC_ILL	= ADC1->JDR1;
				ADC1->SR &= ~(0x04);
				if (ADC_ILL < 0x40)
						ADC_ILL = 100;
				else if (ADC_ILL < 0x400)
						ADC_ILL = 100 +  (100-10) *(ADC_ILL*10 - 0x40*10)/ (0x400-0x40);
				else if (ADC_ILL < 0x850)
						ADC_ILL = 1000+  (200-100) *(ADC_ILL*10 - 0x400*10)/(0x850-0x400);
				else if (ADC_ILL < 0xD00)
						ADC_ILL = 2000+ (400-200) *(ADC_ILL*10 - 0x850*10)/(0xD00-0x850);
				else if (ADC_ILL < 0xF60)
						ADC_ILL = 4000 +  (800-400)*(ADC_ILL*10 - 0xD00*10) / (0xF60-0xD00);
				else 
						ADC_ILL = 8000; 
		}
}

void TIM1_UP_IRQHandler(void) {	// Dot matrix scrolling
		if((TIM1->SR & 0x01) != 0) {
			// Display brightness control
				brightness = (2*brightness + 1*ADC_ILL) / 3;
				TIM1->CCR2 = (brightness * 550) / 8000 + 40;
			// Display control signal update
				if (mode != Time_Set)
						n_shift++;
				if (n_shift >= 4){
						n_shift = 0;
						disp_idx++;
				}
				if (disp_idx >= n_digit) 
						disp_idx = 0;
			// Display contents update (ADC)
				if (n_shift == 0) {
						Disp_TMP = ADC_TMP;
						Disp_ILL = ADC_ILL;
				}
			// Time setting blinking
				if (mode == Time_Set)
					states ^= 0x01;
				TIM1->SR &= ~(0x01);
		}
}

void TIM1_CC_IRQHandler(void) { // Dot matrix scanning
		static u8 d_row = 0;
		static u16 d_col;
		if((TIM1->SR & 0x02) != 0){
				TIM1->SR &= ~(0x02);
				if (d_row < 5){
						GPIOC->ODR &= ~(0xFF);
						GPIOC->ODR |= (~(1<<d_row))&0x00FF;
						d_col = (display[d_row] >> 4);
						GPIOB->ODR = d_col;
				}
				else if (d_row <7)
						GPIOB->ODR = 0;
				else if (d_row == 7){
						GPIOC->ODR &= ~(0xFF);
						GPIOC->ODR |= (~(1<<d_row))&0x00FF;
						GPIOB->ODR = states & 0xFF;
						ADC1->CR2 |= (0x1 << 22);
				}
				d_row++;
				if (d_row >= 8)
						d_row = 0;
		}
		if ((TIM1->SR & 0x04) != 0){
				GPIOB->ODR = 0;
				TIM1->SR &= ~(0x04);
		}
}

void TIM2_IRQHandler(void) {	// Clock
		if ((TIM2->SR & 0x01) != 0){
				Base_Time.SS++;
				if (Base_Time.SS >= 60){
						Base_Time.SS -= 60;
						Base_Time.MM++;
				}
				if (Base_Time.MM >= 60){
						Base_Time.MM -= 60;
						Base_Time.HH++;
				}
				if (Base_Time.HH >= 12)
						states |= 1 << 2;
				if (Base_Time.HH >= 24){
						Base_Time.HH -= 24;
						states &= ~(1 << 2);
				}
				TIM2->SR &= ~(0x01);
				
				if ((states & 0x0200))				// If Clock unset, Dot blinks
						states ^= 0x01;
		}
		else if ((TIM2->SR & 0x02) != 0){	// If no key input, return to time display mode
				Key_cnt++;
				if (Key_cnt >= 3) {
						mode = Time_Disp;
						Mod_cur = 0;
						Key_cnt = 0;
						states &= 0x0FFF;
						states |= (1 << 12);
				}
				TIM2->SR &= ~(0x02);			
		}
}

void TIM3_IRQHandler(void) {	// Key matrix scanning
		static u8 key_row = 0, key_col = 0, n_key = 0, key_idx = 0xFF;
		static u16 CNT = 0;
		if (TIM3->SR & 0x01 != 0) {					// When TIM3_UP interrupt occurs
				key_col = GPIOA->IDR;						// Get Key matrix column inputs
				key_col = (key_col >> 4) & 0x0F;	// Masking Key column inputs
				switch(key_col) {								// Identifying inputs
					case 0:												// No inputs
						break;
					case 1:												// Col[0]
						CNT = TIM2->CNT;
						key_idx = 4*key_row;
						n_key++;
						break;
					case 2:												// Col[1]
						CNT = TIM2->CNT;
						key_idx = 4*key_row + 1;
						n_key++;
						break;
					case 4:												// Col[2];
						CNT = TIM2->CNT;
						key_idx = 4*key_row + 2;
						n_key++;
						break;
					case 8:												// Col[3];
						CNT = TIM2->CNT;
						key_idx = 4*key_row + 3;
						n_key++;
						break;
					default:											// Multiple inputs
						CNT = TIM2->CNT;
						n_key += 2;
				}
				key_row++;
				
				if (key_row == 4){							// Scanning over 16 Keys: completed
						if (n_key == 0) 
								Key_state = key_idx;		// Key_state clear
						else if (n_key == 1){
								if (Key_state != key_idx){
										Key_state = key_idx;
										EXTI->SWIER = 0x01;
								}
								TIM2->CCR1 = (CNT)? CNT - 1 : TIM2->ARR;
								Key_cnt = 0;
						}
						else{
								TIM2->CCR1 = (CNT)? CNT - 1  : TIM2->ARR;
								Key_cnt = 0;
						}
						key_idx = 0xFF;
						key_row = 0;
						n_key = 0;
						CNT = 0;
				}
				GPIOC->BSRR = (1 << (key_row + 8)) | (~(1 << (key_row + 24)) & 0x0F000000);	// Turn on next key row
				TIM3->SR &= ~(0x01);
	}
}

void EXTI0_IRQHandler(void) {			// Valid Key input
		static u8 num;
		if (EXTI->PR & 0x01) {
				if (mode == Time_Disp) {
						TIM2->DIER &= ~(0x02);
						switch (Key_state){
								case 0:
										EXTI->PR |= 0x01;
										return;
								case 1:
										mode = Temp_Disp;
										states &= 0x0FFF;
										states |= (2 << 12);
										break;
								case 2:
										mode = Illu_Disp;
										states &= 0x0FFF;
										states |= (4 << 12);
										break;
								case 3:
										mode = Time_Set;
										states &= 0x0FFF;
										states |= (8 << 12);
								
										TIM2->DIER |= 0x02;	
										Mod_Time.HH = Base_Time.HH;
										Mod_Time.MM = Base_Time.MM;
										Mod_Time.SS = Base_Time.SS;								
										break;
						}
				}
				else if (mode == Temp_Disp){
						TIM2->DIER &= ~(0x02);
						switch (Key_state){
								case 0:
										mode = Time_Disp;
										states &= 0x0FFF;
										states |= (1 << 12);
										break;
								case 1:
										EXTI->PR |= 0x01;
										return;
								case 2:
										mode = Illu_Disp;
										states &= 0x0FFF;
										states |= (4 << 12);
										break;
								case 3:
										states ^= (1 << 10);
										break;
						}
				}
				else if (mode == Illu_Disp){
						TIM2->DIER &= ~(0x02);			
						switch (Key_state){
								case 0:
										mode = Time_Disp;
										states &= 0x0FFF;
										states |= (1 << 12);
										break;
								case 1:
										mode = Temp_Disp;
										states &= 0x0FFF;
										states |= (2 << 12);
										break;
								case 2:
										EXTI->PR |= 0x01;
										return;
								case 3:
										EXTI->PR |= 0x01;
										return;
						}
				}
				else if (mode == Time_Set) {											
						disp_idx = 0;
						n_shift	= 0;
										
						switch (Key_state){
								case 0:					// Up
										switch(Mod_cur) {
											case 0:
												Mod_Time.HH = (Mod_Time.HH + 1) % 24;
												break;
											case 1:
												Mod_Time.MM = (Mod_Time.MM + 1) % 60;
												break;
											case 2:
												Mod_Time.SS = (Mod_Time.SS + 1) % 60;
												break;
										}
										break;
								case 1:					// Ignore
										EXTI->PR = 0x01;
										return;									
								case 2:					// Down
										switch(Mod_cur) {
											case 0:
												Mod_Time.HH = (Mod_Time.HH) ?  Mod_Time.HH - 1 : 23;
												break;
											case 1:
												Mod_Time.MM = (Mod_Time.HH) ? Mod_Time.MM - 1 : 59;
												break;
											case 2:
												Mod_Time.SS = (Mod_Time.SS) ? Mod_Time.SS - 1 : 59;
												break;
										}
										break;
								case 3:					// Confirm
										Mod_cur++;
										if (Mod_cur >= 3){
												Mod_cur = 0;
												Key_cnt = 0;
												Base_Time = Mod_Time;
												mode = Time_Disp;
												states &= 0x0FFF;
												states |= (1 << 12);
												states &= ~(0x01 << 9);
										}
										break;
								case 7:					// Right
										Mod_cur  = (Mod_cur + 1) % 3;
										break;
								case 11:				// 0
										switch (Mod_cur) {
											case 0:
												Mod_Time.HH = (H[0] * 10) % 24;
												break;
											case 1:
												Mod_Time.MM = (M[0] * 10) % 60;
												break;
											case 2:
												Mod_Time.SS = (S[0] * 10) % 60;
												break;
										}
										break;
								case 15:				// Left
										if (Mod_cur == 0)
												states ^= 0x02;
										else 
												Mod_cur--;
										break;
								default:				// 1 ~ 9
										num = (Key_state % 4) * 3 - Key_state / 4 + 4;
										switch (Mod_cur) {
											case 0:
												Mod_Time.HH = (H[0]*10 + num < 24)? H[0]*10 + num: 0;
												break;
											case 1:
												Mod_Time.MM = (M[0]*10 + num < 60)? M[0]*10 + num: 0;
												break;
											case 2:
												Mod_Time.SS = (S[0]*10 + num < 60)? S[0]*10 + num: 0;
												break;
										}
										break;
						}
				}
				if (Key_state < 4){
						disp_idx = 0;
						n_shift = 0;								
				}
				EXTI->PR |= 0x01;
		}
}
