#include "stm32f10x.h"
#include "rc522.h"
#include "delay.h"

/* ===================== RC522 Wiring ===================== */
/*
RC522 wiring:
SDA / CS  -> PA4
SCK       -> PA5
MISO      -> PA6
MOSI      -> PA7
RST       -> PA3
3.3V      -> 3.3V
GND       -> GND
*/

/* ===================== RC522 Register Definition ===================== */

#define CommandReg      0x01
#define CommIEnReg      0x02
#define CommIrqReg      0x04
#define ErrorReg        0x06
#define FIFODataReg     0x09
#define FIFOLevelReg    0x0A
#define ControlReg      0x0C
#define BitFramingReg   0x0D
#define ModeReg         0x11
#define TxControlReg    0x14
#define TxASKReg        0x15
#define TModeReg        0x2A
#define TPrescalerReg   0x2B
#define TReloadRegH     0x2C
#define TReloadRegL     0x2D

/* ===================== SPI1 Init ===================== */

void SPI1_Init(void)
{
    RCC->APB2ENR |= (1 << 2);   // GPIOA clock enable
    RCC->APB2ENR |= (1 << 12);  // SPI1 clock enable

    // PA5 SCK, PA7 MOSI: Alternate function push-pull, 50MHz
    GPIOA->CRL &= ~((0xF << 20) | (0xF << 28));
    GPIOA->CRL |=  ((0xB << 20) | (0xB << 28));

    // PA6 MISO: floating input
    GPIOA->CRL &= ~(0xF << 24);
    GPIOA->CRL |=  (0x4 << 24);

    // PA4 CS: general output push-pull
    GPIOA->CRL &= ~(0xF << 16);
    GPIOA->CRL |=  (0x3 << 16);
    GPIOA->ODR |= (1 << 4);     // CS high

    // PA3 RST: general output push-pull
    GPIOA->CRL &= ~(0xF << 12);
    GPIOA->CRL |=  (0x3 << 12);
    GPIOA->ODR |= (1 << 3);     // RST high

    SPI1->CR1 = 0;
    SPI1->CR1 |= (1 << 2);      // Master mode
    SPI1->CR1 |= (1 << 8);      // SSI
    SPI1->CR1 |= (1 << 9);      // Software slave management
    SPI1->CR1 |= (0x4 << 3);    // Baud rate prescaler
    SPI1->CR1 |= (1 << 6);      // SPI enable
}

uint8_t SPI1_RW(uint8_t data)
{
    while(!(SPI1->SR & (1 << 1)));  // wait TXE
    SPI1->DR = data;

    while(!(SPI1->SR & (1 << 0)));  // wait RXNE
    return (uint8_t)SPI1->DR;
}

/* ===================== RC522 Basic Functions ===================== */

void RC522_CS_Low(void)
{
    GPIOA->ODR &= ~(1 << 4);
}

void RC522_CS_High(void)
{
    GPIOA->ODR |= (1 << 4);
}

void RC522_WriteReg(uint8_t reg, uint8_t value)
{
    RC522_CS_Low();
    SPI1_RW((reg << 1) & 0x7E);
    SPI1_RW(value);
    RC522_CS_High();
}

uint8_t RC522_ReadReg(uint8_t reg)
{
    uint8_t value;

    RC522_CS_Low();
    SPI1_RW(((reg << 1) & 0x7E) | 0x80);
    value = SPI1_RW(0x00);
    RC522_CS_High();

    return value;
}

void RC522_SetBitMask(uint8_t reg, uint8_t mask)
{
    uint8_t tmp;

    tmp = RC522_ReadReg(reg);
    RC522_WriteReg(reg, tmp | mask);
}

void RC522_ClearBitMask(uint8_t reg, uint8_t mask)
{
    uint8_t tmp;

    tmp = RC522_ReadReg(reg);
    RC522_WriteReg(reg, tmp & (~mask));
}

void RC522_Reset(void)
{
    GPIOA->ODR &= ~(1 << 3);
    delay_ms(10);

    GPIOA->ODR |= (1 << 3);
    delay_ms(10);

    RC522_WriteReg(CommandReg, PCD_RESETPHASE);
    delay_ms(10);
}

void RC522_AntennaOn(void)
{
    uint8_t temp;

    temp = RC522_ReadReg(TxControlReg);

    if(!(temp & 0x03))
    {
        RC522_SetBitMask(TxControlReg, 0x03);
    }
}

void RC522_Init(void)
{
    RC522_Reset();

    RC522_WriteReg(TModeReg, 0x8D);
    RC522_WriteReg(TPrescalerReg, 0x3E);
    RC522_WriteReg(TReloadRegL, 30);
    RC522_WriteReg(TReloadRegH, 0);

    RC522_WriteReg(TxASKReg, 0x40);
    RC522_WriteReg(ModeReg, 0x3D);

    RC522_AntennaOn();
}

uint8_t RC522_ToCard(uint8_t command,
                     uint8_t *sendData,
                     uint8_t sendLen,
                     uint8_t *backData,
                     uint16_t *backLen)
{
    uint8_t status = MI_ERR;
    uint8_t irqEn = 0x00;
    uint8_t waitIRq = 0x00;
    uint8_t lastBits;
    uint8_t n;
    uint16_t i;

    if(command == PCD_TRANSCEIVE)
    {
        irqEn = 0x77;
        waitIRq = 0x30;
    }

    RC522_WriteReg(CommIEnReg, irqEn | 0x80);
    RC522_ClearBitMask(CommIrqReg, 0x80);
    RC522_SetBitMask(FIFOLevelReg, 0x80);
    RC522_WriteReg(CommandReg, PCD_IDLE);

    for(i = 0; i < sendLen; i++)
    {
        RC522_WriteReg(FIFODataReg, sendData[i]);
    }

    RC522_WriteReg(CommandReg, command);

    if(command == PCD_TRANSCEIVE)
    {
        RC522_SetBitMask(BitFramingReg, 0x80);
    }

    i = 2000;
    do
    {
        n = RC522_ReadReg(CommIrqReg);
        i--;
    }
    while((i != 0) && !(n & 0x01) && !(n & waitIRq));

    RC522_ClearBitMask(BitFramingReg, 0x80);

    if(i != 0)
    {
        if(!(RC522_ReadReg(ErrorReg) & 0x1B))
        {
            status = MI_OK;

            if(n & irqEn & 0x01)
            {
                status = MI_ERR;
            }

            if(command == PCD_TRANSCEIVE)
            {
                n = RC522_ReadReg(FIFOLevelReg);
                lastBits = RC522_ReadReg(ControlReg) & 0x07;

                if(lastBits)
                {
                    *backLen = (n - 1) * 8 + lastBits;
                }
                else
                {
                    *backLen = n * 8;
                }

                if(n == 0)
                {
                    n = 1;
                }

                if(n > 16)
                {
                    n = 16;
                }

                for(i = 0; i < n; i++)
                {
                    backData[i] = RC522_ReadReg(FIFODataReg);
                }
            }
        }
    }

    return status;
}

uint8_t RC522_Request(uint8_t reqMode, uint8_t *tagType)
{
    uint8_t status;
    uint16_t backBits;

    RC522_WriteReg(BitFramingReg, 0x07);

    tagType[0] = reqMode;

    status = RC522_ToCard(PCD_TRANSCEIVE, tagType, 1, tagType, &backBits);

    if((status != MI_OK) || (backBits != 0x10))
    {
        status = MI_ERR;
    }

    return status;
}

uint8_t RC522_Anticoll(uint8_t *serNum)
{
    uint8_t status;
    uint8_t i;
    uint8_t serNumCheck = 0;
    uint16_t unLen;

    RC522_WriteReg(BitFramingReg, 0x00);

    serNum[0] = PICC_ANTICOLL;
    serNum[1] = 0x20;

    status = RC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

    if(status == MI_OK)
    {
        for(i = 0; i < 4; i++)
        {
            serNumCheck ^= serNum[i];
        }

        if(serNumCheck != serNum[4])
        {
            status = MI_ERR;
        }
    }

    return status;
}