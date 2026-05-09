#include "stm32f10x.h"
#include "led.h"

void LED_Init(void)
{
    RCC->APB2ENR |= (1 << 2);   // GPIOA clock enable

    GPIOA->CRL &= ~(0xF << 0);  // PA0
    GPIOA->CRL |=  (0x3 << 0);  // output push-pull
}

void LED_ON(void)
{
    GPIOA->ODR |= (1 << 0);
}

void LED_OFF(void)
{
    GPIOA->ODR &= ~(1 << 0);
}