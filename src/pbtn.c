#include "pbtn.h"

int checkPbtn(void) { return (GPIOA->IDR & 0x0001) == 0x0001; }

void initPbtn(void) {
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  GPIOA->MODER &= ~0x00000003;
}
