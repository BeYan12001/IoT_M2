#ifndef FUNCTION_MAIN_H
#define FUNCTION_MAIN_H

#include "main.h"
#include "uart.h"
#include "timer.h"
#include "event.h"
#include "ring.h"
#include "terminal_funct.h"

#include <stdint.h>

void shell(char *cmd_line, uint8_t cmd_len);  
void process_ring(char *input_line, uint8_t *input_offset);

#endif