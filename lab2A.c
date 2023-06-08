// 2023/04/19 - UART5 with TivaWare and Interrupts
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "inc/hw_memmap.h" // needed for definition of UART5_BASE
#include "inc/hw_ints.h" // needed for INT_UART5
#include "inc/hw_types.h"
#include "driverlib/pin_map.h" // needed for GPIO_PA0_U0RX and GPIO_PA1_U0TX
// these are needed for the various TivaWare calls below
#include "driverlib/fpu.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/systick.h"
// these are libraries for the temps
#include <stdlib.h>

#define NVIC_SYS_PRI3_R (*((volatile uint32_t *)0xE000ED20))  // address of the NVIC_SYS_PRI3 register
#define RING_BUFFER_SIZE 32

unsigned int currTime = 0;

unsigned char temps[] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned char tempsIndex = 0;

// Ring buffer structure definition
typedef struct {
    uint8_t *buffer;      // Pointer to the buffer array
    uint32_t size;        // Size of the buffer array
    uint32_t read_index;  // Index for reading from the buffer
    uint32_t write_index; // Index for writing to the buffer
} RingBuffer;


struct nvic_st_s{
    volatile unsigned long ctrl;
    volatile unsigned long reload;
    volatile unsigned long current;
};

// Initialize the ring buffer
void ring_buffer_init(RingBuffer *ring_buffer, uint8_t *buffer, uint32_t size) {
    ring_buffer->buffer = buffer;
    ring_buffer->size = size;
    ring_buffer->read_index = 0;
    ring_buffer->write_index = 0;
}

// Write data to the ring buffer
bool ring_buffer_write(RingBuffer *ring_buffer, uint8_t data) {
    uint32_t next_write_index = (ring_buffer->write_index + 1) % ring_buffer->size;

    if (next_write_index == ring_buffer->read_index) {
        return false; // Buffer is full
    }

    ring_buffer->buffer[ring_buffer->write_index] = data;
    ring_buffer->write_index = next_write_index;
    return true;
}

// Read data from the ring buffer
bool ring_buffer_read(RingBuffer *ring_buffer, uint8_t *data) {
    if (ring_buffer->read_index == ring_buffer->write_index) {
        return false; // Buffer is empty
    }

    *data = ring_buffer->buffer[ring_buffer->read_index];
    ring_buffer->read_index = (ring_buffer->read_index + 1) % ring_buffer->size;
    return true;
}

uint8_t buffer_data[RING_BUFFER_SIZE];
RingBuffer ring_buffer;

// returns a random temperature between 20-99 (that represents the current temperature)
unsigned int currTemp(){
    srand(currTime);
    return (rand() % 80) + 20;
}

void addTemp(unsigned int temp){
    temps[tempsIndex] = temp;
    tempsIndex++;
    tempsIndex %= 8;
}

unsigned char getTempAvg(){
    unsigned char avgTemp = 0;
    char i;
    for(i = 0; i < 8; ++i){
        if(temps[i]) avgTemp += temps[i];
    }
    return avgTemp / 8;
}

void printInit(){
    char frame[32]; // this frame size is too big. how did the other group use it? causes a call to the fault ISR; size of 32 works otherwise
    snprintf(frame, sizeof(frame), "Time    Cur T   Avg Temp (last 8");
    int j = 0;
    while(frame[j] != '\0'){
            UARTCharPut(UART0_BASE, frame[j]);
            j++;
    }
    j = 0;
    snprintf(frame, sizeof(frame), "s)  Avg Temp (since started)\r\n");
    while(frame[j] != '\0'){
            UARTCharPut(UART0_BASE, frame[j]);
            j++;
    }

    j = 0;
    snprintf(frame, sizeof(frame), "----    -----   ----------------");
    while(frame[j] != '\0'){
            UARTCharPut(UART0_BASE, frame[j]);
            j++;
    }

    j = 0;
    snprintf(frame, sizeof(frame), "--  ------------------------\r\n");
    while(frame[j] != '\0'){
            UARTCharPut(UART0_BASE, frame[j]);
            j++;
    }
}

void putUART(uint32_t uart, char *cp){
    uint32_t UART_BASE = UART0_BASE + (uart << 12);
    while( *cp!='\0' ){
        UARTCharPut(UART_BASE,*cp++);
    }
}

void uart7_handler(void){

    uint32_t status;
    char cc;

    // Read the interrupt status and clear the interrupt flag
    status = UARTIntStatus(UART7_BASE, true);
    UARTIntClear(UART7_BASE, status);

    // Loop through all available characters in the UART7 receive FIFO
    while(UARTCharsAvail(UART7_BASE)){
        cc = UARTCharGet(UART7_BASE);

        //debug print:
        UARTCharPutNonBlocking(UART0_BASE, cc);

        if(!ring_buffer_write(&ring_buffer, cc)){
            // todo: prevent overflow
        }


    }

}

void SysTick_Handler(void){
    currTime += 1;
    char buffer[11];
    snprintf(buffer, sizeof(buffer) , "<t>%i</t>", currTemp());
    //puts(5, buffer);
    char i = 0;
    while(buffer[i] != '\0'){
        UARTCharPut(UART5_BASE, buffer[i]);
        ++i;
    }
}

int main(void){

    FPUEnable();
    FPULazyStackingEnable();

    // Enable systick:
    // set the clock to 20 MHz
    SysCtlClockSet(SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    struct nvic_st_s *systick = (struct nvic_st_s *)0xE000E010;

    NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & 0x00FFFFFF) | 0x30000000;

    systick->ctrl = (0x1 | 0x2); // enable systick, systick interrupt, and set the alternate clock source, which is the CPU freq/4
    systick->reload = 4999999; // set systick period to be 1 second, given that the CPU freq is 20 MHz, and the alternate clock is 5 MHz
    systick->current = 0; // clear systick counter when initializing

    // enable clock to UART 0, then enable port A for UART 0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // enable clock to UART 5 and 7, enable port E for UART 5 and 7
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART5);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART7);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    // configure pins for the UARTs:
    // UART 0
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    // UART 5
    GPIOPinConfigure(GPIO_PE4_U5RX);
    GPIOPinConfigure(GPIO_PE5_U5TX);
    GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    // UART 7
    GPIOPinConfigure(GPIO_PE0_U7RX);
    GPIOPinConfigure(GPIO_PE1_U7TX);
    GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // set UART baudrates to 115200
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|UART_CONFIG_PAR_NONE));
    UARTConfigSetExpClk(UART5_BASE, SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|UART_CONFIG_PAR_NONE));
    UARTConfigSetExpClk(UART7_BASE, SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|UART_CONFIG_PAR_NONE));

    //DEBUG
    UARTCharPut(UART0_BASE, '\r');

    // enable UART7 RX interrupts
    //IntRegister(INT_UART7, uart7_handler);
    IntEnable(INT_UART7);
    UARTIntEnable(UART7_BASE, UART_INT_RX);

    printInit();

    ring_buffer_init(&ring_buffer, buffer_data, RING_BUFFER_SIZE);

    SysTickIntRegister(SysTick_Handler); // registers SysTick_Handler for interrupt
    IntMasterEnable();

    // wait for interrupts to happen...

    char xmlTempBuf[11]; // size of xml temperature message is 11, including the opening and closing tag, the temperature, and an termination character
    char xmlIndex = 0;
    bool opened = 0;
    bool closing = 0;

    while (1){

        if(ring_buffer_read(&ring_buffer, xmlTempBuf[xmlIndex])){
            // if a character was read, enter this
            char c = xmlTempBuf[xmlIndex];
            if(c == '<'){
                if(!opened) opened = 1;
                else closing = 1;
            }
            if(opened && closing && c == '>'){
                opened = 0;
                closing = 0;

                // read the temperature
                int temperature = 0;
                temperature += atoi(xmlTempBuf[3]) * 10; // tens place
                temperature += atoi(xmlTempBuf[4]); // ones place
                addTemp(temperature);
            }


            //adjust index
            xmlIndex++;
            if(xmlIndex++ == 10) xmlIndex = 0;

        }

    }
}
