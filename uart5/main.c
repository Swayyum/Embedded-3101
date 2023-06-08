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
