/* Lab2A for Embedded Systems
 * Author: Alex Fay, Mark McAfoose
 * id# 801336907, 801149826
 * date: 04/22/2023
 * main.h
 */

#ifndef MAIN_H_
#define MAIN_H_

// includes
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "inc/hw_memmap.h" // needed for definition of UART5_BASE
#include "inc/hw_ints.h" // needed for INT_UART5
#include "driverlib/pin_map.h" // needed for GPIO_PA0_U0RX and GPIO_PA1_U0TX
// these are needed for the various TivaWare calls below
#include "driverlib/fpu.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/adc.h"
#include "driverlib/systick.h"

// defines
#define XML_TEMP_MSG_MAX_SIZE 18 // 10 digits, <t> </t> and null terminator

// function prototypes
void FPUInit();
void GPIOAInit();
void GPIOEInit();
void UART0Init();
void UART5Init();
void UART7Init();
void ADC0Init();
void sysTickInit();
int parseXML();
int calculateAverages(int average, int time);

// global variables
uint32_t rawTemp;
uint32_t *tempBuffer;
char edgeBuf[XML_TEMP_MSG_MAX_SIZE];
char circConsBuf[8 * XML_TEMP_MSG_MAX_SIZE];
volatile int circBufLen;
volatile int circRIndex;
volatile int circWIndex;
volatile int tick;
volatile int go;
int temps[];
int lastAverage;
int histAverage;
int eightArray[4] = {0, 0, 0, 0};
int eightIndex;
int nEightElem;

#endif /* MAIN_H_ */
