#include "timer.h"

// ARM926PXP Timer registers
#define TIMER0_BASE 0x101E2000
#define TIMER1_BASE 0x101E2020
#define TIMER2_BASE 0x101E3000
#define TIMER3_BASE 0x101E3020

// System Controller registers
#define SYSCON_BASE 0x101E0000
#define TIMCLK_OFFSET 0x04  // Timer Clock Control register offset

typedef struct {
    volatile uint32_t Load;      // 0x00
    volatile uint32_t Value;     // 0x04
    volatile uint32_t Control;   // 0x08
    volatile uint32_t IntClr;    // 0x0C
    volatile uint32_t RIS;       // 0x10
    volatile uint32_t MIS;       // 0x14
    volatile uint32_t BGLoad;    // 0x18
} timer_regs_t;

static timer_regs_t* timer = (timer_regs_t*)TIMER0_BASE;
static volatile uint32_t tick_count = 0;

void timer_init(void) {
    // Configurer le System Controller pour utiliser 1MHz au lieu de 32KHz
    volatile uint32_t* timclk = (volatile uint32_t*)(SYSCON_BASE + TIMCLK_OFFSET);
    *timclk = 0x01;  // 0x01 pour 1MHz, 0x00 pour 32KHz
    
    // Configuration du Timer 0
    timer->Load = 0x00;
    timer->Control = 0x00;           // Désactiver le timer
    timer->Load = 0xFFFFFFFF;        // Valeur maximale pour compteur 32-bit
    timer->Control = 0xE2;           // Enable=1, Periodic=1, IntEnable=0, 32-bit, Prescale=0
    
    tick_count = 0;
}

uint32_t timer_get_ticks(void) {
    // Le timer compte à rebours, donc on calcule les ticks écoulés
    return (0xFFFFFFFF - timer->Value);
}

uint32_t timer_get_seconds(void) {
    // Diviseur pour 1MHz
    return timer_get_ticks() / 1000000;
}