#include "terminal_funct.h"

void clear_screen(void* uart)
{
  uart_send(uart, 27);
  uart_send(uart, '[');
  uart_send(uart, '2');
  uart_send(uart, 'J');
  uart_send(uart, 27);
  uart_send(uart, '[');
  uart_send(uart, 'H'); // home cursor
}

void back_space(void* uart)
{
  uart_send(uart, '\b');   // recule
  uart_send(uart, ' ');    // écrase
  uart_send(uart, '\b');   // recule encore
}


void space(void* uart)
{
  uart_send(uart, ' ');
}

void delete_char(void* uart)
{
  uart_send(uart, 27);
  uart_send(uart, '[');
  uart_send(uart, 'P');
}



void cursor_left(void* uart)
{
  uart_send(uart, 27);
  uart_send(uart, '[');
  uart_send(uart, 'D');
}

void cursor_right(void* uart)
{
  uart_send(uart, 27);
  uart_send(uart, '[');
  uart_send(uart, 'C');
}

void cursor_hide(void* uart)
{
  uart_send(uart, 27);
  uart_send(uart, '[');
  uart_send(uart, '?');
  uart_send(uart, '2');
  uart_send(uart, '5');
  uart_send(uart, 'l');
}

void cursor_show(void* uart)
{
  uart_send(uart, 27);
  uart_send(uart, '[');
  uart_send(uart, '?');
  uart_send(uart, '2');
  uart_send(uart, '5');
  uart_send(uart, 'h');
}
