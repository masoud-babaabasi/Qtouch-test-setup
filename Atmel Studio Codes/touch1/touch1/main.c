#include <atmel_start.h>
#include <touch_example.h>
#include <util/delay.h>
int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	touch_example();
	PORTF.DIR = ( 1 << 2);
	/* Replace with your application code */
	while (1) {
	PORTF.OUTTGL = ( 1 << 2 );
	_delay_ms(500);
	}
}
