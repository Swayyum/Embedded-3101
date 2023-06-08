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

int main(void) {

    FPUInit();
    GPIOAInit();
    GPIOEInit();
    UART0Init();
    UART5Init();
    UART7Init();
    ADC0Init();
    sysTickInit();

    circBufLen = 0;
    circRIndex = 0;
    circWIndex = 0;


    histAverage = 0;
    lastAverage = 0;

    char frame[256];
    snprintf(frame, sizeof(frame), "Time    Cur T   Avg Temp (last 8s)  Avg Temp (since started)\r\n----    -----   ------------------  ------------------------\r\n");
    int j = 0;
    while(frame[j] != '\0'){
            UARTCharPut(UART0_BASE, frame[j]);
            j++;
    }

    int time = 0;
    int eightAve = 0;
    int avg = 0;
    eightIndex = 0;
    nEightElem = 0;

    IntMasterEnable();

    while (1){

        if(go){

            IntMasterDisable();

            avg = parseXML();
            eightAve = calculateAverages(avg, time);

            char output[256];
            snprintf(output, sizeof(output), "%d        %d      %d                      %d\r\n", time, avg, eightAve, histAverage);
            j = 0;
            while(output[j] != '\0'){
                    UARTCharPut(UART0_BASE, output[j]);
                    j++;
            }

            circRIndex = 0;
            circWIndex = 0;
            circBufLen = 0;

            go = !go;

            time += 2;
            eightAve = 0;

            IntMasterEnable();

        }
    }
}

void FPUInit(){


    FPUEnable();
    FPULazyStackingEnable();
    return;

}

void GPIOAInit(){

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)){};
    GPIOPinConfigure(GPIO_PA0_U0RX); // FTDI receiver
    GPIOPinConfigure(GPIO_PA1_U0TX); // FTDI transmitter
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    return;

}

void GPIOEInit(){

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE)){};
    GPIOPinConfigure(GPIO_PE4_U5RX); // edge receiver
    GPIOPinConfigure(GPIO_PE5_U5TX); // edge transmitter
    GPIOPinConfigure(GPIO_PE0_U7RX); // consolidator receiver
    GPIOPinConfigure(GPIO_PE1_U7TX); // consolidator transmitter
    GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_0 |
                                     GPIO_PIN_1 |
                                     GPIO_PIN_4 |
                                     GPIO_PIN_5);
    return;

}

void UART0Init(){

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0)){};
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(),
                        115200, UART_CONFIG_WLEN_8 |
                              UART_CONFIG_STOP_ONE |
                              UART_CONFIG_PAR_NONE);
    return;

}

void UART5Init(){

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART5);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART5)){};
    UARTConfigSetExpClk(UART5_BASE, SysCtlClockGet(),
                        115200, UART_CONFIG_WLEN_8 |
                              UART_CONFIG_STOP_ONE |
                              UART_CONFIG_PAR_NONE);
    return;

}

void UART7Init(){

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART7);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART7)){};
    UARTConfigSetExpClk(UART7_BASE, SysCtlClockGet(),
                        115200, UART_CONFIG_WLEN_8 |
                              UART_CONFIG_STOP_ONE |
                              UART_CONFIG_PAR_NONE);
    IntEnable(INT_UART7);
    UARTIntEnable(UART7_BASE, UART_INT_RX);
    return;

}

void ADC0Init(){

    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)){};
    ADCHardwareOversampleConfigure(ADC0_BASE, 64);
    ADCSequenceDisable(ADC0_BASE, 0);
    ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
    // ADC0, labeled sequence 0, set to always trigger, priority 0
    ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_TS |
                                              ADC_CTL_IE |
                                              ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 0);
    ADCIntEnable(ADC0_BASE, 0);
    tempBuffer = &rawTemp;
    IntEnable(INT_ADC0SS0_TM4C123);

}

void sysTickInit(){

    SysTickPeriodSet(1600000);
    SysTickIntEnable();
    SysTickEnable();
    tick = 0; // keep track of how many interrupts have fired

}

int parseXML(){

    int average = 0;
    int numTemps = 0;

    while(circRIndex < circBufLen){

        char test;

        do{
            test = circConsBuf[circRIndex];
            circRIndex++;
        }while(test != '<');

        // now we are on char past open bracket
        // check what msg type
        if (circConsBuf[circRIndex] != 't'){
            // some other kind of msg, ignore
            // should probably flush buffer here
            continue;
        }

        // okay, it is a temp message
        //increment reader by 2 to be at message start
        circRIndex += 2;
        char temp[XML_TEMP_MSG_MAX_SIZE - 3];// buffer for remaining data
        int i = 0;

        do{
            temp[i] = circConsBuf[circRIndex];
            i++;
            circRIndex++;
        }while(circConsBuf[circRIndex] != '<'); //catch closing xml bracket

        temp[i] = '\0'; //terminate
        int result = atoi(temp);
        average += result;
        numTemps++;

    }

    average /= numTemps;

    eightArray[eightIndex] = average;
    eightIndex++;

    if(eightIndex == 4){
        eightIndex = 0;
    }

    if(nEightElem != 4){
        nEightElem++;
    }

    return average;

}

int calculateAverages(int average, int time){

    int k;
    int eightAve = 0;

    for(k = 0; k < nEightElem; k++){
        eightAve += eightArray[k];
    }

    eightAve /= nEightElem;

    if(lastAverage){
        histAverage = ((histAverage * (time >> 1)) + average) / ((time >> 1) + 1);
    }
    else{
        histAverage = average;
    }

    lastAverage = average;

    return eightAve;
}

void uart7_handler(void){

    uint32_t status;
    status = UARTIntStatus(UART7_BASE, true);
    UARTIntClear(UART7_BASE, status);
    while(UARTCharsAvail(UART7_BASE)){
        if(circWIndex == sizeof(circConsBuf)){
                    circWIndex = 0;
        }
        circConsBuf[circWIndex] = UARTCharGet(UART7_BASE);
        circWIndex++;
        if(circBufLen < sizeof(circConsBuf)){
            circBufLen++;
        }
    }
}

void adc0_handler(void){

    uint32_t status;
    status = ADCIntStatusEx(ADC0_BASE, true);
    ADCIntClearEx(ADC0_BASE, status);
    ADCSequenceDataGet(ADC0_BASE, 0, tempBuffer); // get data
    int realTemp = 147.5 - ((75 * 3.3 * rawTemp) / 4096);
    snprintf(edgeBuf, sizeof(edgeBuf), "<t>%d</t>", realTemp);
    int i = 0;
    // edge prints to collector
    while(edgeBuf[i] != '\0'){
        UARTCharPut(UART5_BASE, edgeBuf[i]);
        i++;
    }

}

void systick_handler(void){

    if(tick == 19){
        tick = 0;
        ADCProcessorTrigger(ADC0_BASE, 0);
        go = 1;
    }
    else{
        ADCProcessorTrigger(ADC0_BASE, 0);
        tick++;
    }
}


