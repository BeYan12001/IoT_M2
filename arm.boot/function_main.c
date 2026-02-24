#include "function_main.h"

void process_ring(char *input_line, uint8_t *input_offset)
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

    if (code == '\b' || code == 127)
    {
      if (*input_offset > 0)
      {
        (*input_offset)--;
        back_space(UART0);
      }
      continue;
    }

    // Ctrl+C : annule la ligne en cours et affiche un nouveau prompt
    if (code == 3)
    {
      *input_offset = 0;
      uart_send_string(UART0, "^C");
      print_prompt(UART0);
      continue;
    }

    // traitement normal
    if (code == '\r' || code == '\n')
    {
      shell(input_line, *input_offset);
      *input_offset = 0;
      continue;
    }
    else
    {
      if (*input_offset < 79) 
      {
        input_line[(*input_offset)++] = (char)code;
      }
    }
    uart_send(UART0, code);
  }
}
