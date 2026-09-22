#include "print.h"

/* polling driver for 16550 uart serial on PC COM1 I/O for x86 */

#define COM1_PORT 0x3F8

static inline void outb(unsigned short port, unsigned char val)
{
	__asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline unsigned char inb(unsigned short port)
{
	unsigned char ret;
	__asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

/* Explicitly configure the UART rather than trust that firmware/BIOS
 * already left it in a usable state - keeps this correct regardless of
 * what loaded the binary. Standard init sequence: no IRQs, 38400 baud,
 * 8 bits / no parity / one stop bit, FIFO enabled. */
static int uart_initialized = 0;

static void uart_init(void)
{
	outb(COM1_PORT + 1, 0x00); /* disable UART interrupts - polling only */
	outb(COM1_PORT + 3, 0x80); /* enable DLAB to set the baud rate divisor */
	outb(COM1_PORT + 0, 0x03); /* divisor low byte  (3 -> 38400 baud) */
	outb(COM1_PORT + 1, 0x00); /* divisor high byte */
	outb(COM1_PORT + 3, 0x03); /* DLAB off, 8 bits, no parity, 1 stop bit */
	outb(COM1_PORT + 2, 0xC7); /* enable + clear FIFOs, 14-byte threshold */
	outb(COM1_PORT + 4, 0x0B); /* IRQs off, RTS/DSR set (required by some UART implementations) */

	uart_initialized = 1;
}

static int transmit_empty(void)
{
	return inb(COM1_PORT + 5) & 0x20; /* LSR bit 5: transmit holding register empty */
}

static void uart_putc(char c)
{
	while (!transmit_empty())
		;

	outb(COM1_PORT, (unsigned char)c);
}

void print(char *msg)
{
	if (!uart_initialized)
		uart_init();

	while (*msg)
	{
		if (*msg == '\n')
			uart_putc('\r'); /* most serial terminals expect CRLF */

		uart_putc(*msg);
		msg++;
	}
}
