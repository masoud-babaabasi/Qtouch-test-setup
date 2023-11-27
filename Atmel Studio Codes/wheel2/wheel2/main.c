#include <atmel_start.h>
#include <util/delay.h>
#include "touch_example.h"
#include <usart_basic.h>
#include <touch_example.h>
#include <stdio.h>

void shiftOut(uint32_t data);

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	PORTF.DIR = ( 1 << 2 );
	PORTC.DIR |= ( 1 << 5 ) | ( 1 << 6 ) | ( 1 << 7 );
	char str[50];
	uint8_t uart_size;

	/* Replace with your application code */
	while (1) {
	touch_example();
	if( get_scroller_state(0) ){
		uint8_t pos = get_scroller_position(0);
		uart_size = sprintf(str,"pos:%u\n", pos);
		for( uint8_t i=0 ; i < uart_size ; i++ ) USART_0_write((uint8_t)str[i]);
		}
	}
}

void shiftOut(uint32_t data){
	PORTC.OUTCLR = ( 1 << 5 ) | ( 1 << 6 ) | ( 1 << 7 );
	data &= (0x00ffffff);
	for(int8_t i=23 ; i >= 0 ; i--){
		if( data & ( (uint32_t)1 << i) ) PORTC.OUTSET = ( 1 << 5);
		else PORTC.OUTCLR = ( 1 << 5);
		PORTC.OUTSET = (1 << 7 );
		_delay_us(50);
		PORTC.OUTCLR = (1 << 7 );
		_delay_us(50);
	}
	PORTC.OUTSET = (1 << 6 );
	_delay_us(50);
	PORTC.OUTCLR = (1 << 6 );
}
