#include "ring.h"


void ring_init(ring_t *r)
{
  r->head = 0;
  r->tail = 0;
}

bool_t ring_empty(const ring_t *r)
{
  return (r->head == r->tail) ? TRUE : FALSE;
}

void ring_put(ring_t *r, uint8_t v)
{
  uint32_t next = (r->head + 1u) & RING_MASK;
  if (next == r->tail)
    return; // drop if full
  r->buf[r->head] = v;
  r->head = next;
}

uint8_t ring_get(ring_t *r)
{
  if (r->head == r->tail)
    return 0;
  uint8_t v = r->buf[r->tail];
  r->tail = (r->tail + 1u) & RING_MASK;
  return v;
}
