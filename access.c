#include "access.h"

/*
   Authorized card list.

   Current valid card:
   UID: 2E EF 30 07

   To add a new card:
   1. Scan the new card
   2. Read its UID from serial monitor
   3. Add one new line into whiteList[]
*/
CardInfo whiteList[] =
{
    {{0x2E, 0xEF, 0x30, 0x07}, "Admin Card"},

    // Example cards. Replace these with real cards later.
    {{0x12, 0x34, 0x56, 0x78}, "User Card 1"},
    {{0xA1, 0xB2, 0xC3, 0xD4}, "User Card 2"}
};

#define WHITE_LIST_SIZE  (sizeof(whiteList) / sizeof(whiteList[0]))

uint8_t IsSameUID(uint8_t *uid1, uint8_t *uid2)
{
    uint8_t i;

    for(i = 0; i < UID_LENGTH; i++)
    {
        if(uid1[i] != uid2[i])
        {
            return 0;
        }
    }

    return 1;
}

int FindCardIndex(uint8_t *uid)
{
    uint8_t i;

    for(i = 0; i < WHITE_LIST_SIZE; i++)
    {
        if(IsSameUID(uid, whiteList[i].uid))
        {
            return i;
        }
    }

    return -1;
}

char *GetCardName(int index)
{
    if(index >= 0 && index < WHITE_LIST_SIZE)
    {
        return whiteList[index].name;
    }

    return "Unknown Card";
}

uint8_t IsNewCard(uint8_t *uid)
{
    static uint8_t lastUID[4] = {0, 0, 0, 0};

    if(uid[0] == lastUID[0] &&
       uid[1] == lastUID[1] &&
       uid[2] == lastUID[2] &&
       uid[3] == lastUID[3])
    {
        return 0;   // same card, do not repeat
    }

    lastUID[0] = uid[0];
    lastUID[1] = uid[1];
    lastUID[2] = uid[2];
    lastUID[3] = uid[3];

    return 1;
}