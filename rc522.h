#ifndef __RC522_H
#define __RC522_H

#include "stm32f10x.h"

/* ===================== RC522 Command Definition ===================== */

#define PCD_IDLE        0x00
#define PCD_TRANSCEIVE  0x0C
#define PCD_RESETPHASE  0x0F

#define PICC_REQIDL     0x26
#define PICC_ANTICOLL   0x93

#define MI_OK           0
#define MI_ERR          1

/* ===================== Function Declaration ===================== */

void SPI1_Init(void);
uint8_t SPI1_RW(uint8_t data);

void RC522_CS_Low(void);
void RC522_CS_High(void);

void RC522_WriteReg(uint8_t reg, uint8_t value);
uint8_t RC522_ReadReg(uint8_t reg);

void RC522_SetBitMask(uint8_t reg, uint8_t mask);
void RC522_ClearBitMask(uint8_t reg, uint8_t mask);

void RC522_Reset(void);
void RC522_AntennaOn(void);
void RC522_Init(void);

uint8_t RC522_ToCard(uint8_t command,
                     uint8_t *sendData,
                     uint8_t sendLen,
                     uint8_t *backData,
                     uint16_t *backLen);

uint8_t RC522_Request(uint8_t reqMode, uint8_t *tagType);
uint8_t RC522_Anticoll(uint8_t *serNum);

#endif