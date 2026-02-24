#include "main.h"
#include "uart.h"
#include "timer.h"

/*
 * Define ECHO_ZZZ to have a periodic reminder that this code is polling
 * the UART, actively. This means the processor is running continuously.
 * Polling is of course not the way to go, the processor should halt in
 * a low-power state and wake-up only to handle an interrupt from the UART.
 * But this would require setting up interrupts...
 */
#define ECHO_ZZZ

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

void cursor_up();
void cursor_down();
void cursor_left();
void cursor_right();
void clear_screen();
void clear_line();

/**
 * This is the C entry point, upcalled once the hardware has been setup properly
 * in assembly language, see the startup.s file.
 */
void _start()
{
  timer_init(); // Initialiser le timer
  // core_enable_interrupts();

  uart_send_string(UART0, "\nFor information:\n");
  uart_send_string(UART0, "  - Quit with \"C-a c\" to get to the QEMU console.\n");
  uart_send_string(UART0, "  - Then type in \"quit\" to stop QEMU.\n");

  uart_send_string(UART0, "\nHello world!\n");

  clear_screen();

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

    //wfi();
    uint8_t c;

    // Afficher le timer chaque seconde
    uint32_t current_second = timer_get_seconds();
    if (current_second != last_second)
    {
      clear_screen();
      save_cursor();
      uart_send_string(UART0, "[");
      uint_to_string(current_second, bufferTimer);
      uart_send_string(UART0, bufferTimer);
      uart_send_string(UART0, "s]\n\r");

      for (int i = 0; i < 10; i++)
      {
        for (int j = 0; j < 15; j++)
        {
          uart_send(UART0, buffer[i][j]);
        }
        uart_send(UART0, '\n');
        uart_send(UART0, '\r');
      }

      last_second = current_second;
    }

    if (0 == uart_receive(UART0, &c))
      continue;

    if (c == 13)
    {
      uart_send(UART0, '\r');
      uart_send(UART0, '\n');
    }
    else if (c == 'q')
    {
      cursor_left();
    }
    else if (c == 'd')
    {
      cursor_right();
    }
    else if (c == 'z')
    {
      cursor_right();
      cursor_up();
    }
    else if (c == 's')
    {
      cursor_right();
      cursor_down();
    }
    else
    {
      uart_send(UART0, c);
    }
  }
}

void cursor_left()
{
  uart_send(UART0, 27);
  uart_send(UART0, '[');
  uart_send(UART0, 'D');
}

void cursor_right()
{
  uart_send(UART0, 27);
  uart_send(UART0, '[');
  uart_send(UART0, 'C');
}
void cursor_down()
{
  uart_send(UART0, 27);
  uart_send(UART0, '[');
  uart_send(UART0, 'A');
}
void cursor_up()
{
  uart_send(UART0, 27);
  uart_send(UART0, '[');
  uart_send(UART0, 'B');
}

void clear_line()
{
  uart_send(UART0, 27);
  uart_send(UART0, '[');
  uart_send(UART0, '2');
  uart_send(UART0, 'K');
}
/*
void clear_screen()
{
    uart_send(UART0, 27);
    uart_send(UART0, '[');
    uart_send(UART0, '2');
    uart_send(UART0, 'J');   // clear screen
}
*/


void save_cursor()
{
    uart_send(UART0, 27);
    uart_send(UART0, '[');
    uart_send(UART0, 's');
 
}

