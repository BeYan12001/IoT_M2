#include "main.h"
#include "uart.h"
#include "timer.h"
#include "isr.h"
#include "ring.h"

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

void clear_line()
{
  uart_send(UART0, 27);
  uart_send(UART0, '[');
  uart_send(UART0, '2');
  uart_send(UART0, 'K');
}

void clear_screen()
{
  uart_send(UART0, 27);
  uart_send(UART0, '[');
  uart_send(UART0, '2');
  uart_send(UART0, 'J');
  uart_send(UART0, 27);
  uart_send(UART0, '[');
  uart_send(UART0, 'H'); // home cursor
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
    clear_screen();
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

char line[80];
uint8_t offset;

void process_ring()
{
  while (!ring_empty())
  {
    uint8_t code = ring_get();

    uart_send(UART0, code);

    if (code == '\r' || code == '\n')
    {
      shell(line, offset);
      offset = 0;
    }
    else
    {
      line[offset++] = (char)code;
    }
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

  irqs_setup();
  uart0_irq_init();
  core_enable_interrupts();

  while (1)
  {

    // while (last< offset)
    // wfi(); // plutot halt();
    for (;;)
    {
      process_ring();
      core_disable_interrupts();
      if (ring_empty())
      {
        core_enable_interrupts();
        wfi();
      }
      else
      {
        core_enable_interrupts();
      }
    }

    // utliser circlar buffer, ring, producteur consommateur, LOCK free Ring
  }
}
