#include "led.h"

uint32_t gpio_moder_clear_arr[4] = {GPIO_MODER_MODER6, GPIO_MODER_MODER7,
                                    GPIO_MODER_MODER8, GPIO_MODER_MODER9};
uint32_t gpio_moder_output_arr[4] = {GPIO_MODER_MODER6_0, GPIO_MODER_MODER7_0,
                                     GPIO_MODER_MODER8_0,
                                     GPIO_MODER_MODER9_0};
uint32_t gpio_moder_af_arr[4] = {GPIO_MODER_MODER6_1, GPIO_MODER_MODER7_1,
                                 GPIO_MODER_MODER8_1, GPIO_MODER_MODER9_1};
uint32_t gpio_odr_arr[4] = {GPIO_ODR_ODR_6, GPIO_ODR_ODR_7, GPIO_ODR_ODR_8,
                            GPIO_ODR_ODR_9};
uint32_t gpio_afrh_arr[4] = {0x02000000, 0x20000000, 0x00000002, 0x00000020};

int blinking_led_buff_o[4] = {0, 0, 0, 0};
int blinking_led_buff_p[4];
int blinking_led_buff_o_cntrs[4];
int blinking_led_buff_p_cntrs[4];
int led_static_states[4] = {0, 0, 0, 0};

int led_s(int led_num, int state) {
  led_static_states[led_num] = state;
  // Determine which LEDs to set in 'output' mode, and their ODR
  int gpio_moder_clear = gpio_moder_clear_arr[led_num];
  int gpio_moder_output = gpio_moder_output_arr[led_num];
  int gpio_odr = gpio_odr_arr[led_num];
  // Clear MODER
  GPIOC->MODER &= ~gpio_moder_clear;
  // Configure registers
  GPIOC->MODER |= gpio_moder_output;
  GPIOC->OTYPER |= 0x00000000;
  GPIOC->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR12) | (GPIO_OSPEEDER_OSPEEDR13) |
                    (GPIO_OSPEEDER_OSPEEDR14) | (GPIO_OSPEEDER_OSPEEDR15);

  // Turn pin on or off
  switch (state) {
    case 0:
      GPIOC->ODR &= ~gpio_odr;
      break;
    case 1:
      GPIOC->ODR |= gpio_odr;
      break;
    default:
      return 0;
  }
  return 1;
}

int led_p(int led_num, int duty_cycle) {
  // If LED is blinking, turn off
  disableLedBlink(led_num);
   // Determine which LEDs to set in 'af' mode, and their AFRH

  int gpio_moder_clear = gpio_moder_clear_arr[led_num];
  int gpio_moder_af = gpio_moder_af_arr[led_num];
  int gpio_afrh = gpio_afrh_arr[led_num];

  // Clear moder
  GPIOC->MODER &= ~gpio_moder_clear;
  // Configure registers
  GPIOC->MODER |= gpio_moder_af;
  GPIOC->OTYPER |= 0x00000000;
  GPIOC->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR12) | (GPIO_OSPEEDER_OSPEEDR13) |
                    (GPIO_OSPEEDER_OSPEEDR14) | (GPIO_OSPEEDER_OSPEEDR15);

  // Set alternate function
  int af_reg_idx;
  if(led_num < 2)af_reg_idx = 0;
  else af_reg_idx = 1;
  GPIOC->AFR[af_reg_idx] |= gpio_afrh;

  // Activate timer in PWM mode
  RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
  TIM3->PSC = 0x0054 - 0x0001;
  TIM3->ARR = 0x03E8;

  TIM3->CCMR1 |= (TIM_CCMR1_OC1PE) | (TIM_CCMR1_OC1M_2) | (TIM_CCMR1_OC1M_1);
  TIM3->CCMR1 |= (TIM_CCMR1_OC2PE) | (TIM_CCMR1_OC2M_2) | (TIM_CCMR1_OC2M_1);
  TIM3->CCMR2 |= (TIM_CCMR2_OC3PE) | (TIM_CCMR2_OC3M_2) | (TIM_CCMR2_OC3M_1);
  TIM3->CCMR2 |= (TIM_CCMR2_OC4PE) | (TIM_CCMR2_OC4M_2) | (TIM_CCMR2_OC4M_1);
  TIM3->CCER &=
      ~((TIM_CCER_CC1P) | (TIM_CCER_CC2P) | (TIM_CCER_CC3P) | (TIM_CCER_CC4P));
  TIM3->CR1 |= (TIM_CR1_ARPE) | (TIM_CR1_URS);

  TIM3->EGR |= TIM_EGR_UG;
  TIM3->CCER |=
      (TIM_CCER_CC1E) | (TIM_CCER_CC2E) | (TIM_CCER_CC3E) | (TIM_CCER_CC4E);
  TIM3->CR1 |= TIM_CR1_CEN;

  duty_cycle *= 10;

  switch (led_num) {
    case 0:
      TIM3->CCR1 = duty_cycle;
      break;
    case 1:
      TIM3->CCR2 = duty_cycle;
      break;
    case 2:
      TIM3->CCR3 = duty_cycle;
      break;
    case 3:
      TIM3->CCR4 = duty_cycle;
      break;
  }

  return 1;
}

void initLedBlinkPeripherals(void) {
  RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
  TIM7->PSC = 84 * 100 - 0x0001;

  TIM7->ARR = 10;  // 1ms
  TIM7->CR1 = 0x0084;

  TIM7->CR2 = 0x0000;
  TIM7->CNT = 0x0000;
  TIM7->EGR |= TIM_EGR_UG;
  TIM7->DIER = 0x0001;  // enable

  NVIC_SetPriority(TIM7_IRQn, 0);
  NVIC_EnableIRQ(TIM7_IRQn);
  TIM7->CR1 |= TIM_CR1_CEN;

  return;
}

void led_b(int led_num, int o, int p) {
  blinking_led_buff_o[led_num] = o;
  blinking_led_buff_p[led_num] = p;
  blinking_led_buff_o_cntrs[led_num] = 0;
  blinking_led_buff_p_cntrs[led_num] = 0;
  led_s(led_num, 0);
}

void disableLedBlink(int led_num) { blinking_led_buff_o[led_num] = 0; }

void TIM7_IRQHandler(void) {
  if (TIM7->SR & 0x0001) {
    int led_num, o;
    // Check state of blinking_led buffer for all LEDs
    for (led_num = 0; led_num < 4; led_num++) {
      o = blinking_led_buff_o[led_num];
      if (o != 0) {  // LED is blinking
        int p = blinking_led_buff_p[led_num];

        int* p_cntr = &blinking_led_buff_p_cntrs[led_num];

        (*p_cntr)++;
        (*p_cntr) %= p;
        if (*p_cntr == 0) {
          led_s(led_num, 1);
        }

        // Move turn-off counter only if the LED is on
        if (led_static_states[led_num] == 1) {
          int* o_cntr = &blinking_led_buff_o_cntrs[led_num];
          (*o_cntr)++;
          (*o_cntr) %= o;
          if (*o_cntr == 0) led_s(led_num, 0);
        }
      }
    }
    TIM7->SR = 0x0000;
  }
}

