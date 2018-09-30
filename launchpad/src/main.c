//******************************************************************************
//   MSP432P401 Demo - eUSCI_A0 UART echo at 9600 baud using BRCLK = 12MHz
//
//  Description: This demo echoes back characters received via a PC serial port.
//  SMCLK/ DCO is used as a clock source and the device is put in LPM0
//  The auto-clock enable feature is used by the eUSCI and SMCLK is turned off
//  when the UART is idle and turned on when a receive edge is detected.
//  Note that level shifter hardware is needed to shift between RS232 and MSP
//  voltage levels.
//
//  The example code shows proper initialization of registers
//  and interrupts to receive and transmit data.
//
//                MSP432P401
//             -----------------
//         /|\|                 |
//          | |                 |
//          --|RST              |
//            |                 |
//            |                 |
//            |     P1.3/UCA0TXD|----> PC (echo)
//            |     P1.2/UCA0RXD|<---- PC
//            |                 |
//
//   William Goh
//   Texas Instruments Inc.
//   June 2016 (updated) | June 2014 (created)
//   Built with CCSv6.1, IAR, Keil, GCC
//******************************************************************************
#include "msp.h"
#include "console.h"
#include "eusci.h"
#include "printed_circuit_car.h"
#include <stdio.h>

void init(void)
{
    WDT_A->CTL = WDT_A_CTL_PW |             // Stop watchdog timer
            WDT_A_CTL_HOLD;

    CS->KEY = CS_KEY_VAL;                   // Unlock CS module for register access
    CS->CTL0 = 0;                           // Reset tuning parameters
    CS->CTL0 = CS_CTL0_DCORSEL_3;           // Set DCO to 12MHz (nominal, center of 8-16MHz range)
    CS->CTL1 = CS_CTL1_SELA_2 |             // Select ACLK = REFO
            CS_CTL1_SELS_3 |                // SMCLK = DCO
            CS_CTL1_SELM_3;                 // MCLK = DCO
    CS->KEY = 0;                            // Lock CS module from unintended accesses

    P1->DIR |= BIT0;                        // P1.0 set as output
}

volatile unsigned int ticks = 0;

void SysTick_Handler(void)
{
    ticks++;
}

void delay_ms(unsigned int period) {
	unsigned int curr_count = ticks;
	while (ticks < curr_count + period) {;}
}

void blink_handler(unsigned int blink_int)
{
	static unsigned int curr = 0, prev = 0;
	curr = ticks / blink_int;
	if (curr != prev) P1->OUT ^= BIT0;
	prev = curr;
}

void print_prompt(void)
{
	fputs("[pcc] $ ", stdout);
	fflush(stdout);
}

int main(void)
{

    init();
    SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 1000);
    io_init();
    eusci_init();

    // Enable sleep on exit from ISR
    //SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;

    // Enable global interrupt
    __enable_irq();

    // Enable eUSCIA0 interrupt in NVIC module
    //NVIC->ISER[0] = 1 << ((EUSCIA0_IRQn | EUSCIA2_IRQn) & 31);
    //NVIC->ISER[0] = 1 << ((EUSCIA0_IRQn) & 31);
    NVIC_SetPriority(EUSCIA0_IRQn, 4);
    NVIC_EnableIRQ(EUSCIA0_IRQn);
    NVIC_SetPriority(EUSCIA2_IRQn, 3);
    NVIC_EnableIRQ(EUSCIA2_IRQn);

    printf("\r\nprogram start\r\n");
    print_prompt();
    char temp;
    while (1)
    {
        blink_handler(1000);
        check_input(eusci_get_rx(SERIAL_DEBUG), &entry, &print_prompt);
        //check_input(eusci_get_rx(SERIAL_RADIO), &entry, &print_prompt);
        if (ticks % 250)
        {
            _putc(SERIAL_RADIO, false, 'a');
            if (!_getc(SERIAL_RADIO, false, &temp))
            {
                _putc(SERIAL_DEBUG, true, temp);
            }
        }
    }

    // Enter LPM0
    //__sleep();
    //__no_operation();                       // For debugger
}
