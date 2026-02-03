/*
 * Copyright: Olivier Gruber (olivier dot gruber at acm dot org)
 *
 * This program is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

/*
 * isr.h
 *
 *  Created on: Jan 21, 2021
 *      Author: ogruber
 */

#ifndef ISR_H_
#define ISR_H_

/*
 * Versatile Application Baseboard for ARM926EJ-S User Guide HBI-0118
 *   (https://developer.arm.com/documentation/dui0225/latest)
 *   3. Hardware Description
 *   3.10 Interrupts
 *     Page 111, Figure 3.23
 * The VIC handles a maximum of 32 IRQs.
 */

#define NIRQS 32

/*
 * UARTs
 *       UART2 IRQ = 14
 *       UART1 IRQ = 13
 *       UART0 IRQ = 12
 */
#define UART0_IRQ 12
#define UART0_IRQ_MASK (1<<UART0_IRQ)

#define UART1_IRQ 13
#define UART1_IRQ_MASK (1<<UART1_IRQ)

#define UART2_IRQ 14
#define UART2_IRQ_MASK (1<<UART2_IRQ)

/*
 * Timers:
 *       TIMER(2&3) IRQ = 5
 *       TIMER(0&1) IRQ = 4
 */
#define TIMER3_IRQ 5
#define TIMER3_IRQ_MASK (1<<TIMER3_IRQ_MASK)

#define TIMER2_IRQ 5
#define TIMER2_IRQ_MASK (1<<TIMER2_IRQ_MASK)

#define TIMER1_IRQ 4
#define TIMER1_IRQ_MASK (1<<TIMER1_IRQ_MASK)

#define TIMER0_IRQ 4
#define TIMER0_IRQ_MASK (1<<TIMER0_IRQ_MASK)

#include <stdint.h>
/*
 * VIC behavior:
 */
void irqs_setup();
void irqs_enable();
void irqs_disable();
void wfi(void);

/*
 * Enable the given interrupt,
 * like UART0_IRQ
 */
void irq_enable(uint32_t irq,void(*callback)(uint32_t,void*),void*cookie);

/*
 * Disable the given interrupt,
 * like UART0_IRQ
 */
void irq_disable(uint32_t irq);

#endif /* ISR_H_ */
