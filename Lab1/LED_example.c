#include <stm32f10x.h>

static uint32_t led = 0;

int main(void){

RCC->APB2ENR = 0x00000004;
GPIOA->CRL = 0x00300000;
GPIOA->ODR = 0x0000;
	
while(1){
	
	led = ~led;
	GPIOA->ODR = led << 5;
	for(int i=0; i<300000; i++){}
	
	}
}
