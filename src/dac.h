#ifndef __DAC_H
#define __DAC_H
#include "math.h"
#include "stm32f4xx.h"

#define PI 3.14159
#define FS 40000

#define DAC_BUFF_SIZE 40000

void initDmaDAC1(uint16_t* dBuff, uint16_t size);
int getData4DAC(uint16_t* dac_buff, float a1, int f1, float a2, int f2);
int getTriangle4DAC(uint16_t* dac_buff, int f, float a);
void stopDAC1(void);

#endif
