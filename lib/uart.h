#ifndef UART_H
#define UART_H

#include <stdio.h>

// Got your attention,  just so you know it may or not work with other versions.
#if (__GNUC__ * 100 + __GNUC_MINOR__) != 409
#error "This library was used with gcc-avr package (4.9.2+Atmel3.5.0-1) on Ubuntu 16.04"
#endif

/* Enable USART 1, 2, 3 as required */
#define USART0_ENABLED
//#define USART1_ENABLED
//#define USART2_ENABLED 
//#define USART3_ENABLED

// Size of the circular receive buffer, must be power of 2
// only the first 32 bytes can be accessed quickly using the AVR ldd instruction.
#ifndef UART_RX0_BUFFER_SIZE
	#define UART_RX0_BUFFER_SIZE 32
#endif
#ifndef UART_RX1_BUFFER_SIZE
	#define UART_RX1_BUFFER_SIZE 32
#endif
#ifndef UART_RX2_BUFFER_SIZE
	#define UART_RX2_BUFFER_SIZE 32
#endif
#ifndef UART_RX3_BUFFER_SIZE
	#define UART_RX3_BUFFER_SIZE 32
#endif

#ifndef UART_TX0_BUFFER_SIZE
	#define UART_TX0_BUFFER_SIZE 32
#endif
#ifndef UART_TX1_BUFFER_SIZE
	#define UART_TX1_BUFFER_SIZE 32
#endif
#ifndef UART_TX2_BUFFER_SIZE
	#define UART_TX2_BUFFER_SIZE 32
#endif
#ifndef UART_TX3_BUFFER_SIZE
	#define UART_TX3_BUFFER_SIZE 32
#endif

/* Check buffer sizes are not too large for 8-bit positioning */
#if (UART_RX0_BUFFER_SIZE > 256)
	#error "Buffer too large"
#endif

#if (UART_RX1_BUFFER_SIZE > 256)
	#error "Buffer too large"
#endif

#if (UART_RX2_BUFFER_SIZE > 256)
	#error "Buffer too large"
#endif

#if (UART_RX3_BUFFER_SIZE > 256)
	#error "Buffer too large"
#endif

#define UART_BAUD_SELECT(baudRate,xtalCpu) (((xtalCpu)+8UL*(baudRate))/(16UL*(baudRate))-1UL)
#define UART_BAUD_SELECT_DOUBLE_SPEED(baudRate,xtalCpu) ((((xtalCpu)+4UL*(baudRate))/(8UL*(baudRate))-1)|0x8000)

/* high byte error return code of uart_getc() */
#define UART_FRAME_ERROR      0x0800              /**< Framing Error by UART       */
#define UART_OVERRUN_ERROR    0x0400              /**< Overrun condition by UART   */
#define UART_BUFFER_OVERFLOW  0x0200              /**< receive ringbuffer overflow */
#define UART_NO_DATA          0x0100              /**< no receive data available   */

extern void init_uart0_after_bootloader(void);

extern void uart0_init(uint16_t baudrate);
extern uint16_t uart0_getc(void);
extern uint16_t uart0_peek(void);
extern void uart0_putc(uint8_t data);
extern void uart0_flush(void);
extern uint16_t uart0_available(void);
extern uint16_t uart0_availableForWrite(void);
extern FILE *uartstream0_init(uint32_t baudrate);

extern void uart1_init(uint16_t baudrate);
extern uint16_t uart1_getc(void);
extern uint16_t uart1_peek(void);
extern void uart1_putc(uint8_t data);
extern void uart1_flush(void);
extern uint16_t uart1_available(void);
extern uint16_t uart1_availableForWrite(void);
extern FILE *uartstream1_init(uint32_t baudrate);

extern void uart2_init(uint16_t baudrate);
extern uint16_t uart2_getc(void);
extern uint16_t uart2_peek(void);
extern void uart2_putc(uint8_t data);
extern void uart2_flush(void);
extern uint16_t uart2_available(void);
extern uint16_t uart2_availableForWrite(void);
extern FILE *uartstream2_init(uint32_t baudrate);

extern void uart3_init(uint16_t baudrate);
extern uint16_t uart3_getc(void);
extern uint16_t uart3_peek(void);
extern void uart3_putc(uint8_t data);
extern void uart3_flush(void);
extern uint16_t uart3_available(void);
extern uint16_t uart3_availableForWrite(void);
extern FILE *uartstream3_init(uint32_t baudrate);

#endif // UART_H 
