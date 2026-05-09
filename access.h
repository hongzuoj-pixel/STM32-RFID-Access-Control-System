#ifndef __ACCESS_H
#define __ACCESS_H

#include "stm32f10x.h"

#define UID_LENGTH 4

typedef struct
{
    uint8_t uid[UID_LENGTH];
    char *name;
} CardInfo;

uint8_t IsSameUID(uint8_t *uid1, uint8_t *uid2);
int FindCardIndex(uint8_t *uid);
char *GetCardName(int index);
uint8_t IsNewCard(uint8_t *uid);

#endif