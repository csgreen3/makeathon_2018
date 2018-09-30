#ifndef _CONSOLE__H__
#define _CONSOLE__H__

#include "pcbuffer.h"

#define MAX_TOKENS	16

void check_input(PC_Buffer *rx, void (*exec_command)(int,char**),
                 void (*print_prompt)(void));
void process_input(char *buffer, void (*exec_command)(int,char**));

#endif
