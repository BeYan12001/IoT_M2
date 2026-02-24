#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/* SP804 Timer1 registers (used for cursor blinking) */
#define TIMER1_BASE 0x101E2020

typedef struct
{
  volatile uint32_t Load;    // 0x00
  volatile uint32_t Value;   // 0x04
  volatile uint32_t Control; // 0x08
  volatile uint32_t IntClr;  // 0x0C
  volatile uint32_t RIS;     // 0x10
  volatile uint32_t MIS;     // 0x14
  volatile uint32_t BGLoad;  // 0x18
} timer_regs_t;

void timer_init(void);
uint32_t timer_get_ticks(void);
uint32_t timer_get_seconds(void);

#endif