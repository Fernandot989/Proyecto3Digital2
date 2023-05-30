/*
 * University:     Universidad del Valle de Guatemala
 * Course:         Electrónica Digital 2
 * Author:         Carlos Molina (#21253)
 * Description:    Proyecto 03 - TivaWare
 * Date:           11 de mayo de 2023
 */

//-----------------------------------------------------------------------------
//                                 Libraries
//-----------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/pin_map.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/rom.h"
#include "grlib/grlib.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/timer.h"
//#include "drivers/cfal96x64x16.h"


//-----------------------------------------------------------------------------
//                           Variables declarations
//-----------------------------------------------------------------------------
uint8_t parkinglot1_status;
uint8_t parkinglot2_status;
uint8_t parkinglot3_status;
uint8_t parkinglot4_status;

uint8_t paquete = 0;

uint8_t cont = 0;
uint8_t i;
uint32_t period;

unsigned char recompilado;
//-----------------------------------------------------------------------------
//                           Functions declarations
//-----------------------------------------------------------------------------
void main(void);
void semaforo(void);
void InitUART(void);
void Timer0IntHandler(void);
void UART0IntHandler(void);
void UART1IntHandler(void);
void InitialParkingLot(void);
void ParkingLotReading(void);
void ParkingLotUpdate(void);
void SendData(void);
void setup(void);

//-----------------------------------------------------------------------------
//                                 Main Code
//-----------------------------------------------------------------------------
void main(void){

    //----------------OSCILLATOR----------------

    // Oscillator configured to 8MHz
    SysCtlClockSet(SYSCTL_SYSDIV_2|SYSCTL_USE_OSC|SYSCTL_OSC_INT); //Precision Internal Oscillator (16MHz) --> 8MHz

    //----------------PERIPHERALS---------------

    // Peripherals from Port A are enabled
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Peripherals from Port B are enabled
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    // Peripherals from Port D are enabled
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

    // Peripherals from Port E are enabled
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    // Peripherals from Port F are enabled
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    //----------------OUTPUTS----------------

    // Pin 5, Pin 6, Pin 7 of PORT A are assigned as OUTPUTS
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);

    // Pin 4 of PORT B is assigned as OUTPUT
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_4);

    // Pin 2, Pin 3 of PORT D are assigned as OUTPUTS
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_2|GPIO_PIN_3);

    // Pin 4, Pin 5 of PORT E are assigned as OUTPUTS
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_4|GPIO_PIN_5);

    // Pin 1, Pin 2, Pin 3 of PORT F (LEDs pins) are assigned as OUTPUTS
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

    //----------------INPUTS----------------

    // Pin 5 of PORT B is assigned as INPUT
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_5);

    // Pin 1, Pin 2 of PORT E are assigned as INPUTS
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2);

    //----------------PULL-UP---------------
    /*
    // Activate Weak Pull-Up for Pin 1
    GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    */

    // Global Interruptions are enabled
    IntMasterEnable();

    InitUART();

    while(1)
    {
        ParkingLotReading();
        ParkingLotUpdate();
        SendData();
    }
}

//-----------------------------------------------------------------------------
//                                Functions
//-----------------------------------------------------------------------------

void setup(void){
    ;
}

void SendData(void){
    //UARTCharPutNonBlocking(UART1_BASE, paquete);
    UARTCharPut(UART1_BASE, paquete+48);

}

void ParkingLotUpdate(void){

    // Byte-Construction from the Occupied/Available parking lots (Reference --> If LOT 4 = 1, and LOT 3 = 1, and LOT 2 = 1, and LOT 1 = 0 --> 0b1110 or the same as 0b00001110
    paquete = (parkinglot4_status<<3) | (parkinglot3_status<<2) | (parkinglot2_status<<1) | (parkinglot1_status);
}

void ParkingLotReading(void){

    //---------------- PARKING LOT 1 ----------------
    if (!GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_0)){ // If parking lot #1 is OCCUPIED, then...
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6|GPIO_PIN_7, 0x40);
        parkinglot1_status = 0;
    }
    else if (GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_0)){ // If parking lot #1 is AVAILABLE, then...
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6|GPIO_PIN_7, 0x80);
        parkinglot1_status = 1;
    }

    //---------------- PARKING LOT 2 ----------------
    if (!GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_2)){ // If parking lot #2 is OCCUPIED, then...
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5, 0x00);
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, 0x10);
        parkinglot2_status = 0;
    }
    else if (GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_2)){ // If parking lot #2 is AVAILABLE, then...
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5, 0x20);
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, 0x00);
        parkinglot2_status = 1;
    }

    //---------------- PARKING LOT 3 ----------------
    if (!GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_1)){ // If parking lot #3 is OCCUPIED, then...
        GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4|GPIO_PIN_5, 0x10);
        parkinglot3_status = 0;
    }
    else if (GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_1)){ // If parking lot #3 is AVAILABLE, then...
        GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4|GPIO_PIN_5, 0x20);
        parkinglot3_status = 1;
    }

    //---------------- PARKING LOT 4 ----------------
    if (!GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_5)){ // If parking lot #4 is OCCUPIED, then...
        GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_2|GPIO_PIN_3, 0x04);
        parkinglot4_status = 0;
    }
    else if (GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_5)){ // If parking lot #4 is AVAILABLE, then...
        GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_2|GPIO_PIN_3, 0x08);
        parkinglot4_status = 1;
    }

}

void InitialParkingLot(void){
    ;
}

void semaforo(void){

    // Turn the Green LED solely
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x08);
    // Delay for 3s
    SysCtlDelay(8000000);  // A delay takes 3 clock cycles long. Each clock cycle (or instruction cycle as it is referred in the manual) is equivalent to (1/Oscillator Used).

    // Turn the Green LED blinking
    for(i = 0; i<3; i++){

        // Turn the Green LED off
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);

        // Delay for 500ms or 0.5s
        SysCtlDelay(1333333.333);  // A delay takes 3 clock cycles long. Each clock cycle (or instruction cycle as it is referred in the manual) is equivalent to (1/Oscillator Used).

        // Turn the Green LED solely
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x08);

        // Delay for 500ms or 0.5s
        SysCtlDelay(1333333.333);  // A delay takes 3 clock cycles long. Each clock cycle (or instruction cycle as it is referred in the manual) is equivalent to (1/Oscillator Used).
    }

    // Turn on the Yellow LED solely
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0xA);

    // Delay for 3s
    SysCtlDelay(8000000);  // A delay takes 3 clock cycles long. Each clock cycle (or instruction cycle as it is referred in the manual) is equivalent to (1/Oscillator Used).

    // Turn on the Red LED solely
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x02);

    // Delay for 3s
    SysCtlDelay(8000000);  // A delay takes 3 clock cycles long. Each clock cycle (or instruction cycle as it is referred in the manual) is equivalent to (1/Oscillator Used).

    // Turn the Red LED off
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);

    cont = 0;
}

void InitUART(void){

    // Peripherals from Port B are enabled
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    // Peripherals for UART0 are enabled
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);

    // Mapping of the appropriate GPIO Pins to UART1
    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);

    // Pin 0, Pin 1 of PORTB are assigned as UART
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0|GPIO_PIN_1);

    // Configuration of UART1 Parameters (Module UART0, Baud Rate 115200, 8 data bits, 1 stop bit, None parity)
    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|UART_CONFIG_PAR_NONE) );

    // UART1 Interruption Enabled
    IntEnable(INT_UART1);

    // Transmission is set as the reason of the interruption
    UARTIntEnable(UART1_BASE, UART_INT_TX | UART_INT_RT);

    // Enable UART1
    UARTEnable(UART1_BASE);
}

//-----------------------------------------------------------------------------
//                                 Handler
//-----------------------------------------------------------------------------
void Timer0IntHandler(void){
    ;
}

void UART1IntHandler(void){
    UARTIntClear(UART1_BASE, UART_INT_TX | UART_INT_RT);
    //semaforo();
}


void UART0IntHandler(void){
    UARTIntClear(UART0_BASE, UART_INT_RX | UART_INT_RT);

    recompilado = UARTCharGet(UART0_BASE);

    if (recompilado == 'r'){
        if (cont == 1){
            cont = 0;
        }
        else{
            cont = 1;
        }
    }

    else if (recompilado == 'b'){
        if (cont == 2){
            cont = 0;
        }
        else{
            cont = 2;
        }
    }

    else if (recompilado == 'g'){
        if (cont == 3){
            cont = 0;
        }
        else{
            cont = 3;
        }
    }

    else{
        ;
    }

    /*
    // Si hay algo en el buffer lo leemos, si no, seguimos de largo
    newchar = UARTCharGetNonBlocking(UART0_BASE);
    */


}
