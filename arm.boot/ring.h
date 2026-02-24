#ifndef RING_H
#define RING_H

#include <stdint.h>
#include "main.h"



/* Simple single-producer (IRQ) / single-consumer (main) ring buffer */
#define MAX_CHARS 512u

extern volatile uint32_t head;
extern volatile uint32_t tail;
extern volatile uint8_t buffer[MAX_CHARS];

bool_t ring_empty(void);
bool_t ring_full(void);
void ring_put(uint8_t bits);
uint8_t ring_get(void);


#endif
