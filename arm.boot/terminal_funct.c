#include "terminal_funct.h"

void qemu_exit(void)
{
  /* ARM semihosting: SYS_EXIT (0x18) with ADP_Stopped_ApplicationExit (0x20026) */
  register uint32_t r0 asm("r0") = 0x18;
  register uint32_t r1 asm("r1") = 0x20026;
  asm volatile("svc 0x00123456" : : "r"(r0), "r"(r1));
  while (1)
    ; /* ne devrait pas arriver */
}


void display_status(void *uart, int secondes, int total_events)
{
  char buffer_time[20];
  char buffer_events[20];
  cursor_save(uart);
  cursor_move_top(uart);
  erase_line(uart);
  uart_send_string(uart, "Passing time : ");
  uint_to_string(secondes, buffer_time);
  uart_send_string(uart, buffer_time);
  uart_send_string(uart, " seconds, ");
  uint_to_string(total_events, buffer_events);
  uart_send_string(uart, buffer_events);
  uart_send_string(uart, " events, updated every second");
  cursor_restore(uart);
}


void print_prompt(void* uart)
{
    uart_send(uart, '\r');
    uart_send(uart, '\n');
    uart_send_string(uart, "yan12001@Hippo$:");
}


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

/* Sauvegarde la position courante du curseur (ESC[s) */
void cursor_save(void* uart)
{
  uart_send(uart, 27);
  uart_send(uart, '[');
  uart_send(uart, 's');
}

/* Restaure la position sauvegardée du curseur (ESC[u) */
void cursor_restore(void* uart)
{
  uart_send(uart, 27);
  uart_send(uart, '[');
  uart_send(uart, 'u');
}

/* Déplace le curseur en haut de l'écran : ligne 1, colonne 1 (ESC[1;1H) */
void cursor_move_top(void* uart)
{
  uart_send(uart, 27);
  uart_send(uart, '[');
  uart_send(uart, '1');
  uart_send(uart, ';');
  uart_send(uart, '1');
  uart_send(uart, 'H');
}

/* Efface la ligne courante (ESC[2K) */
void erase_line(void* uart)
{
  uart_send(uart, 27);
  uart_send(uart, '[');
  uart_send(uart, '2');
  uart_send(uart, 'K');
}


