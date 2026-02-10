#ifndef RING_H
#define RING_H

#include <stdint.h>
#include "main.h"



/* Simple single-producer (IRQ) / single-consumer (main) ring buffer */
#define RING_SIZE 128u
#define RING_MASK (RING_SIZE - 1u)


typedef struct
{
  uint8_t buf[RING_SIZE];
  volatile uint32_t head;
  volatile uint32_t tail;
} ring_t;

void ring_init(ring_t *r);

bool_t ring_empty(const ring_t *r);

void ring_put(ring_t *r, uint8_t v);

uint8_t ring_get(ring_t *r);


#endif
