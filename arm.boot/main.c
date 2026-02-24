#include "main.h"
#include "uart.h"
#include "event.h"
#include "timer.h"
#include "isr.h"
#include "ring.h"
#include "terminal_funct.h"
#include "function_main.h"

/*
 * Define ECHO_ZZZ to have a periodic reminder that this code is polling
 * the UART, actively. This means the processor is running continuously.
 * Polling is of course not the way to go, the processor should halt in
 * a low-power state and wake-up only to handle an interrupt from the UART.
 * But this would require setting up interrupts...
 */
#define ECHO_ZZZ

int secondes = 0;
int event_count = 0;  // events en attente dans la queue
int total_events = 0; // compteur total, ne décrémente jamais
static struct event *ready_head = NULL;
char input_line[80];
uint8_t input_offset = 0;

static void rx_bottom_handler(void *cookie);
static void blink_bottom_handler(void *cookie);

static timer_regs_t *timer1 = (timer_regs_t *)TIMER1_BASE;
static volatile bool_t cursor_is_visible = FALSE;
static struct event rx_event = {.cookie = NULL, .react = rx_bottom_handler, .eta = 0, .next = NULL, .posted = FALSE};
static struct event blink_event = {.cookie = NULL, .react = blink_bottom_handler, .eta = 0, .next = NULL, .posted = FALSE};

static void sleep_until_next_event(void)
{
  core_disable_interrupts();
  if (ready_head == NULL)
  {
    wfi();
  }
  core_enable_interrupts();
}

void shell(char *cmd_line, uint8_t cmd_len)
{
  if (cmd_len == 5 &&
      cmd_line[0] == 'c' &&
      cmd_line[1] == 'l' &&
      cmd_line[2] == 'e' &&
      cmd_line[3] == 'a' &&
      cmd_line[4] == 'r')
  {
    clear_screen(UART0);
    display_status(UART0, secondes, total_events);
    print_prompt(UART0);
  }
  else if (cmd_len == 4 &&
           cmd_line[0] == 'q' &&
           cmd_line[1] == 'u' &&
           cmd_line[2] == 'i' &&
           cmd_line[3] == 't')
  {
    uart_send_string(UART0, "\r\nBye!\r\n");
    qemu_exit();
  }
  else if (cmd_len > 5 &&
           cmd_line[0] == 'e' &&
           cmd_line[1] == 'c' &&
           cmd_line[2] == 'h' &&
           cmd_line[3] == 'o' &&
           cmd_line[4] == ' ')
  {
    uart_send_string(UART0, "\r\nEchoing: ");
    uart_send_string(UART0, cmd_line + 5);
    print_prompt(UART0);
  }
  else
  {
    print_prompt(UART0);
  }
}

static void rx_bottom_handler(void *cookie)
{
  (void)cookie;
  process_ring(input_line, &input_offset);
}

static void blink_bottom_handler(void *cookie)
{
  (void)cookie;
  if (cursor_is_visible)
  {
    cursor_hide(UART0);
    cursor_is_visible = FALSE;
    secondes += 1;
    display_status(UART0, secondes, total_events);
  }
  else
  {
    cursor_show(UART0);
    cursor_is_visible = TRUE;
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
  event_post_front(&rx_event, &event_count, &total_events, &ready_head);
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

  /* Ack timer interrupt */
  timer1->IntClr = 1;
  event_post_front(&blink_event, &event_count, &total_events, &ready_head);
}

static void timer1_irq_init(void)
{
  /*
   * 1 MHz timer clock (configured in timer_init()).
   * 500000 ticks -> 0.5s, so cursor toggles every 500 ms.
   */
  timer1->Control = 0x00;
  timer1->Load = 500000;
  timer1->BGLoad = 500000;
  timer1->IntClr = 1;
  /* Enable=1, Periodic=1, IntEnable=1, 32-bit counter */
  timer1->Control = 0xE2;

  irq_enable(TIMER1_IRQ, timer1_irq_handler, NULL);
}


/**
 * This is the C entry point, upcalled once the hardware has been setup properly
 * in assembly language, see the startup.s file.
 */
void _start()
{
  clear_screen(UART0);
  uart_send_string(UART0, "\n");
  uart_send_string(UART0, "\nFor information:\n");
  uart_send_string(UART0, "  - Quit with \"C-a c\" to get to the QEMU console.\n");
  uart_send_string(UART0, "  - Then type in \"quit\" to stop QEMU.\n");
  uart_send_string(UART0, "  - type in my console   \"quit\" \n");
  uart_send_string(UART0, "\n -- My console -- \n");

  timer_init();

  irqs_setup();
  uart0_irq_init();
  timer1_irq_init();
  core_enable_interrupts();

  cursor_hide(UART0);
  cursor_is_visible = FALSE;
  display_status(UART0, secondes, total_events);
  print_prompt(UART0);

  while (1)
  {
    struct event *evt = event_pop(&event_count, &ready_head);
    if (evt != NULL)
    {
      evt->react(evt->cookie);
    }
    else
    {
      sleep_until_next_event();
    }
  }
}
