#include "main.h"
#include "uart.h"
#include "timer.h"
#include "isr.h"
#include "ring.h"
#include "terminal_funct.h"

/*
 * Define ECHO_ZZZ to have a periodic reminder that this code is polling
 * the UART, actively. This means the processor is running continuously.
 * Polling is of course not the way to go, the processor should halt in
 * a low-power state and wake-up only to handle an interrupt from the UART.
 * But this would require setting up interrupts...
 */
#define ECHO_ZZZ

/* PL011 UART interrupt registers */
#define UART_IMSC 0x38 // Interrupt Mask Set/Clear Register
#define UART_ICR 0x44  // Interrupt Clear Register, Dire au device d'effacer le flag interruptions
#define UART_RIS 0x3C  // Raw Interrupt Status Register
#define UART_IFLS 0x34 // Interrupt FIFO Level Select Register,

/* PL011 UART interrupt bits */
#define UART_RXIM (1 << 4) // Receive Interrupt Mask
#define UART_RXIC (1 << 4) // Receive Interrupt Clear
#define UART_TXIM (1 << 5) // Transmit Interrupt Mask
#define UART_TXIC (1 << 5) // Transmit Interrupt Clear

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

static timer_regs_t *timer1 = (timer_regs_t *)TIMER1_BASE;
static volatile bool_t cursor_is_visible = FALSE;

void panic()
{
  while (1)
    ;
}

// Fonction simple pour convertir un nombre en chaîne
void uint_to_string(uint32_t num, char *buffer)
{
  char temp[20];
  int i = 0;

  if (num == 0)
  {
    buffer[0] = '0';
    buffer[1] = '\0';
    return;
  }

  while (num > 0)
  {
    temp[i++] = '0' + (num % 10);
    num /= 10;
  }

  int j = 0;
  while (i > 0)
  {
    buffer[j++] = temp[--i];
  }
  buffer[j] = '\0';
}

void shell(char *line, uint8_t offset)
{
  if (offset == 5 &&
      line[0] == 'c' &&
      line[1] == 'l' &&
      line[2] == 'e' &&
      line[3] == 'a' &&
      line[4] == 'r')
  {
    clear_screen(UART0);
  }
  else
  {
    uart_send(UART0, '\r');
    uart_send(UART0, '\n');
  }
}

static void uart0_irq_handler(uint32_t irq, void *cookie)
{
  (void)irq;
  (void)cookie;
  uint8_t c;

  uint8_t code = uart_receive(UART0, &c);
  while (code)
  {
    ring_put(c);
    code = uart_receive(UART0, &c);
  }

  /* Clear RX interrupt */
  mmio_write32(UART0, UART_ICR, UART_RXIC); // Netooyer le flag d'interruption
}

static void uart0_irq_init(void)
{
  /* Clear any pending interrupts */
  mmio_write32(UART0, UART_ICR, 0x7FF); // Interrupt Clear Register, UARTICR on page 3-21
  /* Enable RX interrupt in the UART */
  mmio_set(UART0, UART_IMSC, UART_RXIM); // Interrupt Mask Set/Clear Register, UARTIMSC on page 3-17
  /* Enable UART0 interrupt in the VIC */
  irq_enable(UART0_IRQ, uart0_irq_handler, NULL);
}

static void timer1_irq_handler(uint32_t irq, void *cookie)
{
  (void)irq;
  (void)cookie;

  if (cursor_is_visible)
  {
    cursor_hide(UART0);
    cursor_is_visible = FALSE;
  }
  else
  {
    cursor_show(UART0);
    cursor_is_visible = TRUE;
  }

  /* Ack timer interrupt */
  timer1->IntClr = 1;
}

static void timer1_irq_init(void)
{
  /*
   * 1 MHz timer clock (configured in timer_init()).
   * 500000 ticks -> 0.5s, so cursor toggles every 500 ms.
   */
  timer1->Control = 0x00;
  timer1->Load = 200000;
  timer1->BGLoad = 200000;
  timer1->IntClr = 1;
  /* Enable=1, Periodic=1, IntEnable=1, 32-bit counter */
  timer1->Control = 0xE2;

  irq_enable(TIMER1_IRQ, timer1_irq_handler, NULL);
}

char line[80];
uint8_t offset;

void process_ring()
{
  static uint8_t esc_state = 0; // 0: normal, 1: got ESC, 2: got ESC[, 3: got ESC[3
  while (!ring_empty())
  {
    uint8_t code = ring_get();

    if (esc_state == 0)
    {
      if (code == 27)
      { // ESC
        esc_state = 1;
        continue;
      }
    }
    else if (esc_state == 1)
    {
      if (code == '[')
      {
        esc_state = 2;
        continue;
      }
      esc_state = 0; // pas une séquence valide
    }
    else if (esc_state == 2)
    {
      if (code == 'D')
      {
        cursor_left(UART0);
      }
      else if (code == 'C')
      {
        cursor_right(UART0);
      }
      else if (code == '3')
      {
        esc_state = 3;
        continue;
      }
      esc_state = 0;
      continue; // ne pas échoer dans la ligne
    }
    else if (esc_state == 3)
    {
      if (code == '~')
      {
        delete_char(UART0);
      }
      esc_state = 0;
      continue; // ne pas échoer dans la ligne
    }

    if (code == '\b')
    {
      back_space(UART0);
      if (offset > 0)
      {
        offset--;
      }
      continue;
    }

    // traitement normal
    if (code == '\r' || code == '\n')
    {
      shell(line, offset);
      offset = 0;
    }
    else
    {
      if (offset < sizeof(line) - 1)
      {
        line[offset++] = (char)code;
      }
    }
    uart_send(UART0, code);
  }
}

/**
 * This is the C entry point, upcalled once the hardware has been setup properly
 * in assembly language, see the startup.s file.
 */
void _start()
{
  uart_send_string(UART0, "\nFor information:\n");
  uart_send_string(UART0, "  - Quit with \"C-a c\" to get to the QEMU console.\n");
  uart_send_string(UART0, "  - Then type in \"quit\" to stop QEMU.\n");
  uart_send_string(UART0, "\n -- My console -- \n");

  timer_init();

  irqs_setup();
  uart0_irq_init();
  timer1_irq_init();
  core_enable_interrupts();

  cursor_hide(UART0);

  while (1)
  {

    process_ring();
    core_disable_interrupts();
    if (ring_empty())
    {
      wfi();
      core_enable_interrupts();
    }
    else
    {
      core_enable_interrupts();
    }

    // utliser circlar buffer, ring, producteur consommateur, LOCK free Ring
  }
}
