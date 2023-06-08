// 2023/04/19 - UART5 with TivaWare and Interrupts
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h" // needed for definition of UART5_BASE
#include "inc/hw_ints.h" // needed for INT_UART5
#include "driverlib/pin_map.h" // needed for GPIO_PA0_U0RX and GPIO_PA1_U0TX
// these are needed for the various TivaWare calls below
#include "driverlib/fpu.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"

int main(void) {

    FPUEnable() ;
    FPULazyStackingEnable() ;

    // wait for interrupts to happen...
    while (1)
        ;
}
SysCtlPeripheralEnable(SYSCTL_PERIPH_UART5) ;
   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE) ;
   GPIOPinConfigure(GPIO_PE4_U5RX) ; // green
   GPIOPinConfigure(GPIO_PE5_U5TX) ; // white
   GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_PIN_5) ;
   UARTConfigSetExpClk(UART5_BASE, SysCtlClockGet(), 115200,
        (UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|UART_CONFIG_PAR_NONE) ) ;
   puts("Hello from UART5!\r\n") ;
   void puts ( char *cp ) {
       while( *cp!='\0' ) {
           UARTCharPut(UART5_BASE,*cp++) ;
       }
   }