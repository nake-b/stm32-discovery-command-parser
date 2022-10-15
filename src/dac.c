#include "dac.h"
#include "usart.h"

// Euclid algorithm
int gcd(int a, int b) {
  int temp;
  while (b != 0) {
    temp = a % b;

    a = b;
    b = temp;
  }
  return a;
}

void initDmaDAC1(uint16_t* dBuff, uint16_t size) {
  /// wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
  /// PA4 -> DAC1_OUT
  /// wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;  //
  GPIOA->MODER |= GPIO_MODER_MODER4;    //

  /// wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
  /// setup TIM6 for generation of request
  /// wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
  RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;  //
  TIM6->PSC = 0x0064 - 0x0001;         //
                                       //
  TIM6->ARR = 0x0015;
  ;  
  TIM6->CR1 = 0x0084;         //
                              //
  TIM6->CR2 = TIM_CR2_MMS_1;  //

  TIM6->EGR |= TIM_EGR_UG;   //
  TIM6->CR1 |= TIM_CR1_CEN;  //

  /// wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
  /// Enable DAC1 DMA
  /// wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
  RCC->APB1ENR |= RCC_APB1ENR_DACEN;  //

  DAC->CR &= ~(DAC_CR_EN1);

  DAC->CR = (DAC_CR_DMAEN1);  //
  DAC->CR |= (DAC_CR_TEN1);

  //
  DAC->CR |= DAC_CR_EN1;  //

  /// wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
  /// Init DMA controller for circular transfer of data for DAC1
  /// wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;  //

  DMA1_Stream5->CR = 0x00000000;  //
  while ((DMA1_Stream5->CR & DMA_SxCR_EN) == DMA_SxCR_EN)
    ;  //

  DMA1->LIFCR = 0xFFFFFFFF;
  DMA1->HIFCR = 0xFFFFFFFF;

  DMA1_Stream5->PAR = (uint32_t)&DAC->DHR12R1;  //
  DMA1_Stream5->M0AR = (uint32_t)dBuff;         //
  DMA1_Stream5->NDTR = size;  // 
                              // //

  DMA1_Stream5->CR |= DMA_SxCR_CHSEL;  //
  DMA1_Stream5->CR |=
      DMA_SxCR_PL;                    //
                                      // 														//
  DMA1_Stream5->CR |= DMA_SxCR_MINC;  //
                                      //
  DMA1_Stream5->CR |= DMA_SxCR_CIRC;  //

  DMA1_Stream5->CR |= DMA_SxCR_PSIZE_0;  //
                                         //
  DMA1_Stream5->CR |= DMA_SxCR_MSIZE_0;  //
                                         //
  DMA1_Stream5->CR |= DMA_SxCR_DIR_0;    //
                                         //
  DMA1_Stream5->CR |= DMA_SxCR_EN;       //
  return;
}

int getData4DAC(uint16_t* dac_buff, float a1, int f1, float a2, int f2) {
  // Sampling frequency fs=40kHz, step size for t is then 1/fs = 25e-6
  float t_step = 1 / ((float)FS);

  // Frequency of a linear combination of 2 sinusiodal signals with frequencies f1 and f2
  // is their GCD, GCD(f1, f2))
  int f = gcd(f1, f2);
  float period = 1 / ((float)f);

  float y = 0;
  float t = 0;

  // With this DBS, exactly one signal period will be written
  int dbs = (int)ceil(FS * period);
  int n;
  for (n = 0; n < (dbs); n++) {
    y = a1 * sinf(2 * PI * f1 * t) + a2 * sinf(2 * PI * f2 * t) + 0.5;
    dac_buff[n] = (uint16_t)(y * 4095);
    t += t_step;
  }
  return dbs;
}

int getTriangle4DAC(uint16_t* dac_buff, int f, float a) {
  float t_step = 1 / ((float)FS);

  float t3 = 1 / ((float)f);
  float t2 = 3 * t3 / 4;
  float t1 = t3 / 4;

  float t = 0;
  float y = 0;

  int dbs = (int)ceil(t3 * FS);
  int n;
  for (n = 0; n < (dbs); n++) {
    if (t < t1)
      y = t * a / t1;
    else if (t < t2)
      y = a - 2 * a * (t - t1) / (t2 - t1);
    else if (t < t3)
      y = -a + a * (t - t2) / (t3 - t2);
    else {
      t -= t3;
      y = t * a / t1;
    }
    y += fabs(a);
    dac_buff[n] = (uint16_t)(y * 4095);
    t += t_step;
  }
  return dbs;
}

void stopDAC1(void) {
  DMA1_Stream5->CR &= ~DMA_SxCR_EN;  //
  DAC->CR &= ~DAC_CR_EN1;            //
  TIM6->CR1 &= ~TIM_CR1_CEN;         //
  return;
}

