#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "adc.h"
#include "dac.h"
#include "delay.h"
#include "help.h"
#include "led.h"
#include "math.h"
#include "pbtn.h"
#include "stm32f4xx.h"
#include "usart.h"

#define ADC_BUFF_SIZE 10000
#define COMMAND_ARGUMENTS_NUMBER 6

static uint16_t dac_buff[DAC_BUFF_SIZE];
static uint16_t adc_buff0[ADC_BUFF_SIZE];
static uint16_t adc_buff1[ADC_BUFF_SIZE];
extern uint8_t commands[COMMANDS_NUMBER][COMMAND_SIZE];
char command_substring_buff[COMMAND_ARGUMENTS_NUMBER][100];

void printCmd(char *cmd) {
  printUSART2("\n  ");
  printUSART2(cmd);
  printUSART2("\n\n");
}

void printError(void) { printCmd("ERROR: Unknown command"); }

void printCmdStart(void) { printUSART2("\n$ "); }

int isFloat(char *str) {
  char *ptr;
  double d = strtod(str, &ptr);
  if (ptr == str)
    return 0;
  else if (ptr[strspn(ptr, " \t\r\n")] != '\0')
    return 0;
  else
    return 1;
}

int isNumber(char *text) {
  int j;
  j = strlen(text);
  while (j--) {
    if (text[j] >= '0' && text[j] <= '9') continue;

    return 0;
  }
  return 1;
}

long int strToInt(char *str) {
  long int num = strtol(str, (char **)NULL, 10);
  return num;
}

float strToFloat(char *str) {
  float res = strtof(str, NULL);
  return res;
}

int floatStrIsInRange(char *floatStr, float a, float b) {
  int is_float = isFloat(floatStr);
  if (!is_float) return 0;
  float flt = strToFloat(floatStr);
  return ((a <= flt) && (flt <= b));
}

int numStrIsInRange(char *numStr, int a, int b) {
  int isNum = isNumber(numStr);
  if (!isNum) return 0;

  long int num = strToInt(numStr);
  if ((num > INT_MAX) || (num < INT_MIN)) return 0;
  return ((a <= num) && (num <= b));
}

void initPeripherals(void) {
  // LED: 
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
  initLedBlinkPeripherals();

  // PBTN:
  initPbtn();

  // USART2
  initUSART2(USART2_BAUDRATE_460800);
  enIrqUSART2();
}

// Returns number of strings in a command. If the number is bigger than 6, returns 0
int parseCommand(char *cmd) {
  int command_start_idx = 0;
  int command_end_idx = 0;
  int substring_idx = 0;
  int substr_len;

  while (command_start_idx < strlen(cmd)) {
    if (cmd[command_end_idx] == ' ' || cmd[command_end_idx] == '\0') {
      substr_len = command_end_idx - command_start_idx;
      if (substr_len > 0) {
        if (substring_idx == COMMAND_ARGUMENTS_NUMBER) return 0;
        strncpy(command_substring_buff[substring_idx], cmd + command_start_idx,
                substr_len);
        command_substring_buff[substring_idx++][substr_len] = '\0';
      }
      command_start_idx = ++command_end_idx;
    } else {
      ++command_end_idx;
    }
  }

  return substring_idx;
}

int handlePbtnCommand(int str_num) {
  // Check if there is only one command string
  if (str_num != 1) return 0;

  int pbtn_state = checkPbtn();
  // Write state
  switch (pbtn_state) {
    case 0:
      printCmd("Pushbutton inactive");
      break;
    case 1:
      printCmd("Pushbutton active");
      break;
    default:
      return 0;
      break;
  }

  return 1;
}

int handleHelpCommand(int str_num) {
  if (str_num != 1) return 0;

  printHelp();
  return 1;
}

int main(void) {
  initPeripherals();
  int dac_buff_size;

  printUSART2("\n\n==============================\n");
  printUSART2(" STM Command Line Parser v1.0 \n");
  printUSART2("==============================\n\n");
  printCmdStart();

  while (1) {
    if (command_ridx != command_widx) {
      // Read data, update read index
      char *cmd = commands[command_ridx];  
      command_ridx = (++command_ridx) % COMMANDS_NUMBER;

      int str_num = parseCommand(cmd);
      if (!str_num) {
        printError();
        printCmdStart();
        continue;
      }
      
      // Take first part of command and parse
      char *first_substr = command_substring_buff[0];

      if (strcmp(first_substr, "pbtn") == 0) {
        int ok = handlePbtnCommand(str_num);
        if (!ok) printError();
      } else if (strcmp(first_substr, "help") == 0) {
        int ok = handleHelpCommand(str_num);
        if (!ok) printError();
      } else if (str_num < 2)
        printError();  // All other commands have more arguments
      else if (strlen(command_substring_buff[1]) != 1)
        printError();  // Second argument is only 1 char long
      else {
        char arg1 = command_substring_buff[1][0];
        if (strcmp(first_substr, "led") == 0) {
          if (str_num < 4)
            printError();  // LED commands always have at least 3 args
          else {
            char *arg2 = command_substring_buff[2];
            char *arg3 = command_substring_buff[3];
            if (!numStrIsInRange(arg2, 0, 3))
              printError();  // There are only 4 LEDs
            else {
              int led_num = strToInt(arg2);
              switch (arg1) {
                case ('s'):
                  if (str_num != 4) {
                    printError();
                    break;
                  }
                  if (!(numStrIsInRange(arg3, 0, 1))) {
                    printError();
                    break;
                  }
                  int static_state = strToInt(arg3);
                  disableLedBlink(led_num);
                  led_s(led_num, static_state);
                  printUSART2("\n  LED: [%d] set to static state [%d]\n\n",
                              led_num, static_state);
                  break;
                case ('p'):
                  if (str_num != 4) {
                    printError();
                    break;
                  }
                  if (!(numStrIsInRange(arg3, 0, 100))) {
                    printError();
                    break;
                  }
                  int duty_cycle = strToInt(arg3);
                  led_p(led_num, duty_cycle);
                  printUSART2(
                      "\n  LED [%d] set to pwm state, duty cycle [%d]\n\n",
                      led_num, duty_cycle);
                  break;
                case ('b'):
                  if (str_num != 5) {
                    printError();
                    break;
                  }
                  char *arg4 = command_substring_buff[4];
                  if (!((numStrIsInRange(arg3, 0, 60000)) && (numStrIsInRange(arg4, 0, 60000)))) {
                    printError();
                    break;
                  }
                  int o = strToInt(arg3);
                  int p = strToInt(arg4);
                  if (o >= p) {
                    printError();
                    break;
                  }
                  led_b(led_num, o, p);
                  printUSART2(
                      "\n  LED [%d] set to blink state, on time [%d]ms & "
                      "period [%d]ms\n\n",
                      led_num, o, p);
                  break;
                default:
                  printError();
                  break;
              }
            }
          }
        } else if (strcmp(first_substr, "dac1") == 0) {
          switch (arg1) {
            case ('s'):
              switch (str_num) {
                case 2:
                  stopDAC1();
                  printCmd("DAC1 deinitialized");
                  break;
                case 6:;
                  char *f1 = command_substring_buff[2];
                  char *a1 = command_substring_buff[3];
                  char *f2 = command_substring_buff[4];
                  char *a2 = command_substring_buff[5];
                  if (!((numStrIsInRange(f1, 10, 10000)) && (isFloat(a1)) &&
                        (numStrIsInRange(f2, 10, 10000)) && (isFloat(a2)))) {
                    printError();
                    break;
                  }
                  float a1f = strToFloat(a1);
                  float a2f = strToFloat(a2);
                  if (fabs(a1f) + fabs(a2f) > 0.49) {
                    printError();
                    break;
                  }
                  int f1i = strToInt(f1);
                  int f2i = strToInt(f2);
                  dac_buff_size = getData4DAC(dac_buff, a1f, f1i, a2f, f2i);
                  initDmaDAC1(dac_buff, dac_buff_size);
                  printUSART2(
                      "\n  DAC1 generates linear combination of sine waves, a1 "
                      "= [%f], a2=[%f], f1=[%d]Hz, f2=[%d]Hz, on pin PA4\n\n",
                      a1f, a2f, f1i, f2i);
                  break;
                default:
                  printError();
                  break;
              }
              break;
            case ('t'):
              if (str_num != 4) {
                printError();
                break;
              }
              char *f = command_substring_buff[2];
              char *a = command_substring_buff[3];
              if (!((numStrIsInRange(f, 10, 10000)) && (isFloat(a)))) {
                printError();
                break;
              }
              float af = strToFloat(a);
              int fi = strToInt(f);
              dac_buff_size = getTriangle4DAC(dac_buff, fi, af);
              initDmaDAC1(dac_buff, dac_buff_size);
              printUSART2(
                  "\n  DAC1 generates triangle wave of amplitude a = [%f], and "
                  "frequency f = [%d]Hz, on pin PA4\n\n",
                  af, fi);
              break;
            default:
              printError();
              break;
          }
        } else if (strcmp(first_substr, "adc1") == 0) {
          if (str_num != 2)
            printError();
          else {
            switch (arg1) {
              case ('r'):
                initDmaADC1(adc_buff0, adc_buff1, ADC_BUFF_SIZE);
                printCmd("ADC initialized on pin PA1");
                break;
              case ('s'):
                stopADC1();
                printCmd("ADC1 deinitialized");
                break;
              default:
                printError();
                break;
            }
          }
        } else
          printError();  // No more commands
      }
      printCmdStart();
    }
  }

  return 0;
}

