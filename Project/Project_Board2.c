#include <stm32f10x.h>

/*-----------Prototyping-----------*/
void Disp_Set(void);

typedef enum{
   Disconnect = 0, HandShake, Disp_On
}Type_mode;

typedef enum{					// Type_digit
		zero = (u8)0,
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

/*-----------Global variables----------*/

u8 font5x4 [18][5] = {   // font data
   {7,5,5,5,7},   //0
   {6,2,2,2,7},   //1
   {7,1,7,4,7},   //2
   {7,1,7,1,7},   //3
   {5,5,7,1,1},   //4
   {7,4,7,1,7},   //5
   {7,4,7,5,7},   //6
   {7,1,1,1,1},   //7
   {7,5,7,5,7},   //8
   {7,5,7,1,7},   //9
   {1,1,1,1,1},   //l
   {0,0,5,2,5},   //x
   {7,4,7,4,4},   //F
   {7,4,4,4,7},   //C
   {3,3,0,0,0},   // degree
   {0,0,0,0,2},   // dot
   {0,2,0,2,0},   // colon
   {0,0,0,0,0}    // SP
};

   // display control signal
u32 display[5];               // Display pattern
Type_digit	rx_data[8];               // Recieved data

   // Status signal
Type_mode mode = Disconnect;
u8  IsStable = 0;
u8  n_receive;               // number of recieved data
u8  data;


int main(void){
/*-----------Peripheral Setting-----------*/
   // RCC setting
      RCC->APB2ENR = 0x0000481D;   // USART1, TIM1, IOPA~C, AFIO clock enable

   // AFIO setting
      AFIO->MAPR=0x02002000;      // REMAP PB3, PB4 as GPIO & CAN disable to use USART1 RTS

   // GPIO setting
      GPIOA->CRL = 0x00300000;  // PA5: output (LED for USART states)
      GPIOA->CRH = 0x000B0800;   // PA10: RX(input pull-down), PA12: RTS(AF output)
      
      GPIOB->CRL = 0x33333333;   // PB0-7:output push-pull (Dot matrix Col)
      GPIOB->CRH = 0x33333333;   // PB8-15:output push-pull (Dot matrix Col)
   
      GPIOC->CRL = 0x33333333;   // PC0-7:output(Dot matrix row)
      GPIOC->CRL = 0x33333333;   // PC0-7:output(Dot matrix row)
      GPIOC->ODR = 0x00FF;         // Dot_row : inactive
   
   // Timer_1 setting (UP - Blinking, CH1 - Display scan, CH1 - PWM(brightness))
      TIM1->CR1 = 0x04;      // URS set
      TIM1->CR2 = 0x00;      
      TIM1->PSC = 124;    // PSC + 1 = 125 division (72MHz -> 576KHz)
      TIM1->ARR = 599;    // ARR + 1 = 600 division (576KHz -> 960Hz)   : Refresh rate = 120Hz
      TIM1->RCR = 119;      // RCR + 1 = 120 division (960Hz -> 8Hz) : LED control
      TIM1->DIER = 0x07;   // CC2IE, CC1IE, UIE set
      TIM1->CCMR1 = 0x1010;   // OC2M = '001' : Active on match, OC1M = '001' : Active on match, CC1S = '00' : Output Compare mode
      TIM1->CCER = 0x11;   // CC1E, CC2E set : Channel 1&2 enable
      TIM1->CCR1 = 0;       // TIM1_CNT == 0 -> CC1 interrupt (Next Row ON)
      TIM1->CCR2 = 0xFFF; // TIM1_CNT == 0xFFF -> CC2 interrupt (Row off)

   // USART setting
      USART1->BRR = 0x4E2;    // USART1 Baud rate : 57.6 KBps
      USART1->CR1 = 0x00000024;      // RXNEIE, RE set : simplex mode (Reciever)
      USART1->CR2 = 0x00000000;      
      USART1->CR3 = 0x00000000;      // RTSE = '0': default

/*-----------Interrupt Setting-----------*/
   // NVIC register setting
      NVIC->ISER[0] = 0x0A000000;   // TIM1_UP, TIM1_CC enable
      NVIC->ISER[1] = 0x00000020; // USART1 enable
      
/*-----------Module Enabling-----------*/
   // TIM - CEN
      TIM1->CR1 |= 0x01;      // TIM1_CEN set
   
   // USART enable
      USART1->CR1 |= 0x00002000; // UE set : USART1 enable
      
/*-------------Main function-------------*/   
      u8 i;
      while(1) {
            __WFI();
            if (mode != Disp_On)
                  continue;
            for (i = 1; i < 6; i++){
               display[0] = (display[0] << 4) + font5x4[rx_data[i]][0];
               display[1] = (display[1] << 4) + font5x4[rx_data[i]][1];
               display[2] = (display[2] << 4) + font5x4[rx_data[i]][2];
               display[3] = (display[3] << 4) + font5x4[rx_data[i]][3];
               display[4] = (display[4] << 4) + font5x4[rx_data[i]][4];                  
            }
            display[0] = display[0] << rx_data[6];
            display[1] = display[1] << rx_data[6];
            display[2] = display[2] << rx_data[6];
            display[3] = display[3] << rx_data[6];
            display[4] = display[4] << rx_data[6];
      }
}

/*-----------ISR Handler-----------*/
void TIM1_UP_IRQHandler(void) {   // 8Hz - Blinking & brightness control
      if((TIM1->SR & 0x01) != 0) {
         // Display brightness control
            if (mode == Disp_On)   
                  TIM1->CCR2 = (rx_data[0] * 550) / 255 + 40;
         // LED blinks
            else{
                  GPIOA->ODR ^= (1u << 5);
            }
            TIM1->SR &= ~(0x01u);
      }
}

void TIM1_CC_IRQHandler(void) { // Dot matrix scanning
      static u8 d_row = 0;
      static u16 d_col;
      if((TIM1->SR & 0x02) != 0){
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
                  GPIOB->ODR = rx_data[7];
            }
            d_row++;
            if (d_row >= 8)
                  d_row = 0;
            TIM1->SR &= ~(0x02);
      }
      if ((TIM1->SR & 0x04) != 0){
            GPIOB->ODR = 0;
            TIM1->SR &= ~(0x04);
      }
}

void USART1_IRQHandler(void) {
      if (USART1->SR & 0x20) {      // RXNE == '1'
            if (USART1->SR & 0x02){ // FE == '1' & NE == '1' : Frame Error
                  data = USART1->DR;      // FE reset
                  mode = Disconnect;
                  TIM1->DIER = 0x01;   		// TIM1_CH1&CH2 interrupt disable
									IsStable = 0;
									USART1->CR3 &= ~(0x100);
                  GPIOB->ODR = 0;         // Display OFF
            }
            else {
                  data = USART1->DR;
                  if (mode == Disconnect) {
                        if (data)
                              n_receive = 0;
                        else
                              n_receive++;
                        if (n_receive >= 6){
                              mode = HandShake;
                              n_receive = 0;
															IsStable = 1;
															USART1->CR3 |= 0x100;
                        }
                  }
                  else if (mode == HandShake){
                        if (data){
                              n_receive = 0;
                              rx_data[n_receive++] = data;
                              mode = Disp_On;
															GPIOA->ODR |= 0x20;		// LED On
                              TIM1->DIER = 0x07;		// TIM1_CH1&2 interrupt enable
                        }
                  }
                  else {
                        rx_data[n_receive++] = data;
                        n_receive %= 8;
                  }
            }
      }
}