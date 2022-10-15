#ifndef __ADC1_H
#define __ADC1_H

#include "stm32f4xx.h"

void initDmaADC1(uint16_t* dBuff1, uint16_t* dBuff2, uint16_t size);
void stopADC1();

#endif

