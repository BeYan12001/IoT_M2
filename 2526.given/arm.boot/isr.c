#include "main.h"
#include "isr.h"
#include "isr-mmio.h"

extern void _irqs_setup(void);
extern void _irqs_enable(void);
extern void _irqs_disable(void);
extern void _wfi(void);

typedef struct {
  void (*callback)(uint32_t, void *);
  void *cookie;
} irq_entry_t;

static irq_entry_t irq_table[NIRQS];

void irqs_setup() {
  _irqs_setup();
}

void core_enable_interrupts() { 
  _irqs_enable();
}

void core_disable_interrupts() {
  _irqs_disable();
}

void wfi(void) {
  _wfi();
}

void irq_enable(uint32_t irq, void (*callback)(uint32_t, void *), void *cookie) {
  if (irq >= NIRQS)
    return;

  irq_table[irq].callback = callback;       // On remplit la table des IRQs callback
  irq_table[irq].cookie = cookie;           // On remplit la table des IRQs cookie

  /* Force IRQ (not FIQ) */
  mmio_clear((void *)VIC_BASE_ADDR, VICINTSELECT, (1u << irq));    // On force IRQ (normal)
  /* Enable in VIC */
  mmio_set((void *)VIC_BASE_ADDR, VICINTENABLE, (1u << irq));  // active l’IRQ dans le VIC avec un masque
}

void irq_disable(uint32_t irq) {
  if (irq >= NIRQS)
    return;

  mmio_write32((void *)VIC_BASE_ADDR, VICINTCLEAR, (1u << irq));  // Désactive l’IRQ dans le VIC
  irq_table[irq].callback = NULL;  // on  remet à NULL le callback et le cookie
  irq_table[irq].cookie = NULL;  // on  remet à NULL le callback et le cookie
}

/* Called from exception.s IRQ handler */
void isr_handler(void) {
  uint32_t status = mmio_read32((void *)VIC_BASE_ADDR, VICIRQSTATUS);
  if (status == 0)  // Si aucune interruption active
    return;

  for (uint32_t i = 0; i < NIRQS; i++) {    // On parcourt toutes les IRQ possibles
    if (status & (1u << i)) {    // si un IRQ est active
      if (irq_table[i].callback) {
        irq_table[i].callback(i, irq_table[i].cookie);   // On appelle le callback associé , rempli dans irq_enable
      }
    }
  }
}
