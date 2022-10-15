#include "adc.h"

void initDmaADC1(uint16_t* dBuff1, uint16_t* dBuff2, uint16_t size) {
  /// ADC1 -> PA1 IN1
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; /** GPIOA Periph clock enable */
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;  /** ADC1 Periph clock enable */
  GPIOA->MODER |= GPIO_MODER_MODER1;   /** Set Analog mode on pin 1 */
  GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR1;  /** No pullup or pulldown */

  ADC1->CR1 = 0x00000800;  // 12-bit resolution (15 ADCCLK cycles)
                           // discontinious mode on regular channels
                           // number of regular channels to be converted in
                           // discontinuous mode: 1

  ADC1->SMPR2 = 0x00000000;  // conversion time 3 cycles of 84MHz clock!
  ADC1->SQR1 = 0x00000000;   // select one channel in regular sequence
  ADC1->SQR2 = 0x00000000;
  ADC1->SQR3 = 0x00000001;  // select channel 1
  ADC1->CR2 = 0x00000200;

  ADC->CCR = 0x00010000;  // ADC clock 84/4 = 21MHz!

  /// setup TIM2 for ADC1 TRGO event
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;  // enable TIM2
  TIM2->PSC = 0x0064 - 0x0001;

  TIM2->ARR = 0x0015;  // 40 kHz
  TIM2->CR1 = 0x0084;  // ARPE On, CMS disable, Up counting
                       // UEV disable, TIM2 enable
  TIM2->CR2 = 0x0000;
  TIM2->CR2 =
      TIM_CR2_MMS_1;  // Update event is used to as trigger output (TRGO)

  TIM2->EGR |= TIM_EGR_UG;   // update event, reload all config p363
  TIM2->CR1 |= TIM_CR1_CEN;  // start counter

  /// setup ADC with DMA
  ADC1->CR2 = ADC_CR2_DMA;          // enable DMA
  ADC1->CR2 |= ADC_CR2_DDS;         // enable DDS
  ADC1->CR2 |= ADC_CR2_EXTEN_0;     // trigger detection on the rising edge
  ADC1->CR2 |= (ADC_CR2_EXTSEL_1 | ADC_CR2_EXTSEL_2);  // 1000: Timer 2 TRGO event
  ADC1->CR2 |= ADC_CR2_ADON;

  /// setup DMA2 controller for ADC1 p179
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;  // enable DMA2 clock

  DMA2_Stream0->CR = 0x00000000;  // disable stream 0 (ADC1)
  while ((DMA2_Stream0->CR & DMA_SxCR_EN) == DMA_SxCR_EN)
    ;  // wait until the DMA transfer is completed

  DMA2->LISR = 0x00000000;
  DMA2->HISR = 0x00000000;

  DMA2_Stream0->PAR =
      (uint32_t)&ADC1->DR;  // peripheral port register address (ADC1)
  DMA2_Stream0->M0AR = (uint32_t)dBuff1;  // memory address of the first buffer
  DMA2_Stream0->M1AR = (uint32_t)dBuff2;  // memory address of the second buffer
  DMA2_Stream0->NDTR = size;              // number of samples to write

  DMA2_Stream0->CR &= ~DMA_SxCR_CHSEL;  // select channel 0 for ADC1
  DMA2_Stream0->CR |= DMA_SxCR_PL;      // select stream priority to very high
                                        // - DMA is flow controller
                                        // - Peripheral address pointer is fixed
  DMA2_Stream0->CR |= DMA_SxCR_MINC;    // Memory address pointer is incremented
  DMA2_Stream0->CR |= DMA_SxCR_DBM;     // Double buffer mode
  DMA2_Stream0->CR |=
      DMA_SxCR_PSIZE_0;  // Peripheral data size: Half Word 16-bit
  DMA2_Stream0->CR |= DMA_SxCR_MSIZE_0;  // Memory data size: Half Word 16-bit
  DMA2_Stream0->CR &=
      ~DMA_SxCR_DIR;  // Data transfer direction: 00 -> Peripheral-to-memory
  DMA2_Stream0->CR |= DMA_SxCR_EN;  // enable stream 0 (ADC1)
  return;
}

void stopADC1(void) {
  DMA2_Stream0->CR &= ~DMA_SxCR_EN;  // disable stream 0 (ADC1)
  ADC1->CR2 &= ~ADC_CR2_ADON;
  TIM2->CR1 &= ~TIM_CR1_CEN;  // start counter
  return;
}
