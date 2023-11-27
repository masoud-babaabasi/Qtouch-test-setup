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

	for(uint8_t i = 0 ; i < 24 ; i++){
		shiftOut(1LU << i);
		_delay_ms(50);
	}

	/* Replace with your application code */
	while (1) {
	touch_example();
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