#include <atmel_start.h>
#include <util/delay.h>
#include "touch_example.h"
//#include <usart_basic.h>
#include <touch_example.h>
#include <stdio.h>

void shiftOut(uint32_t data);
int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	char str[50];
	uint8_t uart_size;
	PORTE.DIR = ( 1 << 2 ) | ( 1 << 3 );
	PORTF.DIR = ( 1 << 0 ) ;

	//uart_size = sprintf(str,"Start testing\n");
	//for( uint8_t i=0 ; i < uart_size ; i++ ) USART_0_write((uint8_t)str[i]);

	for(uint8_t i = 0 ; i < 24 ; i++){
		shiftOut(1LU << i);
		_delay_ms(50);
	}
	/* Replace with your application code */
	while (1) {
	touch_example();
	/*if( get_surface_status() )	{
		uint8_t r = get_surface_position(0);
		uint8_t t = get_surface_position(1);
		uart_size = sprintf(str,"r:%u,t:%u\n", r , t);
		for( uint8_t i=0 ; i < uart_size ; i++ ) USART_0_write((uint8_t)str[i]);
	}*/
	}
}

void shiftOut(uint32_t data){
	PORTE.OUTCLR = ( 1 << 2 ) | ( 1 << 3 );
	PORTF.OUTCLR = ( 1 << 0 ) ;
	data &= (0x00ffffff);
	for(int8_t i=23 ; i >= 0 ; i--){
		if( data & ( (uint32_t)1 << i) ) PORTE.OUTSET = ( 1 << 2);
		else PORTE.OUTCLR = ( 1 << 2);
		PORTF.OUTSET = (1 << 0 );
		_delay_us(50);
		PORTF.OUTCLR = (1 << 0 );
		_delay_us(50);
	}
	PORTE.OUTSET = (1 << 3 );
	_delay_us(50);
	PORTE.OUTCLR = (1 << 3 );
}
