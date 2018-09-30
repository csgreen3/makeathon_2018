#include <stdlib.h>
#include "eusci.h"

PC_Buffer *tx_buf[NUM_INTERFACES], *rx_buf[NUM_INTERFACES];

PC_Buffer *eusci_get_tx(EUSCI_A_Type* eusci)
{
    if (eusci == SERIAL_DEBUG) return tx_buf[0];
    return tx_buf[1];
}

PC_Buffer *eusci_get_rx(EUSCI_A_Type* eusci)
{
    if (eusci == SERIAL_DEBUG) return rx_buf[0];
    return rx_buf[1];
}

void radio_init(void)
{
    // radio on A2
    P3->SEL0 |= BIT2 | BIT3;

    // Configure UART
    EUSCI_A2->CTLW0 |= EUSCI_A_CTLW0_SWRST; // Put eUSCI in reset
    EUSCI_A2->CTLW0 = EUSCI_A_CTLW0_SWRST | // Remain eUSCI in reset
            EUSCI_A_CTLW0_SSEL__SMCLK;      // Configure eUSCI clock source for SMCLK

    // Baud Rate calculation
    // 12000000/(16*9600) = 78.125
    // Fractional portion = 0.125
    // User's Guide Table 21-4: UCBRSx = 0x10
    // UCBRFx = int ( (78.125-78)*16) = 2
    EUSCI_A2->BRW = 78;                     // 12000000/16/9600
    EUSCI_A2->MCTLW = (2 << EUSCI_A_MCTLW_BRF_OFS) |
            EUSCI_A_MCTLW_OS16;

    EUSCI_A2->CTLW0 &= ~EUSCI_A_CTLW0_SWRST; // Initialize eUSCI
    EUSCI_A2->IFG &= ~EUSCI_A_IFG_RXIFG;     // Clear eUSCI RX interrupt flag
    EUSCI_A2->IE |= EUSCI_A_IE_RXIE;         // Enable USCI_A0 RX interrupt

    tx_buf[1] = (PC_Buffer *) malloc(sizeof(PC_Buffer));
    rx_buf[1] = (PC_Buffer *) malloc(sizeof(PC_Buffer));
    if (!tx_buf[1] || !rx_buf[1])
        return;
    if (!pc_buffer_init(tx_buf[1], 64))
        return;
    pc_buffer_init(rx_buf[1], 64);
}

void eusci_init(void)
{
    // usb on A0
    P1->SEL0 |= BIT2 | BIT3;                // set 2-UART pin as secondary function

    // Configure UART
    EUSCI_A0->CTLW0 |= EUSCI_A_CTLW0_SWRST; // Put eUSCI in reset
    EUSCI_A0->CTLW0 = EUSCI_A_CTLW0_SWRST | // Remain eUSCI in reset
            EUSCI_A_CTLW0_SSEL__SMCLK;      // Configure eUSCI clock source for SMCLK

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

    radio_init();
}

int _getc(EUSCI_A_Type* eusci, bool block, char *c)
{
    PC_Buffer *rx = eusci_get_rx(eusci);

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
    PC_Buffer *tx = eusci_get_tx(eusci);

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

void EUSCIA0_IRQHandler(void)
{
    static char prev1 = '\0', prev2 = '\0';
    __disable_irq();
    eusci_a_handler(EUSCI_A0, tx_buf[0], rx_buf[0], &prev1, &prev2);
    __enable_irq();
}

void EUSCIA2_IRQHandler(void)
{
    static char prev1 = '\0', prev2 = '\0';
    __disable_irq();
    eusci_a_handler(EUSCI_A2, tx_buf[1], rx_buf[1], &prev1, &prev2);
    __enable_irq();
}
