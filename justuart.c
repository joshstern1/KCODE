/**************************************************************************/
/**************************************************************************/

/*Software License Agreement (BSD License)

Copyright (c) 2012, Adafruit Industries
All rights reserved.*/


/**************************************************************************/

#include <asf.h>
#include <string.h>
#include <board.h>
#include <delay.h>
#include "conf_example.h"

static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		#ifdef CONF_UART_CHAR_LENGTH
		.charlength = CONF_UART_CHAR_LENGTH,
		#endif
		.paritytype = CONF_UART_PARITY,
		#ifdef CONF_UART_STOP_BITS
		.stopbits = CONF_UART_STOP_BITS,
		#endif
	};
	
	/* Configure console UART. */
	//sysclk_enable_peripheral_clock(USART0);
	stdio_serial_init(USART0, &uart_serial_options);
}


int main (void) {

	sysclk_init();
	board_init();
	configure_console();

	while(1){

				 uint8_t message[10] = {0xAA, 0x32, 0x33, 0x31, 0x32, 0x33, 0x41, 0x42, 0x43, 0x6B};

				 usart_putchar(USART0, message[0]);
				 usart_putchar(USART0, message[1]);
				 usart_putchar(USART0, message[2]);
				 usart_putchar(USART0, message[3]);
				 usart_putchar(USART0, message[4]);
				 usart_putchar(USART0, message[5]);
				 usart_putchar(USART0, message[6]);
				 usart_putchar(USART0, message[7]);
				 usart_putchar(USART0, message[8]);
				 usart_putchar(USART0, message[9]);
				 //}
				 delay_s(10);

			
		}
}


