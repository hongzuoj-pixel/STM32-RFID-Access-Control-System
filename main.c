#include "stm32f10x.h"
#include "delay.h"
#include "led.h"
#include "buzzer.h"
#include "usart.h"
#include "access.h"
#include "rc522.h"

/* ===================== System State Definition ===================== */

typedef enum
{
    STATE_IDLE = 0,     // Waiting for card
    STATE_SCAN,         // Reading card UID
    STATE_CHECK,        // Checking whitelist
    STATE_GRANTED,      // Access granted
    STATE_DENIED,       // Access denied
    STATE_ALARM         // Alarm and temporary lock
} SystemState;

/* ===================== Global Variables ===================== */

static uint8_t tagType[2];
static uint8_t uid[5];

static int cardIndex = -1;
static uint8_t failedCount = 0;

static SystemState currentState = STATE_IDLE;

/* ===================== Serial Structured Log Function ===================== */
/*
   This function sends a structured log line through USART.

   Format:
   LOG,UID,USER,RESULT,FAIL_COUNT

   Example:
   LOG,2E EF 30 07,Admin Card,GRANTED,0
   LOG,AD 8D 17 07,Unknown Card,DENIED,1
   LOG,AD 8D 17 07,Unknown Card,ALARM,3

   This format will be used by the Python logger later.
*/

void USART1_SendAccessLog(uint8_t *cardUID, char *userName, char *result, uint8_t failCount)
{
    USART1_SendString("LOG,");

    USART1_SendHex(cardUID[0]);
    USART1_SendChar(' ');
    USART1_SendHex(cardUID[1]);
    USART1_SendChar(' ');
    USART1_SendHex(cardUID[2]);
    USART1_SendChar(' ');
    USART1_SendHex(cardUID[3]);

    USART1_SendChar(',');
    USART1_SendString(userName);

    USART1_SendChar(',');
    USART1_SendString(result);

    USART1_SendChar(',');
    USART1_SendChar(failCount + '0');

    USART1_SendString("\r\n");
}

/* ===================== State Handler Functions ===================== */

void State_IDLE(void)
{
    uint8_t status;

    status = RC522_Request(PICC_REQIDL, tagType);

    if(status == MI_OK)
    {
        currentState = STATE_SCAN;
    }
    else
    {
        currentState = STATE_IDLE;
    }
}

void State_SCAN(void)
{
    uint8_t status;

    status = RC522_Anticoll(uid);

    if(status == MI_OK)
    {
        if(IsNewCard(uid))
        {
            USART1_SendString("[INFO] Card detected\r\n");
            USART1_SendUID(uid);

            currentState = STATE_CHECK;
        }
        else
        {
            currentState = STATE_IDLE;
        }
    }
    else
    {
        currentState = STATE_IDLE;
    }
}

void State_CHECK(void)
{
    cardIndex = FindCardIndex(uid);

    if(cardIndex >= 0)
    {
        currentState = STATE_GRANTED;
    }
    else
    {
        currentState = STATE_DENIED;
    }
}

void State_GRANTED(void)
{
    failedCount = 0;

    USART1_SendString("[ACCESS] GRANTED\r\n");
    USART1_SendString("[USER] ");
    USART1_SendString(GetCardName(cardIndex));
    USART1_SendString("\r\n");
    USART1_SendString("[FAIL COUNT] 0\r\n\r\n");

    USART1_SendAccessLog(uid, GetCardName(cardIndex), "GRANTED", failedCount);

    LED_ON();
    Beep_OK();

    delay_ms(1000);
    LED_OFF();

    currentState = STATE_IDLE;
}

void State_DENIED(void)
{
    failedCount++;

    USART1_SendString("[ACCESS] DENIED\r\n");
    USART1_SendString("[USER] Unknown Card\r\n");
    USART1_SendString("[FAIL COUNT] ");
    USART1_SendChar(failedCount + '0');
    USART1_SendString("\r\n\r\n");

    USART1_SendAccessLog(uid, "Unknown Card", "DENIED", failedCount);

    LED_OFF();
    Beep_FAIL();

    if(failedCount >= 3)
    {
        currentState = STATE_ALARM;
    }
    else
    {
        currentState = STATE_IDLE;
    }
}

void State_ALARM(void)
{
    USART1_SendString("[ALARM] Too many failed attempts!\r\n");
    USART1_SendString("[SYSTEM] Locked for 5 seconds\r\n\r\n");

    USART1_SendAccessLog(uid, "Unknown Card", "ALARM", failedCount);

    Beep_Alarm();

    LED_ON();
    delay_ms(500);
    LED_OFF();
    delay_ms(500);
    LED_ON();
    delay_ms(500);
    LED_OFF();

    delay_ms(5000);

    failedCount = 0;

    USART1_SendString("[SYSTEM] Unlock now\r\n");
    USART1_SendString("[FAIL COUNT] 0\r\n\r\n");

    currentState = STATE_IDLE;
}

/* ===================== System Init ===================== */

void System_Init(void)
{
    SPI1_Init();
    RC522_Init();

    LED_Init();
    LED_OFF();

    Buzzer_Init();
    Buzzer_OFF();

    USART1_Init();

    USART1_SendString("\r\n");
    USART1_SendString("================================\r\n");
    USART1_SendString(" RFID Access Control System\r\n");
    USART1_SendString(" System Start\r\n");
    USART1_SendString(" Version: v2.0 Serial Logging\r\n");
    USART1_SendString("================================\r\n\r\n");

    currentState = STATE_IDLE;
}

/* ===================== main ===================== */

int main(void)
{
    System_Init();

    while(1)
    {
        switch(currentState)
        {
            case STATE_IDLE:
                State_IDLE();
                break;

            case STATE_SCAN:
                State_SCAN();
                break;

            case STATE_CHECK:
                State_CHECK();
                break;

            case STATE_GRANTED:
                State_GRANTED();
                break;

            case STATE_DENIED:
                State_DENIED();
                break;

            case STATE_ALARM:
                State_ALARM();
                break;

            default:
                currentState = STATE_IDLE;
                break;
        }

        delay_ms(100);
    }
}