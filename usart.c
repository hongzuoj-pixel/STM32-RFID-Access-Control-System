#include "stm32f10x.h"
#include "usart.h"

void USART1_Init(void)
{
    RCC->APB2ENR |= (1 << 2);    // GPIOA clock enable
    RCC->APB2ENR |= (1 << 14);   // USART1 clock enable

    // PA9 = USART1_TX, alternate function push-pull
    GPIOA->CRH &= ~(0xF << 4);
    GPIOA->CRH |=  (0xB << 4);

    // PA10 = USART1_RX, floating input
    GPIOA->CRH &= ~(0xF << 8);
    GPIOA->CRH |=  (0x4 << 8);

    /*
       Serial monitor:
       Baud rate: 115200
       Data bits: 8
       Stop bits: 1
       Parity: None
    */
    USART1->BRR = 0x0271;        // 115200 @ 72MHz

    USART1->CR1 |= (1 << 13);    // USART enable
    USART1->CR1 |= (1 << 3);     // transmitter enable
    USART1->CR1 |= (1 << 2);     // receiver enable
}

void USART1_SendChar(char c)
{
    while(!(USART1->SR & (1 << 7)));
    USART1->DR = c;
}

void USART1_SendString(char *s)
{
    while(*s)
    {
        USART1_SendChar(*s++);
    }
}

void USART1_SendHex(uint8_t data)
{
    char hex[] = "0123456789ABCDEF";

    USART1_SendChar(hex[(data >> 4) & 0x0F]);
    USART1_SendChar(hex[data & 0x0F]);
}

void USART1_SendUID(uint8_t *uid)
{
    USART1_SendString("UID: ");
    USART1_SendHex(uid[0]);
    USART1_SendChar(' ');
    USART1_SendHex(uid[1]);
    USART1_SendChar(' ');
    USART1_SendHex(uid[2]);
    USART1_SendChar(' ');
    USART1_SendHex(uid[3]);
    USART1_SendString("\r\n");
}