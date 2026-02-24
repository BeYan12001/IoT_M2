#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>
#include "main.h"

void qemu_exit(void);

void display_status(void *uart, int secondes, int total_events);

void print_prompt(void* uart);

void clear_screen(void* uart);

void back_space(void* uart);

void space(void* uart);

void delete_char(void* uart);

void cursor_left(void* uart);

void cursor_right(void* uart);

void cursor_hide(void* uart);

void cursor_show(void* uart);

void cursor_save(void* uart);

void cursor_restore(void* uart);

void cursor_move_top(void* uart);

void erase_line(void* uart);

#endif
