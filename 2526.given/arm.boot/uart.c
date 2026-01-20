#include "main.h"
#include "uart.h"

/**
 * PL011_T UART
 *     http://infocenter.arm.com/help/topic/com.arm.doc.ddi0183f/DDI0183.pdf
 *
 * UARTDR: Data Register   (0x00)
 *    To read received bytes
 *    To write bytes to send
 *    Bit Fields:
 *      15:12 reserved
 *      11:08 error flags
 *       7:0  data bits
 * UARTFR:  Flag Register  (0x18)
 *    Bit Fields:
 *      7:  TXFE  transmit FIFO empty
 *      6:  RXFF  receive FIFO full
 *      5:  TXFF  transmit FIFO full
 *      4:  RXFE  receive FIFO empty
 *      3:  BUSY  set when the UART is busy transmitting data
 */

#define UART_DR 0x00
#define UART_FR 0x18

#define UART_TXFE (1<<7)
#define UART_RXFF (1<<6)
#define UART_TXFF (1<<5)
#define UART_RXFE (1<<4)
#define UART_BUSY (1<<3)

/*
 * See "uart.h"
 */
int uart_receive(void* uart, uint8_t *b) {
  uint16_t* uart_fr = (uint16_t*) (uart + UART_FR);
  uint16_t* uart_dr = (uint16_t*) (uart + UART_DR);
  if (*uart_fr & UART_RXFE)
    return 0;
  *b = (uint8_t)(*uart_dr & 0xff);
  return 1;
}

/*
 * See "uart.h"
 */
void uart_send(void* uart, uint8_t b) {
  uint16_t* uart_fr = (uint16_t*) (uart + UART_FR);
  uint16_t* uart_dr = (uint16_t*) (uart + UART_DR);
  while (*uart_fr & UART_TXFF)
    ;
  *uart_dr = (uint16_t)b;
}

/*
 * See "uart.h"
 */
void uart_send_string(void* uart, const unsigned char *s) {
  while (*s != '\0') {
    // the following line only works because characters in C
    // are ASCII characters, encoded on 8 bits.
    uart_send(uart, (uint8_t)*s);
    s++;
  }
}
