#include <atmel_start.h>
#include <touch_example.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
extern volatile uint8_t measurement_done_touch;
int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	
	char str_tx[50];
	uint8_t uart_size_tx;
	uart_size_tx = sprintf(str_tx , "Hello masoud test touch2!!\n");
	for(uint8_t i= 0 ; i < uart_size_tx ; i++) USART_write((uint8_t)str_tx[i]);
	PORTF.DIR = ( 1 << 2);
	/* Replace with your application code */
	calibrate_node(0);
	while (1) {
		//PORTF.OUTTGL = ( 1 << 2 );
		//_delay_ms(500);
		//touch_example();
		touch_process();
		if (measurement_done_touch == 1) {
			measurement_done_touch = 0;
			uint8_t scroller_status   = get_scroller_state(0);
			uint8_t pos= get_scroller_position(0);
			memset(str_tx,0,50);
			uart_size_tx = sprintf(str_tx , "scroller=%d,sattuse=%d\n", pos , scroller_status);
			for(uint8_t i= 0 ; i < uart_size_tx ; i++){
				while (!USART_is_tx_ready());
				USART_write((uint8_t)str_tx[i]);
				while (USART_is_tx_busy());
			 }
		}
	}
}
