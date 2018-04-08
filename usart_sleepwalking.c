

#include <asf.h>
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


int main(void)
{
	/* Initialize the SAM system */
	sysclk_init();
	board_init();

	/* Initialize the UART console */
	configure_console();


	delay_s(1);


		  
	while (1) {
		usart_putchar(USART0, 0x7F);
		//ioport_toggle_pin_level(LED0_GPIO);
		//delay_ms(1000);
	}
}
