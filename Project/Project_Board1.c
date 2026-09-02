#include <stm32f10x.h>

/*-----------Prototyping-----------*/
#define FLASH_ADDR 0x08008000		// address of FLASH PAGE_32
#define TEST_DATA 0
void Tx_Set(void);

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
	u8	HH;				// 0 ~ 23 
	u8	MM;				// 0 ~ 59
	u8	SS;				// 0 ~ 59
}Struct_time;


/*-----------Global variables----------*/
Type_mode mode = Time_Disp;
	// Display control signals
Type_digit	digits[10];	// display contents
u8 	tx_data[8]={255,0,}; 	// Data to transmit
u8 	brightness = 255;	// Display brightness to transmit
u8	n_digit;
u8	disp_idx;
u8 	n_shift;
	
	// Status signal
u16	states;							// Dot matrix Row7: state representation 
u8	n_transmit;					// number of transmitted data
u8	IsConnect = 0;			// USART: 0-Disconnected, 1-Connected

	// Data storage
Struct_time	Base_Time = {12, 0, 0};	// Current Time
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
		RCC->APB2ENR = 0x00004A1D;	// USART1, TIM1, ADC1, IOPA-C, AFIO clock enable
		RCC->APB1ENR = 0x00000003; 	// TIM2, TIM3 enable
	
	// AFIO setting
//		AFIO->MAPR=0x00002000;   	// REMAP CAN :disable to use USART1_RTS
	
	// GPIO setting
		GPIOA->CRL = 0x00300000;  // PA0&1: Analog Input(ADC), PA5: output (LED for USART states)
		GPIOA->CRH = 0x000080B0;	// PA9: TX(AF output), PA11: CTS(input pull-up/down)
		GPIOA->ODR = 0x0800;			// PA11 : pull-up 

		GPIOB->CRH = 0x00008888;  // PB8-11: input (Key matrix col)
		GPIOB->ODR = 0x0000;			// PB8-11: pull-down (On - High, Off - Low)
	
		GPIOC->CRH = 0x00003333;  // PC8-11: output push-pull (Key matrix Row)
		GPIOC->ODR = 0x0000;			// Key_Row : Inactive
		
	// ADC setting (CH0 : PA0 - GL5537, CH1 : PA1 - TMP36)
		ADC1->CR1 = 0x000005A0;   // JAUTO, SCAN, JEOCIE, EOCIE set
		ADC1->CR2 = 0x001EF004;		// EXTTRIG, JEXTTRIG, CAL set, CON reset, JEXTSEL : JSWSTART, EXTSEL : SWSTART
		ADC1->SMPR2 = 0x003F;   	// Sample time for channel 0 & 1: 239.5 cylces (SMPx = '111')
		ADC1->SQR1 = 0x00000000;  // L = '0000' (1 conversion)
		ADC1->SQR2 = 0x00000000;	
		ADC1->SQR3 = 0x00000001; 	// Regular 1st conversion : channel 1 (temperature)
		ADC1->JSQR = 0x00000000; 	// injected 1st conversion : channel 0 (illumination), JL = '00' (1 conversion)

	// Timer_1 setting (UP - Scrolling, CH1 - ADC & USART)
		TIM1->CR1 = 0x04;	// URS set
		TIM1->CR2 = 0x00;	
		TIM1->PSC = 719;	// PSC + 1 = 720 division (72MHz -> 100KHz)
		TIM1->ARR = 2499;	// ARR + 1 = 2500 division (100KHz -> 40Hz) : TIM1_CC frequency
		TIM1->RCR = 4;	// RCR + 1 = 5 division (40Hz -> 8Hz) : TIM1_UP frequency
		TIM1->DIER = 0x03;	// CC1IE reset, UIE set
		TIM1->CCMR1 = 0x0010; // OC1M = '001': Active on match, CC1S = '00' : Output Compare mode
		TIM1->CCER = 0x01; 	// CC1E set : Channel 1 enable
		TIM1->CCR1 = 300;	// TIM1->CNT == 0000 : Key scanning
		
	// Timer_2 setting (UP - Clock, CH1 - Time_Set Return)
		TIM2->CR1 = 0x04;
		TIM2->CR2 = 0x00;
		TIM2->PSC = 7199;	// PSC + 1 = 7200 division (72MHz -> 10KHz)
		TIM2->ARR = 9999;	// ARR + 1 = 600 division (10KHz -> 1Hz)	: TIM2_UP frequency
		TIM2->DIER = 0x03;	// CC1IE set, UIE set
		TIM2->CCMR1 = 0x0010;	// OC1M = '001': Active on match, CC1S = '00' : Output Compare mode
		TIM2->CCER = 0x01;	// CC1E set : Channel 1 enable
		TIM2->CCR1 = 0xFFFF;	// Default - CH1 disable
		
	// Timer_3 setting (UP - Key scan)
		TIM3->CR1 = 0x04;
		TIM3->CR2 = 0x00;
		TIM3->PSC = 124;	// PSC + 1 = 125 division (72MHz -> 576KHz)
		TIM3->ARR = 599;	// ARR + 1 = 600 division (576KHz -> 960Hz) : Key scan rate = 240 Hz
		TIM3->DIER = 0x01;
				
	// USART setting
		USART1->BRR = 0x4E2; 	// USART1 Baud rate : 57.6 Kbps
		USART1->CR1 = 0x00000008;		// TE = '1' : simplex mode (Transmitter)
		USART1->CR2 = 0x00000000;
		
/*-----------Interrupt Setting-----------*/
	// NVIC register setting
		NVIC->ISER[0] = 0x3A040040;	// EXTI0, ADC1_2, TIM1_UP, TIM1_CC, TIM2, TIM3 interrupt enable
		NVIC->ISER[1] = 0x00000020; // USART1 interrupt enable
	// EXTI register setting	
		EXTI->IMR = 0x01;						// EXTI0 enable
		EXTI->FTSR = 0x00;
		EXTI->RTSR = 0x00;
		
/*-----------Module Enabling-----------*/
	// ADC - ADON
		ADC1->CR2 |= 0x00000001;  // ADON set

	// TIM - CEN
		TIM1->CR1 |= 0x01;		// TIM1_CEN set
		TIM2->CR1 |= 0x01;		// TIM2_CEN set
		TIM3->CR1 &= ~(0x00);	// TIM3_CEN clear
		
	// GPIO - Key Row
		GPIOC->ODR = 0x0000;	// Key_Row[0] active
		
	// USART enable
		USART1->CR1 |= 0x00002000;	// UE set :	USART1 enable
		USART1->CR1 |= 0x01;	// SBK set :	Send break character	
		USART1->CR3 = 0x00000400;		// CTSIE set, CTSE set : default

	
/*---------Initial State setting-	-------*/
		states = *((u16*)FLASH_ADDR);	// Read previous states
		states &= 0xFF00;
		states |= 0x0206;				// Clock undefined & P.M.
		states &= ~(0x02);
		states |= (states & 0x800)? 0x02: 0x00;	// Restore 12/24 setting
		
		switch(states >> 12){
			case 2:
				mode = Temp_Disp;
				break;
			case 4:
				mode = Illu_Disp;
				break;
			default:
				mode = Time_Disp;
				break;
		}
		
/*-----------Main function-----------*/
		
		
		while(1) {	
			
		// Check USART Communication
			while (IsConnect == 0) {
					TIM3->CR1 &= ~(0x01);	// Key matrix off
					TIM2->DIER &= ~(0x02);
					__WFI();
					if (IsConnect){
							TIM3->CR1 |= 0x01;	// Key matrix on
							if (mode == Time_Set)
									TIM2->DIER |= (0x02);
					}	
			}
			if (mode == Time_Disp){
					H[1] = (states & 0x0800) ?  Base_Time.HH : Base_Time.HH % 12;
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
					
					Tx_Set();
					__WFI();
			}
			else if (mode == Temp_Disp){
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
					
					Tx_Set();
					__WFI();
			}
			else if (mode == Illu_Disp){
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
					
					Tx_Set();
					__WFI();
			}
			else if (mode == Time_Set) {				
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
					
					n_digit = 4;
					Tx_Set();
					__WFI();
			}
		}
}
/*-----------User defined function-----------*/
void Tx_Set(void)	{
		tx_data[0] = brightness;
		tx_data[1] = digits[(disp_idx) % n_digit];
		tx_data[2] = digits[(disp_idx + 1) % n_digit];
		tx_data[3] = digits[(disp_idx + 2) % n_digit];
		tx_data[4] = digits[(disp_idx + 3) % n_digit];
		tx_data[5] = digits[(disp_idx + 4) % n_digit];
		tx_data[6] = n_shift;
		tx_data[7] = (u8)(states & 0xFF);
}

/*-----------ISR Handler-----------*/
void ADC1_2_IRQHandler(void) {	// TIM36, GL5537 Calibratoin & brightness update
		if (ADC1->SR & 0x02){		// EOC : channel_1(TMP36)
				ADC_TMP = ADC1->DR;
				ADC1->SR &= ~(0x02);
				ADC_TMP = (states & 0x400)? ((ADC_TMP*18)/80 + 572) : (ADC_TMP/8 + 140);	// Fahrenheit : Celcius
		}
		if (ADC1->SR & 0x04){		// JEOC : channel_0(GL5537)
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

void USART1_IRQHandler(void) {	// CTS interrupt -> Send next
		if (USART1->SR & 0x80){			// TXE == '1' : TDR empty
				n_transmit++;
				if (n_transmit < 8){
						USART1->DR = (IsConnect)? tx_data[n_transmit]: TEST_DATA;					// Send next data
				}
		}
		USART1->SR &= ~(0x0200);
}

void TIM1_UP_IRQHandler(void){	// 8Hz - Scrolling &  Blinking & brigtness
		if (TIM1->SR & 0x01){			
				if (IsConnect == 0)				// if Display disconnected
						GPIOA->ODR ^= (1u << 5);	// LED blink
				else {
						brightness /= 2;
						brightness += ADC_ILL * 120 / 8000 + 5;

						if (mode != Time_Set){	// Scrolling
								n_shift++;
								if (n_shift >= 4){
										n_shift = 0;
										disp_idx++;
								}
								if (disp_idx >= n_digit) 
										disp_idx = 0;
								if ((states & 0x0200))				// If Clock unset
										states ^= 0x01;						// Dot blink
						}
						else 				//  If mode == Time_set
								states ^= 0x01;								// Dot blink
				}
				TIM1->SR &= ~(0x01);
		}
}

void TIM1_CC_IRQHandler(void) {	// 40Hz - ADC & USART trigger
		if (TIM1->SR & 0x02){
				ADC1->CR2 |= (0x1 << 22);	// ADC1 - SWSTART, JSWSTART = '1' : conversion start
				if (n_transmit >= 8) {
						IsConnect = 1;
						GPIOA->ODR |= (1u << 5);
						USART1->DR = tx_data[0];
						n_transmit = 0;
				}
				else{	// Previous tx_data sending : Error
						if (IsConnect){
								USART1->CR1 |= 0x01;	// SBK set :	Send break character
						}
						else if(USART1->SR & 0x0080){	// TXE = '1'
								USART1->DR = TEST_DATA;
								USART1->CR3 |= 0x0200; // CTSE set;
						}
						else{
								USART1->DR = TEST_DATA;
								USART1->CR3 &= ~(0x0200);	//CTSE reset;
						}
						n_transmit = 0;
						IsConnect = 0;

				}
				TIM1->SR &= ~(0x02);
		}
}

void TIM2_IRQHandler(void) {	// 1Hz - UP : Clock & ADC Display update, CH1 : 3 seconds violation
		if (TIM2->SR & 0x01){			// TIM2_UP
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
			// ADC display update
				Disp_TMP = ADC_TMP;
				Disp_ILL = ADC_ILL;
				
				TIM2->SR &= ~(0x01);
		}
		else if ((TIM2->SR & 0x02) != 0){	// TIM2_CH1
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

void TIM3_IRQHandler(void) {	// 960Hz - Key matrix scanning
		static u16 key_row = 0, key_col = 0, n_key = 0, key_idx = 0xFF;
		static u16 CNT = 0;
		if (TIM3->SR & 0x01) {		// TIM3_UP
				key_col = GPIOB->IDR;						// Get Key matrix column inputs
				key_col = (key_col >> 8) & 0x0F;	// Masking Key column inputs
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
										TIM2->DIER |= 0x02;	
										Mod_Time.HH = Base_Time.HH;
										Mod_Time.MM = Base_Time.MM;
										Mod_Time.SS = Base_Time.SS;		
										EXTI->PR |= 0x01;
										disp_idx = 0;
										n_shift	= 0;
										return;
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
										Disp_TMP = ADC_TMP;
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
										EXTI->PR |= 0x01;
										return;
								case 1:					// Ignore
										EXTI->PR = 0x01;
										return;									
								case 2:					// Down
										switch(Mod_cur) {
											case 0:
												Mod_Time.HH = (Mod_Time.HH) ?  Mod_Time.HH - 1 : 23;
												break;
											case 1:
												Mod_Time.MM = (Mod_Time.MM) ? Mod_Time.MM - 1 : 59;
												break;
											case 2:
												Mod_Time.SS = (Mod_Time.SS) ? Mod_Time.SS - 1 : 59;
												break;
										}
										EXTI->PR |= 0x01;
										return;
								case 3:					// Confirm
										Mod_cur++;
										if (Mod_cur < 3){
												EXTI->PR |= 0x01;
												return;
										}
										Mod_cur = 0;
										Key_cnt = 0;
										Base_Time = Mod_Time;
										mode = Time_Disp;
										states &= 0x0FFF;
										states |= 0x1001;
										states &= ~(0x01 << 9);
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
												states ^= 0x0802;
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
					
				//flash erase
					FLASH->KEYR =0x45670123;	// Unlocking sequence to access FLASH->CR register
          FLASH->KEYR =0xCDEF89AB;
          FLASH->CR |= (1<<1);  	// PER = '1' : Page erase
          FLASH->AR = FLASH_ADDR;	//page to erase (page32 0x08008000)
          FLASH->CR |= (1<<6);		// STRT = '1' : start erase operation
          while(FLASH->SR & 0x1) {;}	// wait until FLASH operation complete.
					FLASH->CR &= ~(1<<1);		// PER = '0'
       
         //flash programming
					FLASH->CR |= (1<<0);   // PG = '1' : programming set
					*((u16*)FLASH_ADDR) = states;
          while(FLASH->SR & 0x1) {;}	// wait until FLASH operation complete
					FLASH->CR &= ~(1<<0); // PG = '0' : programming reset
					FLASH->CR |= 1<<7; 		// LOCK = '1' : lock FPEC & FLASH->CR
				}
				EXTI->PR |= 0x01;
		}
}
