#include "delay.h"

void delay_ms(unsigned int t)
{
    unsigned int i, j;

    for(i = 0; i < t; i++)
    {
        for(j = 0; j < 800; j++);
    }
}