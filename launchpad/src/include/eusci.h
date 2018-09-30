#ifndef _EUSCI_H__
#define _EUSCI_H__

#include "msp.h"
#include "pcbuffer.h"
#include <stdbool.h>

#define SERIAL_DEBUG EUSCI_A0
#define SERIAL_RADIO EUSCI_A2

#define NUM_INTERFACES 2

void eusci_init(void);

int _getc(EUSCI_A_Type* eusci, bool block, char *c);
int _putc(EUSCI_A_Type* eusci, bool block, char data);
PC_Buffer *eusci_get_tx(EUSCI_A_Type* eusci);
PC_Buffer *eusci_get_rx(EUSCI_A_Type* eusci);

#endif
