#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>
#include "main.h"


void clear_screen(void* uart);

void back_space(void* uart);

void space(void* uart);

void delete_char(void* uart);

void cursor_left(void* uart);

void cursor_right(void* uart);

void cursor_hide(void* uart);

void cursor_show(void* uart);

#endif
