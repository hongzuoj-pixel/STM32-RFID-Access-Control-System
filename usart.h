#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"

void USART1_Init(void);
void USART1_SendChar(char c);
void USART1_SendString(char *s);
void USART1_SendHex(uint8_t data);
void USART1_SendUID(uint8_t *uid);

#endif