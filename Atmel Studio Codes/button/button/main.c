#include <atmel_start.h>
#include <util/delay.h>
#include <usart_basic_example.h>
extern volatile uint8_t measurement_done_touch;
extern uint8_t key_status;
uint8_t pre_key_statuse = 0;


void shiftOut(uint32_t data);

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	//USART_test_usart_basic();
	PORTF.DIR = ( 1 << 2 );
	PORTC.DIR |= ( 1 << 5 ) | ( 1 << 6 ) | ( 1 << 7 );
	/* Replace with your application code */
	for(uint32_t i=0 ; i < 24 ; i++ ){
		shiftOut( (uint32_t)1 << i );
		_delay_ms(100);
	}
	while (1) {
	touch_process();
	if (measurement_done_touch == 1) {
		measurement_done_touch = 0;
		key_status = get_sensor_state(0) & KEY_TOUCHED_MASK;
		if( pre_key_statuse != key_status ){
			
		}
		if (0u != key_status) {
			PORTF.OUTSET = ( 1 << 2 );
			} else {
			// LED_OFF
			PORTF.OUTCLR = ( 1 << 2 );
		}
		pre_key_statuse = key_status;
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
