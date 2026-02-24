#ifndef EVENT_H
#define EVENT_H

#include <stdint.h>
#include "main.h"

struct event
{
  void *cookie;
  void (*react)(void *cookie);
  uint64_t eta; // Estimated Time of Arrival
  struct event *next;
  volatile bool_t posted;
  uint32_t event_count;
};

void event_post_front(struct event *evt, int *event_count, int *total_events, struct event **ready_head);

struct event *event_pop(int *event_count, struct event **ready_head);

#endif 