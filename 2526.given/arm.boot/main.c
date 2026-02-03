#include "main.h"
#include "uart.h"
#include "timer.h"
#include "isr.h"

/*
 * Define ECHO_ZZZ to have a periodic reminder that this code is polling
 * the UART, actively. This means the processor is running continuously.
 * Polling is of course not the way to go, the processor should halt in
 * a low-power state and wake-up only to handle an interrupt from the UART.
 * But this would require setting up interrupts...
 */
#define ECHO_ZZZ

/* PL011 UART interrupt registers */
#define UART_IMSC 0x38             // Interrupt Mask Set/Clear Register
#define UART_ICR  0x44             // Interrupt Clear Register, Dire au device d'effacer le flag interruptions
#define UART_RIS  0x3C             // Raw Interrupt Status Register
#define UART_IFLS 0x34            // Interrupt FIFO Level Select Register, 

/* PL011 UART interrupt bits */
#define UART_RXIM (1<<4)          // Receive Interrupt Mask
#define UART_RXIC (1<<4)          // Receive Interrupt Clear 
#define UART_TXIM (1<<5)          // Transmit Interrupt Mask
#define UART_TXIC (1<<5)          // Transmit Interrupt Clear

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

static void uart0_irq_handler(uint32_t irq, void *cookie)
{
  (void)irq;
  (void)cookie;

  uint8_t c;
  while (uart_receive(UART0, &c)) {
    if (c == 13) {
      uart_send(UART0, '\r');
      uart_send(UART0, '\n');
    } else {
      uart_send(UART0, c);
    }
  }

  /* Clear RX interrupt */
  mmio_write32(UART0, UART_ICR, UART_RXIC);    // Netooyer le flag d'interruption
}

static void uart0_irq_init(void)
{
  /* Clear any pending interrupts */
  mmio_write32(UART0, UART_ICR, 0x7FF); // Interrupt Clear Register, UARTICR on page 3-21
  /* Enable RX interrupt in the UART */
  mmio_set(UART0, UART_IMSC, UART_RXIM);    // Interrupt Mask Set/Clear Register, UARTIMSC on page 3-17
  /* Enable UART0 interrupt in the VIC */
  irq_enable(UART0_IRQ, uart0_irq_handler, NULL);
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
  uart_send_string(UART0, "\nHello world!\n");

  irqs_setup();
  uart0_irq_init();
  irqs_enable();
 
  char buffer[10][15];
  char bufferTimer[20];
  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 15; j++)
    {
      buffer[i][j] = '.';
    }
  }

  uint32_t last_second = 0;

  while (1)
  {
    wfi();
  }
}

