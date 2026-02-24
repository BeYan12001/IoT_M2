#include "event.h"

void event_post_front(struct event *evt, int *event_count, int *total_events, struct event **ready_head)
{
  if (evt->posted)
    return;
  evt->posted = TRUE;
  evt->next = *ready_head;
  *ready_head = evt;
  (*event_count)++;
  (*total_events)++;   // <-- incrémente à chaque événement posté
}

struct event *event_pop(int *event_count, struct event **ready_head)
{
  struct event *evt = *ready_head;
  if (evt == NULL)
    return NULL;
  *ready_head = evt->next;
  evt->next = NULL;
  evt->posted = FALSE;
  (*event_count)--;  // compteur global
  return evt;
}