/*
    AVR Interrupt-Driven UART with stdio redirect
    Copyright (C) 2019 Ronald Sutherland

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Hacked from UART library by Andy Gock
    https://github.com/andygock/avr-uart

    Added Streams from Steve Rodgerson
    https://github.com/hwstar

*/

#include <stdio.h>
#include <util/atomic.h>
#include "uart.h"

//  constants and macros

/* size of RX/TX buffers */
#define UART_RX0_BUFFER_MASK ( UART_RX0_BUFFER_SIZE - 1)
#define UART_RX1_BUFFER_MASK ( UART_RX1_BUFFER_SIZE - 1)
#define UART_RX2_BUFFER_MASK ( UART_RX2_BUFFER_SIZE - 1)
#define UART_RX3_BUFFER_MASK ( UART_RX3_BUFFER_SIZE - 1)

#define UART_TX0_BUFFER_MASK ( UART_TX0_BUFFER_SIZE - 1)
#define UART_TX1_BUFFER_MASK ( UART_TX1_BUFFER_SIZE - 1)
#define UART_TX2_BUFFER_MASK ( UART_TX2_BUFFER_SIZE - 1)
#define UART_TX3_BUFFER_MASK ( UART_TX3_BUFFER_SIZE - 1)

#if ( UART_RX0_BUFFER_SIZE & UART_RX0_BUFFER_MASK )
    #error RX0 buffer size is not a power of 2
#endif
#if ( UART_TX0_BUFFER_SIZE & UART_TX0_BUFFER_MASK )
    #error TX0 buffer size is not a power of 2
#endif

#if ( UART_RX1_BUFFER_SIZE & UART_RX1_BUFFER_MASK )
    #error RX1 buffer size is not a power of 2
#endif
#if ( UART_TX1_BUFFER_SIZE & UART_TX1_BUFFER_MASK )
    #error TX1 buffer size is not a power of 2
#endif

#if ( UART_RX2_BUFFER_SIZE & UART_RX2_BUFFER_MASK )
    #error RX2 buffer size is not a power of 2
#endif
#if ( UART_TX2_BUFFER_SIZE & UART_TX2_BUFFER_MASK )
    #error TX2 buffer size is not a power of 2
#endif

#if ( UART_RX3_BUFFER_SIZE & UART_RX3_BUFFER_MASK )
    #error RX3 buffer size is not a power of 2
#endif
#if ( UART_TX3_BUFFER_SIZE & UART_TX3_BUFFER_MASK )
    #error TX3 buffer size is not a power of 2
#endif

#if defined(__AVR_AT90S2313__) || defined(__AVR_AT90S4414__) || \
    defined(__AVR_AT90S4434__) || defined(__AVR_AT90S8515__) || \
    defined(__AVR_AT90S8535__) || defined(__AVR_ATmega103__)
    /* old AVR classic or ATmega103 with one UART */
    #define AT90_UART
    #define UART0_RECEIVE_INTERRUPT   UART_RX_vect
    #define UART0_TRANSMIT_INTERRUPT  UART_UDRE_vect
    #define UART0_STATUS   USR
    #define UART0_CONTROL  UCR
    #define UART0_DATA     UDR  
    #define UART0_UDRIE    UDRIE
#elif defined(__AVR_AT90S2333__) || defined(__AVR_AT90S4433__)
    /* old AVR classic with one UART */
    #define AT90_UART
    #define UART0_RECEIVE_INTERRUPT   UART_RX_vect
    #define UART0_TRANSMIT_INTERRUPT  UART_UDRE_vect
    #define UART0_STATUS   UCSRA
    #define UART0_CONTROL  UCSRB
    #define UART0_DATA     UDR 
    #define UART0_UDRIE    UDRIE
#elif  defined(__AVR_ATmega8__)  || defined(__AVR_ATmega16__) || \
    defined(__AVR_ATmega32__) || defined(__AVR_ATmega323__)
    /* ATmega with one USART */
    #define ATMEGA_USART
    #define UART0_RECEIVE_INTERRUPT   USART_RXC_vect
    #define UART0_TRANSMIT_INTERRUPT  USART_UDRE_vect
    #define UART0_STATUS   UCSRA
    #define UART0_CONTROL  UCSRB
    #define UART0_DATA     UDR
    #define UART0_UDRIE    UDRIE
#elif defined(__AVR_ATmega8U2__) || defined(__AVR_ATmega16U2__) || \
    defined(__AVR_ATmega16U4__) || defined(__AVR_ATmega32U2__) || \
    defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega32U6__)
    /* ATmega with one USART, but is called USART1 (untested) */
    #define ATMEGA_USART1
    #define UART1_RECEIVE_INTERRUPT   USART1_RX_vect
    #define UART1_TRANSMIT_INTERRUPT  USART1_UDRE_vect
    #define UART1_STATUS   UCSR1A
    #define UART1_CONTROL  UCSR1B
    #define UART1_DATA     UDR1
    #define UART1_UDRIE    UDRIE1
#elif  defined(__AVR_ATmega8515__) || defined(__AVR_ATmega8535__)
    /* ATmega with one USART */
    #define ATMEGA_USART
    #define UART0_RECEIVE_INTERRUPT   USART_RX_vect
    #define UART0_TRANSMIT_INTERRUPT  USART_UDRE_vect
    #define UART0_STATUS   UCSRA
    #define UART0_CONTROL  UCSRB
    #define UART0_DATA     UDR
    #define UART0_UDRIE    UDRIE
#elif defined(__AVR_ATmega163__) 
    /* ATmega163 with one UART */
    #define ATMEGA_UART
    #define UART0_RECEIVE_INTERRUPT   UART_RX_vect
    #define UART0_TRANSMIT_INTERRUPT  UART_UDRE_vect
    #define UART0_STATUS   UCSRA
    #define UART0_CONTROL  UCSRB
    #define UART0_DATA     UDR
    #define UART0_UDRIE    UDRIE
#elif defined(__AVR_ATmega162__) 
    /* ATmega with two USART */
    #define ATMEGA_USART0
    #define ATMEGA_USART1
    #define UART0_RECEIVE_INTERRUPT   USART0_RXC_vect
    #define UART1_RECEIVE_INTERRUPT   USART1_RXC_vect
    #define UART0_TRANSMIT_INTERRUPT  USART0_UDRE_vect
    #define UART1_TRANSMIT_INTERRUPT  USART1_UDRE_vect
    #define UART0_STATUS   UCSR0A
    #define UART0_CONTROL  UCSR0B
    #define UART0_DATA     UDR0
    #define UART0_UDRIE    UDRIE0
    #define UART1_STATUS   UCSR1A
    #define UART1_CONTROL  UCSR1B
    #define UART1_DATA     UDR1
    #define UART1_UDRIE    UDRIE1
#elif defined(__AVR_ATmega64__) || defined(__AVR_ATmega128__) 
    /* ATmega with two USART */
    #define ATMEGA_USART0
    #define ATMEGA_USART1
    #define UART0_RECEIVE_INTERRUPT   USART0_RX_vect
    #define UART1_RECEIVE_INTERRUPT   USART1_RX_vect
    #define UART0_TRANSMIT_INTERRUPT  USART0_UDRE_vect
    #define UART1_TRANSMIT_INTERRUPT  USART1_UDRE_vect
    #define UART0_STATUS   UCSR0A
    #define UART0_CONTROL  UCSR0B
    #define UART0_DATA     UDR0
    #define UART0_UDRIE    UDRIE0
    #define UART1_STATUS   UCSR1A
    #define UART1_CONTROL  UCSR1B
    #define UART1_DATA     UDR1
    #define UART1_UDRIE    UDRIE1
#elif defined(__AVR_ATmega161__)
    /* ATmega with UART */
    #error "AVR ATmega161 currently not supported by this libaray !"
#elif defined(__AVR_ATmega169__) 
    /* ATmega with one USART */
    #define ATMEGA_USART
    #define UART0_RECEIVE_INTERRUPT   USART0_RX_vect
    #define UART0_TRANSMIT_INTERRUPT  USART0_UDRE_vect
    #define UART0_STATUS   UCSRA
    #define UART0_CONTROL  UCSRB
    #define UART0_DATA     UDR
    #define UART0_UDRIE    UDRIE
#elif defined(__AVR_ATmega48__) ||defined(__AVR_ATmega88__) || \
    defined(__AVR_ATmega168__) || defined(__AVR_ATmega48P__) || \
    defined(__AVR_ATmega88P__) || defined(__AVR_ATmega168P__) || \
    defined(__AVR_ATmega328P__)
    #define ATMEGA_USART0
    #define UART0_RECEIVE_INTERRUPT   USART_RX_vect
    #define UART0_TRANSMIT_INTERRUPT  USART_UDRE_vect
    #define UART0_STATUS   UCSR0A
    #define UART0_CONTROL  UCSR0B
    #define UART0_DATA     UDR0
    #define UART0_UDRIE    UDRIE0
#elif defined(__AVR_ATtiny2313__) || defined(__AVR_ATtiny2313A__) || \
    defined(__AVR_ATtiny4313__)
    #define ATMEGA_USART
    #define UART0_RECEIVE_INTERRUPT   USART_RX_vect 
    #define UART0_TRANSMIT_INTERRUPT  USART_UDRE_vect
    #define UART0_STATUS   UCSRA
    #define UART0_CONTROL  UCSRB
    #define UART0_DATA     UDR
    #define UART0_UDRIE    UDRIE
#elif defined(__AVR_ATmega329__) || defined(__AVR_ATmega649__) || \
    defined(__AVR_ATmega325__) ||defined(__AVR_ATmega3250__) || \
    defined(__AVR_ATmega645__) ||defined(__AVR_ATmega6450__)
    /* ATmega with one USART */
    #define ATMEGA_USART0
    #define UART0_RECEIVE_INTERRUPT   USART0_RX_vect
    #define UART0_TRANSMIT_INTERRUPT  USART0_UDRE_vect
    #define UART0_STATUS   UCSR0A
    #define UART0_CONTROL  UCSR0B
    #define UART0_DATA     UDR0
    #define UART0_UDRIE    UDRIE0
#elif defined(__AVR_ATmega3290__) || defined(__AVR_ATmega6490__)
    /* ATmega with one USART */
    #define ATMEGA_USART0
    #define UART0_RECEIVE_INTERRUPT   USART_RX_vect
    #define UART0_TRANSMIT_INTERRUPT  USART0_UDRE_vect
    #define UART0_STATUS   UCSR0A
    #define UART0_CONTROL  UCSR0B
    #define UART0_DATA     UDR0
    #define UART0_UDRIE    UDRIE0
#elif defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__) || \
    defined(__AVR_ATmega640__)
    /* ATmega with four USART */
    #define ATMEGA_USART0
    #define ATMEGA_USART1
    #define ATMEGA_USART2
    #define ATMEGA_USART3
    #define UART0_RECEIVE_INTERRUPT   USART0_RX_vect
    #define UART1_RECEIVE_INTERRUPT   USART1_RX_vect
    #define UART2_RECEIVE_INTERRUPT   USART2_RX_vect
    #define UART3_RECEIVE_INTERRUPT   USART3_RX_vect
    #define UART0_TRANSMIT_INTERRUPT  USART0_UDRE_vect
    #define UART1_TRANSMIT_INTERRUPT  USART1_UDRE_vect
    #define UART2_TRANSMIT_INTERRUPT  USART2_UDRE_vect
    #define UART3_TRANSMIT_INTERRUPT  USART3_UDRE_vect
    #define UART0_STATUS   UCSR0A
    #define UART0_CONTROL  UCSR0B
    #define UART0_DATA     UDR0
    #define UART0_UDRIE    UDRIE0
    #define UART1_STATUS   UCSR1A
    #define UART1_CONTROL  UCSR1B
    #define UART1_DATA     UDR1
    #define UART1_UDRIE    UDRIE1  
    #define UART2_STATUS   UCSR2A
    #define UART2_CONTROL  UCSR2B
    #define UART2_DATA     UDR2
    #define UART2_UDRIE    UDRIE2  
    #define UART3_STATUS   UCSR3A
    #define UART3_CONTROL  UCSR3B
    #define UART3_DATA     UDR3
    #define UART3_UDRIE    UDRIE3  
#elif defined(__AVR_ATmega644__)
    /* ATmega with one USART */
    #define ATMEGA_USART0
    #define UART0_RECEIVE_INTERRUPT   USART0_RX_vect
    #define UART0_TRANSMIT_INTERRUPT  USART0_UDRE_vect
    #define UART0_STATUS   UCSR0A
    #define UART0_CONTROL  UCSR0B
    #define UART0_DATA     UDR0
    #define UART0_UDRIE    UDRIE0 
#elif defined(__AVR_ATmega164P__) || defined(__AVR_ATmega324P__) \
    || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)
    /* ATmega with two USART */
    #define ATMEGA_USART0
    #define ATMEGA_USART1
    #define UART0_RECEIVE_INTERRUPT   USART0_RX_vect
    #define UART1_RECEIVE_INTERRUPT   USART1_RX_vect
    #define UART0_TRANSMIT_INTERRUPT  USART0_UDRE_vect
    #define UART1_TRANSMIT_INTERRUPT  USART1_UDRE_vect
    #define UART0_STATUS   UCSR0A
    #define UART0_CONTROL  UCSR0B
    #define UART0_DATA     UDR0
    #define UART0_UDRIE    UDRIE0
    #define UART1_STATUS   UCSR1A
    #define UART1_CONTROL  UCSR1B
    #define UART1_DATA     UDR1
    #define UART1_UDRIE    UDRIE1
#elif defined(__AVR_ATmega328PB__)
    #define ATMEGA_USART0
    #define ATMEGA_USART1
    #define UART0_RECEIVE_INTERRUPT   USART0_RX_vect
    #define UART1_RECEIVE_INTERRUPT   USART1_RX_vect
    #define UART0_TRANSMIT_INTERRUPT  USART0_UDRE_vect
    #define UART1_TRANSMIT_INTERRUPT  USART1_UDRE_vect
    #define UART0_STATUS   UCSR0A
    #define UART0_CONTROL  UCSR0B
    #define UART0_DATA     UDR0
    #define UART0_UDRIE    UDRIE0
    #define UART1_STATUS   UCSR1A
    #define UART1_CONTROL  UCSR1B
    #define UART1_DATA     UDR1
    #define UART1_UDRIE    UDRIE1
#elif defined(__AVR_ATmega324PB__)
    #define ATMEGA_USART0
    #define ATMEGA_USART1
    #define ATMEGA_USART2
    #define UART0_RECEIVE_INTERRUPT   USART0_RX_vect
    #define UART1_RECEIVE_INTERRUPT   USART1_RX_vect
    #define UART2_RECEIVE_INTERRUPT   USART2_RX_vect
    #define UART0_TRANSMIT_INTERRUPT  USART0_UDRE_vect
    #define UART1_TRANSMIT_INTERRUPT  USART1_UDRE_vect
    #define UART2_TRANSMIT_INTERRUPT  USART2_UDRE_vect
    #define UART0_STATUS   UCSR0A
    #define UART0_CONTROL  UCSR0B
    #define UART0_DATA     UDR0
    #define UART0_UDRIE    UDRIE
    #define UART1_STATUS   UCSR1A
    #define UART1_CONTROL  UCSR1B
    #define UART1_DATA     UDR1
    #define UART1_UDRIE    UDRIE
    #define UART2_STATUS   UCSR2A
    #define UART2_CONTROL  UCSR2B
    #define UART2_DATA     UDR2
    #define UART2_UDRIE    UDRIE
#else
    #error "no UART definition for MCU available"
#endif

/* Module global variables  */
#if defined( USART0_ENABLED )
    #if defined( ATMEGA_USART ) || defined( ATMEGA_USART0 )
        static volatile uint8_t UART_TxBuf[UART_TX0_BUFFER_SIZE];
        static volatile uint8_t UART_RxBuf[UART_RX0_BUFFER_SIZE];
        static volatile uint8_t UART0_TxHead;
        static volatile uint8_t UART0_TxTail;
        static volatile uint8_t UART0_RxHead;
        static volatile uint8_t UART0_RxTail;
        static volatile uint8_t UART0_LastRxError;
    #endif
#endif

#if defined( USART1_ENABLED )
    #if defined( ATMEGA_USART1 )
        static volatile uint8_t UART1_TxBuf[UART_TX1_BUFFER_SIZE];
        static volatile uint8_t UART1_RxBuf[UART_RX1_BUFFER_SIZE];
        static volatile uint8_t UART1_TxHead;
        static volatile uint8_t UART1_TxTail;
        static volatile uint8_t UART1_RxHead;
        static volatile uint8_t UART1_RxTail;
        static volatile uint8_t UART1_LastRxError;
    #endif
#endif

#if defined( USART2_ENABLED )
    #if defined( ATMEGA_USART2 )
        static volatile uint8_t UART2_TxBuf[UART_TX2_BUFFER_SIZE];
        static volatile uint8_t UART2_RxBuf[UART_RX2_BUFFER_SIZE];
        static volatile uint8_t UART2_TxHead;
        static volatile uint8_t UART2_TxTail;
        static volatile uint8_t UART2_RxHead;
        static volatile uint8_t UART2_RxTail;
        static volatile uint8_t UART2_LastRxError;
    #endif
#endif

#if defined( USART3_ENABLED )
    #if defined( ATMEGA_USART3 )
        static volatile uint8_t UART3_TxBuf[UART_TX3_BUFFER_SIZE];
        static volatile uint8_t UART3_RxBuf[UART_RX3_BUFFER_SIZE];
        static volatile uint8_t UART3_TxHead;
        static volatile uint8_t UART3_TxTail;
        static volatile uint8_t UART3_RxHead;
        static volatile uint8_t UART3_RxTail;
        static volatile uint8_t UART3_LastRxError;
    #endif
#endif

/* the bootloader connects pins to UART0  disconnect them
    so they can be used as normal digital i/o; they may be
    reconnected by uart0_init() */
void init_uart0_after_bootloader()
{
#if defined(UCSRB)
	UCSRB = 0;
#elif defined(UCSR0B)
	UCSR0B = 0;
#endif
}

#if defined(AT90_UART) || defined(ATMEGA_USART) || defined(ATMEGA_USART0) 

ISR(UART0_RECEIVE_INTERRUPT)
{
    uint16_t tmphead;
    uint8_t data;
    uint8_t usr;
    uint8_t lastRxError;
 
    /* read UART status register and UART data register */ 
    usr  = UART0_STATUS;
    data = UART0_DATA;
    
#if defined( AT90_UART )
    lastRxError = (usr & ((1<<FE)|(1<<DOR)) );
#elif defined( ATMEGA_USART )
    lastRxError = (usr & ((1<<FE)|(1<<DOR)) );
#elif defined( ATMEGA_USART0 )
    lastRxError = (usr & ((1<<FE0)|(1<<DOR0)) );
#elif defined ( ATMEGA_UART )
    lastRxError = (usr & ((1<<FE)|(1<<DOR)) );
#endif
        
    /* calculate buffer index */ 
    tmphead = ( UART0_RxHead + 1) & UART_RX0_BUFFER_MASK;
    
    if ( tmphead == UART0_RxTail ) {
        /* error: receive buffer overflow */
        lastRxError = UART_BUFFER_OVERFLOW >> 8;
    } else {
        /* store new index */
        UART0_RxHead = tmphead;
        /* store received data in buffer */
        UART_RxBuf[tmphead] = data;
    }
    UART0_LastRxError = lastRxError;   
}


ISR(UART0_TRANSMIT_INTERRUPT)
{
    uint16_t tmptail;

    if ( UART0_TxHead != UART0_TxTail) {
        /* calculate and store new buffer index */
        tmptail = (UART0_TxTail + 1) & UART_TX0_BUFFER_MASK;
        UART0_TxTail = tmptail;
        /* get one byte from buffer and write it to UART */
        UART0_DATA = UART_TxBuf[tmptail];  /* start transmission */
    } else {
        /* tx buffer empty, disable UDRE interrupt */
        UART0_CONTROL &= ~(1<<UART0_UDRIE);
    }
}

void uart0_init(uint16_t baudrate)
{
    /* UART glitch: how to avoid, this is how optiboot does it.
          UART0_STATUS = _BV(U2X0); //Double speed mode 
          UART0_CONTROL = _BV(RXEN0) | _BV(TXEN0); // enable TX and RX glitch free
          UCSR0C = (1<<UCSZ00) | (1<<UCSZ01); // control frame format
          UBRR0L = (uint8_t)( (F_CPU + BAUD * 4L) / (BAUD * 8L) - 1 );
    */
    
    UART0_TxHead = 0;
    UART0_TxTail = 0;
    UART0_RxHead = 0;
    UART0_RxTail = 0;

//Double speed mode if needed
#if defined( ATMEGA_USART ) 
    if ( baudrate & 0x8000 ) {
        UART0_STATUS = (1<<U2X);  //Enable 2x speed
        baudrate &= ~0x8000;
    }
#elif defined ( ATMEGA_USART0 )
    if ( baudrate & 0x8000 ) {
        UART0_STATUS = (1<<U2X0);  //Enable 2x speed
        baudrate &= ~0x8000;
    }
#elif defined ( ATMEGA_UART )
    if ( baudrate & 0x8000 ) {
        UART0_STATUS = (1<<U2X);  //Enable 2x speed
        baudrate &= ~0x8000;
    }
#endif
    
#if defined( AT90_UART ) 
    /* enable UART receiver and transmitter and receive complete interrupt */
    UART0_CONTROL = (1<<RXCIE)|(1<<RXEN)|(1<<TXEN);

    /* set baud rate */
    UBRR = (uint8_t)baudrate;

#elif defined (ATMEGA_USART)
    /* Enable USART receiver and transmitter and receive complete interrupt */
    UART0_CONTROL = (1<<RXCIE)|(1<<RXEN)|(1<<TXEN);

    /* Set frame format: asynchronous, 8data, no parity, 1stop bit */
#ifdef URSEL
    UCSRC = (1<<URSEL)|(3<<UCSZ0);
#else
    UCSRC = (3<<UCSZ0);
#endif /* defined( URSEL ) */

    /* Set ATMEGA_USART baud rate */
    UBRRH = (uint8_t)(baudrate>>8);
    UBRRL = (uint8_t) baudrate;

#elif defined ( ATMEGA_USART0 )
    /* Enable USART receiver and transmitter and receive complete interrupt */
    UART0_CONTROL = (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);

    /* Set frame format: asynchronous, 8data, no parity, 1stop bit */
#ifdef URSEL0
    UCSR0C = (1<<URSEL0)|(3<<UCSZ00);
#else
    UCSR0C = (3<<UCSZ00);
#endif /* defined( ATMEGA_USART0 ) */

    /* Set ATMEGA_USART0 baud rate */
    UBRR0H = (uint8_t)(baudrate>>8);
    UBRR0L = (uint8_t) baudrate;

#elif defined ( ATMEGA_UART )
    /* Enable UART receiver and transmitter and receive complete interrupt */
    UART0_CONTROL = (1<<RXCIE)|(1<<RXEN)|(1<<TXEN);

    /* set ATMEGA_UART baud rate */
    UBRRHI = (uint8_t)(baudrate>>8);
    UBRR   = (uint8_t) baudrate;

#endif  /* defined( ATMEGA_UART ) */

} /* uart0_init */

uint16_t uart0_getc(void)
{
    uint16_t tmptail;
    uint8_t data;

    if ( UART0_RxHead == UART0_RxTail ) {
        return UART_NO_DATA;   /* no data available */
    }

    /* calculate /store buffer index */
    tmptail = (UART0_RxTail + 1) & UART_RX0_BUFFER_MASK;
    UART0_RxTail = tmptail;

    /* get data from receive buffer */
    data = UART_RxBuf[tmptail];

    return (UART0_LastRxError << 8) + data;

} /* uart0_getc */

void uart0_putc(uint8_t data)
{
    uint16_t tmphead;

    tmphead  = (UART0_TxHead + 1) & UART_TX0_BUFFER_MASK;

    while ( tmphead == UART0_TxTail ) {
        ;/* wait for free space in buffer */
    }

    UART_TxBuf[tmphead] = data;
    UART0_TxHead = tmphead;

    /* enable UDRE interrupt */
    UART0_CONTROL    |= (1<<UART0_UDRIE);

} /* uart0_putc */

/* Flush bytes from the transmit buffer  */
void uart0_flush(void)
{
    ATOMIC_BLOCK ( ATOMIC_RESTORESTATE ) // may not be needed
    {
        UART0_TxHead = UART0_TxTail;
    }
} /* uart0_flush */

/* number of bytes available in the receive buffer */
uint16_t uart0_available(void)
{
    return (UART_RX0_BUFFER_SIZE + UART0_RxHead - UART0_RxTail) & UART_RX0_BUFFER_MASK;
} /* uart0_available */

/* number of bytes available for writing to the transmit buffer without blocking */
uint16_t uart0_availableForWrite(void)
{
    return (UART_TX0_BUFFER_SIZE - ( (UART_TX0_BUFFER_SIZE + UART0_TxHead - UART0_TxTail) & UART_TX0_BUFFER_MASK ) );
} /* uart0_availableForWrite*/

/* Allow UART0 to be used as a stream for printf, scanf, etc... */
static int uartstream0_putchar(char c, FILE *stream);
static int uartstream0_getchar(FILE *stream);

// Stream declaration for stdio
static FILE uartstream0_f = FDEV_SETUP_STREAM(uartstream0_putchar, uartstream0_getchar, _FDEV_SETUP_RW);

/* Initialize the file handle, return the file handle  */
FILE *uartstream0_init(uint32_t baudrate)
{
	uart0_init(UART_BAUD_SELECT(baudrate, F_CPU));
	return &uartstream0_f;
}	

static int uartstream0_putchar(char c, FILE *stream)
{
	uart0_putc((uint8_t) c);
	return 0;
}

static int uartstream0_getchar(FILE *stream)
{
	uint16_t res;
	while( !(uart0_available()) );  // wait for input
    res = uart0_getc();
    if(res == '\r') res = '\n';
	return (int) (res & 0xFF);
}

#endif /* defined(AT90_UART) || defined(ATMEGA_USART) || defined(ATMEGA_USART0) */

#if defined( USART1_ENABLED )

/*
 * these functions are only for ATmegas with two USART
 */
#if defined( ATMEGA_USART1 )

ISR(UART1_RECEIVE_INTERRUPT)
{
    uint16_t tmphead;
    uint8_t data;
    uint8_t usr;
    uint8_t lastRxError;

    /* read UART status register and UART data register */
    usr  = UART1_STATUS;
    data = UART1_DATA;

    lastRxError = (usr & ((1<<FE1)|(1<<DOR1)) );

    /* calculate buffer index */
    tmphead = ( UART1_RxHead + 1) & UART_RX1_BUFFER_MASK;

    if ( tmphead == UART1_RxTail ) {
        /* error: receive buffer overflow */
        lastRxError = UART_BUFFER_OVERFLOW >> 8;
    } else {
        /* store new index */
        UART1_RxHead = tmphead;
        /* store received data in buffer */
        UART1_RxBuf[tmphead] = data;
    }
    UART1_LastRxError = lastRxError;
}

ISR(UART1_TRANSMIT_INTERRUPT)
{
    uint16_t tmptail;

    if ( UART1_TxHead != UART1_TxTail) {
        /* calculate and store new buffer index */
        tmptail = (UART1_TxTail + 1) & UART_TX1_BUFFER_MASK;
        UART1_TxTail = tmptail;
        /* get one byte from buffer and write it to UART */
        UART1_DATA = UART1_TxBuf[tmptail];  /* start transmission */
    } else {
        /* tx buffer empty, disable UDRE interrupt */
        UART1_CONTROL &= ~(1<<UART1_UDRIE);
    }
}

void uart1_init(uint16_t baudrate)
{
    UART1_TxHead = 0;
    UART1_TxTail = 0;
    UART1_RxHead = 0;
    UART1_RxTail = 0;

    /* Double speed mode if needed */
    if ( baudrate & 0x8000 ) {
        UART1_STATUS = (1<<U2X1);  //Enable 2x speed
        baudrate &= ~0x8000;
    }

    /* Enable USART receiver and transmitter and receive complete interrupt */
    UART1_CONTROL = (1<<RXCIE1)|(1<<RXEN1)|(1<<TXEN1);

    /* Set frame format: asynchronous, 8data, no parity, 1stop bit */
#ifdef URSEL1
    UCSR1C = (1<<URSEL1)|(3<<UCSZ10);
#else
    UCSR1C = (3<<UCSZ10);
#endif

    /* Set baud rate */
    UBRR1H = (uint8_t)(baudrate>>8);
    UBRR1L = (uint8_t) baudrate;
} /* uart_init */

/* byte from ring buffer */
uint16_t uart1_getc(void)
{
    uint16_t tmptail;
    uint8_t data;

    if ( UART1_RxHead == UART1_RxTail ) {
        return UART_NO_DATA;   /* no data available */
    }

    /* calculate /store buffer index */
    tmptail = (UART1_RxTail + 1) & UART_RX1_BUFFER_MASK;
    UART1_RxTail = tmptail;

    /* get data from receive buffer */
    data = UART1_RxBuf[tmptail];

    return (UART1_LastRxError << 8) + data;

} /* uart1_getc */

/* write byte to ringbuffer for transmitting via UART */
void uart1_putc(uint8_t data)
{
    uint16_t tmphead;

    tmphead  = (UART1_TxHead + 1) & UART_TX1_BUFFER_MASK;

    while ( tmphead == UART1_TxTail ) {
        ;/* wait for free space in buffer */
    }

    UART1_TxBuf[tmphead] = data;
    UART1_TxHead = tmphead;

    /* enable UDRE interrupt */
    UART1_CONTROL    |= (1<<UART1_UDRIE);

} /* uart1_putc */

/* Flush bytes from the transmit buffer  */
void uart1_flush(void)
{
    ATOMIC_BLOCK ( ATOMIC_RESTORESTATE ) // may not be needed
    {
        UART1_TxHead = UART1_TxTail;
    }
} /* uart1_flush */

/* number of bytes waiting in the receive buffer */
uint16_t uart1_available(void)
{
    return (UART_RX1_BUFFER_SIZE + UART1_RxHead - UART1_RxTail) & UART_RX1_BUFFER_MASK;
} /* uart1_available */

/* number of bytes available for writing to the transmit buffer without blocking */
uint16_t uart1_availableForWrite(void)
{
    return (UART_TX1_BUFFER_SIZE - ( (UART_TX1_BUFFER_SIZE + UART1_TxHead - UART1_TxTail) & UART_TX1_BUFFER_MASK ) );
} /* uart1_availableForWrite*/

/* Allow UART1 to be used as a stream for printf, scanf, etc... */
static int uartstream1_putchar(char c, FILE *stream);
static int uartstream1_getchar(FILE *stream);

// Stream declaration for stdio
static FILE uartstream1_f = FDEV_SETUP_STREAM(uartstream1_putchar, uartstream1_getchar, _FDEV_SETUP_RW);

/* Initialize the file handle, return the file handle  */
FILE *uartstream1_init(uint32_t baudrate)
{
	uart1_init(UART_BAUD_SELECT(baudrate, F_CPU));
	return &uartstream1_f;
}	

static int uartstream1_putchar(char c, FILE *stream)
{
	uart1_putc((uint8_t) c);
	return 0;
}

static int uartstream1_getchar(FILE *stream)
{
	uint16_t res;
	while( !(uart1_available()) );  // wait for input
    res = uart1_getc();
    if(res == '\r') res = '\n';
	return (int) (res & 0xFF);
}

#endif /* defined( ATMEGA_USART1 ) */

#endif /* defined( USART1_ENABLED ) */

#if defined( USART2_ENABLED )

/*
 * these functions are only for ATmegas with four USART
 */
#if defined( ATMEGA_USART2 )

ISR(UART2_RECEIVE_INTERRUPT)
{
    uint16_t tmphead;
    uint8_t data;
    uint8_t usr;
    uint8_t lastRxError;

    /* read UART status register and UART data register */
    usr  = UART2_STATUS;
    data = UART2_DATA;

    lastRxError = (usr & ((1<<FE2)|(1<<DOR2)) );

    /* calculate buffer index */
    tmphead = ( UART2_RxHead + 1) & UART_RX2_BUFFER_MASK;

    if ( tmphead == UART2_RxTail ) {
        /* error: receive buffer overflow */
        lastRxError = UART_BUFFER_OVERFLOW >> 8;
    } else {
        /* store new index */
        UART2_RxHead = tmphead;
        /* store received data in buffer */
        UART2_RxBuf[tmphead] = data;
    }
    UART2_LastRxError = lastRxError;
}


ISR(UART2_TRANSMIT_INTERRUPT)
{
    uint16_t tmptail;


    if ( UART2_TxHead != UART2_TxTail) {
        /* calculate and store new buffer index */
        tmptail = (UART2_TxTail + 1) & UART_TX2_BUFFER_MASK;
        UART2_TxTail = tmptail;
        /* get one byte from buffer and write it to UART */
        UART2_DATA = UART2_TxBuf[tmptail];  /* start transmission */
    } else {
        /* tx buffer empty, disable UDRE interrupt */
        UART2_CONTROL &= ~(1<<UART2_UDRIE);
    }
}

void uart2_init(uint16_t baudrate)
{
    UART2_TxHead = 0;
    UART2_TxTail = 0;
    UART2_RxHead = 0;
    UART2_RxTail = 0;

    /* Double speed mode if needed */
    if ( baudrate & 0x8000 ) {
        UART2_STATUS = (1<<U2X2);  //Enable 2x speed
        baudrate &= ~0x8000;
    }

    /* Enable USART receiver and transmitter and receive complete interrupt */
    UART2_CONTROL = (1<<RXCIE2)|(1<<RXEN2)|(1<<TXEN2);

    /* Set frame format: asynchronous, 8data, no parity, 1stop bit */
#ifdef URSEL2
    UCSR2C = (1<<URSEL2)|(3<<UCSZ20);
#else
    UCSR2C = (3<<UCSZ20);
#endif

    /* Set baud rate */
    UBRR2H = (uint8_t)(baudrate>>8);
    UBRR2L = (uint8_t) baudrate;
} /* uart_init */


/* return byte from ring buffer */
uint16_t uart2_getc(void)
{
    uint16_t tmptail;
    uint8_t data;

    if ( UART2_RxHead == UART2_RxTail ) {
        return UART_NO_DATA;   /* no data available */
    }

    /* calculate /store buffer index */
    tmptail = (UART2_RxTail + 1) & UART_RX2_BUFFER_MASK;
    UART2_RxTail = tmptail;

    /* get data from receive buffer */
    data = UART2_RxBuf[tmptail];

    return (UART2_LastRxError << 8) + data;

} /* uart2_getc */

/* write byte to ring buffer for transmitting via UART */
void uart2_putc(uint8_t data)
{
    uint16_t tmphead;

    tmphead  = (UART2_TxHead + 1) & UART_TX2_BUFFER_MASK;

    while ( tmphead == UART2_TxTail ) {
        ;/* wait for free space in buffer */
    }

    UART2_TxBuf[tmphead] = data;
    UART2_TxHead = tmphead;

    /* enable UDRE interrupt */
    UART2_CONTROL    |= (1<<UART2_UDRIE);
} /* uart2_putc */

/* Flush bytes from the transmit buffer  */
void uart2_flush(void)
{
    ATOMIC_BLOCK ( ATOMIC_RESTORESTATE ) // may not be needed
    {
        UART2_TxHead = UART2_TxTail;
    }
} /* uart2_flush */

/* number of bytes waiting in the receive buffer */
uint16_t uart2_available(void)
{
    return (UART_RX2_BUFFER_SIZE + UART2_RxHead - UART2_RxTail) & UART_RX2_BUFFER_MASK;
} /* uart2_available */

/* number of bytes available for writing to the transmit buffer without blocking */
uint16_t uart2_availableForWrite(void)
{
    return (UART_TX2_BUFFER_SIZE - ( (UART_TX2_BUFFER_SIZE + UART2_TxHead - UART2_TxTail) & UART_TX2_BUFFER_MASK ) );
} /* uart2_availableForWrite*/

/* Allow UART2 to be used as a stream for printf, scanf, etc... */
static int uartstream2_putchar(char c, FILE *stream);
static int uartstream2_getchar(FILE *stream);

// Stream declaration for stdio
static FILE uartstream2_f = FDEV_SETUP_STREAM(uartstream2_putchar, uartstream2_getchar, _FDEV_SETUP_RW);

/* Initialize the file handle, return the file handle  */
FILE *uartstream2_init(uint32_t baudrate)
{
	uart2_init(UART_BAUD_SELECT(baudrate, F_CPU));
	return &uartstream2_f;
}	

static int uartstream2_putchar(char c, FILE *stream)
{
	uart2_putc((uint8_t) c);
	return 0;
}

static int uartstream2_getchar(FILE *stream)
{
	uint16_t res;
	while( !(uart2_available()) );  // wait for input
    res = uart2_getc();
    if(res == '\r') res = '\n';
	return (int) (res & 0xFF);
}

#endif  /* defined( ATMEGA_USART2 ) */

#endif /* defined( USART2_ENABLED ) */

#if defined( USART3_ENABLED )

/*
 * these functions are only for ATmegas with four USART
 */
#if defined( ATMEGA_USART3 )

ISR(UART3_RECEIVE_INTERRUPT)
{
    uint16_t tmphead;
    uint8_t data;
    uint8_t usr;
    uint8_t lastRxError;


    /* read UART status register and UART data register */
    usr  = UART3_STATUS;
    data = UART3_DATA;

    lastRxError = (usr & ((1<<FE3)|(1<<DOR3)) );

    /* calculate buffer index */
    tmphead = ( UART3_RxHead + 1) & UART_RX3_BUFFER_MASK;

    if ( tmphead == UART3_RxTail ) {
        /* error: receive buffer overflow */
        lastRxError = UART_BUFFER_OVERFLOW >> 8;
    } else {
        /* store new index */
        UART3_RxHead = tmphead;
        /* store received data in buffer */
        UART3_RxBuf[tmphead] = data;
    }
    UART3_LastRxError = lastRxError;
}

ISR(UART3_TRANSMIT_INTERRUPT)
{
    uint16_t tmptail;

    if ( UART3_TxHead != UART3_TxTail) {
        /* calculate and store new buffer index */
        tmptail = (UART3_TxTail + 1) & UART_TX3_BUFFER_MASK;
        UART3_TxTail = tmptail;
        /* get one byte from buffer and write it to UART */
        UART3_DATA = UART3_TxBuf[tmptail];  /* start transmission */
    } else {
        /* tx buffer empty, disable UDRE interrupt */
        UART3_CONTROL &= ~(1<<UART3_UDRIE);
    }
}

void uart3_init(uint16_t baudrate)
{
    UART3_TxHead = 0;
    UART3_TxTail = 0;
    UART3_RxHead = 0;
    UART3_RxTail = 0;

    /* Double speed mode if needed */
    if ( baudrate & 0x8000 ) {
        UART3_STATUS = (1<<U2X3);  //Enable 2x speed
        baudrate &= ~0x8000;
    }

    /* Enable USART receiver and transmitter and receive complete interrupt */
    UART3_CONTROL = (1<<RXCIE3)|(1<<RXEN3)|(1<<TXEN3);

    /* Set frame format: asynchronous, 8data, no parity, 1stop bit */
#ifdef URSEL3
    UCSR3C = (1<<URSEL3)|(3<<UCSZ30);
#else
    UCSR3C = (3<<UCSZ30);
#endif

    /* Set baud rate */
    UBRR3H = (uint8_t)(baudrate>>8);
    UBRR3L = (uint8_t) baudrate;
} /* uart_init */


/* return byte from ring buffer */
uint16_t uart3_getc(void)
{
    uint16_t tmptail;
    uint8_t data;

    if ( UART3_RxHead == UART3_RxTail ) {
        return UART_NO_DATA;   /* no data available */
    }

    /* calculate /store buffer index */
    tmptail = (UART3_RxTail + 1) & UART_RX3_BUFFER_MASK;
    UART3_RxTail = tmptail;

    /* get data from receive buffer */
    data = UART3_RxBuf[tmptail];

    return (UART3_LastRxError << 8) + data;

} /* uart3_getc */

/* write byte to ringbuffer for transmitting */
void uart3_putc(uint8_t data)
{
    uint16_t tmphead;
    tmphead  = (UART3_TxHead + 1) & UART_TX3_BUFFER_MASK;

    while ( tmphead == UART3_TxTail ) 
    {
        ;/* wait for free space in buffer */
    }

    UART3_TxBuf[tmphead] = data;
    UART3_TxHead = tmphead;

    /* enable UDRE interrupt */
    UART3_CONTROL    |= (1<<UART3_UDRIE);
} /* uart3_putc */

/* Flush bytes from the transmit buffer  */
void uart3_flush(void)
{
    ATOMIC_BLOCK ( ATOMIC_RESTORESTATE ) // may not be needed
    {
        UART3_TxHead = UART3_TxTail;
    }
} /* uart3_flush */

/* number of bytes waiting in the receive buffer */
uint16_t uart3_available(void)
{
    return (UART_RX3_BUFFER_SIZE + UART3_RxHead - UART3_RxTail) & UART_RX3_BUFFER_MASK;
} /* uart3_available */

/* number of bytes available for writing to the transmit buffer without blocking */
uint16_t uart3_availableForWrite(void)
{
    return (UART_TX3_BUFFER_SIZE - ( (UART_TX3_BUFFER_SIZE + UART3_TxHead - UART3_TxTail) & UART_TX3_BUFFER_MASK ) );
} /* uart3_availableForWrite*/

/* Allow UART3 to be used as a stream for printf, scanf, etc... */
static int uartstream3_putchar(char c, FILE *stream);
static int uartstream3_getchar(FILE *stream);

// Stream declaration
static FILE uartstream3_f = FDEV_SETUP_STREAM(uartstream3_putchar, uartstream3_getchar, _FDEV_SETUP_RW);

/* Initialize the file handle, return the file handle  */
FILE *uartstream3_init(uint32_t baudrate)
{
	uart3_init(UART_BAUD_SELECT(baudrate, F_CPU));
	return &uartstream3_f;
}	

static int uartstream3_putchar(char c, FILE *stream)
{
	uart3_putc((uint8_t) c);
	return 0;
}

static int uartstream3_getchar(FILE *stream)
{
	uint16_t res;
	while( !(uart3_available()) );  // wait for input
    res = uart3_getc();
    if(res == '\r') res = '\n';
	return (int) (res & 0xFF);
}

#endif /* defined( ATMEGA_USART3 ) */

#endif /* defined( USART3_ENABLED ) */
