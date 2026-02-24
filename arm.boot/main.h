/*
 * test.h
 *
 *  Created on: Jan 12, 2021
 *      Author: ogruber
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>
#include <stddef.h>

typedef uint8_t bool_t;
#define TRUE 1
#define FALSE 0

#define ONESHOT 1
#define WRAPPING 0

#ifndef NULL
#define NULL (void*)0
#endif

struct star{
  int delay;
  int line;
  int col;
  int index;
  int charac[8];
  bool_t display;
};

void moveStar(struct star *s);
void cursor(int c, int l);
void push_first_event();
void echo(uint32_t uartno, void *cookie);

void panic();
void kprintf(const char *fmt, ...);

__inline__
__attribute__((always_inline))
uint32_t mmio_read8(void* bar, uint8_t offset) {
  return *((uint8_t*)(bar+offset));
}

__inline__
__attribute__((always_inline))
void mmio_write8(void* bar, uint32_t offset, uint8_t value) {
  *((uint8_t*)(bar+offset)) = value;
}

__inline__
__attribute__((always_inline))
uint16_t mmio_read16(void* bar, uint32_t offset) {
  return *((uint16_t*)(bar+offset));
}

__inline__
__attribute__((always_inline))
void mmio_write16(void* bar, uint32_t offset, uint16_t value) {
  *((uint16_t*)(bar+offset)) = value;
}

__inline__
__attribute__((always_inline))
uint32_t mmio_read32(void* bar, uint32_t offset) {
  return *((uint32_t*)(bar+offset));
}

__inline__
__attribute__((always_inline))
void mmio_write32(void* bar, uint32_t offset, uint32_t value) {
  *((uint32_t*)(bar+offset)) = value;
}

__inline__
__attribute__((always_inline))
void mmio_set(void* bar, uint32_t offset, uint32_t bits) {
  uint32_t value = *((uint32_t*)(bar+offset));
  value |= bits;
  *((uint32_t*)(bar+offset)) = value;
}

__inline__
__attribute__((always_inline))
void mmio_clear(void* bar, uint32_t offset, uint32_t bits) {
  uint32_t value = *((uint32_t*)(bar+offset));
  value &= ~bits;
  *((uint32_t*)(bar+offset)) = value;
}

#endif /* MAIN_H_ */
