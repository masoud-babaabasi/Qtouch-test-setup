#include <atmel_start.h>
#include <util/delay.h>
#include <touch_example.h>
#include <usart_basic_example.h>
extern volatile uint8_t measurement_done_touch;
extern uint8_t key_status;
uint8_t pre_key_statuse = 0;


void shiftOut(uint32_t data);


int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	PORTF.DIR = ( 1 << 2 );
	PORTC.DIR |= ( 1 << 5 ) | ( 1 << 6 ) | ( 1 << 7 );
	for(uint32_t i=0 ; i <= 24 ; i++ ){
		shiftOut( (uint32_t)1 << i );
		_delay_ms(100);
	}
	/* Replace with your application code */
	while (1) {
		touch_example();
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
