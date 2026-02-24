#ifndef _UART_H_ 
#define _UART_H_ 

/**
 * Look at the document describing the Versatile Application Board:
 *
 *    Versatile Application Baseboard for ARM926EJ-S (HBI-0118)
 *
 * Also referenced as DUI0225, downloadable from:
 *
 *  http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0225d/I999714.html
 *
 * Look at the memory map, section 4.1, page 137, to find the base addresses for all
 * the devices. We only use here the UARTs.
 */
#define UART0 (void*)0x101f1000
#define UART1 (void*)0x101f2000
#define UART2 (void*)0x101f3000

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

/*
 * Receive a byte from the given uart, this is a non-blocking call.
 * Returns 0 if there are no byte available.
 * Returns 1 if a character was read.
 */
int uart_receive(void* uart, uint8_t *b);

/*
 * Sends a byte through the given uart, this is a blocking call.
 * The code spins until there is room in the UART TX FIFO queue 
 * to send the given byte.
 */
void uart_send(void* uart, uint8_t b);

/*
 * This is a wrapper function, provided for simplicity,
 * it sends a C string through the given uart, assuming
 * the characters are 8-bit ASCII characters.
 */
void uart_send_string(void* uart, const unsigned char *s);

// Fonction simple pour convertir un nombre en chaîne
void uint_to_string(uint32_t num, char *buffer);

#endif /* _UART_H_ */
