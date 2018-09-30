#include <stdlib.h>
#include "eusci.h"

PC_Buffer *tx_buf[NUM_INTERFACES], *rx_buf[NUM_INTERFACES];

PC_Buffer *eusci_get_tx(EUSCI_A_Type* eusci)
{
    // TODO, use other interfaces
    if (eusci != SERIAL_DEBUG)
    {
        return NULL;
    }

    return tx_buf[0];
}

PC_Buffer *eusci_get_rx(EUSCI_A_Type* eusci)
{
    // TODO, use other interfaces
    if (eusci != SERIAL_DEBUG)
    {
        return NULL;
    }

    return rx_buf[0];
}

void eusci_init(void)
{
    // Configure UART pins
    P1->SEL0 |= BIT2 | BIT3;                // set 2-UART pin as secondary function

    // Configure UART
    EUSCI_A0->CTLW0 |= EUSCI_A_CTLW0_SWRST; // Put eUSCI in reset
    EUSCI_A0->CTLW0 = EUSCI_A_CTLW0_SWRST | // Remain eUSCI in reset
            EUSCI_B_CTLW0_SSEL__SMCLK;      // Configure eUSCI clock source for SMCLK

    // Baud Rate calculation
    // N = fBRCLK/115200 = 104.167 (> 16), N/16 = 6.5104
    // OS16 = 1, UCBRx = INT(N/16) = 6,
    // UCBRFx = INT([(N/16) - INT(N/16)] * 16) = 8
    // UCBRSx = look-up 0.5104 = 0xAA
    EUSCI_A0->BRW = 6;
    EUSCI_A0->MCTLW = (0xAA << EUSCI_A_MCTLW_BRS_OFS) |
                      (0x8 << EUSCI_A_MCTLW_BRF_OFS) |
                      EUSCI_A_MCTLW_OS16;

    EUSCI_A0->CTLW0 &= ~EUSCI_A_CTLW0_SWRST; // Initialize eUSCI
    EUSCI_A0->IFG &= ~EUSCI_A_IFG_RXIFG;     // Clear eUSCI RX interrupt flag
    EUSCI_A0->IE |= EUSCI_A_IE_RXIE;         // Enable USCI_A0 RX interrupt

    tx_buf[0] = (PC_Buffer *) malloc(sizeof(PC_Buffer));
    rx_buf[0] = (PC_Buffer *) malloc(sizeof(PC_Buffer));
    if (!tx_buf[0] || !rx_buf[0])
        return;
    if (!pc_buffer_init(tx_buf[0], 64))
        return;
    pc_buffer_init(rx_buf[0], 64);
}

int _getc(EUSCI_A_Type* eusci, bool block, char *c)
{
    PC_Buffer *rx;

    // TODO, use other interfaces
    if (eusci != SERIAL_DEBUG)
    {
        return 1;
    }

    rx = rx_buf[0];

    /* check if a character can be retrieved */
    if (pc_buffer_empty(rx)) {
        if (!block) return 1;
        while (pc_buffer_empty(rx)) {;}
    }

    /* safely write the retrieved character */
    __disable_irq();
    pc_buffer_remove(rx, c);
    __enable_irq();

    return 0;
}

int _putc(EUSCI_A_Type* eusci, bool block, char data)
{
    PC_Buffer *tx;

    // TODO, use other interfaces
    if (eusci != SERIAL_DEBUG)
    {
        return 1;
    }

    tx = tx_buf[0];

    /* check if a character can be added */
    if (pc_buffer_full(tx)) {
        if (!block) return 1;
        while (pc_buffer_full(tx)) {;}
    }

    /* safely add the desired character */
    __disable_irq();
    pc_buffer_add(tx, data);
    __enable_irq();

    /* set TX-empty interrupt enable flag */
    eusci->IE |= EUSCI_A_IE_TXIE;

    return 0;
}

void eusci_a_handler(EUSCI_A_Type* eusci, PC_Buffer *tx, PC_Buffer *rx,
                     char *prev, char *prev2)
{
    char curr;

    /* character received */
    if (eusci->IFG & EUSCI_A_IFG_RXIFG) {
        curr = eusci->RXBUF;
        
        /* backspace */
        if (curr == 0x08 || curr == 0x7F) {
            if (!pc_buffer_empty(rx)) {
                rx->produce_count--;

                /* delete the character in console */
                if (!pc_buffer_full(tx)) pc_buffer_add(tx, 0x08);
                if (!pc_buffer_full(tx)) pc_buffer_add(tx, ' ');
                if (!pc_buffer_full(tx)) pc_buffer_add(tx, 0x08);
                eusci->IE |= EUSCI_A_IE_TXIE;
            }
        }
        
        /* otherwise add the character, don't allow arrow keys or other escaped characters */
        else if ((*prev != 0x5B && *prev2 != 0x1B) && curr != 0x1B && curr != 0x5B) {
            if (NEWLINE_GUARD(curr, *prev)) rx->message_available++;
            if (!pc_buffer_full(rx)) pc_buffer_add(rx, curr);
            if (!pc_buffer_full(tx)) {
                pc_buffer_add(tx, curr);
                if (curr == '\r') pc_buffer_add(tx, '\n');
                eusci->IE |= EUSCI_A_IE_TXIE;
            }
        }

        *prev2 = *prev;
        *prev = curr;
    }
    
    /* character ready to be sent */
    if (eusci->IFG & EUSCI_A_IFG_TXIFG) {
        if (!pc_buffer_empty(tx))
            pc_buffer_remove(tx, (char *) &eusci->TXBUF);
        else
            eusci->IE &= ~EUSCI_A_IE_TXIE;
    }
}

// UART interrupt service routine
void EUSCIA0_IRQHandler(void)
{
    static char prev1 = '\0', prev2 = '\0';

    /*
    if (EUSCI_A0->IFG & EUSCI_A_IFG_RXIFG)
    {
        // Check if the TX buffer is empty first
        while(!(EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG));

        // Echo the received character back
        EUSCI_A0->TXBUF = EUSCI_A0->RXBUF;
    }
    */

    eusci_a_handler(EUSCI_A0, tx_buf[0], rx_buf[0], &prev1, &prev2);
}
