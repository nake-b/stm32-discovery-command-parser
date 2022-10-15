#ifndef __LED_H
#define __LED_H

#include "stm32f4xx.h"

int led_s(int led_num, int state);
int led_p(int led_num, int duty_cycle);
void led_b(int led_num, int o, int p);

void initLedBlinkPeripherals(void);
void disableLedBlink(int led_num);

#endif
