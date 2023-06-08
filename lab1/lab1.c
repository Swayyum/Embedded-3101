#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "lab1-mealy.h"

#define NVIC_SYS_PRI3_R (*((volatile uint32_t *)0xE000ED20))  // address of the NVIC_SYS_PRI3 register

#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line){
    while(1);
}
#endif

struct nvic_st_s{
    volatile unsigned long ctrl;
    volatile unsigned long reload;
    volatile unsigned long current;
};

// state = 0; // the initial state of the LED encoded as RGB (000 = black/off)
char buttonPressed = 0; // set to 1 if the button was pressed during systick period

// writes the current state to the D1 LED
void stateToLED(){
    // convert the state encoding to a mask for the data register
    unsigned int LED_MASK = 0;
    if(state & RED_MASK) LED_MASK = LED_MASK | GPIO_PIN_1;
    if(state & BLUE_MASK) LED_MASK = LED_MASK | GPIO_PIN_2;
    if(state & GREEN_MASK) LED_MASK = LED_MASK | GPIO_PIN_3;

    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, LED_MASK); // red
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, LED_MASK); // blue
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, LED_MASK); // green
}

void SysTick_Handler(void){

    if((GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4) == 0)){ // if the button was pressed at any point during the systick period
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_PIN_2); // debug LED
        state = nextState(state, buttonPressed);
        stateToLED();

        // loop while button is still pressed
        while(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4) == 0){
        }
    }
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0);

    buttonPressed = 0;
}

int main(void){

    // set the clock to 80 MHz
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    struct nvic_st_s *systick = (struct nvic_st_s *)0xE000E010;

    NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & 0x00FFFFFF) | 0x30000000;

    SysTickIntRegister(SysTick_Handler); // registers SysTick_Handler for interrupt

    systick->ctrl |= (0x1 | 0x2 | 0x4); // enable systick, systick interrupt, and set the clock source to the processor running @ 80 MHz
    systick->reload = 3999999; // set systick period to be 50 ms
    systick->current = 0; // clear systick counter when initializing

    // enable the GPIO port for the LEDs and switch
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    // for debug LED:
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_2); // Red LED

    // Check if the peripheral access is enabled
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)){
    }

    // Set inputs and outputs:
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1); // Red LED
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2); // Blue LED
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3); // Green LED
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4); // Switch 1 Button

    // enable the pull up resistor for the SW1 button input
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    while(1){
        if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4) == 0){ // if button is pressed:
            buttonPressed = 1;
        }
//        else{
//            // Turn off the LED
//            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
//        }
    }
}
