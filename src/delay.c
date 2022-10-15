#include "delay.h"

void delay_ms(uint32_t ms) {
  RCC->APB1ENR |= RCC_APB1ENR_TIM12EN;  //
  TIM12->PSC = 0x0054 - 0x0001;         // APB1@42MHz
                                        //
  TIM12->ARR = 0x03E8;                  // reload 1000 us
  TIM12->CR1 = 0x0084;                  // ARPE On, CMS disable, Up counting

  TIM12->EGR |= TIM_EGR_UG;   // reload all config p363
  TIM12->CR1 |= TIM_CR1_CEN;  // start counter
  while (ms > 0) {
    while ((TIM12->SR & TIM_SR_UIF) == 0x0000)
      ;  // wait for update event

    TIM12->SR &= ~TIM_SR_UIF;  // clear the update event interrupt flag
    ms--;
  }
  TIM12->CR1 &= ~TIM_CR1_CEN;            // stop counter
  RCC->APB1ENR &= ~RCC_APB1ENR_TIM12EN;  // disable TIM
}

