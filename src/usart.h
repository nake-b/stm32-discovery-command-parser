#ifndef __USART2_H_
#define __USART2_H_

#include <stdarg.h>
#include <stdio.h>
#include "misc.h"
#include "stm32f4xx.h"

#define COMMAND_SIZE 30
#define COMMANDS_NUMBER 20

#define USART2_BAUDRATE_921600 0x0000002D
#define USART2_BAUDRATE_460800 0x0000005B
#define USART2_BAUDRATE_230400 0x000000B6
#define USART2_BAUDRATE_115200 0x0000016C
#define USART2_BAUDRATE_9600 0x00001117

uint8_t commands[COMMANDS_NUMBER]
                [COMMAND_SIZE];  
volatile uint16_t command_widx;
volatile uint16_t command_ridx;

void initUSART2(uint32_t baudrate);
void putcharUSART2(uint8_t data);
void printUSART2(char * str, ... );
void sprintUSART2(uint8_t * str);

void enIrqUSART2(void);
#endif
