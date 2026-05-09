#include "stm32f10x.h"
#include "buzzer.h"
#include "delay.h"

void Buzzer_Init(void)
{
    RCC->APB2ENR |= (1 << 2);   // GPIOA clock enable

    GPIOA->CRL &= ~(0xF << 4);  // PA1
    GPIOA->CRL |=  (0x3 << 4);  // output push-pull

    GPIOA->ODR |= (1 << 1);     // buzzer off
}

void Buzzer_ON(void)
{
    GPIOA->ODR &= ~(1 << 1);    // low level active
}

void Buzzer_OFF(void)
{
    GPIOA->ODR |= (1 << 1);
}

void Beep_OK(void)
{
    Buzzer_ON();
    delay_ms(200);
    Buzzer_OFF();
}

void Beep_FAIL(void)
{
    uint8_t i;

    for(i = 0; i < 3; i++)
    {
        Buzzer_ON();
        delay_ms(100);
        Buzzer_OFF();
        delay_ms(100);
    }
}

void Beep_Alarm(void)
{
    uint8_t i;

    for(i = 0; i < 5; i++)
    {
        Buzzer_ON();
        delay_ms(300);
        Buzzer_OFF();
        delay_ms(200);
    }
}